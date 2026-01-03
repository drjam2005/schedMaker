#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <objects.h>
#include <unordered_map>
#include <vector>
#include <string>

class Scheduler {
public:
    std::vector<std::string> dayNames;                     // "M","T","W","TH","FRI","SAT","SUN"
    std::unordered_map<std::string, std::vector<int>> dayMap; // "M" -> {0}, "TTH" -> {1,3}
    std::vector<subject> subjects;
    std::string filePath;
    std::vector<Gap> userGaps;

    Scheduler(std::string filePath);

    void addSubject(subject subj);
    void addGap(const std::string& day, int start_min, int end_min);
    void parseFile();

    std::vector<subject> getSubjectsWithGaps();
    std::vector<schedule> generateSchedule();
};

#endif
