#pragma once
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")

#include <string>
#include <vector>
#include <mutex>

class SocPkg
{
private:
	int dataLength;
	int _dataLength;
	std::string data;
	std::vector<char> recvVec;
	std::vector<char> _data;
	const static int HEADER_SIZE = 8;
	int Ptr;
	int headerCnt;
	int dataCnt;
	char* header;
	bool readState;// 0: read header state; 1: read data state
	std::mutex locker;
public:
	const static int BUFFER_SIZE = 2048;
	char *buffer;
	long recvBufferSize;
	SOCKET _socket;
	int SocNum;

private:
	void _init()
	{
		buffer = new char[BUFFER_SIZE + 1];
		recvBufferSize = 0;
		
		dataLength = 0;
		_dataLength = 0;
		data = "";
		recvVec.reserve(4096000);
		Ptr = 0;
		headerCnt = 0;
		dataCnt = 0;
		header = new char[HEADER_SIZE + 1];
		memset(header, '\0', HEADER_SIZE + 1);
		readState = false;
	}

	bool CheckHeader()
	{
		if (header[0] == 'S' && header[1] == 'T' && header[6] == 'E' && header[7] == 'D')
		{
			char* getLength = new char[HEADER_SIZE - 4 + 1];
			memcpy(getLength, header + 2, sizeof(char) * (HEADER_SIZE - 4));
			getLength[HEADER_SIZE - 4] = '\0';
			_dataLength = std::strtol(getLength, NULL, 16) - HEADER_SIZE;
			_data = std::vector<char>(_dataLength + 1, '\0');
			recvVec.reserve(_dataLength + 1);
			return true;
		}
		return false;
	}

	bool GetHeader()
	{
		while (Ptr < recvBufferSize)
		{
			headerCnt--;
			if (headerCnt >= 0)
				header[(HEADER_SIZE - 1) - headerCnt] = buffer[Ptr++];// Write to header
			if (headerCnt == 0)
				if (CheckHeader())
					return true;
		}
		return false;
	}

	bool GetData()
	{
		while (Ptr < recvBufferSize)
		{
			dataCnt--;
			if (dataCnt >= 0)
				_data[(_dataLength - 1) - dataCnt] = buffer[Ptr++];// Write to _data
			if (dataCnt == 0)
				return true;
		}
		return false;
	}

public:
	SocPkg(SOCKET socket, int num = -1) : _socket(socket), SocNum(num) { _init(); }

	void DataDecode()
	{
		Ptr = 0;
		while (Ptr < recvBufferSize)
			if (!readState)// Start at correct header
			{
				headerCnt = headerCnt == 0 ? HEADER_SIZE : headerCnt;
				if (GetHeader())
					readState = true;// Set to read data mode
			}
			else
			{
				dataCnt = dataCnt == 0 ? _dataLength : dataCnt;
				if (GetData())
				{
					readState = false;// Set to read header mode
					data = std::string(_data.begin(), _data.end());
					recvVec.clear();
					for (int i = 0; i < _dataLength; i++)
						recvVec.push_back(_data[i]);
					dataLength = _dataLength;
				}
			}
	}

	void getDataChr(char* outChr)
	{
		locker.lock();
		outChr = new char[this->dataLength + 1];
		strcpy(outChr, data.c_str());
		locker.unlock();
	}

	void getDataStr(std::string& strOut)
	{
		locker.lock();
		strOut = data;
		locker.unlock();
	}

	void getDataVec(std::vector<char>& outVec)
	{
		locker.lock();
		outVec = recvVec;
		locker.unlock();
	}

	void getDataLen(int& size)
	{
		locker.lock();
		size = dataLength;
		locker.unlock();
	}
};