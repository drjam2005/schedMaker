#include "objects.h"
#include <renderer.h>
#include <raylib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>

Renderer::Renderer() {
    // Day mappings
    umap["M"]   = {0}; umap["T"] = {1}; umap["W"] = {2};
    umap["TH"]  = {3}; umap["FRI"] = {4}; umap["SAT"] = {5}; umap["SUN"] = {6};
    umap["MW"]  = {0,2}; umap["TTH"] = {1,3};

    days = {"M","T","W","TH","FRI","SAT","SUN"};

    // Table configuration
    topMargin = 30.0f;
    rows = 27;
    cols = 7;
    scaleX = 0.8f;
    scaleY = 0.8f;

    // Initialize member variables
    gapX = 0; gapY = 0;
    tableWidth = tableHeight = 0;
    xOffset = yOffset = 0;
}

// --- Sync gaps to scheduler ---
void Renderer::SyncGapsToScheduler(Scheduler& scheduler) {
    scheduler.userGaps.clear();
    for(auto &g : gapVisuals){
        // Convert row units to minutes
        int start_min = round(g.startRow) * 30 + 450;
        int end_min   = round(g.endRow) * 30 + 450;
        if(end_min <= start_min) end_min = start_min + 30;

        scheduler.addGap(g.day, start_min, end_min);
    }
}


// --- Subject color ---
Color Renderer::getSubjectColor(const std::string& subj) {
    if(subjectColors.find(subj) != subjectColors.end()) return subjectColors[subj];

    size_t hash = std::hash<std::string>{}(subj+":3");
    Color c = {
        static_cast<unsigned char>((hash & 0xFF0000) >> 16),
        static_cast<unsigned char>((hash & 0x00FF00) >> 8),
        static_cast<unsigned char>(hash & 0x0000FF),
        255
    };
    subjectColors[subj] = c;
    return c;
}

// --- Update mouse interactions ---
void Renderer::Update() {
    // --- Recalculate table metrics ---
    gapX = (GetScreenWidth() * scaleX) / cols;
    gapY = (GetScreenHeight() * scaleY) / rows;
    tableWidth  = gapX * cols;
    tableHeight = gapY * rows;
    xOffset = (GetScreenWidth() - tableWidth) / 2.0f;
    yOffset = (GetScreenHeight() - tableHeight) / 2.0f;

    Vector2 mouse = GetMousePosition();
    bool mouseReleased = IsMouseButtonReleased(MOUSE_LEFT_BUTTON);

    // --- Dragging gaps ---
    for(auto &g : gapVisuals){
        for(int dayIdx : umap[g.day]){
            float x = xOffset + dayIdx * gapX;
            float pixelStart = yOffset + g.startRow * gapY;
            float pixelEnd   = yOffset + g.endRow * gapY;

            // Check mouse press on start/end/body
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                if(mouse.x >= x && mouse.x <= x + gapX){
                    if(mouse.y >= pixelStart - 5 && mouse.y <= pixelStart + 5)
                        g.draggingStart = true;
                    else if(mouse.y >= pixelEnd - 5 && mouse.y <= pixelEnd + 5)
                        g.draggingEnd = true;
                    else if(mouse.y >= pixelStart && mouse.y <= pixelEnd){
                        g.draggingBody = true;
                        g.dragOffset = mouse.y - pixelStart;
                    }
                }
            }
        }

        // Move start/end/body using row units
        if(g.draggingStart){
            g.startRow = (mouse.y - yOffset) / gapY;
            if(g.startRow < 0) g.startRow = 0;
            if(g.startRow > g.endRow - 1) g.startRow = g.endRow - 1;
        }
        if(g.draggingEnd){
            g.endRow = (mouse.y - yOffset) / gapY;
            if(g.endRow > rows) g.endRow = rows;
            if(g.endRow < g.startRow + 1) g.endRow = g.startRow + 1;
        }
        if(g.draggingBody){
            float newStart = (mouse.y - yOffset - g.dragOffset) / gapY;
            float newEnd = newStart + (g.endRow - g.startRow);
            if(newStart < 0) { newStart = 0; newEnd = newStart + (g.endRow - g.startRow); }
            if(newEnd > rows) { newEnd = rows; newStart = newEnd - (g.endRow - g.startRow); }
            g.startRow = newStart;
            g.endRow = newEnd;
        }

        // Release mouse
        if(mouseReleased){
            g.draggingStart = g.draggingEnd = g.draggingBody = false;

            // Snap to nearest row
            g.startRow = round(g.startRow);
            g.endRow   = round(g.endRow);
        }
    }

    // --- Add Gap buttons ---
    for(int i = 0; i < cols; ++i){
        float x = xOffset + i * gapX;
        float y = yOffset + tableHeight + 10;
        float w = gapX, h = 20;

        if(CheckCollisionPointRec(mouse, {x, y, w, h}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
            GapVisual g;
            g.day = days[i];
            g.startRow = rows - 4;
            g.endRow = rows;
            gapVisuals.push_back(g);
        }
    }

    // --- Clear All button ---
    float clearX = xOffset + tableWidth - 100;
    float clearY = yOffset + tableHeight + 10;
    float clearW = 90, clearH = 20;

    if(CheckCollisionPointRec(mouse, {clearX, clearY, clearW, clearH}) && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
        gapVisuals.clear();
    }
}

// --- Render table, schedules, gaps, and buttons ---
void Renderer::Render() {
    // Recalculate table metrics
    gapX = (GetScreenWidth() * scaleX) / cols;
    gapY = (GetScreenHeight() * scaleY) / rows;
    tableWidth  = gapX * cols;
    tableHeight = gapY * rows;
    xOffset = (GetScreenWidth() - tableWidth) / 2.0f;
    yOffset = (GetScreenHeight() - tableHeight) / 2.0f;

    // Grid
    for(int i = 0; i < cols; ++i){
        for(int j = 0; j < rows; ++j){
            DrawRectangle(xOffset + i*gapX + 1, yOffset + j*gapY + 1, gapX - 2, gapY - 2, DARKGRAY);
        }
    }

    // Grid lines
    for(int i = 0; i <= cols; ++i) DrawRectangle(xOffset + i*gapX, yOffset, 2, tableHeight, WHITE);
    for(int j = 0; j <= rows; ++j) DrawRectangle(xOffset, yOffset + j*gapY, tableWidth, 2, WHITE);

    // Day labels
    for(int i = 0; i < cols; ++i){
        DrawText(days[i].c_str(), xOffset + i*gapX + gapX/2 - 20, yOffset - 25, 20, WHITE);
    }

    // Schedules
    for(auto &s : scheduleToRender){
        if(s.subject_code == "GAP") continue;
        Color subjColor = getSubjectColor(s.subject_code);

        for(int dayIdx : umap[s.days]){
            float yPos = yOffset + ((s.start_to_min() - 450)/30.0f) * gapY;
            float height = ((s.end_to_min() - s.start_to_min())/30.0f) * gapY;
            float x = xOffset + dayIdx*gapX;
            DrawRectangle(x, yPos, gapX, height, subjColor);
            DrawText(s.subject_code.c_str(), x + 5, yPos + 5, 12, BLACK);
            DrawText(s.section.c_str(), x + 5, yPos + 17, 12, BLACK);
        }
    }

    // Gaps
    for(auto &g : gapVisuals){
        for(int dayIdx : umap[g.day]){
            float x = xOffset + dayIdx*gapX;
            float pixelStart = yOffset + g.startRow * gapY;
            float pixelEnd   = yOffset + g.endRow * gapY;
            float height = pixelEnd - pixelStart;

            DrawRectangle(x, pixelStart, gapX, height, Fade(RED, 0.5f));
            DrawRectangleLines(x, pixelStart, gapX, height, RED);
            DrawRectangle(x, pixelStart - 3, gapX, 6, RED);
            DrawRectangle(x, pixelEnd - 3, gapX, 6, RED);
        }
    }

    // Add Gap buttons
    for(int i = 0; i < cols; ++i){
        float x = xOffset + i*gapX;
        float y = yOffset + tableHeight + 10;
        DrawRectangle(x, y, gapX, 20, DARKGRAY);
        DrawText("Add Gap", x + 5, y + 2, 14, WHITE);
    }

    // Clear All button
    float clearX = xOffset + tableWidth - 100;
    float clearY = yOffset + tableHeight + 10;
    DrawRectangle(clearX, clearY, 90, 20, DARKGRAY);
    DrawText("Clear All", clearX + 5, clearY + 2, 14, WHITE);

    // No schedules message
    if(scheduleToRender.empty()){
        std::string msg = "No Valid Schedules";
        int fontSize = 70;
        int textWidth = MeasureText(msg.c_str(), fontSize);
        float x = xOffset + tableWidth/2.0f - textWidth/2.0f;
        float y = yOffset + tableHeight/2.0f - fontSize/2.0f;
        DrawRectangleRec({x - 20, y - 10, static_cast<float>(textWidth + 40), static_cast<float>(fontSize + 20)}, Fade(BLACK, 0.5f));
        DrawText(msg.c_str(), static_cast<int>(x), static_cast<int>(y), fontSize, RED);
    }
}

