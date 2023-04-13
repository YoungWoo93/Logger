#include "logger.h"
#include "define.h"
#include "logMessage.h"
#include "MessageLogger.h"

#include <ctime>
#include <Windows.h>
#include <thread>
#include <string>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <fstream>

#include "MemoryPool/MemoryPool/MemoryPool.h"
#include "customDataStructure/customDataStructure/queue_MemoryRecycle.h"
#include "customDataStructure/customDataStructure/queue_Sync.h"

#ifdef _DEBUG
#pragma comment(lib, "MemoryPoolD")

#else
#pragma comment(lib, "MemoryPool")

#endif

logger g_logger;


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

//bool dirCheck(const string& dir)
//{
//	DWORD attribute;
//	size_t index = dir.rfind('\\', 0);
//
//	while (index != string::npos)
//	{
//		string subDir = string(dir.begin(), dir.begin() + index);
//		attribute = GetFileAttributesA(subDir.c_str());
//
//		if (attribute == INVALID_FILE_ATTRIBUTES || !(attribute & FILE_ATTRIBUTE_DIRECTORY))
//		{
//			if (CreateDirectoryA(subDir.c_str(), NULL) == 0)
//				return false;
//		}
//
//		index = dir.find('\\', index + 1);
//	}
//
//
//	attribute = GetFileAttributesA(dir.c_str());
//
//	if (attribute == INVALID_FILE_ATTRIBUTES || !(attribute & FILE_ATTRIBUTE_DIRECTORY)) {
//		return CreateDirectoryA(dir.c_str(), NULL) != 0;
//	}
//
//	return true;
//}

bool dirCheck(const string& dir)
{
	DWORD attribute = GetFileAttributesA(dir.c_str());;
	
	if (attribute == INVALID_FILE_ATTRIBUTES || !(attribute & FILE_ATTRIBUTE_DIRECTORY)) 
	{
		string subDir(dir.begin(), dir.begin() +  dir.rfind('\\'));

		if (dirCheck(subDir))
			return CreateDirectoryA(dir.c_str(), NULL) != 0;
		else
			return false;
	}

	return true;
}


unsigned loggingThread(void* arg)
{
	logger* l = (logger*)arg;
	queue_Sync<logMessage2*> localQueue;
	logMessage2* msg = nullptr;

	while (g_logger.runFlag) {
		int ret = WaitForSingleObject(g_logger.loggingEvent, INFINITE);

		if (ret != WAIT_OBJECT_0) {
			printf("logging thread Error\n\tWaitForSingleObject return : %d\n\tGetLastError : %d", ret, GetLastError());
			return 0;
		}

		if (g_logger.pauseFlag) {
			ResetEvent(g_logger.loggingEvent);
			continue;
		}

		do {
			EnterCriticalSection(&g_logger.loggerCS);
			while (!l->logMessageQueue.empty()) {
				if (!l->logMessageQueue.pop(msg))
				{
					// 에러처리 필요
					break;
				}
				localQueue.push(msg);
			}
			ResetEvent(g_logger.loggingEvent);
			LeaveCriticalSection(&g_logger.loggerCS);

			while (!localQueue.empty()) {
				if (!localQueue.pop(msg))
				{
					//에러처리 필요
					break;
				}
				g_logger.logging(*msg);

				EnterCriticalSection(&g_logger.messagePoolCS);
				l->logMessagePool.Free(msg);
				LeaveCriticalSection(&g_logger.messagePoolCS);
			}
		} while (!l->logMessageQueue.empty());
	}

	return 0;
}


logger::logger() {
	std::time_t now_c = chrono::system_clock::to_time_t(std::chrono::system_clock::now());
	tm t;
	localtime_s(&t, &now_c);
	std::stringstream dirName;
	t.tm_hour += 9;
	t.tm_mday += t.tm_hour / 24;
	t.tm_hour %= 24;

	dirName << t.tm_year + 1900 << '-'
		<< setw(2) << setfill('0') << t.tm_mon + 1 << '-'
		<< setw(2) << setfill('0') << t.tm_mday << '_'
		<< setw(2) << setfill('0') << t.tm_hour << '-'
		<< setw(2) << setfill('0') << t.tm_min << '-'
		<< setw(2) << setfill('0') << t.tm_sec;

	setLoggingDir(dirName.str());
	setLoggingLevel(logLevel::All);

	runFlag = true;
	InitializeCriticalSection(&loggerCS);
	InitializeCriticalSection(&messagePoolCS);

	loggingEvent = CreateEventA(nullptr, true, false, "loggingEvent2");
	if (loggingEvent == (HANDLE)(-1))
	{
		cout << "create logging event fail, Error " << GetLastError() << endl;
		abort();
	}

	hThread = (HANDLE)_beginthreadex(nullptr, 0, loggingThread, this, 0, nullptr);
	if (hThread == (HANDLE)(-1))
	{
		cout << "logger thread init fail, Error " << GetLastError() << endl;
		abort();
	}
}
logger::~logger() {
	runFlag = false;
	SetEvent(loggingEvent);

	WaitForSingleObject(hThread, INFINITE);
	DeleteCriticalSection(&loggerCS);
	DeleteCriticalSection(&messagePoolCS);

	std::cout << logMessagePool.GetCapacityCount() << "/" << logMessagePool.GetUseCount() << endl;
}

extern int dayCounter;
void logger::logging(const logMessage2& msg)
{
	std::ofstream logFile;
	logStrStructure LS;
	logStringParser(msg, LS);

	string loggingDir = logFileDir + "\\" + to_string(dayCounter);
	if (!dirCheck(loggingDir)) {
		std::cerr << "Error opening dir: " << GetLastError() << std::endl << loggingDir << endl;
		return;
	}

	if (msg.writeMode & LO_TXT)
	{
		std::string textLine;
		TEXTstringify(LS, textLine);
		//default Logging
		std::string defaultFileName(loggingDir + "\\defaultLog.txt");
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


		std::string fileName(loggingDir + "\\" + msg.logWriteFileName + ".txt");
		//target File logging
		if (msg.logWriteFileName != "")
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
	if (msg.writeMode & LO_CSV)
	{
		std::string csvLine;
		std::string fileName;
		CSVstringify(LS, csvLine);

		if (msg.logWriteFileName != "")
			fileName = string(loggingDir + "\\default.csv");
		else
			fileName = string(loggingDir + "\\" + msg.logWriteFileName + ".csv");

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
	if (msg.writeMode & LO_CMD)
	{
		std::string textLine;

		TEXTstringify(LS, textLine);
		std::cout << textLine.c_str();
	}
}

bool			logger::setLoggingDir(const std::string& _logFileDir) {
	char fullPath[MAX_PATH];
	_fullpath(fullPath, _logFileDir.c_str(), MAX_PATH);

	logFileDir = string(fullPath);

	return true;
}
std::string		logger::getLoggingDir() {
	return logFileDir;
}

//bool			setLoggingDBA(const std::string logDBA);
//std::string	getLoggingDBA();

bool			logger::setLoggingLevel(const logLevel _levelThreshold) {
	levelThreshold = _levelThreshold;

	return true;
}
logLevel		logger::getLoggingLevel() {
	return levelThreshold;
}

void			logger::loggingPause() {
	pauseFlag = true;
}
void			logger::loggingResume() {
	pauseFlag = false;
}