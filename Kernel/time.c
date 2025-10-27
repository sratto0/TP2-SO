// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include "./include/time.h"
#include <stdint.h>

static uint64_t ticks = 0;
static sleeping_process_t sleeping_processes[MAX_PROCESSES];
static uint64_t next_tick = UINT64_MAX;
static void unblock_sleeping_processes();

uint64_t timerHandler(uint64_t rsp) {
	ticks++;
	unblock_sleeping_processes();
	rsp = (uint64_t) schedule((void*) rsp);
	return rsp;
}

uint64_t ticksElapsed() {
	return ticks;
}

int secondsElapsed() {
	return ticks / 18;
}

void init_sleeping_processes(){
	for(int i = 0; i < MAX_PROCESSES; i++){
		sleeping_processes[i].pid = NO_PID;
	}
}

void remove_sleeping_process(int64_t pid){
	if(pid < 0 || pid >= MAX_PROCESSES){
		return;
	}
	sleeping_processes[pid].pid = NO_PID;
}

static void unblock_sleeping_processes(){
	if(ticks < next_tick)
		return;
	for(int i = 0; i < MAX_PROCESSES; i++){
		if(sleeping_processes[i].pid != NO_PID && sleeping_processes[i].wake_up_tick <= ticks){
			unblock_process(sleeping_processes[i].pid);
			sleeping_processes[i].pid = NO_PID;
		}else if(sleeping_processes[i].pid != NO_PID && sleeping_processes[i].wake_up_tick < next_tick){
			next_tick = sleeping_processes[i].wake_up_tick;
		}
	}
}

void sleep(uint32_t sleeping_ticks){
	int64_t pid = get_current_pid();
	if(pid < 0 || pid >= MAX_PROCESSES){
		return;
	}
	sleeping_processes[pid].wake_up_tick = ticks + sleeping_ticks;
	sleeping_processes[pid].pid = pid;
	if(sleeping_processes[pid].wake_up_tick < next_tick)
		next_tick = sleeping_processes[pid].wake_up_tick;
	sleep_block(pid, 1);
}
