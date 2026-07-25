#include "scheduler.h"
#include "defaultSched.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <ctime>
#include <raylib.h>

static inline std::string trim(const std::string& s);

void parseTime(const std::string& timeStr, int &hour, int &min) {
    std::string value = trim(timeStr);
    std::transform(value.begin(), value.end(), value.begin(),
                   [](unsigned char c) { return std::toupper(c); });

    size_t pos = value.find(':');
    if (pos == std::string::npos) { hour = min = 0; return; }

    try {
        hour = std::stoi(value.substr(0, pos));
        min = std::stoi(value.substr(pos + 1, 2));

        const bool isAM = value.find("AM", pos + 1) != std::string::npos;
        const bool isPM = value.find("PM", pos + 1) != std::string::npos;
        const bool isNoon = value.find("NN", pos + 1) != std::string::npos;
        if (isAM && hour == 12) hour = 0;
        if (isPM && hour != 12) hour += 12;
        if (isNoon) hour = 12;
    } catch(...) { hour = min = 0; }
}

Scheduler::Scheduler(std::string filePath) {
    this->filePath = filePath;
	ensureScheduleFileExists();
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

static std::vector<std::string> splitTabFields(const std::string& line) {
    std::vector<std::string> fields;
    std::stringstream stream(line);
    std::string field;

    while (std::getline(stream, field, '\t')) fields.push_back(trim(field));
    return fields;
}

static bool parseMeeting(const std::string& value, schedule& result) {
    std::istringstream stream(value);
    std::string start, separator, end;
    if (!(stream >> result.days >> start >> separator >> end) || separator != "-") return false;

    parseTime(start, result.start_hour, result.start_min);
    parseTime(end, result.end_hour, result.end_min);
    return true;
}

static bool isClosedRow(const std::vector<std::string>& fields) {
    if (fields.empty()) return false;

    std::string availability = fields.back();
    std::transform(availability.begin(), availability.end(), availability.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return availability == "CLOSED";
}

static void addTabularSchedule(std::vector<subject>& subjects,
                               const std::string& subjectCode,
                               const std::string& section,
                               const schedule& meeting) {
    auto subjectIt = std::find_if(subjects.begin(), subjects.end(),
        [&](const subject& value) { return value.subject_code == subjectCode; });
    if (subjectIt == subjects.end()) {
        subjects.push_back(subject());
        subjectIt = std::prev(subjects.end());
        subjectIt->subject_code = subjectCode;
    }

    auto slotIt = std::find_if(subjectIt->slots.begin(), subjectIt->slots.end(),
        [&](const slot& value) {
            // A registrar row describes one meeting of a section.  A section
            // may have several rows (for example, separate lecture and lab
            // meetings), so its section name—not the per-row instructor
            // text—is the offering identity.
            return value.section == section;
        });
    if (slotIt == subjectIt->slots.end()) {
        subjectIt->slots.push_back(slot());
        slotIt = std::prev(subjectIt->slots.end());
        slotIt->subject_code = subjectCode;
        slotIt->section = section;
        slotIt->professor = meeting.professor;
    }

    // Ignore an exact duplicate while retaining distinct meetings for the
    // same section.  This makes an accidental duplicate export harmless.
    const auto duplicate = std::find_if(slotIt->schedules.begin(), slotIt->schedules.end(),
        [&](const schedule& value) {
            return value.days == meeting.days &&
                   value.start_hour == meeting.start_hour &&
                   value.start_min == meeting.start_min &&
                   value.end_hour == meeting.end_hour &&
                   value.end_min == meeting.end_min &&
                   value.room == meeting.room;
        });
    if (duplicate == slotIt->schedules.end()) {
        slotIt->schedules.push_back(meeting);
    }
}

// ---- Parse registrar-exported tab-separated rows ----
void Scheduler::parseFile() {
    subjects.clear();
    std::ifstream file(filePath);

	if(!FileExists(filePath.c_str())) {
		std::cerr << TextFormat("%s doesn't exist", filePath.c_str()) << '\n';
	}

    if (!file.is_open())  {
        std::cerr << "Failed to open: " << filePath << "\n";
	}

    std::string line;
    while (std::getline(file, line)) {
        const std::vector<std::string> fields = splitTabFields(line);

        // SUBJECT CODE, SUBJECT, UNITS, SECTION, DAY - TIME, ROOM,
        // INSTRUCTOR, OPEN SLOTS.  Headers and incomplete rows are ignored.
        if (fields.size() != 8 || fields[0].empty() || fields[3].empty()) continue;
        //if (isClosedRow(fields)) continue;

        schedule parsed{};
        if (!parseMeeting(fields[4], parsed)) continue;

        parsed.subject_code = fields[0];
        parsed.section = fields[3];
        parsed.room = fields[5];
        parsed.professor = fields[6];
        addTabularSchedule(subjects, parsed.subject_code, parsed.section, parsed);
    }
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


bool Scheduler::violatesConsecutive(const std::vector<schedule>& schedules) {
    for (int d = 0; d < 7; ++d) { // M=0..SUN=6
        std::vector<schedule> daySchedules;

        for (const auto &s : schedules) {
            if (s.subject_code == "GAP") continue;
            auto days = scheduleDays(s.days, dayMap);
            if (std::find(days.begin(), days.end(), d) != days.end())
                daySchedules.push_back(s);
        }

        if(daySchedules.size() < static_cast<size_t>(maxConsecutiveSchedules))
            continue;

        // Sort by start time
        std::sort(daySchedules.begin(), daySchedules.end(),
                  [](const schedule &a, const schedule &b) {
                      return a.start_to_min() < b.start_to_min();
                  });

        // Check for consecutive schedules with no gaps
        for (size_t i = 0; i + maxConsecutiveSchedules - 1 < daySchedules.size(); ++i) {
            bool consecutive = true;
            for (int j = 0; j < maxConsecutiveSchedules - 1; ++j) {
                if (daySchedules[i + j].end_to_min() != daySchedules[i + j + 1].start_to_min()) {
                    consecutive = false;
                    break;
                }
            }
            if (consecutive) return true; // violates rule
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
        if(violatesConsecutive(placed)) {
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

    return {};
}





void Scheduler::ensureScheduleFileExists()
{
	if (FileExists(filePath.c_str()))
		return;

	std::ofstream out(filePath);
	if (!out) {
		std::cerr << "Failed to create " << filePath << '\n';
		return;
	}

	out << defaultSched;

	std::cout << "Created default schedule: " << filePath << '\n';
}
