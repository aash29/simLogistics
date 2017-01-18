/*
* Copyright (c) 2006-2013 Erin Catto http://www.box2d.org
*
* This software is provided 'as-is', without any express or implied
* warranty.  In no event will the authors be held liable for any damages
* arising from the use of this software.
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely, subject to the following restrictions:
* 1. The origin of this software must not be misrepresented; you must not
* claim that you wrote the original software. If you use this software
* in a product, an acknowledgment in the product documentation would be
* appreciated but is not required.
* 2. Altered source versions must be plainly marked as such, and must not be
* misrepresented as being the original software.
* 3. This notice may not be removed or altered from any source distribution.
*/

#include "imgui.h"
#include "RenderGL3.h"
#include "DebugDraw.h"
#include "Test.h"
#include "Car.h"

#include "imgui_impl_glfw_gl3.h"

#include "graph.h"

#if defined(__APPLE__)
#include <OpenGL/gl3.h>
#else

#include <GL/glew.h>

#endif

#include <GLFW/glfw3.h>
#include "imgui_impl_glfw_gl3.h"


#include <GLFW/glfw3.h>
#include <stdio.h>
//#include "entityx/entityx.h"
#include "gamelogic.h"

#include <vector>


#ifdef _MSC_VER
#define snprintf _snprintf
#endif

namespace exn = entityx;



//
struct UIState {
    bool showMenu;
    int scroll;
    int scrollarea1;
    bool mouseOverMenu;
    bool chooseTest;
};



//
namespace {
    GLFWwindow *mainWindow = NULL;
    UIState ui;

    int32 testIndex = 0;
    int32 testSelection = 0;
    int32 testCount = 0;
    TestEntry *entry;
    Test *test;
    Settings settings;
    bool rightMouseDown;
    b2Vec2 lastp;
}

//
static void sCreateUI() {
    ui.showMenu = true;
    ui.scroll = 0;
    ui.scrollarea1 = 0;
    ui.chooseTest = false;
    ui.mouseOverMenu = false;

    // Init UI
    const char *fontPath = "DroidSans.ttf";

    if (RenderGLInit(fontPath) == false) {
        fprintf(stderr, "Could not init GUI renderer.\n");
        assert(false);
        return;
    }

}


void drawSquare()
{

	int gridN = 10, gridM = 10;

	glColor3f(1.0, 0.0, 0.0);

	for (int j = -gridN; j<=gridN; j++)
	{
		g_debugDraw.DrawSegment(b2Vec2(j, -gridM), b2Vec2(j, gridM), b2Color(1.f, 1.f, 1.f));
	}

	for (int i = -gridM; i<=gridM; i++)
	{
		g_debugDraw.DrawSegment(b2Vec2(-gridN, i), b2Vec2(gridN, i), b2Color(1.f, 1.f, 1.f));
	}


}


void spawnWall(int x, int y)
{
    entityx::Entity w1 = ex.entities.create();
    w1.assign<BaseProperties>("wall",false);
    w1.assign<Position>(x, y);
    w1.assign<Renderable>("W");
}

//
static void sResizeWindow(GLFWwindow *, int width, int height) {
    g_camera.m_width = width;
    g_camera.m_height = height;
}

//
static void sKeyCallback(GLFWwindow *, int key, int scancode, int action, int mods) {
    ImGuiIO &io = ImGui::GetIO();

    if (!io.WantCaptureKeyboard) {
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ENTER:
                    break;

                case GLFW_KEY_ESCAPE:
                    // Quit
                    glfwSetWindowShouldClose(mainWindow, GL_TRUE);
                    break;

                case GLFW_KEY_LEFT:
                    // Pan left
                    if (mods == GLFW_MOD_CONTROL) {
                        b2Vec2 newOrigin(2.0f, 0.0f);
                        test->ShiftOrigin(newOrigin);
                    }
                    else {
                        g_camera.m_center.x -= 0.5f;
                    }
                    break;

                case GLFW_KEY_RIGHT:
                    // Pan right
                    if (mods == GLFW_MOD_CONTROL) {
                        b2Vec2 newOrigin(-2.0f, 0.0f);
                        test->ShiftOrigin(newOrigin);
                    }
                    else {
                        g_camera.m_center.x += 0.5f;
                    }
                    break;

                case GLFW_KEY_DOWN:
                    // Pan down
                    if (mods == GLFW_MOD_CONTROL) {
                        b2Vec2 newOrigin(0.0f, 2.0f);
                        test->ShiftOrigin(newOrigin);
                    }
                    else {
                        g_camera.m_center.y -= 0.5f;
                    }
                    break;

                case GLFW_KEY_UP:
                    // Pan up
                    if (mods == GLFW_MOD_CONTROL) {
                        b2Vec2 newOrigin(0.0f, -2.0f);
                        test->ShiftOrigin(newOrigin);
                    }
                    else {
                        g_camera.m_center.y += 0.5f;
                    }
                    break;

                case GLFW_KEY_HOME:
                    // Reset view
                    g_camera.m_zoom = 1.0f;
                    g_camera.m_center.Set(0.0f, 20.0f);
                    break;

                case GLFW_KEY_Z:
                    // Zoom out
                    g_camera.m_zoom = b2Min(1.1f * g_camera.m_zoom, 20.0f);
                    break;

                case GLFW_KEY_X:
                    // Zoom in
                    g_camera.m_zoom = b2Max(0.9f * g_camera.m_zoom, 0.02f);
                    break;

                case GLFW_KEY_R:
                    // Reset test
                    delete test;
                    test = entry->createFcn();
                    break;


                case GLFW_KEY_SPACE:
                    // Pause
                    settings.pause = !settings.pause;
                    break;

                case GLFW_KEY_LEFT_BRACKET:
                    // Switch to previous test
                    --testSelection;
                    if (testSelection < 0) {
                        testSelection = testCount - 1;
                    }
                    break;

                case GLFW_KEY_RIGHT_BRACKET:
                    // Switch to next test
                    ++testSelection;
                    if (testSelection == testCount) {
                        testSelection = 0;
                    }
                    break;

                case GLFW_KEY_TAB:
                    ui.showMenu = !ui.showMenu;

                default:
                    if (test) {
                        test->Keyboard(key);
                    }
            }

        }
        else if (action == GLFW_RELEASE) {
            test->KeyboardUp(key);
        }
    }


    //if (io.WantCaptureKeyboard)
        if (action == GLFW_PRESS)
            io.KeysDown[key] = true;
        if (action == GLFW_RELEASE)
            io.KeysDown[key] = false;

        (void) mods; // Modifiers are not reliable across systems
        io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
        io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
        io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
        io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
    };

//
static void sMouseButton(GLFWwindow *, int32 button, int32 action, int32 mods) {
    double xd, yd;
    glfwGetCursorPos(mainWindow, &xd, &yd);
    b2Vec2 ps((float32) xd, (float32) yd);

    ImGuiIO &io = ImGui::GetIO();
    if (!io.WantCaptureMouse) {

        // Use the mouse to move things around.
        if (button == GLFW_MOUSE_BUTTON_1) {
            //<##>
            //ps.Set(0, 0);
            b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
            if (action == GLFW_PRESS) {
                if (mods == GLFW_MOD_SHIFT) {
                    test->ShiftMouseDown(pw);
                }
                else {
                    test->MouseDown(pw);

                    int cx = floor(pw.x);
                    int cy = floor(pw.y);

                    currentCell.x = cx;
                    currentCell.y = cy;

                    /*
                    ImGui::OpenPopup("popup from button");
                    if (ImGui::BeginPopup("popup from button")) {
                        ImGui::MenuItem("New");
                        ImGui::EndPopup();
                    }
                     */


                    ex.events.emit<MouseClickEvent>(MouseClickEvent(pw.x, pw.y));


                }
            }

            if (action == GLFW_RELEASE) {
                test->MouseUp(pw);
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_2) {
            if (action == GLFW_PRESS) {
                lastp = g_camera.ConvertScreenToWorld(ps);
                rightMouseDown = true;
                b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);
                test->RightMouseDown(pw);
            }

            if (action == GLFW_RELEASE) {
                rightMouseDown = false;
            }
        }
        else if (button == GLFW_MOUSE_BUTTON_3) {
            b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);

            if (action == GLFW_PRESS) {
                test->MiddleMouseDown(pw);
            }
        }
    }

    }

//
static void sMouseMotion(GLFWwindow *, double xd, double yd) {
    b2Vec2 ps((float) xd, (float) yd);

    b2Vec2 pw = g_camera.ConvertScreenToWorld(ps);

    test->MouseMove(pw);

    if (rightMouseDown) {
        b2Vec2 diff = pw - lastp;
        g_camera.m_center.x -= diff.x;
        g_camera.m_center.y -= diff.y;
        lastp = g_camera.ConvertScreenToWorld(ps);
    }
}

//
static void sScrollCallback(GLFWwindow *, double, double dy) {
    if (ui.mouseOverMenu) {
        ui.scroll = -int(dy);
    }
    else {
        if (dy > 0) {
            g_camera.m_zoom /= 1.1f;
        }
        else {
            g_camera.m_zoom *= 1.1f;
        }
    }
}

//
static void sSimulate() {
    glEnable(GL_DEPTH_TEST);
    test->Step(&settings);

    test->DrawTitle(entry->name);

    glDisable(GL_DEPTH_TEST);
}

//
static void sInterface() {

    bool show_another_window = true;
    int menuWidth = 200;
    ui.mouseOverMenu = false;

    ImGui::SetNextWindowSize(ImVec2(250, 250), ImGuiSetCond_FirstUseEver);
    bool over = ImGui::Begin("Testbed Controls");
    //if (over) ui.mouseOverMenu = true;

    ImGui::Separator();

	ImGui::Text("Camera: %g,%g", g_camera.m_center.x, g_camera.m_center.y);
    ImGui::Text("Current cell: %d,%d", currentCell.x, currentCell.y);
    static char str1[50];
    ImGui::InputText("test",str1,IM_ARRAYSIZE(str1));



    ex.entities.each<Position, BaseProperties>([](exn::Entity entity, Position &position, BaseProperties &baseproperties) {
        if ((position.x==currentCell.x) && (position.y==currentCell.y))
        {
            //ImGui::Text(baseproperties.name.c_str());
        }
    });

    ex.entities.each<Position, Renderable>([](exn::Entity entity, Position &position ,Renderable &renderable) {
        char buffer[1];
        snprintf(buffer, 4, (char*)(&renderable.glyph));
        //snprintf(buffer, 6, "lalala");
        b2Vec2 p1 = g_camera.ConvertWorldToScreen(b2Vec2(position.x,position.y));
        AddGfxCmdText(p1.x,g_camera.m_height-p1.y,TEXT_ALIGN_LEFT, buffer, WHITE);
    });

    AppLog::instance()->Draw("simlog");


    ImGui::End();

}


//
int main(int argc, char **argv) {
#if defined(_WIN32)
    // Enable memory-leak reports
    _CrtSetDbgFlag(_CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG));
#endif

    g_camera.m_width = 1024;
    g_camera.m_height = 1024;

    if (glfwInit() == 0) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    char title[64];
    sprintf(title, "simLogistics");


    mainWindow = glfwCreateWindow(g_camera.m_width, g_camera.m_height, title, NULL, NULL);

    //gl3wInit();

    // Setup ImGui binding

    ImGui_ImplGlfwGL3_Init(mainWindow, true);

    if (mainWindow == NULL) {
        fprintf(stderr, "Failed to open GLFW mainWindow.\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(mainWindow);
    printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));


    glfwSetWindowSizeCallback(mainWindow, sResizeWindow);
    glfwSetKeyCallback(mainWindow, sKeyCallback);
    glfwSetCharCallback(mainWindow, ImGui_ImplGlfwGL3_CharCallback);
    //glfwSetInputMode(mainWindow, GLFW_STICKY_KEYS, 1);
    glfwSetMouseButtonCallback(mainWindow, sMouseButton);
    glfwSetCursorPosCallback(mainWindow, sMouseMotion);
    glfwSetScrollCallback(mainWindow, sScrollCallback);

#if defined(__APPLE__) == FALSE
    //glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }
#endif

    g_debugDraw.Create();

    sCreateUI();


    entry = g_testEntries;
    test = entry->createFcn();

    // Control the frame rate. One draw per monitor refresh.
    glfwSwapInterval(1);

    double time1 = glfwGetTime();
    double frameTime = 0.0;

    glClearColor(0.3f, 0.3f, 0.3f, 1.f);



    ex.systems.add<ClickResponseSystem>();
    ex.systems.add<SerializationSystem>();
    ex.systems.add<ActionSystem>();

    //ex.systems.add<entityx::deps::Dependency<BaseProperties, Position, Renderable>>();

    ex.systems.configure();


	entityx::Entity entity = ex.entities.create();
    entity.assign<BaseProperties>("food",true);
    entity.assign<Position>(0, -1);
	entity.assign<Edible>("food");
    entity.assign<Renderable>("F");


    entityx::Entity door = ex.entities.create();
    door.assign<Position>(0, 0);
    door.assign<BaseProperties>("door",false);
    door.assign<Renderable>("C");


    spawnWall(1,0);
    spawnWall(1,-1);
    spawnWall(1,-2);
    spawnWall(0,-2);
    spawnWall(-1,-2);
    spawnWall(-1,-1);
    spawnWall(-1,0);
    //w1.component<Position>().get()->x=1;




    ex.events.emit<MoveEvent>(entity,Position(1,1),Position(0,0));

    //ex.events.emit<OpenEvent>(door,door);


    entityx::Entity agent = ex.entities.create();
    agent.assign<BaseProperties>("agent",true);
    agent.assign<Position>(0, 5);
    agent.assign<Renderable>("A");
    agent.assign<Agent>();

    MoveEvent* m1 = new MoveEvent(agent,Position(5,0),Position(5,1));
    agent.component<Agent>().get()->plan.push_back(new MoveEvent(agent,Position(5,0),Position(5,1)));

    //ex.events.emit<>(m1);

    static MoveEventHandler mvh;
    Channel::add<MoveEvent*>(&mvh);

    std::vector<GameEvent*> v0;
    v0.push_back(m1);

    Channel::broadcast(v0[0]);
/*
    static event_dispatcher events;

    events.listen("move",
                  [] (MoveEvent moveevent)
                  {
                      if (isPassable(moveevent.to.x,moveevent.to.y))
                      {
                          entityx::Entity a1 = moveevent.actor;
                          a1.remove<Position>();
                          a1.assign_from_copy<Position>(moveevent.to);
                          AppLog::instance()->AddLog("Moved from (%d,%d) to (%d,%d) \n",moveevent.from.x,moveevent.from.y,moveevent.to.x,moveevent.to.y);
                      } else
                          //effects
                      {
                          AppLog::instance()->AddLog("Move to (%d,%d) impossible \n",moveevent.to.x,moveevent.to.y);
                      }

                  });

*/
    //events.fire("move", agent.component<Agent>().get()->plan[0]);

    std::vector<GameEvent*> v1;
    v1.push_back(new MoveEvent(agent,Position(5,0),Position(5,1)));
    v1[0]->execute();

    //agent.component<Agent>().get()->plan[0]->execute(*m1);
    //m1->execute(*m1);




    while (!glfwWindowShouldClose(mainWindow)) {
        glfwGetWindowSize(mainWindow, &g_camera.m_width, &g_camera.m_height);
        glViewport(0, 0, g_camera.m_width, g_camera.m_height);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();



        unsigned char mousebutton = 0;
        int mscroll = ui.scroll;
        ui.scroll = 0;

        double xd, yd;
        glfwGetCursorPos(mainWindow, &xd, &yd);
        int mousex = int(xd);
        int mousey = int(yd);

        mousey = g_camera.m_height - mousey;
        int leftButton = glfwGetMouseButton(mainWindow, GLFW_MOUSE_BUTTON_LEFT);




        ex.systems.update<ClickResponseSystem>(1);
        ex.systems.update<SerializationSystem>(1);
        ex.systems.update<ActionSystem>(1);


        // 1. Show a simple window
        // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"

		drawSquare();

        sInterface();

        // Measure speed
        double time2 = glfwGetTime();
        double alpha = 0.9f;
        frameTime = alpha * frameTime + (1.0 - alpha) * (time2 - time1);
        time1 = time2;

        char buffer[32];
        snprintf(buffer, 32, "%.1f ms", 1000.0 * frameTime);
        AddGfxCmdText(5, 5, TEXT_ALIGN_LEFT, buffer, WHITE);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        RenderGLFlush(g_camera.m_width, g_camera.m_height);
        g_debugDraw.Flush();

        ImGui::Render();
        glfwSwapBuffers(mainWindow);

        //glfwPollEvents();

    }

    g_debugDraw.Destroy();
    RenderGLDestroy();
    glfwTerminate();

    return 0;
}
