#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "entityx/entityx.h"

entityx::EntityX ex;

namespace exn = entityx;


struct Agent {
	Agent(const std::vector<std::string> plan) :
		plan(plan) {}
	std::vector<std::string> plan;
};
/*
struct BaseProperties {
	BaseProperties(char* name) :
		name(name) {}
	char* name;
};
 */

struct BaseProperties {
	BaseProperties(const char* nameInit)
			 {
					 strcpy(name,nameInit);
			 }
	char name[50];
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

struct SelectEvent
{
	SelectEvent(entityx::Entity entity)
	: entity(entity) {}
	entityx::Entity entity;
};


struct MouseClickEvent
{
	MouseClickEvent( const float x, float y)
			: x(x), y(y) {}
	float x, y;
};



class ClickResponseSystem : public exn::System<ClickResponseSystem>, public exn::Receiver<ClickResponseSystem> {
public:
	void configure(entityx::EventManager &event_manager)  {
		event_manager.subscribe<MouseClickEvent>(*this);
	}
	void receive(const MouseClickEvent &mouseEvent) {
		exn::ComponentHandle<Position> position;

		int cx = floor(mouseEvent.x);
		int cy = floor(mouseEvent.y);


		ex.entities.each<Position, BaseProperties>([cx,cy](exn::Entity entity, Position &position, BaseProperties &baseproperties) {
			if ((position.x==cx) && (position.y==cy))
			{
				ex.events.emit<SelectEvent>(entity);

				//ImGui::Text(baseproperties.name.c_str());
			}
		});



		//ImGui::Text("alala");
		//em1.emit<SelectEvent>()

		/*
		for (exn::Entity entity : es.entities_with_components(position)) {

			events.emit<Position>(currentCell);
		}
		*/
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, exn::TimeDelta dt) override {}
};


class SerializationSystem : public exn::System<SerializationSystem>, public exn::Receiver<SerializationSystem> {
public:

	entityx::Entity targetEntity;
	void configure(entityx::EventManager &event_manager)  {
		event_manager.subscribe<SelectEvent>(*this);
		//event_manager.subscribe<SelectEvent>(*this);
	}

	void receive(const SelectEvent &selectEvent) {

		targetEntity=selectEvent.entity;

		//ImGui::Text("alala");
		//em1.emit<SelectEvent>()

		/*
		for (exn::Entity entity : es.entities_with_components(position)) {

			events.emit<Position>(currentCell);
		}
		*/
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, exn::TimeDelta dt) override {
		if (targetEntity.id()!=targetEntity.INVALID){
			if (targetEntity.has_component<BaseProperties>()) {
				BaseProperties* b1 = targetEntity.component<BaseProperties>().get();
				ImGui::InputText("name", b1->name, IM_ARRAYSIZE(b1->name));
				//ImGui::Text(targetEntity.component<BaseProperties>().get()->name.c_str());
			};
		}
	}

};


class Level : public entityx::EntityX {
public:
	explicit Level(std::string filename) {
		/*
		systems.add<DebugSystem>();
		systems.add<MovementSystem>();
		systems.add<CollisionSystem>();
		 */
		systems.configure();

		//level.load(filename);
/*
		for (auto e : level.entity_data()) {
			entityx::Entity entity = entities.create();
			entity.assign<Position>(rand() % 100, rand() % 100);
			entity.assign<Direction>((rand() % 10) - 5, (rand() % 10) - 5);
		}
		*/
	}

	void update(entityx::TimeDelta dt) {
		/*
		systems.update<DebugSystem>(dt);
		systems.update<MovementSystem>(dt);
		systems.update<CollisionSystem>(dt);
		 */
	}
};


#endif