#pragma once
#include <string>
#include <Windows.h>

namespace Fiber {
	class Process
	{
	public:
		static Process* Launch(std::string imagePath, std::string parameters);
		void Kill();
		int Pid();
		~Process();
	private:
		Process(PROCESS_INFORMATION processInfo, STARTUPINFOA startupInfo);
		PROCESS_INFORMATION _processInformation;
		STARTUPINFOA _startupInfo;
	};
}


