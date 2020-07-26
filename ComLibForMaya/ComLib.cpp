#include "ComLib.h"

ComLib::ComLib(const std::string& fileMapName, const size_t& buffSize, TYPE type) {
	mType = type;

	mSize = buffSize << 20; //Converts from Megabytes to bytes

	hFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,			//Memory not associated with an existing file
		NULL,							//Default
		PAGE_READWRITE,					//Read/write access to PageFile
		(DWORD)0,
		mSize + sizeof(size_t) * 2,		//Max size of object (added space for the head and tail pointers)
		(LPCWSTR)fileMapName.c_str());	//Name of shared memory

	if (hFileMap == NULL) {
		printf("Error! \n");
		exit(EXIT_FAILURE);
	}
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		//printf("The file mapping already exists! \n"); //Debug
	}

	mData = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, mSize); //mData always points to beginning of data
	mCircBuffer = (char*)mData;	//Buffer is cast to char* and initialized to beginning of data

	mHead = (size_t*)mCircBuffer;		//Initialize head, first in the buffer
	mTail = ((size_t*)mCircBuffer + 1);	//Initialize tail, 8 bytes forward in the buffer

	if (type == PRODUCER) {
		//The head and the tail are stored first in memory and therefore this offset has to be considered when initialized
		*mHead = sizeof(size_t) * 2;
		*mTail = sizeof(size_t) * 2;
	}
}

ComLib::~ComLib() {
	ReleaseMutex(hMutex);
	UnmapViewOfFile((LPCVOID)mData);
	CloseHandle(hFileMap);
}

bool ComLib::send(const void* msg, const size_t length) {
	size_t msgSize = length + sizeof(Header);

	size_t padding = 0;
	if (msgSize % 64 != 0) { //If the mesage size is not a multiple of 64, then add padding
		size_t multiplier = (size_t)ceil((msgSize / 64.0f)); //See how many times it can be divided by 64, rounds upwards so it'll always be at least 1
		padding = (64 * multiplier) - msgSize;
		msgSize += padding;
	}

	if (getFreeMemory() > msgSize) { //Check that there is space in the buffer for the message
		WaitForSingleObject(hMutex, INFINITE); //Lock mutex

		Header header = { length }; //Save neccessary information for the consumer into a header
		memcpy(mCircBuffer + *mHead, &header, sizeof(Header));

		memcpy(mCircBuffer + *mHead + sizeof(Header), msg, length); //Copy the message (only) and put it after the header

		*mHead += msgSize;

		ReleaseMutex(hMutex);
		return true;
	}
	else if (*mHead == *mTail && msgSize < *mHead) {
		Header header = { length }; //We still need to leave a header
		memcpy(mCircBuffer + *mHead, &header, sizeof(Header));

		*mHead = sizeof(size_t) * 2; //Then reset the pointer to the beginning of memory
		ReleaseMutex(hMutex);
		return false;
	}

	return false; //If there is no space for the message, it will return false. Wait for consumer to read.
}

bool ComLib::recv(char* msg, size_t& length) {
	if (*mTail != *mHead && length > 0) {
		WaitForSingleObject(hMutex, INFINITE); //Lock mutex

		size_t msgSize = length + sizeof(Header);

		size_t padding = 0;
		if (msgSize % 64 != 0) { //If the mesage size is not a multiple of 64, then add padding
			size_t multiplier = (size_t)ceil((msgSize / 64.0f)); //See how many times it can be divided by 64, rounds upwards so it'll always be at least 1
			padding = (64 * multiplier) - msgSize;
			msgSize += padding;
		}

		if (mSize - *mTail < msgSize) { //The message is bigger than the remaining space in the buffer.
			*mTail = sizeof(size_t) * 2; //Reset the pointer to the beginning of memory. The message is located here instead since it didn't fit.
			ReleaseMutex(hMutex);
			return false;
		}

		memcpy(msg, mCircBuffer + *mTail + sizeof(Header), length); //Copy the message (only). It comes after the header

		*mTail += msgSize;

		ReleaseMutex(hMutex);
		return true;
	}

	return false; //Message is either length 0 or the producer has to write more messages
}

size_t ComLib::nextSize() {
	if (*mTail != *mHead) {
		Header* header = (Header*)(mCircBuffer + *mTail);
		return header->msgSize;
	}
	else {
		return 0;
	}
}

size_t ComLib::getSizeBytes() const {
	return this->mSize;
}

size_t ComLib::getFreeMemory() { //The math differs depending on where the head and tail are positioned in relation to each other.
	size_t freeMemory;
	if (*mHead > *mTail) { //Head is in front of tail
		//The whole memory minus as far as the head has written gives the remaining memory.
		//This is assuming we won't cut messages. If the whole message won't fit into the remaining buffer, we say that there is no space for it.
		freeMemory = (mSize - *mHead /*+ *mTail*/) - (sizeof(size_t) * 2);
	}
	else if (*mHead < *mTail) { //Head is behind tail
		//The tail position minus the head position in the buffer gives the remaining memory (basically the gap between them).
		freeMemory = (*mTail - *mHead) - (sizeof(size_t) * 2);
	}
	else { //The head and the tail are in the same place.
		if (*mHead == sizeof(size_t) * 2) {
			freeMemory = mSize; //The buffer is empty
		}
		else if (*mHead == mSize) {
			freeMemory = 0; //The buffer is full
		}
		else {
			freeMemory = (mSize - *mHead) - (sizeof(size_t) * 2); //There's still space left in the buffer
		}
	}

	return freeMemory;
}
