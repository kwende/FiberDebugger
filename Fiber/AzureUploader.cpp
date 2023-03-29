#include "AzureUploader.h"
#include "azure/storage/blobs.hpp"

using namespace Fiber; 
using namespace Azure::Storage::Blobs;

#include <iostream>

AzureUploader::AzureUploader(std::string connectionString, std::string containerName)
{
	_connectionString = connectionString; 
	_containerName = containerName; 
}

bool AzureUploader::Upload(std::string filePath, std::string blobName)
{
	std::cout << _connectionString << std::endl; 

	auto containerClient
		= BlobContainerClient::CreateFromConnectionString(_connectionString, _containerName);
	containerClient.CreateIfNotExists(); 

	auto blobClient = containerClient.GetBlockBlobClient(blobName);

	Azure::Core::IO::FileBodyStream fileStream(filePath); 

	auto response = blobClient.Upload(fileStream); 

	return true; 
}