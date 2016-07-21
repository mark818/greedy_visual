#pragma once
#include "stdafx.h"

class Node;

class Scheduler {
public:
	Scheduler(unsigned int) noexcept;
	std::pair<unsigned int, unsigned int> new_task(unsigned int start, unsigned int duration) noexcept;
	~Scheduler() noexcept; 
	void reset(unsigned int) noexcept;

	Scheduler(const Scheduler &) = delete;
	Scheduler(const Scheduler &&) = delete;
	Scheduler * operator = (const Scheduler &) = delete;

private:
	static unsigned long _stdcall worker(void*) noexcept;
	std::vector<Node> nodes;
	HANDLE worker_thread;
	bool execute = true;
	static Scheduler *self;
};