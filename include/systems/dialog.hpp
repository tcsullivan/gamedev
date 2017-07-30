#ifndef SYSTEM_DIALOG_HPP_
#define SYSTEM_DIALOG_HPP_

#include <entityx/entityx.h>

#include <events.hpp>

class DialogSystem : public entityx::System<DialogSystem>, public entityx::Receiver<DialogSystem> {
public:
	void configure(entityx::EventManager&);
	bool receive(const MouseClickEvent&);
	void update(entityx::EntityManager&, entityx::EventManager&, entityx::TimeDelta) override;
};


#endif // SYSTEM_DIALOG_HPP_
