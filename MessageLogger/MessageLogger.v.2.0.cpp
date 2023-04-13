#include "MessageLogger.v.2.0.h"

#include <Windows.h>
#include <deque>
#include <string>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "MemoryPool/MemoryPool/MemoryPool.h"
#include "customDataStructure/customDataStructure/queue_MemoryRecycle.h"
#include "customDataStructure/customDataStructure/queue_Sync.h"
#ifdef _DEBUG
#pragma comment(lib, "MemoryPoolD")

#else
#pragma comment(lib, "MemoryPool")

#endif

using namespace std;




string logFileDir2 = "";
logLevel levelThreshold;
bool pauseFlag;
bool runFlag;
HANDLE hThread;
HANDLE loggingEvent2;



queue_MemoryRecycle<logMessage2*> logMessageQueue(100);
ObjectPool_sigle<logMessage2> logMessagePool(100);
//ObjectPool<logMessage2> logMessagePool(100);
CRITICAL_SECTION loggerCS;
CRITICAL_SECTION messagePoolCS;


unsigned loggingThreadStart2(void* arg)
{
	queue_Sync<logMessage2*> localQueue;
	logMessage2* msg = nullptr;

	while(runFlag) {
		int ret = WaitForSingleObject(loggingEvent2, INFINITE);
		
		if (ret != WAIT_OBJECT_0) {
			printf("logging thread Error\n\tWaitForSingleObject return : %d\n\tGetLastError : %d", ret, GetLastError());
			return 0;
		}

		if (pauseFlag) {
			ResetEvent(loggingEvent2);
			continue;
		}

		do {
			EnterCriticalSection(&loggerCS);
			while(!logMessageQueue.empty()) {
				if(!logMessageQueue.pop(msg))
				{
					// 에러처리 필요
					break;
				}
				localQueue.push(msg);
			}
			ResetEvent(loggingEvent2);
			LeaveCriticalSection(&loggerCS);

			while (!localQueue.empty()) {
				if (!localQueue.pop(msg))
				{
					//에러처리 필요
					break;
				}
				logging(msg);

				EnterCriticalSection(&messagePoolCS);
				logMessagePool.Free(msg);
				LeaveCriticalSection(&messagePoolCS);
			}
		} while (!logMessageQueue.empty());
	}
}

#include <ctime>

class logger {
public:
	logger() : dayDir(0), dayCounter(0){
		std::time_t now_c = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm t;
		localtime_s(&t, &now_c);
		std::stringstream dirName;

		dirName << t.tm_year + 1900 << '-'
			<< setw(2) << setfill('0') << t.tm_mon + 1 << '-'
			<< setw(2) << setfill('0') << t.tm_mday << '_'
			<< setw(2) << setfill('0') << t.tm_hour + 9 << '-'
			<< setw(2) << setfill('0') << t.tm_min << '-'
			<< setw(2) << setfill('0') << t.tm_sec;
		
		setLoggingDir(dirName.str());
		setLoggingLevel(logLevel::All);

		runFlag = true;
		InitializeCriticalSection(&loggerCS);
		InitializeCriticalSection(&messagePoolCS);

		loggingEvent2 = CreateEventA(nullptr, true, false, "loggingEvent2");
		if (loggingEvent2 == (HANDLE)(-1))
		{
			cout << "create logging event fail, Error " << GetLastError() << endl;
			abort();
		}

		hThread = (HANDLE)_beginthreadex(nullptr, 0, loggingThreadStart2, nullptr, 0, nullptr);
		if (hThread == (HANDLE)(-1))
		{
			cout << "logger thread init fail, Error " << GetLastError() << endl;
			abort();
		}
	}
	~logger() {
		runFlag = false;
		SetEvent(loggingEvent2);

		WaitForSingleObject(hThread, 10000);
		DeleteCriticalSection(&loggerCS);
		DeleteCriticalSection(&messagePoolCS);
	}
public:
	int dayDir;
	int dayCounter;
};

logger l;



bool			setLoggingDir(const std::string _logFileDir2) {
	char fullPath[MAX_PATH];
	_fullpath(fullPath, _logFileDir2.c_str(), MAX_PATH);

	logFileDir2 = string(fullPath);

	return true;
}
std::string		getLoggingDir() {
	return logFileDir2;
}

//bool			setLoggingDBA(const std::string logDBA);
//std::string	getLoggingDBA();

bool			setLoggingLevel(const logLevel _levelThreshold) {
	levelThreshold = _levelThreshold;

	return true;
}
logLevel		getLoggingLevel() {
	return levelThreshold;
}

void			loggingPause() {
	pauseFlag = true;
}
void			loggingResume() {
	pauseFlag = false;
}

void			writeReq(const std::string& _logCppName, const int _line, const logLevel _level,
	const BYTE _writeMode, const std::string _message, const std::string _logWriteFileName) 
{
	if ((int)levelThreshold < (int)_level)
		return;
	
	EnterCriticalSection(&messagePoolCS);
	logMessage2* msg = logMessagePool.Alloc();
	LeaveCriticalSection(&messagePoolCS);

	msg->level = _level;
	msg->writeMode = _writeMode;
	msg->line = _line;
	msg->threadID = GetCurrentThreadId();
	msg->time = std::chrono::system_clock::now();
	msg->logCppName = _logCppName;
	msg->message = _message;
	msg->logWriteFileName = _logWriteFileName;

	EnterCriticalSection(&loggerCS);
	logMessageQueue.push(msg);
	LeaveCriticalSection(&loggerCS);

	SetEvent(loggingEvent2);
}



#include <fstream>
#include <chrono>


struct logStrStructure {
	string _time;
	string _where;
	string _level;
	string _message;
};

void timeParser(const logMessage2& msg, logStrStructure& logStr, stringstream& ss);
void logPointParser(const logMessage2& msg, logStrStructure& logStr, stringstream& ss);
void logLevelParser(const logMessage2& msg, logStrStructure& logStr, stringstream& ss);
void logStringParser(const logMessage2& msg, logStrStructure& logStr)
{
	stringstream ss;

	timeParser(msg, logStr, ss);
	logPointParser(msg, logStr, ss);
	logLevelParser(msg, logStr, ss);

	logStr._message = msg.message;
}

void timeParser(const logMessage2& msg, logStrStructure& logStr, stringstream& ss)
{
	unsigned long long int msTime = std::chrono::duration_cast<std::chrono::milliseconds>(msg.time.time_since_epoch()).count();
	int ms = msTime % 1000;
	msTime /= 1000;
	int sec = msTime % 60;
	msTime /= 60;
	int min = msTime % 60;
	msTime /= 60;
	int hour = msTime % 24;
	msTime /= 24;

	if (l.dayCounter != msTime) {
		l.dayCounter = msTime;
		l.dayDir++;
	}

	ss << "[" << std::setw(2) << std::setfill('0') << hour + 9 << 
		":" << std::setw(2) << std::setfill('0') << min << 
		":" << std::setw(2) << std::setfill('0') << sec << 
		"." << std::setw(3) << std::setfill('0') << ms << 
		"]";

	logStr._time = ss.str();
	ss.str("");
}

void logPointParser(const logMessage2& msg, logStrStructure& logStr, stringstream& ss)
{
	ss << "[" << msg.logCppName << ":" << msg.line << " (threadID :" << msg.threadID << ")]";
	logStr._where = ss.str();

	ss.str("");
}

void logLevelParser(const logMessage2& msg, logStrStructure& logStr, stringstream& ss)
{
	switch (msg.level)
	{
	case logLevel::Off:
		logStr._level = "[ OFF ]";
		break;
	case logLevel::Fatal:
		logStr._level = "[Fatal]";
		break;
	case logLevel::Error:
		logStr._level = "[Error]";
		break;
	case logLevel::Warning:
		logStr._level = "[Warning]";
		break;
	case logLevel::Info:
		logStr._level = "[Info]";
		break;
	case logLevel::Debug:
		logStr._level = "[Debug]";
		break;
	case logLevel::All:
		logStr._level = "[ ALL ]";
		break;
	}
}

void TEXTstringify(const logStrStructure& logStr, std::string& str)
{
	str = logStr._time + "\t" + logStr._where + "\t" + logStr._level + "\t" + logStr._message + "\n";
}
void CSVstringify(const logStrStructure& logStr, std::string& str)
{
	string tempMessage = logStr._message;
	str = logStr._time + "," + logStr._where + "," + logStr._level + ",";

	if (tempMessage.find(',') != std::string::npos || tempMessage.find('\"') != std::string::npos)
	{
		str += '\"';
		for (auto character : tempMessage)
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
		str += tempMessage;
		str += ",";
	}
	str += "\n";
}

bool dirCheck(const string& dir) 
{
	DWORD attribute;
	int index = dir.find('\\', 0);

	while (index != string::npos)
	{
		string subDir = string(dir.begin(), dir.begin() + index);
		attribute = GetFileAttributesA(subDir.c_str());

		if (attribute == INVALID_FILE_ATTRIBUTES || !(attribute & FILE_ATTRIBUTE_DIRECTORY))
		{
			if (CreateDirectoryA(subDir.c_str(), NULL) == 0)
				return false;
		}
		
		index = dir.find('\\', index + 1);
	}


	attribute = GetFileAttributesA(dir.c_str());

	if (attribute == INVALID_FILE_ATTRIBUTES || !(attribute & FILE_ATTRIBUTE_DIRECTORY)) {
		return CreateDirectoryA(dir.c_str(), NULL) != 0;
	}

	return true;
}

void logging(logMessage2* msg)
{
	std::ofstream logFile;
	logStrStructure LS;
	logStringParser(*msg, LS);

	string loggingDir = logFileDir2 + "\\" + to_string(l.dayDir) + "\\";
	if (!dirCheck(loggingDir)) {
		std::cerr << "Error opening dir: " << GetLastError() << std::endl << loggingDir << endl;
		return;
	}

	if (msg->writeMode & LO_TXT)
	{
		std::string textLine;
		TEXTstringify(LS, textLine);
		//default Logging
		std::string defaultFileName(loggingDir + "defaultLog.txt");
		{
			HANDLE logFile = CreateFileA(
				defaultFileName.c_str(),
				GENERIC_WRITE,
				FILE_SHARE_READ,
				NULL,
				OPEN_ALWAYS,
				FILE_ATTRIBUTE_NORMAL,
				NULL
			);
			if (logFile == INVALID_HANDLE_VALUE) {
				std::cerr << "Error opening file: " << GetLastError() << std::endl;
			}

			SetFilePointer(logFile, 0, NULL, FILE_END);
			DWORD bytesWritten = 0;
			if (!WriteFile(logFile, textLine.c_str(), textLine.size(), &bytesWritten, NULL)) {
				std::cerr << "Error writing to file: " << GetLastError() << std::endl;
			}
			CloseHandle(logFile);
		}
	

		std::string fileName(loggingDir + msg->logWriteFileName + ".txt");
		//target File logging
		if(msg->logWriteFileName != "")
		{
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
			}

			SetFilePointer(logFile, 0, NULL, FILE_END);
			DWORD bytesWritten = 0;
			if (!WriteFile(logFile, textLine.c_str(), textLine.size(), &bytesWritten, NULL)) {
				std::cerr << "Error writing to file: " << GetLastError() << std::endl;
			}
			CloseHandle(logFile);
		}
	}
	if (msg->writeMode & LO_CSV)
	{
		std::string csvLine;
		CSVstringify(LS, csvLine);

		if (msg->logWriteFileName != "")
			msg->logWriteFileName = "default";

		std::string fileName(loggingDir + msg->logWriteFileName + ".csv");

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
		if (!WriteFile(logFile, csvLine.c_str(), csvLine.size(), &bytesWritten, NULL)) {
			std::cerr << "Error writing to file: " << GetLastError() << std::endl;
		}
		CloseHandle(logFile);
	}
	if (msg->writeMode & LO_CMD)
	{
		std::string textLine;

		TEXTstringify(LS, textLine);
		std::cout << textLine << std::endl;
	}
}


















