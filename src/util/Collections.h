// Author: https://github.com/AMXerSurf
#pragma once

#include <stdexcept>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <functional>
#include <initializer_list>

namespace AEX {

	class collections_error : public std::runtime_error {
		private:
		protected:
		public:
			collections_error(const std::string& message) : runtime_error(message) {}
	};

	template<typename T>
	class ArrayList {
		public:
			static const uint32_t DEFAULT_INITIAL_SIZE = 10u;
			//tested - against, low to high
			using BinarySearchFunction = int32_t(*)(const T& tested, const T& against);
		private:
			T* data;
			uint32_t dataSize;
			uint32_t currentSize;
			typedef T* iterator;
			typedef const T* const_iterator;

			void resize(uint32_t index) noexcept {
				if(!data) {
					dataSize = DEFAULT_INITIAL_SIZE;
					while(dataSize <= index) {
						dataSize *= 2u;
					}
					data = new T[dataSize];
				} else if(index >= dataSize) {
					uint32_t oldSize = dataSize;
					while(dataSize <= index) {
						dataSize *= 2u;
					}
					T* oldData = data;
					data = new T[dataSize];
					for(uint32_t i = 0u;i < oldSize;i++) {
						data[i] = std::move(oldData[i]);
					}
					delete[] oldData;
				}
			}
		protected:
		public:
			ArrayList(uint32_t initalSize = 0u) noexcept : data(nullptr), dataSize(0u), currentSize(0u) {
				if(initalSize > 0) {
					resize(initalSize - 1);
				}
			}

			ArrayList(std::initializer_list<T> list) noexcept : data(nullptr), dataSize(0u), currentSize(0u) {
				uint32_t elementCount = static_cast<uint32_t>(list.size());
				if(elementCount > 0) {
					resize(elementCount - 1u);
				}
				for(auto& item : list) {
					add(item);
				}
			}

			//Copy constructor
			ArrayList(const ArrayList<T>& list) noexcept : data(nullptr), dataSize(0u), currentSize(0u) {
				if(list.currentSize > 0u) {
					data = new T[list.dataSize];
					for(uint32_t i = 0u;i < list.currentSize;i++) {
						data[i] = list.data[i];
					}
					dataSize = list.dataSize;
					currentSize = list.currentSize;
				}
			}

			//Move constructor
			ArrayList(ArrayList<T>&& list) noexcept : data(nullptr), dataSize(0u), currentSize(0u) {
				data = list.data;
				list.data = nullptr;
				dataSize = list.dataSize;
				list.dataSize = 0u;
				currentSize = list.currentSize;
				list.currentSize = 0u;
			}

			~ArrayList() noexcept {
				if(data) {
					delete[] data;
				}
			}

			void reserve(uint32_t size) noexcept {
				if(size > dataSize) {
					resize(size - 1u);
				}
			}

			const T* getData() const noexcept {
				return data;
			}

			T* getModifiableData() noexcept {
				return data;
			}

			T* toArray(T* array = nullptr, uint32_t arraySize = UINT32_MAX) noexcept {
				if(currentSize > 0u) {
					if(arraySize == UINT32_MAX) {
						arraySize = currentSize;
					}
					arraySize = std::min(arraySize, currentSize);
					if(array == nullptr) {
						array = new T[arraySize];
					}
					for(uint32_t i = 0u;i < arraySize;i++) {
						array[i] = data[i];
					}
				}
				return array;
			}

			void add(const T& element) noexcept {
				if(!data || currentSize >= dataSize) {
					resize(currentSize);
				}
				data[currentSize++] = element;
			}

			void add(T&& element) noexcept {
				if(!data || currentSize >= dataSize) {
					resize(currentSize);
				}
				data[currentSize++] = std::forward<T>(element);
			}

			void push(const T& element) noexcept {
				add(element);
			}

			void push(T&& element) noexcept {
				add(std::forward<T>(element));
			}

			void add(uint32_t index, const T& element) noexcept {
				if(index > currentSize) {
					index = currentSize;
				}
				if(!data || currentSize >= dataSize) {
					resize(currentSize);
				}
				if(index < currentSize) {
					for(uint32_t i = currentSize;i > index;i--) {
						data[i] = std::move(data[i - 1u]);
					}
				}
				data[index] = element;
				currentSize++;
			}

			void add(uint32_t index, T&& element) noexcept {
				if(index > currentSize) {
					index = currentSize;
				}
				if(!data || currentSize >= dataSize) {
					resize(currentSize);
				}
				if(index < currentSize) {
					for(uint32_t i = currentSize;i > index;i--) {
						data[i] = std::move(data[i - 1u]);
					}
				}
				data[index] = std::forward<T>(element);
				currentSize++;
			}

			void set(uint32_t index, const T& element) noexcept {
				if(index < currentSize) {
					data[index] = element;
				}
			}

			void set(uint32_t index, T&& element) noexcept {
				if(index < currentSize) {
					data[index] = std::forward<T>(element);
				}
			}

			uint32_t binarySearch(const T& element, BinarySearchFunction binarySearchFunction) noexcept {
				if(currentSize == 0u) {
					return 0u;
				}
				//uint32_t wont work because it can not be negative
				//Start / End / Middle can become negative when the element is to be inserted at first place
				//int64_t to fit all values an uint32_t can fit
				//int32_t wont work because its fits less positive values than a uint32_t
				int64_t start = 0;
				int64_t end = static_cast<int64_t>(currentSize - 1u);
				int64_t middle = 0;
				bool toStart = false;
				while(start <= end) {
					middle = (end - start) / 2 + start;
					const T& middleElement = data[middle];
					int32_t comparisonResult = binarySearchFunction(element, middleElement);
					if(comparisonResult > 0) {
						start = middle + 1;
						toStart = false;
					} else if(comparisonResult < 0) {
						end = middle - 1;
						toStart = true;
					} else {
						return static_cast<uint32_t>(middle);
					}
				}
				if(toStart) {
					return static_cast<uint32_t>(middle);
				} else {
					return static_cast<uint32_t>(middle + 1);
				}
			}

			void addOrdered(const T& element, BinarySearchFunction binarySearchFunction) noexcept {
				if(!data || currentSize >= dataSize) {
					resize(currentSize);
				}
				add(binarySearch(element, binarySearchFunction), element);
			}

			void addOrdered(T&& element, BinarySearchFunction binarySearchFunction) noexcept {
				if(!data || currentSize >= dataSize) {
					resize(currentSize);
				}
				add(binarySearch(element, binarySearchFunction), element);
			}

			void addAll(const ArrayList<T>& list) noexcept {
				if(list.currentSize < 1u) {
					return;
				}
				if(!data || (currentSize - 1u + list.currentSize) >= dataSize) {
					resize(currentSize - 1u + list.currentSize);
				}
				for(uint32_t i = 0u;i < list.currentSize;i++) {
					data[currentSize++] = list[i];
				}
			}

			bool contains(const T& element) const noexcept {
				for(uint32_t i = 0u;i < currentSize;i++) {
					if(data[i] == element) {
						return true;
					}
				}
				return false;
			}

			bool containsAll(const ArrayList<T>& list) const noexcept {
				for(const T& element : list) {
					if(!contains(element)) {
						return false;
					}
				}
				return true;
			}

			uint32_t size() const noexcept {
				return currentSize;
			}

			//This is extremly dangerous, use it only if you know what you are doing
			void manuallySetSize(uint32_t newSize) noexcept {
				currentSize = newSize;
			}

			uint32_t capacity() const noexcept {
				return dataSize;
			}

			T& top() noexcept {
				return get(currentSize - 1u);
			}

			const T& top() const noexcept {
				return get(currentSize - 1u);
			}

			T& bottom() {
				return get(0u);
			}

			const T& bottom() const noexcept {
				return get(0u);
			}

			void clear(bool freeMemory = false) noexcept {
				if(freeMemory) {
					if(data) {
						delete[] data;
						data = nullptr;
					}
					dataSize = 0u;
				}
				currentSize = 0u;
			}

			T* extractAndClear() noexcept {
				T* currentData = data;
				data = nullptr;
				dataSize = 0u;
				currentSize = 0u;
				return currentData;
			}

			T removeIndex(uint32_t index, bool unorderedFastRemove = false) {
				if(currentSize > 0u) {
					if(index == currentSize - 1u) {
						currentSize--;
						return data[index];
					} else if(index < currentSize - 1u) {
						currentSize--;
						T result = data[index];
						if(unorderedFastRemove) {
							data[index] = std::move(data[currentSize]);
						} else {
							for(uint32_t i = index;i < currentSize;i++) {
								data[i] = std::move(data[i + 1u]);
							}
						}
						return result;
					}
				}
				throw collections_error("Invalid index");
			}

			bool removeElement(const T& element, bool unorderedFastRemove = false) {
				for(uint32_t i = 0u;i < currentSize;i++) {
					if(data[i] == element) {
						removeIndex(i, unorderedFastRemove);
						return true;
					}
				}
				return false;
			}

			bool removeElement(const T& element, uint32_t& removedIndex, bool unorderedFastRemove = false) {
				for(uint32_t i = 0u;i < currentSize;i++) {
					if(data[i] == element) {
						removeIndex(i, unorderedFastRemove);
						removedIndex = i;
						return true;
					}
				}
				return false;
			}

			bool removeAll(const ArrayList<T>& list, bool unorderedFastRemove = false) {
				bool result = false;
				for(uint32_t i = 0u;i < list.currentSize;i++) {
					result |= removeElement(list[i], unorderedFastRemove);
				}
				return result;
			}

			bool removeAll(const T& element, bool unorderedFastRemove = false) {
				bool result = false;
				for(uint32_t i = 0u;i < currentSize;i++) {
					if(data[i] == element) {
						removeIndex(i, unorderedFastRemove);
						result = true;
					}
				}
				return result;
			}

			T pop() {
				return removeIndex(currentSize - 1u);
			}

			T poll() {
				return removeIndex(0u);
			}

			bool isEmpty() const noexcept {
				return currentSize == 0u;
			}

			T& get(uint32_t index) noexcept {
				return data[index];
			}

			const T& get(uint32_t index) const noexcept {
				return data[index];
			}

			uint32_t indexOf(const T& element) const noexcept {
				for(uint32_t i = 0u;i < currentSize;i++) {
					if(data[i] == element) {
						return i;
					}
				}
				return UINT32_MAX;
			}

			T& operator[](uint32_t index) noexcept {
				return data[index];
			}

			const T& operator[](uint32_t index) const noexcept {
				return data[index];
			}

			iterator begin() noexcept {
				return currentSize > 0u ? &data[0u] : nullptr;
			}

			const_iterator begin() const noexcept {
				return currentSize > 0u ? &data[0u] : nullptr;
			}

			iterator end() noexcept {
				return currentSize > 0u ? (&data[currentSize - 1u]) + 1u : nullptr;
			}

			const_iterator end() const noexcept {
				return currentSize > 0u ? (&data[currentSize - 1u]) + 1u : nullptr;
			}

			//Copy assignment
			ArrayList<T>& operator=(const ArrayList<T>& list) noexcept {
				if(&list == this) {
					return *this;
				}
				if(data) {
					delete[] data;
				}
				if(list.currentSize > 0u) {
					dataSize = list.dataSize;
					currentSize = list.currentSize;
					data = new T[list.dataSize];
					for(uint32_t i = 0u;i < list.currentSize;i++) {
						data[i] = list.data[i];
					}
				} else {
					data = nullptr;
					dataSize = 0u;
					currentSize = 0u;
				}
				return *this;
			}

			//Move assignment
			ArrayList<T>& operator=(ArrayList<T>&& list) noexcept {
				if(&list == this) {
					return *this;
				}
				if(data) {
					delete[] data;
				}
				data = list.data;
				list.data = nullptr;
				dataSize = list.dataSize;
				list.dataSize = 0u;
				currentSize = list.currentSize;
				list.currentSize = 0u;
				return *this;
			}
	};

}