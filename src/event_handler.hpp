#ifndef EVENT_HANDLER_HPP_INCLUDED
#define EVENT_HANDLER_HPP_INCLUDED

#include <vector>

#include "formula.hpp"
#include "wml_command_fwd.hpp"
#include "wml_node_fwd.hpp"

namespace game_logic
{

class world;

class event_handler
{
public:
	explicit event_handler(wml::const_node_ptr node);
	void handle(const formula_callable& info, world& world);

	void add_filter(formula_ptr f);
	wml::const_node_ptr write() const;

private:
	std::vector<formula_ptr> filters_;
	std::vector<const_wml_command_ptr> commands_;
	wml::const_node_ptr node_;
	bool first_time_only_;
	bool already_run_;
};

}

#endif
