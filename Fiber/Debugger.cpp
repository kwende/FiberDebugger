#include "Debugger.h"
#include <iostream>
using namespace Fiber; 

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

Debugger::Debugger(std::shared_ptr<Process> process)
{
	_process = process;
}

Debugger* Debugger::Attach(std::shared_ptr<Process> process)
{
	Debugger* debugger = nullptr; 
	if (::DebugActiveProcess(process->Pid()))
	{
		spdlog::debug("Debugger successfully attached to process w/ pid {0}.", process->Pid()); 
		debugger = new Debugger(process);
	}
	else
	{
		spdlog::error("Failed to attach to process w/ pid {0}", process->Pid()); 
	}
	return debugger; 
}

void Debugger::Detach()
{
	if (DebugActiveProcessStop(_process->Pid()))
	{
		spdlog::debug("Successfully detached from process w/ pid {0}.", _process->Pid()); 
	}
	else
	{
		spdlog::error("Failed to detach from the process w/ pid {0}.", _process->Pid()); 
	}
}

DEBUG_EVENT Debugger::GetCrashEvent()
{
	return _crashEvent; 
}

WaitResult Debugger::Wait() 
{
	WaitResult result = WaitResult::NormalExit;

	DEBUG_EVENT debugEvent;
	bool stillRunning = true; 
	while (stillRunning && ::WaitForDebugEvent(&debugEvent, INFINITE))
	{
		DWORD dwContinueStatus = DBG_CONTINUE;
		switch (debugEvent.dwDebugEventCode)
		{
		case EXCEPTION_DEBUG_EVENT:
			switch (debugEvent.u.Exception.ExceptionRecord.ExceptionCode)
			{
			case EXCEPTION_SINGLE_STEP: 
			case EXCEPTION_BREAKPOINT:
				dwContinueStatus = DBG_CONTINUE; 
				break;

			default:
				if (debugEvent.u.Exception.dwFirstChance)
				{
					dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				}
				else
				{
					spdlog::info("Second chance exception encountered w/ code {0:x}. Breaking from the wait loop.", debugEvent.u.Exception.ExceptionRecord.ExceptionCode); 
					dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					stillRunning = false; 
					_crashEvent = debugEvent; 
					result = WaitResult::Crash; 
				}
				break;
			}
			break; 
		case EXIT_PROCESS_DEBUG_EVENT:
			stillRunning = false; 
			break; 
		}

		ContinueDebugEvent(debugEvent.dwProcessId,
			debugEvent.dwThreadId,
			dwContinueStatus);
	}

	spdlog::debug("Leaving debug wait loop."); 
	return result; 
}