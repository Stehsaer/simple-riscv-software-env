#pragma once

#include "process.hpp"
#include <queue>

extern struct Scheduler_queue
{
	std::queue<Process::Id> ready;
	std::optional<Process::Id> running;
} scheduler_queue;
