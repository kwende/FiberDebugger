#include "Debugger.h"
#include <iostream>
using namespace Fiber; 

Debugger::Debugger(std::shared_ptr<Process> process)
{
	_process = process;
}

Debugger* Debugger::Attach(std::shared_ptr<Process> process)
{
	Debugger* debugger = nullptr; 
	if (::DebugActiveProcess(process->Pid()))
	{
		debugger = new Debugger(process);
	}
	return debugger; 
}

void Debugger::Detach()
{
	DebugActiveProcessStop(_process->Pid()); 
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
	int exceptionChanceCount = 0; 
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
				exceptionChanceCount++; 
				if (exceptionChanceCount == 1)
				{
					dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
				}
				else
				{
					dwContinueStatus = DBG_EXCEPTION_NOT_HANDLED;
					stillRunning = false; 
					_crashEvent = debugEvent; 
					result = WaitResult::Crash; 
				}
				break;
			}
			break; 
		case EXIT_PROCESS_DEBUG_EVENT:
			std::cout << "exit" << std::endl;
			stillRunning = false; 
			break; 
		default:
			exceptionChanceCount = 0; 
		}

		ContinueDebugEvent(debugEvent.dwProcessId,
			debugEvent.dwThreadId,
			dwContinueStatus);
	}

	return result; 
}