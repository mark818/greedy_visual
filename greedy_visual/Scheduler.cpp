#include "stdafx.h"
#include "Resource.h"
#include "Node.h"
#include "Scheduler.h"

using namespace std;

Scheduler *Scheduler::self = 0;

Scheduler::Scheduler(unsigned int num) noexcept : nodes(num) {
	self = this;
	worker_thread = CreateThread(0, 0, worker, 0, 0, 0);
}

pair<unsigned int, unsigned int> Scheduler::new_task(unsigned int start, unsigned int duration) noexcept
{
	auto iter = min_element(nodes.begin(), nodes.end());
	unsigned int index = static_cast<unsigned int>(iter - nodes.begin());
	iter->synchronized_append(start, duration);
	return make_pair<unsigned int, unsigned int> (move(index), static_cast<unsigned int>(iter->size()));
}

Scheduler::~Scheduler() noexcept {
	execute = false;
	WaitForSingleObject(worker_thread, INFINITE);
}

void Scheduler::reset(unsigned int num) noexcept {
	execute = false;
	WaitForSingleObject(worker_thread, INFINITE);
	CloseHandle(worker_thread);
	nodes.clear();
	nodes.resize(num);
	execute = true;
	worker_thread = CreateThread(0, 0, worker, 0, 0, 0);
}

unsigned long _stdcall Scheduler::worker(void*) noexcept {
	while (self->execute) {
		for (size_t i = 0; i < self->nodes.size(); i++) {
			if (self->nodes[i].synchronized_tick_and_remove()) {
				unsigned int size = self->nodes[i].size();
				PostMessage(hwnd, WM_COMMAND, COLOR_NODE, i << 16 | size);
			}
		}
		Sleep(1000);
	}
	return 0;
}
