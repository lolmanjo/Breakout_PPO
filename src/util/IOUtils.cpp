// Author: https://github.com/AMXerSurf
#include "IOUtils.h"

#include "StringUtils.h"
#include "compression/GZip.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace AEX;

static const std::string LINE_INDICATOR = "\n";

//############################ io_error ############################

io_error::io_error(const std::string& message) : runtime_error(message) {}

//############################ DirectoryIterator ############################

DirectoryIterator::DirectoryIterator(const std::filesystem::directory_iterator& iterator) : iterator(iterator) {}

DirectoryIterator::DirectoryIterator(const DirectoryIterator& iterator) noexcept : iterator(iterator.iterator) {}

DirectoryIterator DirectoryIterator::begin() noexcept {
	return *this;
}

DirectoryIterator DirectoryIterator::end() noexcept {
	return DirectoryIterator(std::filesystem::directory_iterator());
}

DirectoryEntry DirectoryIterator::operator*() {
	return { (*iterator).path().generic_u8string(), (*iterator).path().filename().generic_u8string(), (*iterator).path().extension().generic_u8string(), (*iterator).is_directory() };
}

DirectoryEntry DirectoryIterator::operator->() {
	return { (*iterator).path().generic_u8string(), (*iterator).path().filename().generic_u8string(), (*iterator).path().extension().generic_u8string(), (*iterator).is_directory() };
}

DirectoryIterator& DirectoryIterator::operator++() {
	iterator++;
	return *this;
}

bool DirectoryIterator::operator==(const DirectoryIterator& iterator) const {
	return this->iterator == iterator.iterator;
}

bool DirectoryIterator::operator!=(const DirectoryIterator& iterator) const {
	return this->iterator != iterator.iterator;
}

//############################ IOUtils ############################

int8_t* IOUtils::readBytesFromFile(const std::string& filename, uint64_t& dataSize) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(!std::filesystem::exists(fileLocation) || !std::filesystem::is_regular_file(fileLocation)) {
		throw io_error("File not found! Filename: " + filename);
	}
	std::ifstream stream;
	stream.open(fileLocation, std::ios::binary | std::ios::in);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	dataSize = static_cast<uint64_t>(std::filesystem::file_size(fileLocation));
	int8_t* buffer = new int8_t[dataSize];
	stream.seekg(0);
	stream.read(reinterpret_cast<char*>(buffer), dataSize);
	stream.close();
	return buffer;
}

void IOUtils::writeBytesToFile(const std::string& filename, bool recreate, int8_t* data, uint64_t dataSize) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(recreate && std::filesystem::exists(fileLocation)) {
		try {
			std::filesystem::remove(fileLocation);
		} catch(...) {
			throw io_error("Filesystem error! Could not remove a file. Filename: " + filename);
		}
	}
	std::ofstream stream;
	stream.open(fileLocation, std::ios::binary | std::ios::out | std::ios::app);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	stream.write(reinterpret_cast<char*>(data), dataSize);
	stream.flush();
	stream.close();
}

int8_t* IOUtils::readCompressedBytesFromFile(const std::string& filename, uint64_t& dataSize) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(!std::filesystem::exists(fileLocation) || !std::filesystem::is_regular_file(fileLocation)) {
		throw io_error("File not found! Filename: " + filename);
	}
	std::ifstream stream;
	stream.open(fileLocation, std::ios::binary | std::ios::in);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	uint64_t inputSize = static_cast<uint64_t>(std::filesystem::file_size(fileLocation));
	int8_t* inputData = new int8_t[inputSize];
	stream.seekg(0);
	stream.read(reinterpret_cast<char*>(inputData), inputSize);
	stream.close();
	GZipDecompressor decompressor;
	int8_t* outputData = decompressor.decompress(inputData, inputSize, dataSize);
	delete[] inputData;
	return outputData;
}

void IOUtils::writeCompressedBytesToFile(const std::string& filename, bool recreate, int8_t* data, uint64_t dataSize) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(recreate && std::filesystem::exists(fileLocation)) {
		try {
			std::filesystem::remove(fileLocation);
		} catch(...) {
			throw io_error("Filesystem error! Could not remove a file. Filename: " + filename);
		}
	}
	std::ofstream stream;
	stream.open(fileLocation, std::ios::binary | std::ios::out | std::ios::app);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	uint64_t outputSize;
	GZipCompressor compressor;
	int8_t* outputData = compressor.compress(data, dataSize, outputSize);
	stream.write(reinterpret_cast<char*>(outputData), outputSize);
	stream.flush();
	stream.close();
	delete[] outputData;
}

std::string IOUtils::readStringFromFile(const std::string& filename) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(!std::filesystem::exists(fileLocation)) {
		throw io_error("File not found! Filename: " + filename);
	}
	std::string result;
	std::ifstream stream;
	stream.open(fileLocation, std::ios::in);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	stream.seekg(0);
	std::string line;
	while(std::getline(stream, line)) {
		result += line + "\n";
	}
	stream.close();
	result = result.substr(0, result.size() - 1);
	return result;
}

void IOUtils::writeStringToFile(const std::string& filename, bool recreate, const std::string& data) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(recreate && std::filesystem::exists(fileLocation)) {
		try {
			std::filesystem::remove(fileLocation);
		} catch(...) {
			throw io_error("Filesystem error! Could not remove a file. Filename: " + filename);
		}
	}
	std::ofstream stream;
	stream.open(fileLocation, std::ios::out | std::ios::app);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	stream << data << std::endl;
	stream.flush();
	stream.close();
}

void IOUtils::readFile(const std::string& filename, ArrayList<std::string>& data) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(!std::filesystem::exists(fileLocation)) {
		throw io_error("File not found! Filename: " + filename);
	}
	std::ifstream stream;
	stream.open(fileLocation, std::ios::in);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	stream.seekg(0);
	std::string line;
	while(std::getline(stream, line)) {
		data.add(line);
	}
	stream.close();
}

void IOUtils::writeFile(const std::string& filename, bool recreate, const ArrayList<std::string>& data) {
	std::filesystem::path fileLocation = std::filesystem::u8path(filename);
	if(recreate && std::filesystem::exists(fileLocation)) {
		try {
			std::filesystem::remove(fileLocation);
		} catch(...) {
			throw io_error("Filesystem error! Could not remove a file. Filename: " + filename);
		}
	}
	std::ofstream stream;
	stream.open(fileLocation, std::ios::out | std::ios::app);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + filename);
	}
	for(const std::string& line : data) {
		stream << line << std::endl;
	}
	stream.flush();
	stream.close();
}

std::string IOUtils::readCompressedStringFromFile(const std::string& filename) {
	uint64_t dataSize;
	int8_t* uncompressedData = readCompressedBytesFromFile(filename, dataSize);
	std::string result = std::string(reinterpret_cast<char*>(uncompressedData));
	delete[] uncompressedData;
	return result;
}

void IOUtils::writeCompressedStringToFile(const std::string& filename, bool recreate, const std::string& data) {
	writeCompressedBytesToFile(filename, recreate, const_cast<int8_t*>(reinterpret_cast<const int8_t*>(data.c_str())), data.length() + 1);
}

void IOUtils::readCompressedFile(const std::string& filename, ArrayList<std::string>& data) {
	StringUtils::split(readCompressedStringFromFile(filename), LINE_INDICATOR, data);
}

void IOUtils::writeCompressedFile(const std::string& filename, const ArrayList<std::string>& data) {
	std::string dataString;
	for(const std::string& line : data) {
		dataString += line + LINE_INDICATOR;
	}
	dataString = dataString.substr(0, dataString.size() - 1);
	writeCompressedStringToFile(filename, true, dataString);
}

int8_t* IOUtils::readBytesFromFile(const ResourceLocation& location, uint64_t& dataSize) {
	std::filesystem::path fileLocation = std::filesystem::u8path(location.filename);
	if(!std::filesystem::exists(fileLocation) || !std::filesystem::is_regular_file(fileLocation)) {
		throw io_error("File not found! Filename: " + location.filename);
	}
	std::ifstream stream;
	stream.open(fileLocation, std::ios::binary | std::ios::in);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + location.filename);
	}
	dataSize = static_cast<uint64_t>(std::filesystem::file_size(fileLocation));
	//Check the validity of the operation
	if(location.size > 0) {
		if((dataSize - location.offset) < location.size) {
			throw io_error("Invalid read operation! Requested section is larger than the acutal file. Filename: " + location.filename + " File size: " + std::to_string(dataSize) + " Requested offset: " + std::to_string(location.offset) + " Requested size: " + std::to_string(location.size));
		}
		dataSize = location.size;
	} else {
		dataSize -= location.offset;
	}
	int8_t* buffer = new int8_t[dataSize];
	stream.seekg(location.offset);
	stream.read(reinterpret_cast<char*>(buffer), dataSize);
	stream.close();
	return buffer;
}

int8_t* IOUtils::readCompressedBytesFromFile(const ResourceLocation& location, uint64_t& dataSize) {
	std::filesystem::path fileLocation = std::filesystem::u8path(location.filename);
	if(!std::filesystem::exists(fileLocation) || !std::filesystem::is_regular_file(fileLocation)) {
		throw io_error("File not found! Filename: " + location.filename);
	}
	std::ifstream stream;
	stream.open(fileLocation, std::ios::binary | std::ios::in);
	if(!stream) {
		throw io_error("Failed to open the stream! Filename: " + location.filename);
	}
	uint64_t inputSize = static_cast<uint64_t>(std::filesystem::file_size(fileLocation));
	//Check the validity of the operation
	if(location.size > 0) {
		if((dataSize - location.offset) < location.size) {
			throw io_error("Invalid read operation! Requested section is larger than the acutal file. Filename: " + location.filename + " File size: " + std::to_string(inputSize) + " Requested offset: " + std::to_string(location.offset) + " Requested size: " + std::to_string(location.size));
		}
		inputSize = location.size;
	} else {
		inputSize -= location.offset;
	}
	int8_t* inputData = new int8_t[inputSize];
	stream.seekg(location.offset);
	stream.read(reinterpret_cast<char*>(inputData), inputSize);
	stream.close();
	GZipDecompressor decompressor;
	int8_t* outputData = decompressor.decompress(inputData, inputSize, dataSize);
	delete[] inputData;
	return outputData;
}

std::string IOUtils::readStringFromFile(const ResourceLocation& location) {
	uint64_t dataSize;
	int8_t* data = readBytesFromFile(location, dataSize);
	std::string result = std::string(reinterpret_cast<char*>(data), dataSize);
	StringUtils::replace(result, "\r", "");
	delete[] data;
	return result;
}

void IOUtils::readFile(const ResourceLocation& location, ArrayList<std::string>& data) {
	StringUtils::split(readStringFromFile(location), LINE_INDICATOR, data, -1, true);
}

std::string IOUtils::readCompressedStringFromFile(const ResourceLocation& location) {
	uint64_t dataSize;
	int8_t* uncompressedData = readCompressedBytesFromFile(location, dataSize);
	std::string result = std::string(reinterpret_cast<char*>(uncompressedData), dataSize);
	StringUtils::replace(result, "\r", "");
	delete[] uncompressedData;
	return result;
}

void IOUtils::readCompressedFile(const ResourceLocation& location, ArrayList<std::string>& data) {
	StringUtils::split(readCompressedStringFromFile(location), LINE_INDICATOR, data, -1, true);
}

std::string IOUtils::getFilteredFilename(const std::string& filename) {
	std::string filteredFilename = filename;
	//Characters that cause problems in filenames on various systems
	StringUtils::replace(filteredFilename, "/", "");
	StringUtils::replace(filteredFilename, "\\", "");
	StringUtils::replace(filteredFilename, " ", "");
	StringUtils::replace(filteredFilename, "<", "");
	StringUtils::replace(filteredFilename, ">", "");
	StringUtils::replace(filteredFilename, "$", "");
	StringUtils::replace(filteredFilename, "@", "");
	StringUtils::replace(filteredFilename, "+", "");
	StringUtils::replace(filteredFilename, "%", "");
	StringUtils::replace(filteredFilename, "*", "");
	StringUtils::replace(filteredFilename, "'", "");
	StringUtils::replace(filteredFilename, "\"", "");
	StringUtils::replace(filteredFilename, "=", "");
	StringUtils::replace(filteredFilename, "|", "");
	StringUtils::replace(filteredFilename, "!", "");
	StringUtils::replace(filteredFilename, "`", "");
	StringUtils::replace(filteredFilename, "&", "");
	StringUtils::replace(filteredFilename, "?", "");
	StringUtils::replace(filteredFilename, "}", "");
	StringUtils::replace(filteredFilename, "{", "");
	StringUtils::replace(filteredFilename, ":", "");
	StringUtils::replace(filteredFilename, "#", "");
	StringUtils::replace(filteredFilename, "\t", "");
	return filteredFilename;
}

void IOUtils::createDirectory(const std::string& filename) {
	std::filesystem::create_directories(std::filesystem::u8path(filename));
}

std::string IOUtils::getFileExtension(const std::string& filename) {
	return std::filesystem::u8path(filename).extension().u8string();
}

std::string IOUtils::getFileName(const std::string& filename) {
	return std::filesystem::u8path(filename).filename().u8string();
}

void IOUtils::copyFolder(const std::string& sourceFolder, const std::string& destinationFolder, bool recursive) {
	std::filesystem::copy(std::filesystem::u8path(sourceFolder), std::filesystem::u8path(destinationFolder), std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::directories_only | (recursive ? std::filesystem::copy_options::recursive : std::filesystem::copy_options::none));
}

void IOUtils::renameFolder(const std::string& folder, const std::string& newName) {
	ArrayList<std::string> split;
	StringUtils::split(folder, "/", split, UINT32_MAX, true);
	std::string newFilename = "";
	if(split.size() > 1) {
		for(uint32_t i = 0;i < split.size() - 1;i++) {
			newFilename += split[i] + "/";
		}
	}
	newFilename += newName;
	std::filesystem::rename(std::filesystem::u8path(folder), std::filesystem::u8path(newName));
}

void IOUtils::moveFolder(const std::string& sourceFolder, const std::string& destinationFolder, bool recursive) {
	std::filesystem::copy(std::filesystem::u8path(sourceFolder), std::filesystem::u8path(destinationFolder), std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::directories_only | (recursive ? std::filesystem::copy_options::recursive : std::filesystem::copy_options::none));
	std::filesystem::remove(std::filesystem::u8path(sourceFolder));
}

void IOUtils::copyFile(const std::string& sourceFilename, const std::string& destinationFolder) {
	std::filesystem::copy_file(std::filesystem::u8path(sourceFilename), std::filesystem::u8path(destinationFolder + "/" + getFileName(sourceFilename)), std::filesystem::copy_options::overwrite_existing);
}

void IOUtils::renameFile(const std::string& filename, const std::string& newName) {
	ArrayList<std::string> split;
	StringUtils::split(filename, "/", split, UINT32_MAX, true);
	std::string newFilename = "";
	if(split.size() > 1) {
		for(uint32_t i = 0;i < split.size() - 1;i++) {
			newFilename += split[i] + "/";
		}
	}
	newFilename += newName;
	std::filesystem::rename(std::filesystem::u8path(filename), std::filesystem::u8path(newFilename));
}

void IOUtils::moveFile(const std::string& sourceFilename, const std::string& destinationFolder) {
	std::filesystem::copy_file(std::filesystem::u8path(sourceFilename), std::filesystem::u8path(destinationFolder + "/" + getFileName(sourceFilename)), std::filesystem::copy_options::overwrite_existing);
	std::filesystem::remove(std::filesystem::u8path(sourceFilename));
}

bool IOUtils::isFile(const std::string& filename) {
	return std::filesystem::is_regular_file(std::filesystem::u8path(filename));
}

bool IOUtils::isDirectory(const std::string& filename) {
	return std::filesystem::is_directory(std::filesystem::u8path(filename));
}

bool IOUtils::exists(const std::string& filename) {
	return std::filesystem::exists(std::filesystem::u8path(filename));
}

void IOUtils::remove(const std::string& filename) {
	std::filesystem::remove(std::filesystem::u8path(filename));
}

uint64_t IOUtils::getFileSize(const std::string& filename) {
	return std::filesystem::file_size(std::filesystem::u8path(filename));
}

uint64_t IOUtils::getFileSize(const ResourceLocation& location) {
	return location.size == 0 ? getFileSize(location.filename) : location.size;
}

DirectoryIterator IOUtils::getDirectoryIterator(const std::string& filename) {
	return DirectoryIterator(std::filesystem::directory_iterator(std::filesystem::u8path(filename)));
}

std::string IOUtils::getWorkingDirectory() {
	return std::filesystem::current_path().generic_u8string();
}

void IOUtils::setWorkingDirectory(const std::string& folder) {
	std::filesystem::current_path(std::filesystem::u8path(folder));
}