#include <gametime.hpp>

#include <common.hpp>

static unsigned int tickCount = 0;
static float deltaTime = 1;

// millisecond timers
static unsigned int currentTime = 0;
static unsigned int prevTime;

static float accum = 0.0f;

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
        	if (!currentTime)
        		currentTime = prevTime = millis();

        	currentTime = millis();
        	deltaTime	= currentTime - prevTime;
        	prevTime	= currentTime;
        }

        bool tickHasPassed(void) {
            accum += deltaTime;
            if (accum > MSEC_PER_TICK) {
        		accum = 0.0f;
                return true;
        	}

            return false;
        }
    }
}
