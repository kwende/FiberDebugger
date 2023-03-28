#pragma once
#include <Windows.h>
#include <imagehlp.h>
#include <string>
#include "Debugger.h"
class Dumper
{
public:
	static bool WriteMiniDump(std::string miniDumpPath, std::shared_ptr<Fiber::Debugger> debugger);
};

