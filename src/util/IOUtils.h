// Author: https://github.com/AMXerSurf
#pragma once

#include <string>
#include "Collections.h"
#include <stdexcept>
#include <filesystem>

namespace AEX {

	class io_error : public std::runtime_error {
		private:
		protected:
		public:
			explicit io_error(const std::string& message);
	};

	struct ResourceLocation {
		std::string filename;
		uint64_t offset;
		uint64_t size;
	};

	struct DirectoryEntry {
		std::string filename;	//Path including file name e.g. .../folder/file.txt
		std::string name;		//Just the name of a file
		std::string extension;	//The file extension of the file
		bool isDirectory;
	};

	class DirectoryIterator {
		private:
			std::filesystem::directory_iterator iterator;
		protected:
		public:
			DirectoryIterator(const std::filesystem::directory_iterator& iterator);
			DirectoryIterator(const DirectoryIterator& iterator) noexcept;
			DirectoryIterator begin() noexcept;
			DirectoryIterator end() noexcept;
			DirectoryEntry operator*();
			DirectoryEntry operator->();
			DirectoryIterator& operator++();
			bool operator==(const DirectoryIterator& iterator) const;
			bool operator!=(const DirectoryIterator& iterator) const;
	};

	class IOUtils {
		private:
		protected:
		public:
			//Regular access method e.g. filename
			static int8_t* readBytesFromFile(const std::string& filename, uint64_t& dataSize);
			static void writeBytesToFile(const std::string& filename, bool recreate, int8_t* data, uint64_t dataSize);
			static int8_t* readCompressedBytesFromFile(const std::string& filename, uint64_t& dataSize);
			static void writeCompressedBytesToFile(const std::string& filename, bool recreate, int8_t* data, uint64_t dataSize);
			static std::string readStringFromFile(const std::string& filename);
			static void writeStringToFile(const std::string& filename, bool recreate, const std::string& data);
			static void readFile(const std::string& filename, ArrayList<std::string>& data);
			static void writeFile(const std::string& filename, bool recreate, const ArrayList<std::string>& data);
			static std::string readCompressedStringFromFile(const std::string& filename);
			static void writeCompressedStringToFile(const std::string& filename, bool recreate, const std::string& data);
			static void readCompressedFile(const std::string& filename, ArrayList<std::string>& data);
			static void writeCompressedFile(const std::string& filename, const ArrayList<std::string>& data);

			//ResourceLocation based access
			static int8_t* readBytesFromFile(const ResourceLocation& location, uint64_t& dataSize);
			static int8_t* readCompressedBytesFromFile(const ResourceLocation& location, uint64_t& dataSize);
			static std::string readStringFromFile(const ResourceLocation& location);
			static void readFile(const ResourceLocation& location, ArrayList<std::string>& data);
			static std::string readCompressedStringFromFile(const ResourceLocation& location);
			static void readCompressedFile(const ResourceLocation& location, ArrayList<std::string>& data);

			//General IO functions
			static std::string getFilteredFilename(const std::string& filename);
			static void createDirectory(const std::string& filename);
			static std::string getFileExtension(const std::string& filename);
			static std::string getFileName(const std::string& filename);
			static void copyFolder(const std::string& sourceFolder, const std::string& destinationFolder, bool recursive);
			static void renameFolder(const std::string& folder, const std::string& newName);
			static void moveFolder(const std::string& sourceFolder, const std::string& destinationFolder, bool recursive);
			static void copyFile(const std::string& sourceFilename, const std::string& destinationFolder);
			static void renameFile(const std::string& filename, const std::string& newName);
			static void moveFile(const std::string& sourceFilename, const std::string& destinationFolder);
			static bool isFile(const std::string& filename);
			static bool isDirectory(const std::string& filename);
			static bool exists(const std::string& filename);
			static void remove(const std::string& filename);
			static uint64_t getFileSize(const std::string& filename);
			static uint64_t getFileSize(const ResourceLocation& location);
			static DirectoryIterator getDirectoryIterator(const std::string& filename);
			static std::string getWorkingDirectory();
			static void setWorkingDirectory(const std::string& folder);
	};
} 