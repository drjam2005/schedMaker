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
