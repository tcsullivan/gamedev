#ifndef MOB_H_
#define MOB_H_

#include <common.hpp>
#include <entities.hpp>
#include <gametime.hpp>
#include <ui.hpp>

// local library headers
#include <tinyxml2.h>
using namespace tinyxml2;

extern Player *player;
extern std::string currentXML;

class Mob : public Entity {
protected:
    unsigned int actCounter;
    unsigned int actCounterInitial;
    bool ridable;
public:
    Entity *rider;
	bool aggressive;
	std::string heyid;

    Mob(void);
    ~Mob(void);

	void wander(void);
    void ride(Entity *e);
    virtual void act(void) =0;
    virtual bool bindTex(void) =0;
    virtual void createFromXML(const XMLElement *e) =0;
};

constexpr Mob *Mobp(Entity *e) {
    return (Mob *)e;
}

class Page : public Mob {
private:
    std::string pageTexPath;
    GLuint pageTexture;
public:
    Page(void);

    void act(void);
    bool bindTex(void);
    void createFromXML(const XMLElement *e);
};

class Door : public Mob {
public:
    Door(void);

    void act(void);
    bool bindTex(void);
    void createFromXML(const XMLElement *e);
};

class Cat : public Mob {
public:
    Cat(void);

    void act(void);
    bool bindTex(void);
    void createFromXML(const XMLElement *e);
};

class Rabbit : public Mob {
public:
    Rabbit(void);

    void act(void);
    bool bindTex(void);
    void createFromXML(const XMLElement *e);
};

class Bird : public Mob {
private:
    float initialY;
public:
    Bird(void);

    void act(void);
    bool bindTex(void);
    void createFromXML(const XMLElement *e);
};

class Trigger : public Mob {
private:
    std::string id;
    bool triggered;
public:
    Trigger(void);

    void act(void);
    bool bindTex(void);
    void createFromXML(const XMLElement *e);
};

#endif // MOB_H_
