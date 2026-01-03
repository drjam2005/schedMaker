#include <raylib.h>
#include <scheduler.h>

int main(){
	Scheduler scheduler;

	schedule sched1 = {
		.subject_code = "CSMC221",
		.professor = "J.Aureus",
		.days = "TTH",
		.start_hour = 13, .start_min = 30,
		.end_hour = 15, .end_min = 0
	};

	schedule sched2 = {
		.subject_code = "CSMC221",
		.professor = "J.Aureus",
		.days = "TTH",
		.start_hour = 15, .start_min = 00,
		.end_hour = 16, .end_min = 30
	};
	slot slot1;
	slot1.professor = "J.Aureus";
	slot1.subject_code = "CSMC221";
	slot1.addSched(sched1);

	slot slot2;
	slot2.professor = "J.Aureus";
	slot2.subject_code = "CSMC221";
	slot2.addSched(sched2);

	subject sub1;
	sub1.subject_code = "CSMC221";
	sub1.addSlot(slot1);
	sub1.addSlot(slot2);

	schedule sched3 = {
		.subject_code = "CSMC222",
		.professor = "M.Z.Imperial",
		.days = "MW",
		.start_hour = 18, .start_min = 00,
		.end_hour = 19, .end_min = 30
	};

	slot slot3;
	slot3.professor = "M.Z.Imperial";
	slot3.subject_code = "CSMC222";
	slot3.addSched(sched3);

	subject sub2;
	sub2.subject_code = "CSMC222";
	sub2.addSlot(slot3);


	scheduler.addSubject(sub1);
	scheduler.addSubject(sub2);

	scheduler.generateSchedule();
	
	return 0;
}
