#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "entityx/entityx.h"
#include "entityx/deps/Dependencies.h"
#include "coinsLog.h"
#include "eventqueue.h"
#include "lambdaevents.h"

entityx::EntityX ex;


namespace exn = entityx;

//  COMPONENTS


/*
struct BaseProperties {
	BaseProperties(char* name) :
		name(name) {}
	char* name;
};
 */

struct BaseProperties {
	BaseProperties(const char* nameInit, bool init_passable)
			 {
				 strcpy(name,nameInit);
				 passable=init_passable;

			 }
	char name[50];
	bool passable;
};


struct Position {
	Position(const int x, const int y)
		: x(x), y(y) {}
	Position()
	{
		x=0;
		y=0;
	};

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
	Renderable(const char* glyph_init)
	{
		strcpy(glyph,glyph_init);
	}
	Renderable()
	{
		strcpy(glyph,"U");
	}
	char glyph[2];
};



bool isPassable(int x, int y)
{
	bool result = true;
	ex.entities.each<Position, BaseProperties>([x,y,&result](exn::Entity entity, Position &position, BaseProperties &baseproperties) {

		if ((position.x==x) && (position.y==y))
		{
			if (!baseproperties.passable)
			{
				result = false;
			}
		}
	});
	return result;
}

//EVENTS

class Action
{
public:
virtual void execute()=0;
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

struct Agent;

struct ActionEvent
{
	ActionEvent( Agent* init_agent)
			{
				agent=init_agent;
			}
	Agent* agent;
};



//ACTIONS
class MoveAction : public Action {
public:
	MoveAction(entityx::Entity actor, Position from, Position to) :
			actor(actor), from(from), to(to) { };
	entityx::Entity actor;
	Position from;
	Position to;
	 //using GameEvent::execute;
	 virtual void execute() {
		//preconditions
		if (isPassable(to.x, to.y)) {
			entityx::Entity a1 = actor;
			a1.remove<Position>();
			a1.assign_from_copy<Position>(to);
			AppLog::instance()->AddLog("Moved from (%d,%d) to (%d,%d) \n", from.x, from.y,
									   to.x, to.y);
		} else
			//effects
		{
			AppLog::instance()->AddLog("Move to (%d,%d) impossible \n", to.x, to.y);
		}
	}
};


struct OpenAction : public Action
{
public:
	OpenAction(entityx::Entity actor,entityx::Entity target):
			actor(actor),target(target) {};
	entityx::Entity actor;
	entityx::Entity target;
	virtual void execute()
	{
		entityx::Entity t1 = target;
		bool openstate = t1.component<BaseProperties>().get()->passable;
		t1.component<BaseProperties>().get()->passable=!openstate;
		if (!openstate==true)
		{
			strcpy(t1.component<Renderable>().get()->glyph,"O");
		}
		else
		{
			strcpy(t1.component<Renderable>().get()->glyph,"C");
		}
		AppLog::instance()->AddLog("Opened %s \n", t1.component<BaseProperties>().get()->name);

	}

};

struct Agent {
	Agent()
	{
		plan=std::vector<Action*>();
	};
	std::vector<Action*> plan;
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


class ActionSystem : public exn::System<ActionSystem>, public exn::Receiver<ActionSystem> {
public:
	void configure(entityx::EventManager &event_manager)  {
		event_manager.subscribe<ActionEvent>(*this);
		//event_manager.subscribe<OpenAction>(*this);
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, exn::TimeDelta dt) {
		ImGui::Begin("Actions");
		if (ImGui::Button("Act"))
		{
			exn::ComponentHandle<Agent> agent;

			for (exn::Entity entity : es.entities_with_components(agent)) {
				events.emit<ActionEvent>(entity.component<Agent>().get());
			}

		};

		ImGui::End();
	}

	void receive(const ActionEvent &actionevent) {
		if (actionevent.agent->plan.size()>0)
		{
			actionevent.agent->plan[0]->execute();
			//actionevent.agent->plan.pop_back();
			actionevent.agent->plan.erase(actionevent.agent->plan.begin());
		}
	}



};

class SerializationSystem : public exn::System<SerializationSystem>, public exn::Receiver<SerializationSystem> {
public:

	entityx::Entity targetEntity;
	void configure(entityx::EventManager &event_manager)  {
		event_manager.subscribe<SelectEvent>(*this);
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
				if (ImGui::TreeNode("BaseProperties")){
					ImGui::InputText("name", b1->name, IM_ARRAYSIZE(b1->name));
					ImGui::Checkbox("Passable",&(b1->passable));
					ImGui::TreePop();
				}
			};

			if (targetEntity.has_component<Position>()) {
				Position* b1 = targetEntity.component<Position>().get();
				if (ImGui::TreeNode("Position")) {
					ImGui::InputInt("x", &(b1->x));
					ImGui::InputInt("y", &(b1->y));
					ImGui::TreePop();
				}
			};

			if (targetEntity.has_component<Renderable>()) {
				Renderable* b1 = targetEntity.component<Renderable>().get();
				if (ImGui::TreeNode("Renderable")){
					ImGui::InputText("glyph", b1->glyph, 2);
					ImGui::TreePop();
				}
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