
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "battle.hpp"
#include "battle_character.hpp"
#include "battle_map_generator.hpp"
#include "message_dialog.hpp"
#include "foreach.hpp"
#include "gamemap.hpp"
#include "npc_party.hpp"
#include "shop_dialog.hpp"
#include "tile_logic.hpp"
#include "wml_node.hpp"
#include "wml_utils.hpp"
#include "world.hpp"

#include <cstdlib>
#include <iostream>

namespace game_logic
{

npc_party::npc_party(wml::const_node_ptr node, world& game_world)
    : party(node,game_world), aggressive_(wml::get_bool(node,"aggressive"))
{
	dialog_ = node->get_child("dialog");
	if(wml::const_node_ptr dst = node->get_child("destination")) {
		current_destination_ = hex::location(
		             wml::get_attr<int>(dst,"x",-1),
		             wml::get_attr<int>(dst,"y",-1));
	}

	wml::node::const_child_range range = node->get_child_range("wander");
	while(range.first != range.second) {
		const wml::const_node_ptr& n = range.first->second;
		wander_between_.push_back(hex::location(
		     wml::get_attr<int>(n,"x"), wml::get_attr<int>(n,"y")));
		std::cerr << "wander (" << wander_between_.back().x() << "," << wander_between_.back().y() << ")\n";
		++range.first;
	}
}

namespace {

void dialog_sequence(const wml::const_node_ptr& node, party& npc, party& pc, bool nested = false)
{
	if(!node) {
		return;
	}

	wml::node::const_child_range range = node->get_child_range("talk");

	while(range.first != range.second) {
		const wml::const_node_ptr& talk = range.first->second;
		std::vector<std::string> options;
		wml::node::const_child_range op = talk->get_child_range("option");
		while(op.first != op.second) {
			options.push_back((*op.first->second)["text"]);
			++op.first;
		}

		const std::vector<std::string>* options_ptr = options.empty() ? NULL : &options;

		gui::message_dialog dialog(pc, npc, (*talk)["message"],options_ptr, !nested);
		dialog.show_modal();
		int select = dialog.selected();
		
		if(options_ptr) {
			op = talk->get_child_range("option");
			while(select) {
				++op.first;
				--select;
			}

			dialog_sequence(op.first->second, npc, pc, true);
		}
		++range.first;
	}

	if(node->get_child("full_heal").get()) {
		pc.full_heal();
	}

	if(node->get_child("merge_party").get()) {
		std::cerr << "merge party...\n";
		pc.merge_party(npc);
	}

	wml::const_node_ptr shop = node->get_child("shop");
	if(shop) {
		game_dialogs::shop_dialog(pc, wml::get_int(shop,"cost",100),
		                          (*shop)["items"]).show_modal();
	}

	wml::const_node_ptr spar = node->get_child("spar");
	if(spar) {
		boost::shared_ptr<hex::gamemap> battle_map =
		     generate_battle_map(npc.game_world().map(), npc.loc());
		std::vector<battle_character_ptr> chars;

		for(std::vector<character_ptr>::const_iterator i =
		    pc.members().begin(); i != pc.members().end(); ++i) {
			hex::location loc(2 + i - pc.members().begin(), 2);
			chars.push_back(battle_character::make_battle_character(
			      *i,pc,loc,hex::NORTH,*battle_map,
			      npc.game_world().current_time()));
		}

		for(std::vector<character_ptr>::const_iterator i =
		    npc.members().begin(); i != npc.members().end(); ++i) {
			hex::location loc(8 + i - npc.members().begin(), 8);
			chars.push_back(battle_character::make_battle_character(
			      *i,npc,loc,hex::NORTH,*battle_map,
			      npc.game_world().current_time()));
		}

		battle b(chars, *battle_map);
		b.play();

		const bool victory = npc.is_destroyed();
		wml::const_node_ptr action = spar->get_child(victory ? "onvictory" : "ondefeat");

		pc.full_heal();
		npc.full_heal();

		dialog_sequence(action, npc, pc);
	}
}
		
}

void npc_party::friendly_encounter(party& p)
{
	if(!dialog_) {
		return;
	}

	dialog_sequence(dialog_, *this, p);
}

party::TURN_RESULT npc_party::do_turn()
{
	const int start_ticks = SDL_GetTicks();
	std::vector<const_party_ptr> parties;
	if(aggressive_) {
		get_visible_parties(parties);
	}

	std::cerr << "see " << parties.size() << " other parties\n";

	hex::location target;
	int closest = -1;
	foreach(const const_party_ptr& party, parties) {
		if(is_enemy(*party) && (closest == -1 || hex::distance_between(loc(),party->loc()) < closest)) {
			closest = hex::distance_between(loc(),party->loc());
			target = party->loc();
			break;
		}
	}

	if(current_destination_ == loc()) {
		current_destination_ = hex::location();
	}

	if(!wander_between_.empty() &&
	   !map().is_loc_on_map(current_destination_)) {
		current_destination_ = wander_between_[rand()%wander_between_.size()];
		std::cerr << "set dest to " << current_destination_.x() << "," << current_destination_.y() << "\n";
	}

	if(!map().is_loc_on_map(target) &&
	   map().is_loc_on_map(current_destination_)) {
		target = current_destination_;
	}

	if(map().is_loc_on_map(target)) {
		const int current_distance = hex::distance_between(loc(),target);
		int best = -1;
		hex::DIRECTION dir = hex::NULL_DIRECTION;
		hex::location adj[6];
		hex::get_adjacent_tiles(loc(),adj);
		for(int n = 0; n != 6; ++n) {
			if(map().is_loc_on_map(adj[n]) == false) {
				continue;
			}

			if(hex::distance_between(adj[n],target) >= current_distance) {
				continue;
			}

			const int cost = movement_cost(loc(),adj[n]);
			if(cost != -1 && (best == -1 || cost < best)) {
				best = movement_cost(loc(),adj[n]);
				dir = hex::DIRECTION(n);
			}
		}

		if(dir != hex::NULL_DIRECTION) {
			move(dir);
		} else {
			if(!wander_between_.empty()) {
				//we seem to be stuck, so choose a new destination
				current_destination_ = wander_between_[rand()%wander_between_.size()];
			}
			pass();
		}

		const int time_taken = SDL_GetTicks() - start_ticks;
		std::cerr << "AI took " << time_taken << "\n";
		return TURN_COMPLETE;
	}

	pass();
	return TURN_COMPLETE;
}
		
}
