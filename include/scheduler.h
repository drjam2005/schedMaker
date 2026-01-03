#pragma once
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <objects.h>

class Scheduler {
	public:
	std::vector<subject> subjects;
	std::vector<std::vector<schedule>> generatedSchedules;
	std::vector<schedule> thing;

	Scheduler();
	void addSubject(subject subj);

	std::vector<schedule> generateSchedule();
};

#endif
