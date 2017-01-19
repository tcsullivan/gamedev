#include <gametime.hpp>

#include <common.hpp> // millis
#include <config.hpp>

static unsigned int tickCount = 0;
static unsigned int deltaTime = 1;

namespace game {
    namespace time {
        void setTickCount(unsigned int t) {
            tickCount = t;
        }

        unsigned int getTickCount(void) {
            return tickCount;
        }

        unsigned int getDeltaTime(void) {
            return (deltaTime > 0) ? deltaTime : 1;
        }

        void tick(void) {
            tickCount++;
        }

        void tick(unsigned int ticks) {
            tickCount += ticks;
        }

        void mainLoopHandler(void) {
			static unsigned int cur = 0, prev;

			if (cur == 0)
				cur = prev = millis();

			cur = millis();
			deltaTime = cur - prev;
			prev = cur;
        }

        bool tickHasPassed(void) {
			static unsigned int accum = 0;

            accum += deltaTime;
            if (accum > MSEC_PER_TICK) {
        		accum = 0.0f;
                return true;
        	}

            return false;
        }
    }
}
