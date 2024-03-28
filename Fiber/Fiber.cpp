// Fiber.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <string>
#include "argparse/argparse.hpp"

#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

#include <Windows.h>
#include <imagehlp.h>

#include "Process.h"
#include "Debugger.h"
#include "Dumper.h"
#include "AzureUploader.h"

int main(int argc, char* argv[])
{
	auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
	console_sink->set_level(spdlog::level::debug);
	console_sink->set_pattern("[fiber] [%^%l%$] %v");

	auto multiSink = std::make_shared<spdlog::logger>(std::string("fiber"), spdlog::sinks_init_list{  console_sink });
	multiSink->set_level(spdlog::level::debug);
	multiSink->flush_on(spdlog::level::debug);

	spdlog::set_default_logger(multiSink);

	argparse::ArgumentParser program("Fiber");
	program.add_argument("-i", "--imagePath")
		.required()
		.help("Path of EXE to launch.");
	program.add_argument("-a", "--arguments")
		.remaining()
		.help("Arguments to pass to the process."); 
	program.add_argument("-o", "--output")
		.required()
		.help("Name to give the minidump file either in Azure or locally."); 
	program.add_argument("-c", "--connectionString")
		.help("Azure connection string to use when uploading the minidump."); 
	program.add_argument("-b", "--blobName")
		.help("Azure blob name to use when uploading the minidump"); 

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

	spdlog::debug("Successfully parsed command line arguments."); 

	auto imagePath = program.get<std::string>("-i"); 
	auto miniDumpPath = program.get<std::string>("-o"); 
	std::string arguments = ""; 
	if (program.is_used("-a"))
	{
		arguments = program.get<std::string>("-a");
	}

	spdlog::debug("Launching '{0}' with parameters '{1}'", imagePath.c_str(), arguments.c_str());
	auto process = std::shared_ptr<Fiber::Process>(Fiber::Process::Launch(imagePath, arguments));
	if (process != nullptr)
	{
		spdlog::debug("Process '{0}' successfully launched. Attaching debugger.", imagePath.c_str());
		auto debugger = std::shared_ptr<Fiber::Debugger>(Fiber::Debugger::Attach(process));
		auto waitResult = debugger->Wait();
		if (waitResult == Fiber::WaitResult::Crash)
		{
			spdlog::info("Fiber encountered a crash in the child process. Creating minidump..."); 
			if (program.is_used("-c") && program.is_used("-b"))
			{
				Dumper::WriteMiniDump("./temp.dmp", debugger);
				std::string blobNameForUpload = "FiberDumps/" + program.get<std::string>("-o");
				spdlog::info("Minidump generated. Blob name is '{0}'.", blobNameForUpload);
				Fiber::AzureUploader uploader(program.get<std::string>("-c"), program.get<std::string>("-b"));
				uploader.Upload("./temp.dmp", blobNameForUpload);
				DeleteFileA("./temp.dmp"); 
			}
			else
			{
				Dumper::WriteMiniDump(miniDumpPath, debugger);
			}
			debugger->Detach(); 
		}
	}
	else
	{
		spdlog::error("Failed to launch {0}", imagePath); 
	}

	spdlog::info("Fiber is quitting."); 
}

