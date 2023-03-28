#include "Dumper.h"

bool Dumper::WriteMiniDump(std::string miniDumpPath, std::shared_ptr<Fiber::Debugger> debugger)
{
	bool ret = false; 
	MINIDUMP_EXCEPTION_INFORMATION exceptionInformation = { 0 };

	DEBUG_EVENT debugEvent = debugger->GetCrashEvent(); 

	exceptionInformation.ClientPointers = false; 
	exceptionInformation.ThreadId = debugEvent.dwThreadId;

	EXCEPTION_POINTERS exceptionPointers = { 0 }; 

	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, debugEvent.dwProcessId); 
	if (hProcess != 0)
	{
		HANDLE hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, debugEvent.dwThreadId);
		if (hThread != 0)
		{
			CONTEXT threadContext = { 0 }; 
			if (GetThreadContext(hThread, &threadContext))
			{
				exceptionPointers.ContextRecord = &threadContext; 
				exceptionPointers.ExceptionRecord = &debugEvent.u.Exception.ExceptionRecord; 

				exceptionInformation.ExceptionPointers = &exceptionPointers; 

				HANDLE hDumpFile = CreateFileA(
					miniDumpPath.c_str(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
					NULL,
					CREATE_ALWAYS,
					FILE_ATTRIBUTE_NORMAL,
					NULL); 
				if (hDumpFile != INVALID_HANDLE_VALUE)
				{
					BOOL wroteFile = MiniDumpWriteDump(
						hProcess,
						debugEvent.dwProcessId,
						hDumpFile,
						MINIDUMP_TYPE::MiniDumpNormal,
						&exceptionInformation,
						NULL, NULL); 

					ret = wroteFile; 

					CloseHandle(hDumpFile); 
				}
			}
			CloseHandle(hThread); 
		}
		CloseHandle(hProcess); 
	}

	return ret; 
}
