// Author: https://github.com/AMXerSurf
#pragma once

#include <string>
#include <stdexcept>
#include <zlib/zlib.h>

constexpr uint64_t ZLIB_CHUNK_SIZE = 16384;

namespace AEX {

	class gzip_error : public std::runtime_error {
		private:
		protected:
		public:
			explicit gzip_error(std::string message);
	};

	class GZipCompressor {
		private:
			bool finished;
			bool autoFlush;
			z_stream stream;
		protected:
		public:
			explicit GZipCompressor(uint32_t level = 9, bool autoFlush = true);
			~GZipCompressor();
			int8_t* compress(int8_t* inputData, uint64_t inputSize, uint64_t& outputSize);
	};

	class GZipDecompressor {
		private:
			z_stream stream;
		protected:
		public:
			GZipDecompressor();
			~GZipDecompressor();
			int8_t* decompress(int8_t* inputData, uint64_t inputSize, uint64_t& outputSize);
	};
}