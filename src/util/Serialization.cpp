// Author: https://github.com/AMXerSurf
#include "Serialization.h"

using namespace AEX;

//############################ serialization_error ############################

serialization_error::serialization_error(const std::string& message) : runtime_error(message) {}

//############################ Serializer ############################

void Serializer::checkSize(uint64_t itemSize) {
	if(serializedData.size() + itemSize > limit) {
		throw serialization_error("Out of space! Requested size: " + std::to_string(serializedData.size() + itemSize) + " | Limit: " + std::to_string(limit));
	}
}

Serializer::Serializer() : serializedData(), limit(-1) {}

Serializer::Serializer(uint64_t limit) : serializedData(static_cast<uint32_t>(limit)), limit(limit) {}

Serializer::Serializer(Serializer&& serializer) noexcept {
	serializedData = std::move(serializer.serializedData);
	limit = serializer.limit;
}

void Serializer::serialize(uint8_t value) {
	checkSize(1);
	serializedData.add(*reinterpret_cast<const int8_t*>(&value));
}

void Serializer::serialize(int8_t value) {
	checkSize(1);
	serializedData.add(value);
}

void Serializer::serialize(uint16_t value) {
	checkSize(2);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
}

void Serializer::serialize(int16_t value) {
	checkSize(2);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
}

void Serializer::serialize(uint32_t value) {
	checkSize(4);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
	serializedData.add(data[2]);
	serializedData.add(data[3]);
}

void Serializer::serialize(int32_t value) {
	checkSize(4);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
	serializedData.add(data[2]);
	serializedData.add(data[3]);
}

void Serializer::serialize(uint64_t value) {
	checkSize(8);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
	serializedData.add(data[2]);
	serializedData.add(data[3]);
	serializedData.add(data[4]);
	serializedData.add(data[5]);
	serializedData.add(data[6]);
	serializedData.add(data[7]);
}

void Serializer::serialize(int64_t value) {
	checkSize(8);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
	serializedData.add(data[2]);
	serializedData.add(data[3]);
	serializedData.add(data[4]);
	serializedData.add(data[5]);
	serializedData.add(data[6]);
	serializedData.add(data[7]);
}

void Serializer::serialize(float value) {
	checkSize(4);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
	serializedData.add(data[2]);
	serializedData.add(data[3]);
}

void Serializer::serialize(double value) {
	checkSize(8);
	const int8_t* data = reinterpret_cast<const int8_t*>(&value);
	serializedData.add(data[0]);
	serializedData.add(data[1]);
	serializedData.add(data[2]);
	serializedData.add(data[3]);
	serializedData.add(data[4]);
	serializedData.add(data[5]);
	serializedData.add(data[6]);
	serializedData.add(data[7]);
}

void Serializer::serialize(bool value) {
	checkSize(1);
	serializedData.add(static_cast<int8_t>(value));
}

void Serializer::serialize(const std::string& value) {
	uint64_t size = static_cast<uint64_t>(value.length()) + 1;
	checkSize(size);
	const int8_t* data = reinterpret_cast<const int8_t*>(value.c_str());
	for(uint64_t i = 0;i < size;i++) {
		serializedData.add(data[i]);
	}
}

void Serializer::serialize(int8_t const* data, uint64_t size) {
	checkSize(size);
	for(uint64_t i = 0;i < size;i++) {
		serializedData.add(data[i]);
	}
}

void Serializer::reset() {
	serializedData.clear();
}

int8_t* Serializer::getSerializedData() {
	return serializedData.getModifiableData();
}

int8_t* Serializer::extractSerializedData() {
	return serializedData.extractAndClear();
}

uint64_t Serializer::getDataLength() const {
	return serializedData.size();
}

uint64_t Serializer::getLimit() const {
	return limit;
}

Serializer& Serializer::operator=(Serializer&& serializer) noexcept {
	serializedData = std::move(serializer.serializedData);
	limit = serializer.limit;
	return *this;
}

//############################ Deserializer ############################

void Deserializer::checkSize(uint64_t itemSize) {
	if(serializedData) {
		if((currentOffset + itemSize - baseOffset) > dataLength) {
			throw serialization_error("Out of data! Requested size: " + std::to_string(currentOffset + itemSize - baseOffset) + " | Actual size: " + std::to_string(dataLength));
		}
	} else {
		throw serialization_error("No data available!");
	}
}

Deserializer::Deserializer(bool cleanupData) : serializedData(nullptr), dataLength(0), currentOffset(0), baseOffset(0), cleanupData(cleanupData) {}

Deserializer::Deserializer(int8_t* serializedData, uint64_t dataLength, uint64_t baseOffset, bool cleanupData) : serializedData(serializedData), dataLength(dataLength), currentOffset(baseOffset), baseOffset(baseOffset), cleanupData(cleanupData) {}

Deserializer::Deserializer(Deserializer&& deserializer) noexcept {
	serializedData = deserializer.serializedData;
	deserializer.serializedData = nullptr;
	dataLength = deserializer.dataLength;
	deserializer.dataLength = 0;
	baseOffset = deserializer.baseOffset;
	deserializer.baseOffset = 0;
	currentOffset = deserializer.currentOffset;
	deserializer.currentOffset = 0;
	cleanupData = deserializer.cleanupData;
}

Deserializer::~Deserializer() {
	if(serializedData) {
		if(cleanupData) {
			delete[] serializedData;
		}
	}
}

void Deserializer::deserialize(uint8_t& result) {
	checkSize(1);
	result = *reinterpret_cast<uint8_t*>(&serializedData[currentOffset]);
	currentOffset += 1;
}


void Deserializer::deserialize(int8_t& result) {
	checkSize(1);
	result = serializedData[currentOffset];
	currentOffset += 1;
}

void Deserializer::deserialize(uint16_t& result) {
	checkSize(2);
	result = *reinterpret_cast<uint16_t*>(&serializedData[currentOffset]);
	currentOffset += 2;
}

void Deserializer::deserialize(int16_t& result) {
	checkSize(2);
	result = *reinterpret_cast<int16_t*>(&serializedData[currentOffset]);
	currentOffset += 2;
}

void Deserializer::deserialize(uint32_t& result) {
	checkSize(4);
	result = *reinterpret_cast<uint32_t*>(&serializedData[currentOffset]);
	currentOffset += 4;
}

void Deserializer::deserialize(int32_t& result) {
	checkSize(4);
	result = *reinterpret_cast<int32_t*>(&serializedData[currentOffset]);
	currentOffset += 4;
}

void Deserializer::deserialize(uint64_t& result) {
	checkSize(8);
	result = *reinterpret_cast<uint64_t*>(&serializedData[currentOffset]);
	currentOffset += 8;
}

void Deserializer::deserialize(int64_t& result) {
	checkSize(8);
	result = *reinterpret_cast<int64_t*>(&serializedData[currentOffset]);
	currentOffset += 8;
}

void Deserializer::deserialize(float& result) {
	checkSize(4);
	result = *reinterpret_cast<float*>(&serializedData[currentOffset]);
	currentOffset += 4;
}

void Deserializer::deserialize(double& result) {
	checkSize(8);
	result = *reinterpret_cast<double*>(&serializedData[currentOffset]);
	currentOffset += 8;
}

void Deserializer::deserialize(bool& result) {
	checkSize(1);
	result = serializedData[currentOffset] != 0;
	currentOffset += 1;
}

void Deserializer::deserialize(std::string& result) {
	if(serializedData) {
		uint64_t currentLength = 1;
		while((currentOffset + currentLength - baseOffset) <= dataLength && serializedData[currentOffset + currentLength - 1] != 0) {
			currentLength++;
		}
		if((currentOffset + currentLength - baseOffset) > dataLength) {
			throw serialization_error("Out of data! Requested size: " + std::to_string(currentOffset + currentLength - baseOffset) + " | Actual size: " + std::to_string(dataLength));
		}
		result.assign(reinterpret_cast<char*>(&serializedData[currentOffset]), currentLength - 1);
		currentOffset += currentLength;
	} else {
		throw serialization_error("No data available!");
	}
}
void Deserializer::deserialize(int8_t*& data, uint64_t size) {
	checkSize(size);
	data = &serializedData[currentOffset];
	currentOffset += size;
}

void Deserializer::setData(int8_t* serializedData, uint64_t dataLength, uint64_t baseOffset) {
	if(this->serializedData) {
		if(cleanupData) {
			delete[] this->serializedData;
		}
	}
	this->serializedData = serializedData;
	this->dataLength = dataLength;
	this->baseOffset = baseOffset;
	currentOffset = baseOffset;
}

const int8_t* Deserializer::getSerializedData() const {
	return serializedData;
}

uint64_t Deserializer::getDataLength() const {
	return dataLength;
}

uint64_t Deserializer::getCurrentOffset() const {
	return currentOffset;
}

uint64_t Deserializer::getBaseOffset() const {
	return baseOffset;
}

bool Deserializer::shouldCleanupData() const {
	return cleanupData;
}

Deserializer& Deserializer::operator=(Deserializer&& deserializer) noexcept {
	serializedData = deserializer.serializedData;
	deserializer.serializedData = nullptr;
	dataLength = deserializer.dataLength;
	deserializer.dataLength = 0;
	baseOffset = deserializer.baseOffset;
	deserializer.baseOffset = 0;
	currentOffset = deserializer.currentOffset;
	deserializer.currentOffset = 0;
	cleanupData = deserializer.cleanupData;
	return *this;
}
