
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <algorithm>
#include <cmath>

#include "tile_logic.hpp"
#include "util.hpp"

namespace hex
{

using namespace util;

namespace {

const std::string abbrev[] = {"n","ne","se","s","sw","nw",""};
		
}

variant location::get_value(const std::string& key) const
{
	if(key == "x") {
		return variant(x_);
	} else if(key == "y") {
		return variant(y_);
	} else if(key == "valid") {
		return variant(valid());
	} else {
		return variant();
	}
}

const std::string& direction_abbreviation(DIRECTION dir)
{
	return abbrev[dir];
}

unsigned int distance_between(const location& a, const location& b)
{
	const unsigned int hdistance = std::abs(a.x() - b.x());

	const unsigned int vpenalty = ( (is_even(a.x()) && is_odd(b.x()) && (a.y() < b.y()))
		|| (is_even(b.x()) && is_odd(a.x()) && (b.y() < a.y())) ) ? 1 : 0;

	// for any non-negative integer i, i - i/2 - i%2 == i/2
	// previously returned (hdistance + vdistance - vsavings)
	// = hdistance + vdistance - minimum(vdistance,hdistance/2+hdistance%2)
	// = maximum(hdistance, vdistance+hdistance-hdistance/2-hdistance%2)
	// = maximum(hdistance,abs(a.y-b.y)+vpenalty+hdistance/2)

	return std::max<int>(hdistance, std::abs(a.y() - b.y()) + vpenalty + hdistance/2);
}

void get_adjacent_tiles(const location& a, location* res)
{
	*res = tile_in_direction(a,NORTH);
	++res;
	*res = tile_in_direction(a,NORTH_EAST);
	++res;
	*res = tile_in_direction(a,SOUTH_EAST);
	++res;
	*res = tile_in_direction(a,SOUTH);
	++res;
	*res = tile_in_direction(a,SOUTH_WEST);
	++res;
	*res = tile_in_direction(a,NORTH_WEST);
}

location tile_in_direction(const location& a, DIRECTION dir)
{
	switch(dir) {
		case NORTH:
			return location(a.x(),a.y()-1);
		case NORTH_EAST:
			return location(a.x()+1,a.y() - (is_even(a.x()) ? 1:0));
		case SOUTH_EAST:
			return location(a.x()+1,a.y() + (is_odd(a.x()) ? 1:0));
		case SOUTH:
			return location(a.x(),a.y()+1);
		case SOUTH_WEST:
			return location(a.x()-1,a.y() + (is_odd(a.x()) ? 1:0));
		case NORTH_WEST:
			return location(a.x()-1,a.y() - (is_even(a.x()) ? 1:0));
		default:
			return location();
	}
}

void get_tile_ring(const location& a, int radius,
                   std::vector<location>& res)
{
	if(radius == 0) {
		res.push_back(a);
		return;
	}
	
	location loc(a);
	for(int i = 0; i != radius; ++i) {
		loc = tile_in_direction(loc,SOUTH_WEST);
	}

	for(int n = 0; n != 6; ++n) {
		const DIRECTION dir = static_cast<DIRECTION>(n);
		for(int i = 0; i != radius; ++i) {
			res.push_back(loc);
			loc = tile_in_direction(loc,dir);
		}
	}
}

DIRECTION get_adjacent_direction(const location& from, const location& to)
{
	location adj[6];
	get_adjacent_tiles(from,adj);
	for(unsigned int n = 0; n != 6; ++n) {
		if(adj[n] == to) {
			return static_cast<DIRECTION>(n);
		}
	}

	return NULL_DIRECTION;
}

bool tiles_adjacent(const location& a, const location& b)
{
	//two tiles are adjacent if y is different by 1, and x by 0, or if
	//x is different by 1 and y by 0, or if x and y are each different by 1,
	//and the x value of the hex with the greater y value is even

	const int xdiff = std::abs(a.x() - b.x());
	const int ydiff = std::abs(a.y() - b.y());
	return ydiff == 1 && a.x() == b.x() || xdiff == 1 && a.y() == b.y() ||
	       xdiff == 1 && ydiff == 1 && (a.y() > b.y() ? is_even(a.x()) : is_even(b.x()));
}
	
}