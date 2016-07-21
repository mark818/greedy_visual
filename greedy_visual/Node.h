#pragma once
#include "stdafx.h"

class Node {
public:
	Node() = default;
	~Node() noexcept = default;

	unsigned int size() {
		//AcquireSRWLockShared(&lock);
		unsigned int size = static_cast<unsigned int>(tasks.size());
		//ReleaseSRWLockShared(&lock);
		return size;
	}
	void synchronized_append(unsigned int start, unsigned int duration) {
		AcquireSRWLockExclusive(&lock);
		tasks.emplace_back(start, duration);
		ReleaseSRWLockExclusive(&lock);
	}
	bool synchronized_tick_and_remove();

private:
	struct Task {
		Task(unsigned int start, unsigned int duration) noexcept :
		start(start), duration(duration) {}
		bool operator == (const Task & rhs) { return start == rhs.start && duration == rhs.duration; }
		bool operator == (const int) { return false; }
		unsigned int start;
		unsigned int duration;
	};
	std::list<Task> tasks;
	SRWLOCK lock = SRWLOCK_INIT;
};


bool Node::synchronized_tick_and_remove() {
	bool changed = false;
	auto end = tasks.end();
	for (auto iter = tasks.begin(); iter != end;) {
		if (--iter->duration <= 0) {
			changed = true;
			AcquireSRWLockExclusive(&lock);
			iter = tasks.erase(iter);
			ReleaseSRWLockExclusive(&lock);
			continue;
		}
		iter++;
	}
	return changed;
}

inline bool operator< (Node &a, Node &b) {
	return a.size() < b.size();
}