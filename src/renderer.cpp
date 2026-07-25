#include "objects.h"
#include <renderer.h>
#include <raylib.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <cmath>
#include <cctype>

namespace {

std::string trim(const std::string& value) {
    const size_t first = value.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return "";
    const size_t last = value.find_last_not_of(" \t\r\n");
    return value.substr(first, last - first + 1);
}

std::string lowercase(const std::string& value) {
    std::string result = value;
    for (char& character : result) {
        character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
    }
    return result;
}

std::string lettersAndDigits(const std::string& value) {
    std::string result;
    for (unsigned char character : value) {
        if (std::isalnum(character)) result += static_cast<char>(character);
    }
    return result;
}

struct ProfessorName {
    std::string lastName;
    std::string givenNames;
};

ProfessorName splitProfessorName(const std::string& fullName) {
    const std::string name = trim(fullName);
    const size_t comma = name.find(',');
    if (comma != std::string::npos) {
        return {trim(name.substr(0, comma)), trim(name.substr(comma + 1))};
    }

    const size_t lastSpace = name.find_last_of(" \t");
    if (lastSpace == std::string::npos) return {name, ""};
    return {trim(name.substr(lastSpace + 1)), trim(name.substr(0, lastSpace))};
}

std::unordered_map<std::string, std::string> professorLabels(
    const std::vector<schedule>& schedules) {
    std::unordered_map<std::string, ProfessorName> names;
    std::unordered_map<std::string, std::vector<std::string>> namesByLastName;

    for (const schedule& schedule : schedules) {
        if (schedule.subject_code == "GAP" || schedule.professor.empty() ||
            names.find(schedule.professor) != names.end()) {
            continue;
        }

        ProfessorName name = splitProfessorName(schedule.professor);
        const std::string lastNameKey = lowercase(name.lastName);
        names.emplace(schedule.professor, name);
        namesByLastName[lastNameKey].push_back(schedule.professor);
    }

    std::unordered_map<std::string, std::string> labels;
    for (const auto& [lastName, professors] : namesByLastName) {
        if (professors.size() == 1) {
            labels[professors.front()] = names.at(professors.front()).lastName;
            continue;
        }

        // Professors sharing a surname get the shortest leading portion of
        // their given names that makes their schedule labels distinct.
        for (const std::string& professor : professors) {
            const ProfessorName& name = names.at(professor);
            const std::string givenDisplay = lettersAndDigits(name.givenNames);
            const std::string givenKey = lowercase(givenDisplay);
            std::string distinguishingPrefix;

            for (size_t length = 1; length <= givenKey.size(); ++length) {
                const std::string candidate = givenKey.substr(0, length);
                bool unique = true;
                for (const std::string& otherProfessor : professors) {
                    if (otherProfessor == professor) continue;
                    const std::string otherGiven = lowercase(
                        lettersAndDigits(names.at(otherProfessor).givenNames));
                    if (otherGiven.substr(0, length) == candidate) {
                        unique = false;
                        break;
                    }
                }
                if (unique) {
                    distinguishingPrefix = givenDisplay.substr(0, length);
                    break;
                }
            }

            if (distinguishingPrefix.empty()) {
                // Identical or incomplete names cannot be shortened without
                // losing the only available distinction.
                labels[professor] = professor;
            } else {
                labels[professor] = distinguishingPrefix + ". " + name.lastName;
            }
        }
    }

    return labels;
}

} // namespace


Renderer::Renderer() {
    // Day mappings
    umap["M"]   = {0}; umap["T"] = {1}; umap["W"] = {2};
    umap["TH"]  = {3}; umap["FRI"] = {4}; umap["SAT"] = {5}; umap["SUN"] = {6};
    umap["MW"]  = {0,2}; umap["TTH"] = {1,3}; umap["FSA"] = {4,5};

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

    size_t subjHash = std::hash<std::string>{}(subj+":33");
    Color c = {
        static_cast<unsigned char>((subjHash & 0xFF0000) >> 16),
        static_cast<unsigned char>((subjHash & 0x00FF00) >> 8),
        static_cast<unsigned char>(subjHash & 0x0000FF),
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
            g.startRow = rows - 2;
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
            DrawRectangle(xOffset + i*gapX + 1, yOffset + j*gapY + 1, gapX, gapY, DARKGRAY);
        }
    }

    // Grid lines
    for(int i = 0; i <= cols; ++i) DrawRectangle(xOffset + i*gapX, yOffset, 1, tableHeight, WHITE);
    for(int j = 0; j <= rows; ++j) {
		Color clr = ColorBrightness(GRAY, 0.25f);
		if(j == 9) clr = WHITE;
		DrawRectangle(xOffset, yOffset + j*gapY, tableWidth, (j % 2 ? 2 : 1), clr);
	}

    // Day labels
    for(int i = 0; i < cols; ++i){
        DrawTextEx(boldFont, days[i].c_str(), {xOffset + i*gapX + gapX/2 - 20, yOffset - 25}, 25, 5, WHITE);
    }

    const auto professorLabelsToRender = professorLabels(scheduleToRender);

    // Schedules
    for(auto &s : scheduleToRender){
        if(s.subject_code == "GAP") continue;
        Color subjColor = ColorBrightness(ColorContrast(getSubjectColor(s.subject_code), -0.1f), 0.2f);

        for(int dayIdx : umap[s.days]){
            float yPos = yOffset + ((s.start_to_min() - 450)/30.0f) * gapY;
            float height = ((s.end_to_min() - s.start_to_min())/30.0f) * gapY;
            float x = xOffset + dayIdx*gapX;
			Rectangle rec = {x+3, yPos+3, gapX-6.0f, height-6.0f};
			float roundness = 0.3f;
			DrawRectangleRounded(rec, roundness, 10, subjColor);
			DrawRectangleRoundedLinesEx(rec, roundness, 10, 2, ColorBrightness(DARKGRAY, -0.5f));
            DrawTextEx(boldFont, s.subject_code.c_str(), {x + 5, yPos + 7}, 20, 1, BLACK);
            DrawTextEx(boldFont, s.section.c_str(),      {x + 5, yPos + 20}, 20, 1, BLACK);
            const auto professorLabel = professorLabelsToRender.find(s.professor);
            const std::string& professor = professorLabel == professorLabelsToRender.end()
                ? s.professor
                : professorLabel->second;
			Vector2 profDims = MeasureTextEx(boldFont, professor.c_str(), 20, 1);
            DrawTextEx(boldFont, professor.c_str(),      {x + rec.width - profDims.x - 5, yPos + height - 20}, 20, 1, BLACK);
        }
    }

    // Gaps
    for(auto &g : gapVisuals){
        for(int dayIdx : umap[g.day]){
            float x = xOffset + dayIdx*gapX;
            float pixelStart = yOffset + g.startRow * gapY;
            float pixelEnd   = yOffset + g.endRow * gapY;
            float height = pixelEnd - pixelStart;

            DrawRectangle(x, pixelStart, gapX, height, Fade(RED, 0.2f));
            DrawRectangleLines(x, pixelStart, gapX, height, Fade(RED, 1.0f));
            DrawRectangle(x, pixelStart - 3, gapX, 6,       Fade(RED, 0.5f));
            DrawRectangle(x, pixelEnd - 3, gapX, 6,         Fade(RED, 0.5f));
        }
    }

    // Add Gap buttons
    for(int i = 0; i < cols; ++i){
        float x = xOffset + i*gapX;
        float y = yOffset + tableHeight + 10;
        DrawRectangle(x, y, gapX, 20, DARKGRAY);
		//DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
        DrawTextEx(boldFont, "Add Gap", {x + 5, y + 2}, 14, 1, WHITE);
    }

    // Clear All button
    float clearX = xOffset + tableWidth - 100;
    float clearY = yOffset + tableHeight + 10;
    DrawRectangle(clearX, clearY, 90, 20, DARKGRAY);
	//DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
    DrawTextEx(boldFont, "Clear All", {clearX + 5, clearY + 2}, 14, 5, WHITE);

    // No schedules message
    if(scheduleToRender.empty()){
        std::string msg = "No Valid Schedules";
        int fontSize = 70;
        int textWidth = MeasureText(msg.c_str(), fontSize);
        float x = xOffset + tableWidth/2.0f - textWidth/2.0f;
        float y = yOffset + tableHeight/2.0f - fontSize/2.0f;
        DrawRectangleRec({x - 20, y - 10, static_cast<float>(textWidth + 40), static_cast<float>(fontSize + 20)}, Fade(BLACK, 0.5f));
		//DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
        DrawTextEx(boldFont, msg.c_str(), {static_cast<float>(x), static_cast<float>(y)}, fontSize, 1, RED);
    }

	// TIME LABELS (left side)
	for(int r = 0; r < rows+1; ++r){
		int minute = 450 + r * 30; // Start time = 7:30 AM
		int hour = minute / 60;
		int min  = minute % 60;

		char buf[16];
		sprintf(buf, "%d:%02d-", hour, min);

		float yPos = yOffset + r * gapY + gapY/2 - 20;
		//DrawTextEx(Font font, const char *text, Vector2 position, float fontSize, float spacing, Color tint); // Draw text using font and additional parameters
		DrawTextEx(boldFont, buf, {xOffset - 55, yPos}, 20, 1, WHITE);
	}
}
