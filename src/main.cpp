#include "scheduler.h"
#include <raylib.h>
#include <gui.h>
#include <renderer.h>

#define WIDTH 1024
#define HEIGHT 768

int main(){
	InitWindow(WIDTH, HEIGHT, "Overkill Scheduler");
	SetTargetFPS(60);

	Renderer renderer;
	Scheduler scheduler("../scheds.txt");

	while(!WindowShouldClose()){
		renderer.Update();
		renderer.SyncGapsToScheduler(scheduler);
		renderer.scheduleToRender = scheduler.generateSchedule();

		BeginDrawing();
		ClearBackground(BLACK);
		renderer.Render();
		EndDrawing();
}

}
