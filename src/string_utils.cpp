
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include "string_utils.hpp"

#include <cctype>
#include <algorithm>

namespace util
{

bool isnewline(char c)
{
	return c == '\r' || c == '\n';
}

bool portable_isspace(char c)
{
	return isnewline(c) || isspace(c);
}

bool notspace(char c)
{
	return !portable_isspace(c);
}

std::string &strip(std::string &str)
{
	//if all the string contains is whitespace, then the whitespace may
	//have meaning, so don't strip it
	std::string::iterator it = std::find_if(str.begin(), str.end(), notspace);
	if (it == str.end())
		return str;

	str.erase(str.begin(), it);
	str.erase(std::find_if(str.rbegin(), str.rend(), notspace).base(), str.end());

	return str;
}

std::vector< std::string > split(std::string const &val, const std::string& delim)
{
	/* this might be slow but its very convenient so long as you
	   aren't calling it too often */

	std::vector< std::string > res;
	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while (i2 != val.end()) {
		if(delim.find(*i2) != std::string::npos) {
			std::string new_val(i1, i2);
			res.push_back(new_val);
			while(delim.find(*(++i2)) != std::string::npos) {}
			i1 = i2;
		}
		++i2;
	}
	std::string new_val(i1,i2);
	if(!new_val.empty()) {
		res.push_back(new_val);
	}
	return res;
}

std::vector< std::string > split(std::string const &val, char c, int flags)
{
	std::vector< std::string > res;

	std::string::const_iterator i1 = val.begin();
	std::string::const_iterator i2 = val.begin();

	while (i2 != val.end()) {
		if (*i2 == c) {
			std::string new_val(i1, i2);
			if (flags & STRIP_SPACES)
				strip(new_val);
			if (!(flags & REMOVE_EMPTY) || !new_val.empty())
				res.push_back(new_val);
			++i2;
			if (flags & STRIP_SPACES) {
				while (i2 != val.end() && *i2 == ' ')
					++i2;
			}

			i1 = i2;
		} else {
			++i2;
		}
	}

	std::string new_val(i1, i2);
	if (flags & STRIP_SPACES)
		strip(new_val);
	if (!(flags & REMOVE_EMPTY) || !new_val.empty())
		res.push_back(new_val);

	return res;
}

bool string_starts_with(const std::string& target, const std::string& prefix) {
	if(target.length() < prefix.length()) {
		return false;
	}
	std::string target_pfx =  target.substr(0,prefix.length());
	return target_pfx == prefix;
}

std::string strip_string_prefix(const std::string& target, const std::string& prefix) {
	if(target.length() < prefix.length()) {
		return "";
	}
	return target.substr(prefix.length());
}

}
