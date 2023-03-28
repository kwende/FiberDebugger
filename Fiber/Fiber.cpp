// Fiber.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "argparse/argparse.hpp"

#include <Windows.h>
#include <imagehlp.h>

#include "Process.h"
#include "Debugger.h"
#include "Dumper.h"

int main(int argc, char* argv[])
{
	argparse::ArgumentParser program("Fiber");
	program.add_argument("-i", "--imagePath")
		.required()
		.help("Path of EXE to launch.");
	program.add_argument("-a", "--arguments")
		.help("Arguments to pass to the process."); 
	program.add_argument("-o", "--output")
		.required()
		.help("Path at which to store minidump if crash encountered."); 

	try 
	{
		program.parse_args(argc, argv);
	}
	catch (const std::runtime_error& err)
	{
		std::cerr << err.what() << std::endl; 
		std::cerr << program << std::endl; 
		std::exit(1);
	}

	auto imagePath = program.get<std::string>("-i"); 
	auto miniDumpPath = program.get<std::string>("-o"); 
	std::string arguments = ""; 
	if (program.is_used("-a"))
	{
		arguments = program.get<std::string>("-a");
	}

	auto process = std::shared_ptr<Fiber::Process>(Fiber::Process::Launch(imagePath, arguments));
	if (process != nullptr)
	{
		auto debugger = std::shared_ptr<Fiber::Debugger>(Fiber::Debugger::Attach(process));
		auto waitResult = debugger->Wait();
		if (waitResult == Fiber::WaitResult::Crash)
		{
			Dumper::WriteMiniDump(miniDumpPath, debugger); 
			debugger->Detach(); 
		}
	}
}

