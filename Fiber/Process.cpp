#include "Process.h"
using namespace Fiber; 
#include <iostream>

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

Process* Process::Launch(std::string imagePath, std::string parameters)
{   
    PROCESS_INFORMATION processInformation;
    STARTUPINFOA startupInfo;

    ZeroMemory(&processInformation, sizeof(processInformation));
    ZeroMemory(&startupInfo, sizeof(startupInfo));

    spdlog::debug("Image path sent ot process launch is {0}", imagePath); 

    auto lastIndexOfSlash = imagePath.find_last_of("/\\");
    std::string startupDirectory = "."; 
    if (lastIndexOfSlash != std::string::npos)
    {
        startupDirectory = imagePath.substr(0, lastIndexOfSlash); 
        spdlog::debug("Setting the startup directory to '{0}'", startupDirectory); 
    }
    else
    {
        imagePath = ".\\" + imagePath;
    }

    std::string argumentsToUse = ""; 
    if (parameters.length() > 0)
    {
        argumentsToUse = imagePath + " " + parameters; 
        spdlog::debug("Setting the arguments to use to '{0}'", argumentsToUse); 
    }

    startupInfo.cb = sizeof(startupInfo);
    BOOL created = ::CreateProcessA(
        imagePath.c_str(),
        const_cast<char*>(argumentsToUse.c_str()),
        0,
        0,
        FALSE,
        0,
        0,
        startupDirectory.c_str(),
        &startupInfo,
        &processInformation
    );

    Process* process = nullptr;

    if (created)
    {
        spdlog::debug("The process was successfully started."); 
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
    else
    {

        spdlog::error("Failed to start the process. {0}.", std::system_category().message(GetLastError()));
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