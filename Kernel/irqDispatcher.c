// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java:
// https://pvs-studio.com

#include <time.h>
#include <keyboard.h>
#include <stdint.h>

static uint64_t int_20(uint64_t);
static void int_21();

uint64_t irqDispatcher(uint64_t irq, uint64_t rsp) {
	switch (irq) {
		case 0:
			rsp = int_20(rsp);
            break;
		case 1:
			int_21();
			break;
	}
	return rsp;
}

uint64_t int_20(uint64_t rsp) {
    rsp = timerHandler(rsp);
    return rsp;
}

void int_21() {
	keyboardHandler();
}
