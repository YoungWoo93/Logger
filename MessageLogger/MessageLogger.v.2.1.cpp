#include "MessageLogger.v.2.1.h"

#include <Windows.h>
#include <deque>
#include <string>
#include <chrono>
#include <sstream>
#include <iostream>

#include "MemoryPool/MemoryPool/MemoryPool.h"
#include "customDataStructure/customDataStructure/queue_MemoryRecycle.h"
#include "customDataStructure/customDataStructure/queue_Sync.h"
#ifdef _DEBUG
#pragma comment(lib, "MemoryPoolD")

#else
#pragma comment(lib, "MemoryPool")

#endif

using namespace std;




string logFileDir3 = "";//"test\\";
logLevel levelThreadhold3;
bool pauseFlag3;
bool runFlag3;
HANDLE hThread3;
HANDLE loggingEvent3;



queue_MemoryRecycle<logMessage3*> logMessageQueue3(100);
ObjectPool_sigle<logMessage3> logMessagePool3(100);
//ObjectPool<logMessage3> logMessagePool3(100);
CRITICAL_SECTION loggerCS3;
CRITICAL_SECTION messagePoolCS3;


unsigned loggingThreadStart3(void* arg)
{
	queue_Sync<logMessage3*> localQueue;
	logMessage3* msg = nullptr;

	while (runFlag3) {
		int ret = WaitForSingleObject(loggingEvent3, INFINITE);

		if (ret != WAIT_OBJECT_0) {
			printf("logging thread Error\n\tWaitForSingleObject return : %d\n\tGetLastError : %d", ret, GetLastError());
			return 0;
		}

		if (pauseFlag3) {
			ResetEvent(loggingEvent3);
			continue;
		}

		do {
			EnterCriticalSection(&loggerCS3);
			while (!logMessageQueue3.empty()) {
				if (!logMessageQueue3.pop(msg))
				{
					// 에러처리 필요
					break;
				}
				localQueue.push(msg);
			}
			ResetEvent(loggingEvent3);
			LeaveCriticalSection(&loggerCS3);

			while (!localQueue.empty()) {
				if (!localQueue.pop(msg))
				{
					//에러처리 필요
					break;
				}
				logging3(msg);

				EnterCriticalSection(&messagePoolCS3);
				logMessagePool3.Free(msg);
				LeaveCriticalSection(&messagePoolCS3);
			}
		} while (!logMessageQueue3.empty());
	}
}

class logger3 {
public:
	logger3() {
		runFlag3 = true;
		InitializeCriticalSection(&loggerCS3);
		InitializeCriticalSection(&messagePoolCS3);

		loggingEvent3 = CreateEventA(nullptr, true, false, "loggingEvent3");
		if (loggingEvent3 == (HANDLE)(-1))
		{
			cout << "create logging event fail, Error " << GetLastError() << endl;
			abort();
		}

		hThread3 = (HANDLE)_beginthreadex(nullptr, 0, loggingThreadStart3, nullptr, 0, nullptr);
		if (hThread3 == (HANDLE)(-1))
		{
			cout << "logger3 thread init fail, Error " << GetLastError() << endl;
			abort();
		}


	}
	~logger3() {
		runFlag3 = false;
		SetEvent(loggingEvent3);

		//Sleep(10000000);
		WaitForSingleObject(hThread3, INFINITE);
		DeleteCriticalSection(&loggerCS3);
		DeleteCriticalSection(&messagePoolCS3);
		cout << "is call?" << endl;
	}
private:
};

logger3 l3;



void			writeReq3(const std::string& _logCppName, const int _line, const logLevel _level,
	const BYTE _writeMode, const std::string _logWriteFileName, const std::string _message) {

	EnterCriticalSection(&messagePoolCS3);
	logMessage3* msg = logMessagePool3.Alloc();
	LeaveCriticalSection(&messagePoolCS3);

	msg->level = _level;
	msg->writeMode = _writeMode;
	msg->line = _line;
	msg->threadID = GetCurrentThreadId();
	msg->time = std::chrono::system_clock::now();
	msg->logCppName = _logCppName;
	msg->message = _message;
	msg->logWriteFileName = _logWriteFileName;

	EnterCriticalSection(&loggerCS3);
	logMessageQueue3.push(msg);
	LeaveCriticalSection(&loggerCS3);

	SetEvent(loggingEvent3);
}



#include <fstream>
#include <chrono>

static std::string timePointAsString3(const std::chrono::system_clock::time_point& tp) {
	unsigned long long int msTime = std::chrono::duration_cast<std::chrono::milliseconds>(tp.time_since_epoch()).count();

	char buffer[15];

	snprintf(buffer, sizeof(buffer), "%03lld\0", msTime % 1000);
	msTime /= 1000;

	snprintf(buffer + 4, sizeof(buffer), "%02lld\0", msTime % 60);
	msTime /= 60;

	snprintf(buffer + 7, sizeof(buffer), "%02lld\0", msTime % 60);
	msTime /= 60;

	snprintf(buffer + 10, sizeof(buffer), "%02lld\0", (msTime + 9) % 24);

	std::string ts = std::string(buffer + 10) + ":" + (buffer + 7) + ":" + (buffer + 4) + "." + buffer;

	return ts;
}
std::string	logMessageParse3(const logMessage3& msg)
{
	std::string ret = "[";
	ret += timePointAsString3(msg.time) + "] [" + (msg.logCppName + ":" + std::to_string(msg.line)) + " : " + to_string(msg.threadID);
	switch (msg.level)
	{
	case logLevel::Off:
		ret += "] [ OFF ]\t";
		break;
	case logLevel::Fatal:
		ret += "] [Fatal]\t";
		break;
	case logLevel::Error:
		ret += "] [Error]\t";
		break;
	case logLevel::Warning:
		ret += "] [Warning]\t";
		break;
	case logLevel::Info:
		ret += "] [Info]\t";
		break;
	case logLevel::Debug:
		ret += "] [Debug]\t";
		break;
	case logLevel::All:
		ret += " ALL ]\t";
		break;
	}

	ret += msg.message;

	return ret;
}
void CSVstringify3(const logMessage3& msg, std::string& str)
{
	str += timePointAsString3(msg.time);
	str += ",";
	str += msg.logWriteFileName;
	str += ",";
	str += std::to_string(msg.line);
	str += ",";
	str += std::to_string((int)msg.level);
	str += ",";

	if (msg.message.find(',') != std::string::npos || msg.message.find('\"') != std::string::npos)
	{
		str += '\"';
		for (auto character : msg.message)
		{
			if (character == '\"')
				str += '\"';

			str += character;
		}
		str += '\"';
		str += ",";
	}
	else
	{
		str += msg.message;
		str += ",";
	}
}

bool dirCheck3(const std::string& dir)
{
	DWORD dwAttrib = GetFileAttributesA(dir.c_str());

	if (dwAttrib == INVALID_FILE_ATTRIBUTES || !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) {
		return CreateDirectoryA(dir.c_str(), NULL) != 0;
	}

	return true;
}

void logging3(logMessage3* msg)
{
	std::ofstream logFile;
	std::string line;
	std::string message;

	if ((msg->writeMode & LO_TXT) || (msg->writeMode & LO_CMD))
		message = logMessageParse3(*msg);
	if (msg->writeMode & LO_CSV)
		CSVstringify3(*msg, line);

	if (msg->writeMode & LO_TXT)
	{
		std::string fileName(logFileDir3 + msg->logWriteFileName + ".txt");

		HANDLE logFile = CreateFileA(
			fileName.c_str(),
			GENERIC_WRITE,
			FILE_SHARE_READ,
			NULL,
			OPEN_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			NULL
		);
		if (logFile == INVALID_HANDLE_VALUE) {
			std::cerr << "Error opening file: " << GetLastError() << std::endl;
			return;
		}
		SetFilePointer(logFile, 0, NULL, FILE_END);
		DWORD bytesWritten = 0;
		message += '\n';
		if (!WriteFile(logFile, message.c_str(), message.size(), &bytesWritten, NULL)) {
			std::cerr << "Error writing to file: " << GetLastError() << std::endl;
		}
		CloseHandle(logFile);
	}
	if (msg->writeMode & LO_CSV)
	{
		std::string fileName(logFileDir3 + msg->logWriteFileName + ".csv");

		logFile.open(fileName, std::ofstream::out | std::ofstream::app);
		logFile << line << std::endl;
		logFile.close();
	}
	if (msg->writeMode & LO_CMD)
	{
		std::cout << message << std::endl;
	}
}


















