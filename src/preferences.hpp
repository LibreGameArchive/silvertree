#ifndef PREFERENCES_HPP_INCLUDED
#define PREFERENCES_HPP_INCLUDED

#include <GL/glew.h>

#include <string>

bool parse_args(int argc, char** argv);

bool preference_nocombat();
bool preference_maxfps();
bool preference_mipmapping();
bool preference_sliders();

GLenum preference_mipmap_min();
GLenum preference_mipmap_max();
bool preference_anisotropy();

int preference_screen_width();
int preference_screen_height();
unsigned int preference_fullscreen();

const std::string preference_save_file();
const std::string preference_scenario_file();

#endif
