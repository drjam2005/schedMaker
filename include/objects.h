#pragma once
#ifndef OBJECTS_H
#define OBJECTS_h

#include <raylib.h>
#include <string>
#include <vector>

struct schedule {
	std::string subject_code;
	std::string professor;
	std::string days;
	int start_hour, start_min;
	int end_hour, end_min;

	int start_to_min();
	int end_to_min();

	bool isColliding(schedule& other);
};

struct slot {
	std::string subject_code;
	std::string professor;
	std::vector<schedule> schedules;

	void addSched(schedule sched);
};

class subject {
	public:
		std::string subject_code;
		std::vector<slot> slots;

		void addSlot(slot slot);
		bool operator<(const subject& other) const;
};

#endif
