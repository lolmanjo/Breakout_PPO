// Author: https://github.com/AMXerSurf
#include "GZip.h"

#include "../Collections.h"

using namespace AEX;

//############################ gzip_error ############################

gzip_error::gzip_error(std::string message) : runtime_error(message) {}

//############################ GZipCompressor ############################

GZipCompressor::GZipCompressor(uint32_t level, bool autoFlush) : finished(false), autoFlush(autoFlush), stream() {
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	if(level < 1 || level > 9) {
		level = 9;
	}
	if(deflateInit2(&stream, level, Z_DEFLATED, 31, 9, Z_DEFAULT_STRATEGY) != Z_OK) {
		throw gzip_error("Failed to initilize the deflator!");
	}
}

GZipCompressor::~GZipCompressor() {
	deflateEnd(&stream);
}

int8_t* GZipCompressor::compress(int8_t* inputData, uint64_t inputSize, uint64_t& outputSize) {
	if(finished) {
		throw gzip_error("Compressor has already finished!");
	}
	finished = true;
	ArrayList<int8_t> outputBuffer;
	for(uint64_t i = 0;i < inputSize;i += ZLIB_CHUNK_SIZE) {
		uint64_t remainingBytes = inputSize - i;
		uint64_t nextChunckSize = (remainingBytes > ZLIB_CHUNK_SIZE) ? ZLIB_CHUNK_SIZE : remainingBytes;
		bool isLastRound = (remainingBytes <= ZLIB_CHUNK_SIZE);
		stream.avail_in = static_cast<uInt>(nextChunckSize);
		stream.next_in = reinterpret_cast<Bytef*>(&inputData[i]);
		do {
			outputBuffer.reserve(outputBuffer.size() + ZLIB_CHUNK_SIZE);
			stream.avail_out = ZLIB_CHUNK_SIZE;
			stream.next_out = reinterpret_cast<Bytef*>(&outputBuffer.getModifiableData()[outputBuffer.size()]);
			if(deflate(&stream, (autoFlush && isLastRound) ? Z_SYNC_FLUSH : Z_NO_FLUSH) == Z_STREAM_ERROR) {
				throw gzip_error("Error while deflating a chunck!");
			}
			outputBuffer.manuallySetSize(outputBuffer.size() + ZLIB_CHUNK_SIZE - stream.avail_out);
		} while(stream.avail_out == 0);
	}
	Bytef inputDecoy;
	stream.avail_in = 0;
	stream.next_in = &inputDecoy;
	do {
		outputBuffer.reserve(outputBuffer.size() + ZLIB_CHUNK_SIZE);
		stream.avail_out = ZLIB_CHUNK_SIZE;
		stream.next_out = reinterpret_cast<Bytef*>(&outputBuffer.getModifiableData()[outputBuffer.size()]);
		if(deflate(&stream, Z_FINISH) == Z_STREAM_ERROR) {
			throw gzip_error("Error while deflating a chunck!");
		}
		outputBuffer.manuallySetSize(outputBuffer.size() + ZLIB_CHUNK_SIZE - stream.avail_out);
	} while(stream.avail_out == 0);
	outputSize = outputBuffer.size();
	return outputBuffer.extractAndClear();
}

//############################ GZipDecompressor ############################

GZipDecompressor::GZipDecompressor() : stream() {
	stream.zalloc = Z_NULL;
	stream.zfree = Z_NULL;
	stream.opaque = Z_NULL;
	stream.avail_in = 0;
	stream.next_in = Z_NULL;
	if(inflateInit2(&stream, 31) != Z_OK) {
		throw gzip_error("Failed to initilize the inflater!");
	}
}

GZipDecompressor::~GZipDecompressor() {
	inflateEnd(&stream);
}

int8_t* GZipDecompressor::decompress(int8_t* inputData, uint64_t inputSize, uint64_t& outputSize) {
	ArrayList<int8_t> outputBuffer;
	for(uint64_t i = 0;i < inputSize;i += ZLIB_CHUNK_SIZE) {
		uint64_t remainingBytes = inputSize - i;
		uint64_t nextChunckSize = (remainingBytes > ZLIB_CHUNK_SIZE) ? ZLIB_CHUNK_SIZE : remainingBytes;
		stream.avail_in = static_cast<uInt>(nextChunckSize);
		stream.next_in = reinterpret_cast<Bytef*>(&inputData[i]);
		if(stream.avail_in == 0) {
			break;
		}
		do {
			outputBuffer.reserve(outputBuffer.size() + ZLIB_CHUNK_SIZE);
			stream.avail_out = ZLIB_CHUNK_SIZE;
			stream.next_out = reinterpret_cast<Bytef*>(&outputBuffer.getModifiableData()[outputBuffer.size()]);
			if(inflate(&stream, Z_NO_FLUSH) == Z_STREAM_ERROR) {
				throw gzip_error("Error while inflating a chunck!");
			}
			outputBuffer.manuallySetSize(outputBuffer.size() + ZLIB_CHUNK_SIZE - stream.avail_out);
		} while(stream.avail_out == 0);
	}
	outputSize = outputBuffer.size();
	return outputBuffer.extractAndClear();
}