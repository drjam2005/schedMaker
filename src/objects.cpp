#include <objects.h>

int schedule::start_to_min() const {
    return (60 * start_hour) + start_min;
}

int schedule::end_to_min() const {
    return (60 * end_hour) + end_min;
}

bool schedule::isColliding(const schedule& other) const {
    // We need access to the mapping logic.
    // If we can't easily pass the map, we check for exact substring matches
    // or use a helper that handles "T" vs "TH" correctly.

    auto getDays = [](std::string dstr) -> std::vector<std::string> {
        if (dstr == "MW") return {"M", "W"};
        if (dstr == "TTH") return {"T", "TH"};
        return {dstr};
    };

    std::vector<std::string> myDays = getDays(days);
    std::vector<std::string> theirDays = getDays(other.days);

    bool shareDay = false;
    for (const auto& d1 : myDays) {
        for (const auto& d2 : theirDays) {
            if (d1 == d2) { shareDay = true; break; }
        }
    }

    if (!shareDay) return false;

    int s1 = start_to_min();
    int e1 = end_to_min();
    int s2 = other.start_to_min();
    int e2 = other.end_to_min();

    return (s1 < e2 && e1 > s2);
}

void slot::addSched(schedule sched){
    schedules.push_back(sched);
}

void subject::addSlot(slot slot){
    slots.push_back(slot);
}

bool subject::operator<(const subject& other) const {
    if (slots.size() != other.slots.size())
        return slots.size() < other.slots.size();
    return subject_code < other.subject_code;
}


std::string defaultSched = 
R"(//// subject
//SUBJ: <subj-code>
//  PROF: <prof-name>                       // 2 spaces indentation
//    <section>                             // 4 spaces indentationes
//      <date> <startTime> <endTime> <room> // 6 spaces indentationes
//      ...                                 // 6 spaces indentationes
//      <date> <startTime> <endTime> <room> // 6 spaces indentationes

SUBJ: CSDC105
  PROF: Professor1
	ZC21Am
	  MW 9:00 10:30 room1
	  TTH 9:00 10:00 room2
		
	ZC22Am
	  MW 10:30 12:00 room3
	  TTH 10:00 11:00 room4
	
  PROF: Professor2
	ZT21Am
	  MW 18:00 19:00 room5
	  TTH 16:30 18:00 room6

SUBJ: CSMC221
  PROF: View_scheds.txt
	ZC21Am
	  TTH 13:30 15:00 room7
	
	ZC22Am
	  TTH 15:00 16:30 room8
)";
