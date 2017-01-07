#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "entityx/entityx.h"

namespace exn = entityx;


struct Agent {
	Agent(const std::vector<std::string> plan) :
		plan(plan) {}
	std::vector<std::string> plan;
};

struct BaseProperties {
	BaseProperties(const const std::string name) :
		name(name) {}
	std::string name;
};


struct Position {
	Position(const int x, const int y)
		: x(x), y(y) {}
	int x;
	int y;
};

Position currentCell = Position(0, 0);

struct Edible {
	Edible(const std::string name)
		: name(name) {}
	std::string name;
};

struct Renderable
{
	Renderable(const char glyph)
		: glyph(glyph) {}
	char glyph;
};

struct MouseClickEvent
{
	MouseClickEvent( const float x, float y)
	: x(x), y(y) {}
	float x, y;
};



class ClickResponseSystem : public exn::System<ClickResponseSystem>, public exn::Receiver<ClickResponseSystem> {
public:

	void receive(const MouseClickEvent &mouseEvent) {
		exn::ComponentHandle<Position> position;
		/*
		for (exn::Entity entity : es.entities_with_components(position)) {

			events.emit<Position>(currentCell);
		}
		*/
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, exn::TimeDelta dt) override {};
};

#endif