#ifndef MOB_H_
#define MOB_H_

#include <forward_list>
#include <tuple>

#include <common.hpp>
#include <entities.hpp>
#include <gametime.hpp>
#include <ui.hpp>
#include <save_util.hpp>

// local library headers
#include <tinyxml2.h>
using namespace tinyxml2;

extern Player *player;
extern std::string currentXML;

using Drop = std::tuple<std::string, unsigned int, float>;

class Mob : public Entity {
protected:
	XMLElement *xmle;
	std::forward_list<Drop> drop;

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

	virtual void onHit(unsigned int) =0;
	virtual void onDeath(void);

    virtual bool bindTex(void) =0;
};

constexpr Mob *Mobp(Entity *e) {
    return (Mob *)e;
}

class Page : public Mob {
private:
	std::string cId, cValue;
    std::string pageTexPath;
    GLuint pageTexture;
public:
    Page(void);

    void act(void);
	void onHit(unsigned int);
    bool bindTex(void);
    void createFromXML(XMLElement *e, World *w) final;
	void saveToXML(void) final;
};

class Door : public Mob {
public:
    Door(void);

    void act(void);
	void onHit(unsigned int);
    bool bindTex(void);
    void createFromXML(XMLElement *e, World *w) final;
	void saveToXML(void) final;
};

class Cat : public Mob {
public:
    Cat(void);

    void act(void);
	void onHit(unsigned int);
    bool bindTex(void);
	void createFromXML(XMLElement *e, World *w) final;
	void saveToXML(void) final;
};

class Rabbit : public Mob {
public:
    Rabbit(void);

    void act(void);
	void onHit(unsigned int);
    bool bindTex(void);
    void createFromXML(XMLElement *e, World *w) final;
	void saveToXML(void) final;
};

class Bird : public Mob {
private:
    float initialY;
public:
    Bird(void);

    void act(void);
	void onHit(unsigned int);
    bool bindTex(void);
    void createFromXML(XMLElement *e, World *w) final;
	void saveToXML(void) final;
};

class Trigger : public Mob {
private:
    std::string id;
    bool triggered;
public:
	bool notext;

    Trigger(void);

    void act(void);
	void onHit(unsigned int);
    bool bindTex(void);
    void createFromXML(XMLElement *e, World *w) final;
	void saveToXML(void) final;
};

#endif // MOB_H_
