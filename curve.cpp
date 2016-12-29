#include "graph.h"

#include <cmath>
#include <algorithm>

namespace ImGui {
    void GraphTestWindow(float *values, int values_count) {
        static bool animate = true;
        ImGui::Checkbox("Animate", &animate);


        // Create a dummy array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float and the sizeof() of your structure in the Stride parameter.
        /*
        static float values[90] = {0};
        static int values_offset = 0;
        if (animate) {
            static float refresh_time = ImGui::GetTime(); // Create dummy data at fixed 60 hz rate for the demo
            for (; ImGui::GetTime() > refresh_time + 1.0f / 60.0f; refresh_time += 1.0f / 60.0f) {
                static float phase = 0.0f;
                values[values_offset] = cosf(phase);
                values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
                phase += 0.10f * values_offset;
            }
        }

         */
        ImGui::PlotLines("Lines", values, values_count, 0, "buffer", *(std::min_element(values, values + values_count)),
                         *(std::max_element(values, values + values_count)), ImVec2(300, 200));

        ImGui::Separator();


        // Tip: If you do a lot of custom rendering, you probably want to use your own geometrical types and benefit of overloaded operators, etc.
        // Define IM_VEC2_CLASS_EXTRA in imconfig.h to create implicit conversions between your types and ImVec2/ImVec4.
        // ImGui defines overloaded operators but they are internal to imgui.cpp and not exposed outside (to avoid messing with your types)
        // In this example we are not using the maths operators!

/*
        ImGui::DragFloat("Size", &sz, 0.2f, 2.0f, 72.0f, "%.0f");
            ImGui::ColorEdit3("Color", &col.x);
            {
                const ImVec2 p = ImGui::GetCursorScreenPos();
                const ImU32 col32 = ImColor(col);
                float x = p.x + 4.0f, y = p.y + 4.0f, spacing = 8.0f;
                for (int n = 0; n < 2; n++)
                {
                    float thickness = (n == 0) ? 1.0f : 4.0f;
                    draw_list->AddCircle(ImVec2(x+sz*0.5f, y+sz*0.5f), sz*0.5f, col32, 20, thickness); x += sz+spacing;
                    draw_list->AddRect(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 0.0f, ~0, thickness); x += sz+spacing;
                    draw_list->AddRect(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 10.0f, ~0, thickness); x += sz+spacing;
                    draw_list->AddTriangle(ImVec2(x+sz*0.5f, y), ImVec2(x+sz,y+sz-0.5f), ImVec2(x,y+sz-0.5f), col32, thickness); x += sz+spacing;
                    draw_list->AddLine(ImVec2(x, y), ImVec2(x+sz, y   ), col32, thickness); x += sz+spacing;
                    draw_list->AddLine(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, thickness); x += sz+spacing;
                    draw_list->AddLine(ImVec2(x, y), ImVec2(x,    y+sz), col32, thickness); x += spacing;
                    draw_list->AddBezierCurve(ImVec2(x, y), ImVec2(x+sz*1.3f,y+sz*0.3f), ImVec2(x+sz-sz*1.3f,y+sz-sz*0.3f), ImVec2(x+sz, y+sz), col32, thickness);
                    x = p.x + 4;
                    y += sz+spacing;
                }
                draw_list->AddCircleFilled(ImVec2(x+sz*0.5f, y+sz*0.5f), sz*0.5f, col32, 32); x += sz+spacing;
                draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x+sz, y+sz), col32); x += sz+spacing;
                draw_list->AddRectFilled(ImVec2(x, y), ImVec2(x+sz, y+sz), col32, 10.0f); x += sz+spacing;
                draw_list->AddTriangleFilled(ImVec2(x+sz*0.5f, y), ImVec2(x+sz,y+sz-0.5f), ImVec2(x,y+sz-0.5f), col32); x += sz+spacing;
                draw_list->AddRectFilledMultiColor(ImVec2(x, y), ImVec2(x+sz, y+sz), ImColor(0,0,0), ImColor(255,0,0), ImColor(255,255,0), ImColor(0,255,0));
                ImGui::Dummy(ImVec2((sz+spacing)*8, (sz+spacing)*3));
            }
            ImGui::Separator();
            {
                static ImVector<ImVec2> points;
                static bool adding_line = false;
                ImGui::Text("Canvas example");
                if (ImGui::Button("Clear")) points.clear();
                if (points.Size >= 2) { ImGui::SameLine(); if (ImGui::Button("Undo")) { points.pop_back(); points.pop_back(); } }
                ImGui::Text("Left-click and drag to add lines,\nRight-click to undo");

                // Here we are using InvisibleButton() as a convenience to 1) advance the cursor and 2) allows us to use IsItemHovered()
                // However you can draw directly and poll mouse/keyboard by yourself. You can manipulate the cursor using GetCursorPos() and SetCursorPos().
                // If you only use the ImDrawList API, you can notify the owner window of its extends by using SetCursorPos(max).
                ImVec2 canvas_pos = ImGui::GetCursorScreenPos();            // ImDrawList API uses screen coordinates!
                ImVec2 canvas_size = ImGui::GetContentRegionAvail();        // Resize canvas to what's available
                if (canvas_size.x < 50.0f) canvas_size.x = 50.0f;
                if (canvas_size.y < 50.0f) canvas_size.y = 50.0f;
                draw_list->AddRectFilledMultiColor(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(50,50,50), ImColor(50,50,60), ImColor(60,60,70), ImColor(50,50,60));
                draw_list->AddRect(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), ImColor(255,255,255));

                bool adding_preview = false;
                ImGui::InvisibleButton("canvas", canvas_size);
                if (ImGui::IsItemHovered())
                {
                    ImVec2 mouse_pos_in_canvas = ImVec2(ImGui::GetIO().MousePos.x - canvas_pos.x, ImGui::GetIO().MousePos.y - canvas_pos.y);
                    if (!adding_line && ImGui::IsMouseClicked(0))
                    {
                        points.push_back(mouse_pos_in_canvas);
                        adding_line = true;
                    }
                    if (adding_line)
                    {
                        adding_preview = true;
                        points.push_back(mouse_pos_in_canvas);
                        if (!ImGui::GetIO().MouseDown[0])
                            adding_line = adding_preview = false;
                    }
                    if (ImGui::IsMouseClicked(1) && !points.empty())
                    {
                        adding_line = adding_preview = false;
                        points.pop_back();
                        points.pop_back();
                    }
                }
                draw_list->PushClipRect(ImVec4(canvas_pos.x, canvas_pos.y, canvas_pos.x+canvas_size.x, canvas_pos.y+canvas_size.y));      // clip lines within the canvas (if we resize it, etc.)
                for (int i = 0; i < points.Size - 1; i += 2)
                    draw_list->AddLine(ImVec2(canvas_pos.x + points[i].x, canvas_pos.y + points[i].y), ImVec2(canvas_pos.x + points[i+1].x, canvas_pos.y + points[i+1].y), 0xFF00FFFF, 2.0f);
                draw_list->PopClipRect();
                if (adding_preview)
                    points.pop_back();
            }
            ImGui::End();
            */
    }


    float CurveValue(float p, int maxpoints, const ImVec2 *points) {
        if (maxpoints < 2 || points == 0)
            return 0;
        if (p < 0) return points[0].y;

        int left = 0;
        while (left < maxpoints && points[left].x < p && points[left].x != -1) left++;
        if (left) left--;

        if (left == maxpoints - 1)
            return points[maxpoints - 1].y;

        float d = (p - points[left].x) / (points[left + 1].x - points[left].x);

        return points[left].y + (points[left + 1].y - points[left].y) * d;
    }

    int Curve(const char *label, const ImVec2 &size, int maxpoints, ImVec2 *points) {
        int modified = 0;
        int i;
        if (maxpoints < 2 || points == 0)
            return 0;

        if (points[0].x < 0) {
            points[0].x = 0;
            points[0].y = 0;
            points[1].x = 1;
            points[1].y = 1;
            points[2].x = -1;
        }

        ImGuiWindow *window = GetCurrentWindow();
        ImGuiState &g = *GImGui;
        const ImGuiStyle &style = g.Style;
        const ImGuiID id = window->GetID(label);
        if (window->SkipItems)
            return 0;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        ItemSize(bb);
        if (!ItemAdd(bb, NULL))
            return 0;

        const bool hovered = IsHovered(bb, id);

        int max = 0;
        while (max < maxpoints && points[max].x >= 0) max++;

        int kill = 0;
        do {
            if (kill) {
                modified = 1;
                for (i = kill + 1; i < max; i++) {
                    points[i - 1] = points[i];
                }
                max--;
                points[max].x = -1;
                kill = 0;
            }

            for (i = 1; i < max - 1; i++) {
                if (std::abs(points[i].x - points[i - 1].x) < 1 / 128.f) {
                    kill = i;
                }
            }
        }
        while (kill);


        RenderFrame(bb.Min, bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);

        float ht = bb.Max.y - bb.Min.y;
        float wd = bb.Max.x - bb.Min.x;

        if (hovered) {
            SetHoveredID(id);
            if (g.IO.MouseDown[0]) {
                modified = 1;
                ImVec2 pos = (g.IO.MousePos - bb.Min) / (bb.Max - bb.Min);
                pos.y = 1 - pos.y;

                int left = 0;
                while (left < max && points[left].x < pos.x) left++;
                if (left) left--;

                ImVec2 p = points[left] - pos;
                float p1d = sqrt(p.x * p.x + p.y * p.y);
                p = points[left + 1] - pos;
                float p2d = sqrt(p.x * p.x + p.y * p.y);
                int sel = -1;
                if (p1d < (1 / 16.0)) sel = left;
                if (p2d < (1 / 16.0)) sel = left + 1;

                if (sel != -1) {
                    points[sel] = pos;
                }
                else {
                    if (max < maxpoints) {
                        max++;
                        for (i = max; i > left; i--) {
                            points[i] = points[i - 1];
                        }
                        points[left + 1] = pos;
                    }
                    if (max < maxpoints)
                        points[max].x = -1;
                }


                // snap first/last to min/max
                points[0].x = 0;
                points[max - 1].x = 1;
            }
        }

        // bg grid
        window->DrawList->AddLine(
                ImVec2(bb.Min.x, bb.Min.y + ht / 2),
                ImVec2(bb.Max.x, bb.Min.y + ht / 2),
                GetColorU32(ImGuiCol_TextDisabled), 3);

        window->DrawList->AddLine(
                ImVec2(bb.Min.x, bb.Min.y + ht / 4),
                ImVec2(bb.Max.x, bb.Min.y + ht / 4),
                GetColorU32(ImGuiCol_TextDisabled));

        window->DrawList->AddLine(
                ImVec2(bb.Min.x, bb.Min.y + ht / 4 * 3),
                ImVec2(bb.Max.x, bb.Min.y + ht / 4 * 3),
                GetColorU32(ImGuiCol_TextDisabled));

        for (i = 0; i < 9; i++) {
            window->DrawList->AddLine(
                    ImVec2(bb.Min.x + (wd / 10) * (i + 1), bb.Min.y),
                    ImVec2(bb.Min.x + (wd / 10) * (i + 1), bb.Max.y),
                    GetColorU32(ImGuiCol_TextDisabled));
        }

        // lines
        for (i = 1; i < max; i++) {
            ImVec2 a = points[i - 1];
            ImVec2 b = points[i];
            a.y = 1 - a.y;
            b.y = 1 - b.y;
            a = a * (bb.Max - bb.Min) + bb.Min;
            b = b * (bb.Max - bb.Min) + bb.Min;
            window->DrawList->AddLine(a, b, GetColorU32(ImGuiCol_PlotLines));
        }

        if (hovered) {
            // control points
            for (i = 0; i < max; i++) {
                ImVec2 p = points[i];
                p.y = 1 - p.y;
                p = p * (bb.Max - bb.Min) + bb.Min;
                ImVec2 a = p - ImVec2(2, 2);
                ImVec2 b = p + ImVec2(2, 2);
                window->DrawList->AddRect(a, b, GetColorU32(ImGuiCol_PlotLines));
            }
        }

        RenderTextClipped(ImVec2(bb.Min.x, bb.Min.y + style.FramePadding.y), bb.Max, label, NULL, NULL,
                          ImGuiAlign_Center);
        return modified;
    }


};