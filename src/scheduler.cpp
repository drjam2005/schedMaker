#include "scheduler.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <ctime>

void parseTime(const std::string& timeStr, int &hour, int &min) {
    size_t pos = timeStr.find(':');
    if (pos == std::string::npos) { hour = min = 0; return; }
    try {
        hour = std::stoi(timeStr.substr(0, pos));
        min = std::stoi(timeStr.substr(pos + 1));
    } catch(...) { hour = min = 0; }
}

Scheduler::Scheduler(std::string filePath) {
    this->filePath = filePath;
    dayNames = {"M","T","W","TH","FRI","SAT","SUN"};
    dayMap = {
        {"M",{0}}, {"T",{1}}, {"W",{2}}, {"TH",{3}}, {"FRI",{4}}, {"SAT",{5}}, {"SUN",{6}},
        {"MW",{0,2}}, {"TTH",{1,3}}
    };
}

void Scheduler::addSubject(subject subj){
    subjects.push_back(subj);
}

void Scheduler::addGap(const std::string& day, int start_min, int end_min){
    userGaps.push_back({day, start_min, end_min});
}

static inline std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

// ---- Parse input file ----
void Scheduler::parseFile() {
    subjects.clear();
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Failed to open: " << filePath << "\n";
        return;
    }

    std::string line;
    subject currSubj;
    slot currSlot;
    std::string currProf;
    bool inSlot = false;

    auto trim = [](std::string &s) {
        s.erase(0, s.find_first_not_of(" \t"));
        s.erase(s.find_last_not_of(" \t") + 1);
    };

    while (std::getline(file, line)) {
        trim(line);
        if (line.empty()) continue;
        if (line.rfind("//", 0) == 0) continue; // ignore comments

        if (line.rfind("SUBJ:", 0) == 0) {
            // Finish previous slot & subject
            if (inSlot) { currSubj.slots.push_back(currSlot); inSlot = false; }
            if (!currSubj.subject_code.empty()) subjects.push_back(currSubj);

            currSubj = subject();
            currSubj.subject_code = line.substr(5);
            trim(currSubj.subject_code);
        }
        else if (line.rfind("PROF:", 0) == 0) {
            if (inSlot) { currSubj.slots.push_back(currSlot); inSlot = false; }

            currProf = line.substr(5);
            trim(currProf);
        }
        else if (!line.empty() && line.find(' ') == std::string::npos) {
            // Section (assume no spaces in section name)
            if (inSlot) currSubj.slots.push_back(currSlot);

            currSlot = slot();
            currSlot.subject_code = currSubj.subject_code;
            currSlot.professor = currProf;
            currSlot.section = line;
            inSlot = true;
        }
        else {
            // Schedule line: days start end [room]
            if (!inSlot) continue;

            std::istringstream ss(line);
            std::string days, startStr, endStr, room = "IGN";
            ss >> days >> startStr >> endStr;
            if (ss >> room); // optional room

            schedule s;
            s.subject_code = currSubj.subject_code;
            s.professor   = currProf;
            s.section     = currSlot.section;
            s.days        = days;
            s.room        = room;

            parseTime(startStr, s.start_hour, s.start_min);
            parseTime(endStr,   s.end_hour,   s.end_min);

            currSlot.schedules.push_back(s);
        }
    }

    // Push last slot & subject
    if (inSlot) currSubj.slots.push_back(currSlot);
    if (!currSubj.subject_code.empty()) subjects.push_back(currSubj);
}




// ---- Helper: get indices for schedule days ----
static std::vector<int> scheduleDays(const std::string& dayStr, const std::unordered_map<std::string,std::vector<int>>& dayMap){
    auto it = dayMap.find(dayStr);
    if(it != dayMap.end()) return it->second;
    return {};
}

// ---- Helper: check if two schedules share any day ----
static bool schedulesOverlapDay(const schedule &a, const schedule &b, const std::unordered_map<std::string,std::vector<int>>& dayMap){
    std::vector<int> aDays = scheduleDays(a.days, dayMap);
    std::vector<int> bDays = scheduleDays(b.days, dayMap);

    for(int d1 : aDays) for(int d2 : bDays) if(d1==d2) return true;
    return false;
}

// ---- Add user gaps as dummy subjects ----
std::vector<subject> Scheduler::getSubjectsWithGaps(){
    std::vector<subject> allSubjects = subjects;

    for(const Gap &g : userGaps){
        auto it = dayMap.find(g.day);
        if(it==dayMap.end()) continue;

        for(int dayIdx : it->second){
            subject dummy;
            dummy.subject_code = "GAP";
            slot s;
            schedule sch;
            sch.subject_code = "GAP";
            sch.professor    = "USER";
            sch.section      = "X";
            sch.days         = dayNames[dayIdx];
            sch.start_hour   = g.start_min / 60;
            sch.start_min    = g.start_min % 60;
            sch.end_hour     = g.end_min / 60;
            sch.end_min      = g.end_min % 60;

            s.schedules.push_back(sch);
            dummy.slots.push_back(s);
            allSubjects.push_back(dummy);
        }
    }

    return allSubjects;
}

bool Scheduler::violatesThreeConsecutive(const std::vector<schedule>& schedules) {
    // Check each day M=0..SUN=6
    for (int d = 0; d < 7; ++d) {
        // Gather schedules for this day
        std::vector<schedule> daySchedules;
        for (const auto &s : schedules) {
            if (s.subject_code == "GAP") continue;
            auto days = scheduleDays(s.days, dayMap);
            if (std::find(days.begin(), days.end(), d) != days.end())
                daySchedules.push_back(s);
        }

        if(daySchedules.size() < 3) continue;

        // Sort by start time
        std::sort(daySchedules.begin(), daySchedules.end(),
                  [](const schedule &a, const schedule &b) {
                      return a.start_to_min() < b.start_to_min();
                  });

        // Check for 3 consecutive schedules with no gaps
        for (size_t i = 0; i + 2 < daySchedules.size(); ++i) {
            if (daySchedules[i].end_to_min() == daySchedules[i+1].start_to_min() &&
                daySchedules[i+1].end_to_min() == daySchedules[i+2].start_to_min()) {
                return true; // violates rule
            }
        }
    }
    return false; // ok
}

bool Scheduler::backtrackSchedule(
    const std::vector<subject>& subs,
    size_t idx,
    std::vector<schedule>& placed,
    std::vector<schedule>& result
) {
    if(idx >= subs.size()) {  // all subjects placed
        result = placed;
        return true;
    }

    const subject &currSubj = subs[idx];

    for(const slot &sl : currSubj.slots) {
        bool fits = true;

        // Check each schedule in this slot against already placed schedules
        for(const schedule &s : sl.schedules) {
            for(const schedule &p : placed) {
                if(schedulesOverlapDay(s, p, dayMap) && p.isColliding(s)) {
                    fits = false;
                    break;
                }
            }
            if(!fits) break;
        }

        if(!fits) continue;

        // Temporarily add this slot
        placed.insert(placed.end(), sl.schedules.begin(), sl.schedules.end());

        // Check 3 consecutive schedules rule
        if(violatesThreeConsecutive(placed)) {
            placed.erase(placed.end() - sl.schedules.size(), placed.end());
            continue;
        }

        // Recurse to next subject
        if(backtrackSchedule(subs, idx+1, placed, result)) return true;

        // Backtrack
        placed.erase(placed.end() - sl.schedules.size(), placed.end());
    }

    return false; // no slot works for this subject
}

std::vector<schedule> Scheduler::generateSchedule() {
    parseFile();
    auto allSubs = getSubjectsWithGaps();
    std::sort(allSubs.begin(), allSubs.end());

    std::vector<schedule> placed;
    std::vector<schedule> finalSched;
    if(backtrackSchedule(allSubs, 0, placed, finalSched))
        return finalSched;

    return {}; // no valid schedule found
}






