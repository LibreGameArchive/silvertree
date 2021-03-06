#include "battle_character.hpp"
#include "battle_move.hpp"
#include "character.hpp"
#include "formula.hpp"
#include "particle_emitter.hpp"
#include "wml_node.hpp"
#include "wml_utils.hpp"

namespace game_logic
{

const_battle_move_ptr battle_move::standard_move()
{
	battle_move_ptr res(new battle_move());
	res->name_ = "move";
	res->category_ = "move";
	res->moves_ = 2;
	res->min_moves_ = 1;
	res->can_attack_ = true;
	res->must_attack_ = false;
	return res;
}

const_battle_move_ptr battle_move::standard_attack()
{
	battle_move_ptr res(new battle_move());
	res->name_ = "attack";
	res->category_ = "attack";
	res->moves_ = 0;
	res->min_moves_ = 0;
	res->can_attack_ = true;
	res->must_attack_ = true;
	return res;
}

const_battle_move_ptr battle_move::standard_pass()
{
	battle_move_ptr res(new battle_move());
	res->name_ = "pass";
	res->category_ = "defend";
	res->moves_ = 0;
	res->min_moves_ = 0;
	res->can_attack_ = false;
	res->must_attack_ = false;
	res->stats_["initiative"].reset(new formula("initiative/2"));
	return res;
}

battle_move::battle_move(wml::const_node_ptr node)
  : name_(wml::get_str(node,"name")),
    category_(wml::get_str(node,"category")),
    moves_(wml::get_int(node,"moves")),
    min_moves_(wml::get_int(node,"must_move")),
	can_attack_(wml::get_bool(node,"attack") || wml::get_bool(node,"must_attack")),
	must_attack_(wml::get_bool(node,"must_attack")),
	energy_required_(wml::get_int(node,"energy"))
{
	missile_emitter_ = node->get_child("missile_particles");
	wml::const_node_ptr stats = node->get_child("stats");
	if(stats) {
		for(wml::node::const_attr_iterator i = stats->begin_attr();
		    i != stats->end_attr(); ++i) {
			stats_[i->first] = const_formula_ptr(new formula(i->second));
		}
	}

	wml::const_node_ptr mod = node->get_child("modification");
	if(mod) {
		mod_.reset(new battle_modification(mod));
	}
}

int battle_move::get_stat(const std::string& stat,
                          const battle_character& c) const
{
	std::map<std::string,const_formula_ptr>::const_iterator i =
	         stats_.find(stat);
	if(i != stats_.end()) {
		const character::final_stat_callable callable(c.get_character());
		return i->second->execute(callable).as_int() + c.mod_stat(stat);
	} else {
		return c.stat(stat);
	}
}

graphics::particle_emitter_ptr battle_move::create_missile_emitter() const
{
	if(!missile_emitter_) {
		return graphics::particle_emitter_ptr();
	}

	const GLfloat null_pos[] = {0.0,0.0,0.0};
	return graphics::particle_emitter_ptr(new graphics::particle_emitter(missile_emitter_, null_pos, null_pos, null_pos));
}

}
