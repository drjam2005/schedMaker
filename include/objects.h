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
	std::string section;
	std::string room;
	int start_hour, start_min;
	int end_hour, end_min;

	int start_to_min() const;
	int end_to_min() const;

	bool isColliding(const schedule& other) const;
};

struct Gap {
    std::string day;
    int start_min;
    int end_min;
};


struct slot {
	std::string subject_code;
	std::string professor;
    std::string section;
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
