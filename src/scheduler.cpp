#include <scheduler.h>

#include <iostream>
#include <time.h>
#include <algorithm>

Scheduler::Scheduler(){ }

void Scheduler::addSubject(subject subj){
	subjects.push_back(subj);
}

std::vector<schedule> Scheduler::generateSchedule(){
	sort(subjects.begin(), subjects.end());
	std::vector<schedule> madeScheds;

	srand(time(0));
	auto it = subjects.begin();

	do{
		size_t index = rand() % it->slots.size();

	}while(it != subjects.end());
	std::cout << '\n';

}
