#include <objects.h>

int schedule::start_to_min(){
	return (start_hour + (60 * start_min));
}

int schedule::end_to_min(){
	return (end_hour + (60 * end_min));
}

bool schedule::isColliding(schedule& other) {
	if(this->days != other.days)
		return false;
	if(this->start_to_min() <= other.end_to_min())
		return false;
	if(other.start_to_min() <= this->end_to_min())
		return false;
	
	return true;
}
void slot::addSched(schedule sched){
	schedules.push_back(sched);
}

void subject::addSlot(slot slot){
	slots.push_back(slot);
}


bool subject::operator<(const subject& other) const {
	return (this->slots.size() < other.slots.size());
}
