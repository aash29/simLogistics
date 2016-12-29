
#ifndef CAR_H
#define CAR_H

#include <vector>
#include <map>
#include <unordered_set>
#include <iostream>
#include "imgui.h"
#include "graph.h"
#include "DebugDraw.h"
#include "coinsLog.h"



#define jointType b2DistanceJoint


class MultiQueryCallback : public b2QueryCallback {
public:
    MultiQueryCallback(const b2Vec2 &point) {
        m_point = point;

        m_fixtures = new std::unordered_set<b2Fixture*>;
    }

    bool ReportFixture(b2Fixture* fixture) {

        b2Body *body = fixture->GetBody();
        if (body->GetType() == b2_dynamicBody) {
            bool inside = fixture->TestPoint(m_point);
            if (inside) {
                m_fixtures->insert(fixture);

                // We are done, terminate the query.
                //return false;
            }
        }

        // Continue the query.
        return true;
    }

    b2Vec2 m_point;
    std::unordered_set<b2Fixture*>  *m_fixtures;
};

// This is a fun demo that shows off the wheel joint
class Car : public Test {
public:

    struct boxBot {

        struct magnet {
            b2Vec2 pos;
            bool active = false;
            b2Fixture *fix;
        };
        int id;
        b2Body *box;
        b2Body *wheel;
        b2RevoluteJoint *spring;
        b2Fixture *fix;
        float buffer[100];
        magnet magnets[4];
		float speed = 0.f;
        float torque = 0.f;
		float integratedError = 0.f;
        boxBot() {}

    };

    std::vector<boxBot*> bots = std::vector<boxBot*>();


    boxBot* body2Bot(b2Body *b1) {
        for (std::vector<boxBot*>::iterator bb = bots.begin(); bb != bots.end(); bb++) {
            if ((b1 == (*bb)->box) || (b1 == (*bb)->wheel)) {
                return (*bb);
            }
        }
        return nullptr;
    };


    boxBot* SelectBot(b2Vec2 p) {
        b2AABB aabb;
        b2Vec2 d;
        d.Set(0.001f, 0.001f);
        aabb.lowerBound = p - d;
        aabb.upperBound = p + d;

        // Query the world for overlapping shapes.
        MultiQueryCallback callback(p);
        m_world->QueryAABB(&callback, aabb);


        boxBot *b1 = nullptr;
        std::for_each(callback.m_fixtures->begin(),callback.m_fixtures->end(), [this, &b1] (b2Fixture* f1) {
            b2Body *body = f1->GetBody();
            b1 = body2Bot(body);

        });

        return b1;

    }



    void removeBotByID(int id) {
        for (std::vector<boxBot*>::iterator bb = bots.begin(); bb != bots.end(); bb++) {
            if ((*bb)->id==id) {
                //bots->erase( std::remove( bots->begin(), bots->end(), contactBot ), bots->end() );
                bots.erase(bb);
                return;
            }
        }
    };

    int getUID()
    {
        static int  i = -1;
        i++;
        return i;
    }

    boxBot* createBox(float x, float y, int id) {
        boxBot* bb;

        bb = new boxBot();

        bb->id=id;

        b2PolygonShape chassis;
        b2Vec2 vertices[4];



        vertices[0].Set(1.0f, -1.0f);
        bb->magnets[0].pos.Set(1.0f, -1.0f);
        bb->magnets[0].active = false;



        vertices[1].Set(1.0f, 1.0f);
        bb->magnets[1].pos.Set(1.0f, 1.0f);
        bb->magnets[1].active = false;

        vertices[2].Set(-1.0f, 1.0f);
        bb->magnets[2].pos.Set(-1.0f, 1.0f);
        bb->magnets[2].active = false;


        vertices[3].Set(-1.0f, -1.0f);
        bb->magnets[3].pos.Set(-1.0f, -1.0f);
        bb->magnets[3].active = false;




        chassis.Set(vertices, 4);



        b2CircleShape circle;
        circle.m_radius = 0.7f;

        b2BodyDef bd;

        bd.type = b2_dynamicBody;
        bd.position.Set(x, y);


        bb->box = m_world->CreateBody(&bd);

        b2FixtureDef fd0;
        fd0.shape = &chassis;
        fd0.density = 1.0f;
        fd0.friction = 0.9f;
        fd0.filter.groupIndex = 2;
		fd0.filter.categoryBits = 0x0003;


        bb->fix = bb->box->CreateFixture(&fd0);


        b2FixtureDef fd;
        fd.shape = &circle;
        fd.density = 1.0f;
        fd.friction = 0.9f;
        fd.filter.groupIndex = -2;
        //fd.filter.categoryBits=0x0002;
        //fd.filter.categoryBits=0x0002;


        bd.position.Set(x, y);
        bb->wheel = m_world->CreateBody(&bd);
        bb->wheel->CreateFixture(&fd);


        b2RevoluteJointDef jd;
        b2Vec2 axis(0.0f, 0.0f);

        jd.Initialize(bb->box, bb->wheel, bb->wheel->GetPosition());
        jd.motorSpeed = 0.0f;
        jd.maxMotorTorque = 600.0f;
        jd.enableMotor = true;




        bb->spring = (b2RevoluteJoint *) m_world->CreateJoint(&jd);



        b2PolygonShape magFix;


        b2FixtureDef mfd2;
        mfd2.shape = &magFix;
        mfd2.density = 0.001f;
        mfd2.friction = 0.9f;
        mfd2.filter.categoryBits=0x0003;   // magnet triggers


        magFix.SetAsBox(0.5f,0.5f,b2Vec2(0.5f,-0.5f),0.f);
        bb->magnets[0].fix = bb->box->CreateFixture(&mfd2);
        bb->magnets[0].fix->SetSensor(true);
        int* i0 = new int;
        *i0 =0;
        bb->magnets[0].fix->SetUserData(i0);

        magFix.SetAsBox(0.5f,0.5f,b2Vec2(0.5f,0.5f),0.f);
        bb->magnets[1].fix = bb->box->CreateFixture(&mfd2);
        bb->magnets[1].fix->SetSensor(true);
        int* i1 = new int;
        *i1 =1;
        bb->magnets[1].fix->SetUserData(i1);

        magFix.SetAsBox(0.5f,0.5f,b2Vec2(-0.5f,0.5f),0.f);
        bb->magnets[2].fix = bb->box->CreateFixture(&mfd2);
        bb->magnets[2].fix->SetSensor(true);
        int* i2 = new int;
        *i2 =2;
        bb->magnets[2].fix->SetUserData(i2);

        magFix.SetAsBox(0.5f,0.5f,b2Vec2(-0.5f,-0.5f),0.f);
        bb->magnets[3].fix = bb->box->CreateFixture(&mfd2);
        bb->magnets[3].fix->SetSensor(true);
        int* i3 = new int;
        *i3 =3;
        bb->magnets[3].fix->SetUserData(i3);



        return bb;
    };




    void OnSetCurrent(boxBot* bb) {

        if (bb!= nullptr) {

            b2PolygonShape magFix;


            b2FixtureDef mfd2;
            mfd2.shape = &magFix;
            mfd2.density = 0.001f;
            mfd2.friction = 0.9f;
            mfd2.filter.categoryBits=3;   // magnet triggers


            magFix.SetAsBox(0.5f,0.5f,b2Vec2(0.5f,-0.5f),0.f);
            bb->magnets[0].fix = bb->box->CreateFixture(&mfd2);
            bb->magnets[0].fix->SetSensor(true);
            int* i0 = new int;
            *i0 =0;
            bb->magnets[0].fix->SetUserData(i0);

            magFix.SetAsBox(0.5f,0.5f,b2Vec2(0.5f,0.5f),0.f);
            bb->magnets[1].fix = bb->box->CreateFixture(&mfd2);
            bb->magnets[1].fix->SetSensor(true);
            int* i1 = new int;
            *i1 =1;
            bb->magnets[1].fix->SetUserData(i1);

            magFix.SetAsBox(0.5f,0.5f,b2Vec2(-0.5f,0.5f),0.f);
            bb->magnets[2].fix = bb->box->CreateFixture(&mfd2);
            bb->magnets[2].fix->SetSensor(true);
            int* i2 = new int;
            *i2 =2;
            bb->magnets[2].fix->SetUserData(i2);

            magFix.SetAsBox(0.5f,0.5f,b2Vec2(-0.5f,-0.5f),0.f);
            bb->magnets[3].fix = bb->box->CreateFixture(&mfd2);
            bb->magnets[3].fix->SetSensor(true);
            int* i3 = new int;
            *i3 =3;
            bb->magnets[3].fix->SetUserData(i3);






        }
    };


    void OnUnsetCurrent(boxBot* bb) {
        if (bb!= nullptr) {
            bb->box->DestroyFixture(bb->magnets[0].fix);
            bb->box->DestroyFixture(bb->magnets[1].fix);
            bb->box->DestroyFixture(bb->magnets[2].fix);
            bb->box->DestroyFixture(bb->magnets[3].fix);
        }

    };


    void SetCurrent (boxBot* bb)
    {
        if (currentBot!= nullptr) {
            //OnUnsetCurrent(currentBot);
        }
        //currentBot = bb;
        selectedBots->insert(bb);
        //OnSetCurrent(bb);
    }

    Car() {

        m_speed = 100;

        b2Body *ground = NULL;
        {
            b2BodyDef bd;
            ground = m_world->CreateBody(&bd);

            b2EdgeShape shape;

            b2FixtureDef fd;
            fd.shape = &shape;
            fd.density = 0.0f;
            fd.friction = 0.9f;

            shape.Set(b2Vec2(-20.0f, 0.0f), b2Vec2(20.0f, 0.0f));

            ground->CreateFixture(&fd);

            float32 hs[10] = {0.25f, 1.0f, 4.0f, 0.0f, 0.0f, -1.0f, -2.0f, -2.0f, -1.25f, 0.0f};

            float32 groundMap[20][2] = {{-20.0f,20.0f}, {-20.0f,10.0f}, {-15.f,10.0f}, {-10.0f,10.0f}, {-5.f,10.0f},{-5.f,0.0f},{0.0f,0.0f}, {5.0f,0.0f}, {5.0f,10.0f}, {10.f,10.0f}, {15.0f,10.0f}, {20.f,10.0f},{20.f,20.0f}};

            float32 x = 20.0f, y1 = 0.0f, dx = 5.0f;
/*
            for (int32 i = 0; i < 10; ++i) {
                float32 y2 = hs[i];
                shape.Set(b2Vec2(x, y1), b2Vec2(x + dx, y2));
                ground->CreateFixture(&fd);
                y1 = y2;
                x += dx;
            }

*/          float x1 = groundMap[0][0];
            y1 = groundMap[0][1];
            for (int i = 1; i < 13; ++i) {
                float32 x2 = groundMap[i][0];
                float32 y2 = groundMap[i][1];
                shape.Set(b2Vec2(x1, y1), b2Vec2(x2, y2));
                ground->CreateFixture(&fd);
                y1 = y2;
                x1 = x2;
            }
        }



        b2FixtureDef goalZoneDef;

        b2PolygonShape goalZoneShape;
        b2Vec2 vertices[4];

        vertices[0].Set(10.0f, 3.0f);
        vertices[1].Set(10.0f, 15.0f);
        vertices[2].Set(20.0f, 15.0f);
        vertices[3].Set(20.0f, 3.0f);

        goalZoneShape.Set(vertices, 4);


        goalZoneDef.shape = &goalZoneShape;
        goalZoneDef.density = 1.0f;
        goalZoneDef.filter.groupIndex = -2;
        goalZoneDef.filter.categoryBits = 0x0002; // goal zone

        b2Fixture* goalZone = ground->CreateFixture(&goalZoneDef);
        goalZone->SetSensor(true);
//        goalZone->SetUserData()


        //m_car=createBox(0.f,2.f);


        //bots = new std::vector<boxBot*>;
        bots.push_back(createBox(0.f, 9.f, getUID()));
		/*
        bots.push_back(createBox(0.f, 6.f, getUID()));
        bots.push_back(createBox(0.f, 2.f, getUID()));
        bots.push_back(createBox(-2.f, 2.f, getUID()));
        bots.push_back(createBox(2.f, 2.f, getUID()));
		*/
        //bots->push_back(createBox(-10.f, 2.f, getUID()));
        //b1=createBox(0.f,2.f);

        //b2=createBox(2.f,2.f);

        destroyedBots = new std::vector<boxBot*>;


        magnetJoints = new std::map<int,jointType *>;
        currentBot = nullptr;
        selectedBots = new std::unordered_set<boxBot*>;
        //SetCurrent(&((*bots)[0]));

    }

    int symmHash(short int a, short int b)
    {
        if (a>b)
            return symmHash(b,a);
        return  a << 16 |  b;
    };

    void updateJoints() {
        int id = 0;
        for (int i = 0; i < bots.size(); i++) {
            for (int j = i + 1; j < bots.size(); j++) {
                for (int k = 0; k < 4; k++) {
                    for (int l = 0; l < 4; l++) {

                        short int h1 =  i << 8 | k;
                        short int h2 =  j << 8 | l;
                        id = symmHash(h1,h2);

                        b2Vec2 p1 = ((b2PolygonShape *) ((bots)[i]->fix->GetShape()))->GetVertex(k);
                        b2Vec2 p2 = ((b2PolygonShape *) ((bots)[j]->fix->GetShape()))->GetVertex(l);
                        (bots)[i]->magnets[k].pos = (bots)[i]->box->GetWorldPoint(p1);
                        (bots)[j]->magnets[l].pos = (bots)[j]->box->GetWorldPoint(p2);

                        b2Vec2 dir = (bots)[i]->magnets[k].pos - (bots)[j]->magnets[l].pos;
                        float magn = dir.Length();
                        dir.Normalize();
                        //const b2Vec2 force = std::min(100/(magn*magn),100.f)*dir;
                        const b2Vec2 force = 200.f*std::exp(-20.f*magn*magn)*dir;
                        const b2Vec2 pos1 = (bots)[i]->magnets[k].pos;
                        const b2Vec2 pos2 = (bots)[j]->magnets[l].pos;

                        //const b2Vec2 force2 = -std::max(1/magn,10.f)*dir;

                        if (((bots)[i]->magnets[k].active) && ((bots)[j]->magnets[l].active)) {
                            (bots)[j]->box->ApplyForce(force,pos2, true);
                            (bots)[i]->box->ApplyForce(-force,pos1, true);


                            if (((bots)[i]->magnets[k].pos - (bots)[j]->magnets[l].pos).Length() < 0.2f) {
                                b2DistanceJointDef jd;


                                jd.collideConnected = true;
                                jd.length = 0.01f;
                                jd.frequencyHz = 20.0f;
                                jd.dampingRatio=0.5f;
                                jd.bodyA = (bots)[i]->box;
                                jd.bodyB = (bots)[j]->box;
                                jd.localAnchorA.Set(p1.x,p1.y);
                                jd.localAnchorB.Set(p2.x,p2.y);
                                //jd.Initialize((*bots)[i].box, (*bots)[j].box, (*bots)[i].magnets[k].pos,(*bots)[j].magnets[l].pos);
                                std::map<int,jointType *>::iterator j1 = magnetJoints->find(id);
                                if (j1==magnetJoints->end()) {
                                    magnetJoints->insert(std::pair<int, jointType *>(id, (jointType *) m_world->CreateJoint(&jd)));
                                    ImGui::Text("magnet links active: %d", magnetJoints->size());
                                }

/*
                                ImGui::Text("%d",id);

                                b2RevoluteJointDef jd;
                                jd.collideConnected = true;
                                jd.Initialize((*bots)[i].box, (*bots)[j].box, (*bots)[i].magnets[k].pos);
                                std::map<int,b2RevoluteJoint *>::iterator j1 = magnetJoints->find(id);
                                if (j1==magnetJoints->end()) {
                                    magnetJoints->insert(std::pair<int, b2RevoluteJoint *>(id, (b2RevoluteJoint *) m_world->CreateJoint(&jd)));
                                    ImGui::Text("magnet links active: %d", magnetJoints->size());
                                }
*/


                                g_debugDraw.DrawPoint(pos1, 6, b2Color(0.f, 0.f, 1.f));
                            }



                        }
                        else{
                            std::map<int,jointType *>::iterator j1 = magnetJoints->find(id);
                            if (j1!=magnetJoints->end()){

                                m_world->DestroyJoint(j1->second);
                                //m_world->DestroyJoint(magnetJoints->begin()->second);
                                magnetJoints->erase(id);
                                std::cout << "deleted" << id ;
                            }
                        }
                    }
                }
            }
        }
    }


    void Keyboard(int key) {
        switch (key) {
            case GLFW_KEY_A:
                std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {
                   
                }
                );
                //std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {b1->spring->SetMotorSpeed(-m_speed);});
                //if (currentBot!= nullptr)
                //    currentBot->spring->SetMotorSpeed(-m_speed);
                break;

            case GLFW_KEY_D:
                //if (currentBot!= nullptr)
                //    currentBot->spring->SetMotorSpeed(m_speed);
                //std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {b1->spring->SetMotorSpeed(m_speed);});
                //std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {b1->torque=b1->torque-0.5f;});
                std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {
                          
                              }
                );


                break;

            case GLFW_KEY_S:
                //if (currentBot!= nullptr)
                //    currentBot->spring->SetMotorSpeed(0.0f);

                //std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {b1->spring->SetMotorSpeed(0.f);});
                std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {
                                  b1->torque = 0.f;
                              }
                );
                break;


			//case GLFW_KEY_SPACE:
				//pause = !pause;
				//break;

        }
    }

     void KeyboardUp(int key) {
         switch (key) {
             case GLFW_KEY_A:
                 std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot *b1) {

                               }
                 );
                 break;

             case GLFW_KEY_D:
                 std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot *b1) {
                               }
                 );
                 break;
         }

     }


    //boxBot* body2bot(b2)


    void EnterKeyDown(){
        std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {
            for (int i=0; i<4; i++)
                b1->magnets[i].active=!b1->magnets[i].active;});
    };


    void handleFastInput(float dir) {
        bots[0]->torque+=1.f*dir;
    };




    void MouseDown(const b2Vec2 &p) {
        //Test::MouseDown(p);

        m_mouseWorld = p;


        if (m_mouseJoint != NULL) {
            return;
        }


        boxBot *b1 = SelectBot(p);
		if (b1 != nullptr) {
			if (selectedBots->find(b1) != selectedBots->end()) {

				b2AABB aabb;
				b2Vec2 d;
				d.Set(0.001f, 0.001f);
				aabb.lowerBound = p - d;
				aabb.upperBound = p + d;

				// Query the world for overlapping shapes.
				MultiQueryCallback callback(p);
				m_world->QueryAABB(&callback, aabb);
				std::for_each(callback.m_fixtures->begin(), callback.m_fixtures->end(), [this, b1, p](b2Fixture *f1) {
					b2Body *body = f1->GetBody();



					if (f1->GetFilterData().categoryBits == 0x0003) {
						int *udInt = (int *)f1->GetUserData();
						b1->magnets[*udInt].active = !b1->magnets[*udInt].active;
					}
					selectedBots->clear();
					SetCurrent(b1);
					//currentBot = body2Bot(body);
					/*
					if (body != b1->wheel) {

						b2MouseJointDef md;
						md.bodyA = m_groundBody;
						md.bodyB = body;
						md.target = p;
						md.maxForce = 1000.0f * body->GetMass();
						m_mouseJoint = (b2MouseJoint *) m_world->CreateJoint(&md);
						body->SetAwake(true);
					}

					else {
						b2MouseJointDef md;
						md.bodyA = m_groundBody;
						md.bodyB = b1->box;
						md.target = p;
						md.maxForce = 1000.0f * body->GetMass();
						m_mouseJoint = (b2MouseJoint *) m_world->CreateJoint(&md);
						body->SetAwake(true);
					}
					*/
				});
			}
			else {
				selectedBots->clear();
				SetCurrent(b1);
			}
		}
		
	};

	void MouseMove(const b2Vec2 &p)
	{

		m_mouseWorld = p;
		std::for_each(selectedBots->begin(), selectedBots->end(), [this](boxBot* b1) {
			b1->integratedError = 0.f;
		});
	}


    void ShiftMouseDown(const b2Vec2 &p) {
        m_mouseWorld = p;
        boxBot* bb= SelectBot(p);
        if (bb!= nullptr)
            selectedBots->insert(bb);

    }

    void RightMouseDown(const b2Vec2 &p)
    {
        SetCurrent(nullptr);
        selectedBots->clear();
    };

    void MiddleMouseDown(const b2Vec2 &p)
    {
        bots.push_back(createBox(p.x, p.y, getUID()));

        //SetCurrent(&((*bots)[bots->size()-1]));
    };

    void Step(Settings *settings) {

        //g_camera.m_center.x = (*bots)[0].box->GetPosition().x;
        /*
        for (std::vector<boxBot*>::iterator bb = bots.begin(); bb != bots.end(); bb++) {

            for (int i = 0; i < IM_ARRAYSIZE((*bb)->buffer); i++) {
                (*bb)->buffer[i] = (*bb)->buffer[(i + 1) % IM_ARRAYSIZE((*bb)->buffer)];
            }
            (*bb)->buffer[IM_ARRAYSIZE((*bb)->buffer) - 1] = (*bb)->box->GetPosition().x;

            (*bb)->box->ApplyTorque((*bb)->torque,true);

        }
         */

        updateJoints();
        DrawMagnetScheme();

        ImGui::SliderFloat("Torque", &(bots[0]->torque), -150.0f, 150.0f);



        DrawActiveMagnets();

        ImGui::Text("bots in goal: %d", victoryCount);

        coinsLog.Draw("Log");



        //coinsLog.AddLog("left, %g \n",bots[0]->torque );

		ImGui::Checkbox("Manual control", &manualControl);

        static float KI=-200.f,KP=-800.f,KD=-40.f;

		ImGui::SliderFloat("KI", &KI, -1000.0f, 1000.0f);
        ImGui::SliderFloat("KP", &KP, -1000.0f, 0.0f);
        ImGui::SliderFloat("KD", &KD, -150.0f, 0.0f);

		//settings->pause = pause;
		this->settings = settings;

		if (manualControl){

		std::for_each(selectedBots->begin(), selectedBots->end(), [this,settings](boxBot* b1) {
			b2Vec2 z1 = b1->box->GetWorldVector(b2Vec2(1.f, 1.f));
			b2Vec2 z2 = (m_mouseWorld - b1->box->GetPosition());
			//z1.Normalize();
			//z2.Normalize();

			float a1 = b1->box->GetAngle();
            a1+=b2_pi/4.f;
			float a2 = atan2(z2.y, z2.x);

			b1->integratedError += sin(a1 - a2)*1.f / settings->hz;

            g_debugDraw.DrawSegment(b1->box->GetPosition(), m_mouseWorld,b2Color(1.f,1.f,1.f));
            g_debugDraw.DrawSegment(b1->box->GetPosition(), b1->box->GetPosition()+z1,b2Color(1.f,0.f,0.f));

            ImGui::Text("bot angle: %g", a1);
            ImGui::Text("target angle: %g", a2);

            float w1 = b1->box->GetAngularVelocity();

			float MP = KP * sin(a1 - a2);
            float MD = KD * (w1);

			float MI = KI * b1->integratedError;

			b2Vec2 v = b1->box->GetLinearVelocity();
			b2Vec2 p0 = b1->box->GetPosition();
			float dt = 0.01f;
			for (int i=0; i< 300; i++)
			{
				g_debugDraw.DrawSegment(p0, p0 + dt*v, b2Color(1.f, 1.f, 1.f));
				v.y = v.y - 9.8*dt;
				p0 = p0 + dt*v;
				

			};


			b1->box->ApplyTorque(MP+MD+MI, true);

		});
		}


        for (int i = 0; i < bots.size(); i++) {

            /*
            bots[i]->torque=bots[i]->torque+bots[i]->torqueChange;
            if (bots[i]->torque>100.f)
            {
                bots[i]->torque=100.f;
            }

            if (bots[i]->torque<-100.f)
            {
                bots[i]->torque=-100.f;
            }
            */

            bots[i]->box->ApplyTorque(bots[i]->torque,true);
        };




        Test::Step(settings);

        Cleanup();
    }


    void plotGraphs() {


        ImVec2 foo[10];

        foo[0].x = 0; // init data so editor knows to take it from here

        for (int i = 0; i < 10; i++) {
            foo[i].x = i * 0.1f;
            foo[i].y = i * 0.1f;
        }


        float foo2[100];
        for (int i = 0; i < 100; i++) {
            foo2[i] = sinf(i * 0.1f);
        }

        bool f = true;
        ImGui::GraphTestWindow((bots)[0]->buffer, 100);

        ImGui::Curve("Curve", ImVec2(600, 200), 10, foo);

    }


    void DrawActiveMagnets() {
        for (int i = 0; i < bots.size(); i++) {
            for (int j = 0; j < 4; j++) {
                if ((bots)[i]->magnets[j].active) {
                   // g_debugDraw.DrawPoint((*bots)[i]->magnets[j].pos, 5, b2Color(1.f, 0.f, 0.f));
                    const b2Transform& xf = (bots)[i]->box->GetTransform();
                    b2PolygonShape* poly = (b2PolygonShape*)(bots)[i]->magnets[j].fix->GetShape();
                    int32 vertexCount = poly->m_count;
                    b2Assert(vertexCount <= b2_maxPolygonVertices);
                    b2Vec2 vertices[b2_maxPolygonVertices];

                    for (int32 i = 0; i < vertexCount; ++i)
                    {
                        vertices[i] = b2Mul(xf, poly->m_vertices[i]);
                    }
                    b2Color color(0.2f, 0.95f, 0.6f);
                    g_debugDraw.DrawSolidPolygon(vertices, vertexCount, color);
                    //g_debugDraw.DrawCircle((*bots)[i].magnets[j].pos, 0.5f, b2Color(1.f, 1.f, 0.5f));
                }
                else {
                    //g_debugDraw.DrawCircle((*bots)[i].magnets[j].pos, 0.5f, b2Color(0.f, 0.f, 0.0f));
                }
            }
        }
    }

    void DrawMagnetScheme() {

        std::for_each(selectedBots->begin(),selectedBots->end(),[this](boxBot* b1){
           // b2Vec2* p1 = new b2Vec2(b1->box->GetWorldCenter());
            b2Vec2 p1 = b2Vec2(b1->box->GetWorldCenter().x,b1->box->GetWorldCenter().y);
            g_debugDraw.DrawCircle(p1, 1.41f, b2Color(1.f, 1.f, 1.f, 0.5f));
        });

    }

    void Cleanup()
    {
        for (auto it = destroyedBots->begin(); it != destroyedBots->end(); it++)
        {

            m_world->DestroyBody((*it)->box);
            m_world->DestroyBody((*it)->wheel);
            removeBotByID((*it)->id);
        }

        destroyedBots->clear();



    }


    void BeginContact(b2Contact* contact)
    {

        b2Body* targetBody = nullptr;
        if (contact->GetFixtureA()->GetFilterData().categoryBits==2)
        {
            targetBody  = contact->GetFixtureB()->GetBody();
        }
        else
        if (contact->GetFixtureA()->GetFilterData().categoryBits==2)
        {
            targetBody  = contact->GetFixtureA()->GetBody();
        }

        if (targetBody != nullptr){
            boxBot* contactBot = body2Bot(targetBody);

            victoryCount++;
            //destroyedBots->push_back(contactBot);
        }



		b2Body* currentBody = nullptr;
		if (contact->GetFixtureA()->GetFilterData().categoryBits == 3)
		{
			currentBody = contact->GetFixtureA()->GetBody();
		}
		else
		if (contact->GetFixtureB()->GetFilterData().categoryBits == 3)
		{
			currentBody = contact->GetFixtureB()->GetBody();
		}

		if (currentBody != nullptr) {
			boxBot* contactBot = body2Bot(currentBody);
			if (selectedBots->find(contactBot)!= selectedBots->end()){
			    settings->pause = true;
				//settings->hz = 200.f;
			}
			//destroyedBots->push_back(contactBot);
		}

    }

    void EndContact(b2Contact* contact)
    {

        b2Body* targetBody = nullptr;
        if (contact->GetFixtureA()->GetFilterData().categoryBits==2)
        {
            targetBody  = contact->GetFixtureB()->GetBody();
        }


        if (contact->GetFixtureA()->GetFilterData().categoryBits==2)
        {
            targetBody  = contact->GetFixtureA()->GetBody();
        }

        if (targetBody != nullptr){
            boxBot* contactBot = body2Bot(targetBody);

            victoryCount--;
        }



		b2Body* currentBody = nullptr;
		if (contact->GetFixtureA()->GetFilterData().categoryBits == 3)
		{
			currentBody = contact->GetFixtureA()->GetBody();
		}
		else
			if (contact->GetFixtureB()->GetFilterData().categoryBits == 3)
			{
				currentBody = contact->GetFixtureB()->GetBody();
			}

		if (currentBody != nullptr) {
			boxBot* contactBot = body2Bot(currentBody);
			if (selectedBots->find(contactBot) != selectedBots->end()) {
				//pause = true;
				//settings->hz = 60.f;
			}
			//destroyedBots->push_back(contactBot);
		}


    }


    static Test *Create() {
        return new Car;
    }


    //std::vector<boxBot*> bots = std::vector<boxBot*>();
    std::vector<boxBot*> *destroyedBots;

    boxBot *currentBot = nullptr;

    std::unordered_set <boxBot*> *selectedBots;

    float32 m_speed;
    std::map<int,jointType *> *magnetJoints;

    int victoryCount = 0;

	bool manualControl=true;
	bool pause = false;
	Settings* settings;

    AppLog coinsLog;
};

#endif
