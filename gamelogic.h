#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include "entityx/entityx.h"
#include "entityx/deps/Dependencies.h"
#include "coinsLog.h"
#include "eventqueue.h"
#include "lambdaevents.h"
#include "path_finding.h"


entityx::EntityX ex;


namespace exn = entityx;

//  COMPONENTS


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
	bool visible = true;
};


struct Inventory {
	Inventory()
	{
		inventory=std::vector<entityx::Entity>();
	}

	std::vector<entityx::Entity> inventory;
};


bool isPassable(int x, int y)
{
	bool result = true;
	ex.entities.each<Position,BaseProperties>([x,y,&result](exn::Entity entity, Position &position, BaseProperties &baseproperties)  {
		//result = false;
		//AppLog::instance()->AddLog("Checking passability");
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


bool isNear(Position p1, Position p2)
{
	return (std::max(abs(p1.x-p2.x),abs(p1.y-p2.y))<=1);
}

bool isNear(exn::Entity entity1, exn::Entity entity2)
{
	Position* pos1 = entity1.component<Position>().get();
	Position* pos2 = entity2.component<Position>().get();
return (isNear(*pos1,*pos2));
}



//EVENTS

class Action
{
public:
	virtual bool execute()=0;
	virtual void Serialize()
	{
		if (ImGui::Begin("Actions"))
			{
			ImGui::Text("description not provided");
			ImGui::End();
		}
	};
};

struct SelectEvent
{
	SelectEvent(entityx::Entity entity)
	: entity(entity) {}
	entityx::Entity entity;

};

struct SelectCellEvent
{
	SelectCellEvent(int x, int y)
			: x(x), y(y) {}
	int x;
	int y;

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
	 virtual bool execute() {
		//preconditions
		if ((isPassable(to.x, to.y)) && isNear(from,to)) {
			//entityx::Entity a1 = actor;
			//a1.remove<Position>();
			//a1.assign_from_copy<Position>(to);
			actor.component<Position>().get()->x=to.x;
			actor.component<Position>().get()->y=to.y;

			if (actor.has_component<Inventory>())
			{
				for(entityx::Entity e : actor.component<Inventory>().get()->inventory)
				{
					e.component<Position>().get()->x=to.x;
					e.component<Position>().get()->y=to.y;
				}
			}

			AppLog::instance()->AddLog("Moved from (%d,%d) to (%d,%d) \n", from.x, from.y,
									   to.x, to.y);
			return true;
		} else
			//effects
		{
			AppLog::instance()->AddLog("Move to (%d,%d) impossible \n", to.x, to.y);
			return false;
		}
	}

	virtual void Serialize()
	{
		if (ImGui::Begin("Actions"))
		{
			ImGui::Text("Move from (%d,%d) to (%d,%d)", from.x,from.y,to.x,to.y);
			ImGui::End();
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
	virtual bool execute()
	{
		entityx::Entity t1 = target;
		bool openstate = t1.component<BaseProperties>().get()->passable;
		if (isNear(actor,target)) {
			t1.component<BaseProperties>().get()->passable=!openstate;

			if (!openstate == true) {
				strcpy(t1.component<Renderable>().get()->glyph, "O");
			}
			else {
				strcpy(t1.component<Renderable>().get()->glyph, "C");
			}
			AppLog::instance()->AddLog("Opened %s \n", t1.component<BaseProperties>().get()->name);
			return true;
		}
		else
		{
			AppLog::instance()->AddLog("Cannot open %s \n", t1.component<BaseProperties>().get()->name);
			return false;
		}
	}

	virtual void Serialize()
	{
		if (ImGui::Begin("Actions"))
		{
			ImGui::Text("Open (%s, %s)", actor.component<BaseProperties>().get()->name,target.component<BaseProperties>().get()->name);
			ImGui::End();
		}
	}

};


struct TakeAction : public Action
{
public:
	TakeAction(entityx::Entity actor,entityx::Entity target):
			actor(actor),target(target) {};
	entityx::Entity actor;
	entityx::Entity target;
	virtual bool execute()
	{
		entityx::Entity t1 = target;
		if (isNear(actor, target))
		{
			actor.component<Inventory>().get()->inventory.push_back(t1);
			t1.component<Renderable>().get()->visible=false;
			AppLog::instance()->AddLog("Taken %s \n", t1.component<BaseProperties>().get()->name);
			return true;
		}
		else
		{
			AppLog::instance()->AddLog("Cannot take %s \n", t1.component<BaseProperties>().get()->name);
			return false;
		}
	}


	virtual void Serialize()
	{
		if (ImGui::Begin("Actions"))
		{
			ImGui::Text("Take %s", target.component<BaseProperties>().get()->name);
			ImGui::End();
		}
	}
};

struct Agent {
	Agent()
	{
		plan=std::vector<Action*>();
	};
	std::vector<Action*> plan;
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



class RenderSystem : public exn::System<RenderSystem > {
public:

	void update(entityx::EntityManager &es, entityx::EventManager &events, exn::TimeDelta dt) {
		ex.entities.each<Position, Renderable>([](exn::Entity entity, Position &position ,Renderable &renderable) {
			if (renderable.visible)
			{
				b2Vec2 p1 = g_camera.ConvertWorldToScreen(b2Vec2(position.x,position.y));
				AddGfxCmdText(p1.x,g_camera.m_height-p1.y,TEXT_ALIGN_LEFT, renderable.glyph, WHITE);
			}
		});
	}




};

class SerializationSystem : public exn::System<SerializationSystem>, public exn::Receiver<SerializationSystem> {
public:

	std::vector<entityx::Entity> targetEntities;
	void configure(entityx::EventManager &event_manager)  {
		event_manager.subscribe<SelectEvent>(*this);
	}

	void receive(const SelectEvent &selectEvent) {

		targetEntities.push_back(selectEvent.entity);

	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, exn::TimeDelta dt) override {
		int i=0;
		for (auto targetEntity: targetEntities)
		{
			ImGui::Separator();
			ImGui::PushID(i);
			if (ImGui::TreeNode("Entity"))
			{
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
						ImGui::Checkbox("visible",&(b1->visible));
						ImGui::TreePop();
					}
				};
				ImGui::TreePop();
			}
			ImGui::PopID();
			i++;

			if (targetEntity.has_component<Agent>())
			{
				for (auto action : targetEntity.component<Agent>().get()->plan){
					action->Serialize();
				}
			}
		}

		ImGui::Separator();

	}

};



class ClickResponseSystem : public exn::System<ClickResponseSystem>, public exn::Receiver<ClickResponseSystem> {
public:
	void configure(entityx::EventManager &event_manager)  {
		event_manager.subscribe<MouseClickEvent>(*this);
	}
	void receive(const MouseClickEvent &mouseEvent) {

		ex.systems.system<SerializationSystem>().get()->targetEntities.clear();

		exn::ComponentHandle<Position> position;

		int cx = floor(mouseEvent.x);
		int cy = floor(mouseEvent.y);


		ex.entities.each<Position, BaseProperties>([cx,cy](exn::Entity entity, Position &position, BaseProperties &baseproperties) {
			if ((position.x==cx) && (position.y==cy))
			{
				ex.events.emit<SelectEvent>(entity);
			}
		});
	}

	void update(entityx::EntityManager &es, entityx::EventManager &events, exn::TimeDelta dt) override {}
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