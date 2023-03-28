#include "Process.h"
using namespace Fiber; 
#include <iostream>

Process* Process::Launch(std::string imagePath, std::string parameters)
{   
    PROCESS_INFORMATION processInformation;
    STARTUPINFOA startupInfo;

    ZeroMemory(&processInformation, sizeof(processInformation));
    ZeroMemory(&startupInfo, sizeof(startupInfo));

    startupInfo.cb = sizeof(startupInfo);
    BOOL created = ::CreateProcessA(
        imagePath.c_str(),
        const_cast<char*>(parameters.c_str()),
        0,
        0,
        FALSE,
        0,
        0,
        0,
        &startupInfo,
        &processInformation
    );

    Process* process = nullptr;

    if (created)
    {
        ULONGLONG start = GetTickCount64();
        while (GetTickCount64() - start < 5000)
        {
            DWORD dwExitCode = 0; 
            if (GetExitCodeProcess(processInformation.hProcess, &dwExitCode))
            {
                if (dwExitCode == STILL_ACTIVE)
                {
                    process = new Process(processInformation, startupInfo);
                    break; 
                }
            }
        }
    }

    return process;
}

Process::Process(PROCESS_INFORMATION processInfo, STARTUPINFOA startupInfo)
{
    _processInformation = processInfo; 
    _startupInfo = startupInfo; 
}

int Process::Pid()
{
    return _processInformation.dwProcessId;
}

Process::~Process()
{

}