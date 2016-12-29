//
// Created by aash29 on 20.04.16.
//



#ifndef MENU_GRAPH_H
#define MENU_GRAPH_H

#include <math.h>
#include "imgui.h"

#define IMGUI_DEFINE_MATH_OPERATORS

#include "imgui_internal.h"

namespace ImGui {

    void GraphTestWindow(float *values, int values_count);

    int Curve(const char *label, const ImVec2 &size, int maxpoints, ImVec2 *points);

    float CurveValue(float p, int maxpoints, const ImVec2 *points);

    void DrawMagnetScheme();

}

#endif //MENU_GRAPH_H
