#ifndef GAMETIME_H_
#define GAMETIME_H_

namespace gtime {
    void setTickCount(unsigned int t);
    unsigned int getTickCount(void);
    unsigned int getDeltaTime(void);

    void tick(void);
    void tick(unsigned int ticks);
    bool tickHasPassed(void);

    void mainLoopHandler(void);
}

#endif // GAMETIME_H_
