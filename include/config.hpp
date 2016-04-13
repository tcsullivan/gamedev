#ifndef CONFIG_H
#define CONFIG_H

#include <iostream>

#include <SDL2/SDL_mixer.h>

#include <tinyxml2.h>
#include <ui.hpp>


namespace config {
    void read(void);
    void update(void);
    void save(void);
}

#endif //CONFIG_H
