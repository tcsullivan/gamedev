#include <gametime.hpp>

#include <common.hpp>

static unsigned int tickCount = 0;
static unsigned int deltaTime = 1;

// millisecond timers
static unsigned int currentTime = 0;
static unsigned int prevTime, prevPrevTime;

namespace gtime {
    void setTickCount(unsigned int t) {
        tickCount = t;
    }

    unsigned int getTickCount(void) {
        return tickCount;
    }

    unsigned int getDeltaTime(void) {
        return deltaTime;
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
        if (prevPrevTime + MSEC_PER_TICK <= currentTime) {
    		prevPrevTime = currentTime;
            return true;
    	}

        return false;
    }
}
