// Author: https://github.com/AMXerSurf
#pragma once

#include <string>
#include <cstdint>
#include "Collections.h"
#include <stdexcept>

namespace AEX {

	class serialization_error : public std::runtime_error {
		private:
		protected:
		public:
			explicit serialization_error(const std::string& message);
	};

	class Serializer {
		private:
			ArrayList<int8_t> serializedData;
			uint64_t limit;

			void checkSize(uint64_t itemSize);
		protected:
		public:
			Serializer();
			Serializer(uint64_t limit);
			Serializer(Serializer&& serializer) noexcept;
			void serialize(uint8_t value);
			void serialize(int8_t value);
			void serialize(uint16_t value);
			void serialize(int16_t value);
			void serialize(uint32_t value);
			void serialize(int32_t value);
			void serialize(uint64_t value);
			void serialize(int64_t value);
			void serialize(float value);
			void serialize(double value);
			void serialize(bool value);
			void serialize(const std::string& value);
			void serialize(int8_t const* data, uint64_t size);
			void reset();
			int8_t* getSerializedData();
			int8_t* extractSerializedData();
			uint64_t getDataLength() const;
			uint64_t getLimit() const;
			Serializer& operator=(Serializer&& serializer) noexcept;

			template<typename T>
			void serialize(T const* value) {
				value->serialize(*this);
			}

			Serializer(const Serializer& serializer) = delete;
			Serializer& operator=(const Serializer& serializer) = delete;
	};

	class Deserializer {
		private:
			int8_t* serializedData;
			uint64_t dataLength;
			uint64_t currentOffset;
			uint64_t baseOffset;
			bool cleanupData;

			void checkSize(uint64_t itemSize);
		protected:
		public:
			Deserializer(bool cleanupData = false);
			Deserializer(int8_t* serializedData, uint64_t dataLength, uint64_t baseOffset = 0, bool cleanupData = false);
			Deserializer(Deserializer&& deserializer) noexcept;
			~Deserializer();
			void deserialize(uint8_t& result);
			void deserialize(int8_t& result);
			void deserialize(uint16_t& result);
			void deserialize(int16_t& result);
			void deserialize(uint32_t& result);
			void deserialize(int32_t& result);
			void deserialize(uint64_t& result);
			void deserialize(int64_t& result);
			void deserialize(float& result);
			void deserialize(double& result);
			void deserialize(bool& result);
			void deserialize(std::string& result);
			void deserialize(int8_t*& data, uint64_t size);
			void setData(int8_t* serializedData, uint64_t dataLength, uint64_t baseOffset);
			const int8_t* getSerializedData() const;
			uint64_t getDataLength() const;
			uint64_t getCurrentOffset() const;
			uint64_t getBaseOffset() const;
			bool shouldCleanupData() const;
			Deserializer& operator=(Deserializer&& deserializer) noexcept;

			template<typename T>
			void deserialize(T* value) {
				value->deserialize(*this);
			}

			Deserializer(const Deserializer& deserializer) = delete;
			Deserializer& operator=(const Deserializer& deserializer) = delete;
	};
}