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
#include "AzureUploader.h"

//void TestUpload()
//{
//	Fiber::AzureUploader uploader(
//		"DefaultEndpointsProtocol=https;AccountName=qualitycontrol;AccountKey=7BPfgu7XJyzzZhh4X2M9z76EUwtQmn7oXle12Pk9KepctaAoifuCxzObV4y64lynm/Hqx8zx+sGCq08nV52YTA==;EndpointSuffix=core.windows.net", "qualitycontrol"); 
//	uploader.Upload("c:/users/brush/desktop/googliebah.txt", "googliebah.txt");
//}

int main(int argc, char* argv[])
{
	//TestUpload(); 

	argparse::ArgumentParser program("Fiber");
	program.add_argument("-i", "--imagePath")
		.required()
		.help("Path of EXE to launch.");
	program.add_argument("-a", "--arguments")
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
			if (program.is_used("-c") && program.is_used("-b"))
			{
				Dumper::WriteMiniDump("./temp.dmp", debugger);
				Fiber::AzureUploader uploader(program.get<std::string>("-c"), program.get<std::string>("-b")); 
				uploader.Upload("./temp.dmp", program.get<std::string>("-o")); 
				DeleteFileA("./temp.dmp"); 
			}
			else
			{
				Dumper::WriteMiniDump(miniDumpPath, debugger);
			}
			debugger->Detach(); 
		}
	}
}

