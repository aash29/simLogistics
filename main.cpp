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

#include "gamelogic.h"

#include <vector>
#include <map>
#include <string>

#include "path_ex.hpp"


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

auto inGameNames = std::map < std::string, entityx::Entity>();

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

entityx::Entity spawnFood(int x, int y)
{
    entityx::Entity food = ex.entities.create();
    food.assign<BaseProperties>("food",true);
    food.assign<Position>(x, y);
    food.assign<Edible>("food");
    food.assign<Renderable>("F");

    return food;
}


Position transformPathToActionSeq(std::shared_ptr<navigation_path<location_t>> path, entityx::Entity agent) {
    Position cp(0,0);
    for (auto p1 = path->steps.begin(); (std::max(abs(p1->x-path->destination.x),abs(p1->y-path->destination.y))>=1.1f); p1 = p1 + 1) {
        AppLog::instance()->AddLog("%d,%d \n", p1->x, p1->y);
        if (not (*p1==*(p1+1)))
        {
            agent.component<Agent>().get()->plan.push_back(
                new MoveAction(agent, Position(p1->x, p1->y), Position((p1 + 1)->x, (p1 + 1)->y)));
        };
        cp.x=(p1+1)->x;
        cp.y=(p1+1)->y;

        //p0=p1;
    }
    return cp;
}

void goToEntity(entityx::Entity agent, entityx::Entity target)
{
    Position* p1 = &(agent.component<Agent>().get()->currentDestination);
    Position* p2 = target.component<Position>().get();
    location_t st_pos {p1->x,p1->y};
    location_t end_pos {p2->x,p2->y};
    path = find_path<location_t, navigator>(st_pos, end_pos);

    if (path->success) {
        AppLog::instance()->AddLog("path found \n");


        path->steps.push_front(st_pos);
        if (path->steps.size()>0)
        {
            agent.component<Agent>().get()->currentDestination=transformPathToActionSeq(path,agent);

            //=Position(p2->x,p2->y);
        }
    }
}


std::vector<std::string> findPlan()
{
    char *calledPython="/home/aash29/cpp/fast-downward/fast-downward.py";  // it can also be resolved using your PATH environment variable
    //char *pythonArgs[]={calledPython,"--build=release64",  "./fps/logisticsDomain.pddl", "./fps/logisticsProblem.pddl", "--search \"astar(lmcut())\"",NULL};

    char execstr[80] ;

    char * line = NULL;
    size_t len = 0;

    strcpy(execstr, calledPython);
    strcat(execstr, " --build=release64");
    strcat(execstr, " ./logisticsDomain.pddl");
    strcat(execstr, " ./logisticsProblem.pddl");
    strcat(execstr, " --search \"astar(cg())\"");

    char key[] = "Solution found!\n";
    char keyEnd[] = "Plan length";

    std::vector<std::string> result = std::vector<std::string>();
    char currentDir[50];
    getcwd(currentDir,50);

    AppLog::instance()->AddLog("%s",currentDir);

    FILE* in = popen(execstr, "r");
    bool beginPlan=false;
    while (getline(&line, &len, in)!= EOF) {
        AppLog::instance()->AddLog(line);
        if (strcmp (key,line) == 0)
        {
            beginPlan=true;
        }

        if (strstr(line,keyEnd)!=NULL)
        {
            beginPlan=false;
        }
        if (beginPlan)
        {
            result.push_back(std::string(line));
            AppLog::instance()->AddLog(line);
        }
    }
    return result;
}

    vector<std::string> split(const char *str, char c = ' ')
    {
        vector<std::string> result;

        do
        {
            const char *begin = str;

            while(*str != c && *str)
                str++;

            result.push_back(std::string(begin, str));
        } while (0 != *str++);

        return result;
    }


void parsePlan(std::vector<std::string> planText, entityx::Entity agent)
{
    agent.component<Agent>().get()->currentDestination=*agent.component<Position>().get();

    for (int i=2;i<planText.size();i++)
    {
        std::vector<std::string> v1 =split(planText[i].c_str());
        if ("move-to-point" == v1[0])
        {
            std::string target = v1[2];
            goToEntity(agent,inGameNames[target]);
            //AppLog::instance()->AddLog("moving");
        }
        if ("place-in-inventory" == v1[0])
        {
            std::string target = v1[2];
            agent.component<Agent>().get()->plan.push_back(new TakeAction(agent,inGameNames[target]));
            //AppLog::instance()->AddLog("moving");
        }

        if ("make-accessible" == v1[0])
        {
            std::string target = v1[3];
            agent.component<Agent>().get()->plan.push_back(new OpenAction(agent,inGameNames[target]));
        }
    }
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
    ex.systems.add<RenderSystem>();

    ex.systems.configure();


    inGameNames["food1"]=spawnFood(0,-1);

    inGameNames["food2"]=spawnFood(6,6);
    //spawnFood(-7,-7);


    entityx::Entity door = ex.entities.create();
    door.assign<Position>(0, 0);
    door.assign<BaseProperties>("door",false);
    door.assign<Renderable>("C");

    inGameNames["door1-0"]=door;


    entityx::Entity key10 = ex.entities.create();
    key10.assign<Position>(-5, 5);
    key10.assign<BaseProperties>("key",true);
    key10.assign<Renderable>("K");

    inGameNames["key1-0"]=key10;

    spawnWall(1,0);
    spawnWall(1,-1);
    spawnWall(1,-2);
    spawnWall(0,-2);
    spawnWall(-1,-2);
    spawnWall(-1,-1);
    spawnWall(-1,0);

    entityx::Entity agent = ex.entities.create();
    agent.assign<BaseProperties>("agent",true);
    agent.assign<Position>(0, 5);
    agent.assign<Renderable>("A");
    agent.assign<Agent>();
    agent.assign<Inventory>();


    agent.component<Agent>().get()->planText=findPlan();

    parsePlan(agent.component<Agent>().get()->planText, agent);






    //agent.component<Agent>().get()->plan.push_back(new OpenAction(agent,door));
    //agent.component<Agent>().get()->plan.push_back(new MoveAction(agent,Position(0,1),Position(0,0)));
    //agent.component<Agent>().get()->plan.push_back(new MoveAction(agent,Position(0,0),Position(0,1)));




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
        ex.systems.update<RenderSystem>(1);


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
