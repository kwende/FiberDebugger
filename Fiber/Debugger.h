#pragma once
#include <Windows.h>
#include <imagehlp.h>
#include "Process.h"
#include <memory>

namespace Fiber {
	enum class WaitResult
	{
		NormalExit,
		Crash,
	};

	class Debugger
	{
	public:
		static Debugger* Attach(std::shared_ptr<Process> process);
		WaitResult Wait();
		DEBUG_EVENT GetCrashEvent(); 
		void Detach(); 
	private:
		Debugger(std::shared_ptr<Process> process);
		std::shared_ptr<Process> _process; 
		DEBUG_EVENT _crashEvent; 
	};
}