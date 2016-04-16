#ifndef ACTION_H_
#define ACTION_H_

#include <common.hpp>
#include <ui.hpp>

namespace ui {
    namespace action {
        extern bool make;

        // enables the action ui
        void enable(void);
        // disables the action ui
        void disable(void);

        // draws the action ui
        void draw(vec2 loc);
    }
}

#endif // ACTION_H_
