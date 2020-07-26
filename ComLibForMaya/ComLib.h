#pragma once
#include <Windows.h>
#include <string>
#include <iostream>
#include <memory>

class ComLib {
public:
	enum TYPE {
		PRODUCER,
		CONSUMER
	};

	struct Header {
		size_t msgSize; //Size of message
	};

	ComLib(const std::string& fileMapName, const size_t& buffSize, TYPE type);
	~ComLib();

	bool send(const void* msg, const size_t length);
	bool recv(char* msg, size_t& length);
	size_t nextSize();
	size_t getSizeBytes() const;
	size_t getFreeMemory();

private:
	TYPE mType;

	HANDLE hMutex;
	HANDLE hFileMap;

	void* mData;
	char* mCircBuffer; //Circular buffer
	size_t mSize;
	size_t* mHead;
	size_t* mTail;
};