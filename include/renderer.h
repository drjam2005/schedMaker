#pragma once
#ifndef RENDERER_H
#define RENDERER_H
#include <raylib.h>
#include <scheduler.h>

#include <unordered_map>
#include <objects.h>

struct GapVisual {
    std::string day;

    float startRow;
    float endRow;

    bool draggingStart = false;
    bool draggingEnd = false;
    bool draggingBody = false;
    float dragOffset = 0;
};



class Renderer {
public:
    Renderer();

    void Update();
    void Render();
	void SyncGapsToScheduler(Scheduler& scheduler);


    std::vector<schedule> scheduleToRender;

    // --- Gap support ---
    std::vector<GapVisual> gapVisuals;

private:
    std::unordered_map<std::string,std::vector<int>> umap;
    std::unordered_map<std::string, Color> subjectColors;
	float xOffset, yOffset;
    float tableWidth, tableHeight;

    // --- Grid variables ---
    float topMargin;
    int rows, cols;
    float scaleX, scaleY;
    float gapX, gapY;
    std::vector<std::string> days;

    Color getSubjectColor(const std::string& subj);
};



#endif
