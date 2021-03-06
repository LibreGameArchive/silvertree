
/*
   Copyright (C) 2007 by David White <dave.net>
   Part of the Silver Tree Project

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2 or later.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/
#include <iostream>

#include <boost/regex.hpp>

#include "foreach.hpp"
#include "formula_tokenizer.hpp"

namespace formula_tokenizer
{

namespace {
struct token_type {
	boost::regex re;
	TOKEN_TYPE type;
};
}

token get_token(iterator& i1, iterator i2) {
	using boost::regex;

	token_type types[] = { { regex("^(not\\b|and\\b|or\\b|where\\b|d(?=[^a-zA-Z])|\\*|\\+|\\-|\\^|%|/|<=|>=|<|>|!=|=|\\.)"), TOKEN_OPERATOR },
	                       { regex("^'[^']*'"),    TOKEN_STRING_LITERAL },
	                       { regex("^[a-zA-Z_]+"), TOKEN_IDENTIFIER },
	                       { regex("^\\d+"),       TOKEN_INTEGER },
	                       { regex("^\\("),        TOKEN_LPARENS },
	                       { regex("^\\)"),        TOKEN_RPARENS },
						   { regex("^\\["),        TOKEN_LSQUARE },
						   { regex("^\\]"),        TOKEN_RSQUARE },
	                       { regex("^,"),          TOKEN_COMMA },
	                       { regex("^\\s+"),       TOKEN_WHITESPACE } };

	foreach(const token_type& t, types) {
		boost::smatch match;
		if(regex_search(i1, i2, match, t.re)) {
			token res;
			res.type = t.type;
			res.begin = i1;
			i1 = res.end = i1 + match.length();

			return res;
		}
	}

	std::cerr << "Unrecognized token: '" << std::string(i1,i2) << "'\n";
	throw token_error();
}

}

#ifdef UNIT_TEST_TOKENIZER

int main()
{
	using namespace formula_tokenizer;
	std::string test = "(abc + 4 * (5+3))^2";
	std::string::const_iterator i1 = test.begin();
	std::string::const_iterator i2 = test.end();
	TOKEN_TYPE types[] = {TOKEN_LPARENS, TOKEN_IDENTIFIER,
	                      TOKEN_WHITESPACE, TOKEN_OPERATOR,
						  TOKEN_WHITESPACE, TOKEN_INTEGER,
						  TOKEN_WHITESPACE, TOKEN_OPERATOR,
						  TOKEN_WHITESPACE, TOKEN_LPARENS,
						  TOKEN_INTEGER, TOKEN_OPERATOR,
						  TOKEN_INTEGER, TOKEN_RPARENS,
						  TOKEN_RPARENS,
	                      TOKEN_OPERATOR, TOKEN_INTEGER};
	std::string tokens[] = {"(", "abc", " ", "+", " ", "4", " ",
	                        "*", " ", "(", "5", "+", "3", ")", ")"};
	for(int n = 0; n != sizeof(types)/sizeof(*types); ++n) {
		token t = get_token(i1,i2);
		assert(std::string(t.begin,t.end) == tokens[n]);
		assert(t.type == types[n]);

	}
}

#endif
