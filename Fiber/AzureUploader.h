#pragma once
#include <string>

namespace Fiber {
	class AzureUploader
	{
	public:
		AzureUploader(std::string connectionString, std::string containerName);
		bool Upload(std::string filePath, std::string blobName);
	private:
		std::string _connectionString, _containerName;
	};
}
