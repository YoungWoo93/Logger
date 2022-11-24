#include "messageLogger.h"

#include <chrono>
#include <string>
#include <queue>
#include <iostream>
#include <fstream>

#include <Windows.h>


std::queue<logMessage*> logMessageQueue;
std::string fileDir;
logLevel loggingLevel;
//Logger logger;

HANDLE loggingEvent;
SRWLOCK loggerQueueLock;

void loggerInit(const char* name, logLevel level, const char* dir) {
	fileDir = std::string(dir) + getLoggerFileName() + name;
	loggingLevel = level;
	InitializeSRWLock(&loggerQueueLock);
	loggingEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

	HANDLE hThread = CreateThread(NULL, 0, loggingThreadStart, NULL, 0, NULL);
	if (hThread == NULL) {
		printf("logging thread init fail\n");
		return;
	}

	CloseHandle(hThread);
}

std::ifstream::pos_type filesize(const std::string& filename)
{
	std::ifstream in(filename, std::ifstream::ate | std::ifstream::binary);
	return in.tellg();
}


static std::string timePointAsString(const std::chrono::system_clock::time_point& tp) {
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

void CSVstringify(const logMessage& msg, std::string& str)
{
	str += timePointAsString(msg.time);
	str += ",";
	str += msg.file;
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



#include <sstream>
#include <iomanip>

std::string getLoggerFileName()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::stringstream ss;
	tm t;
	localtime_s(&t, &in_time_t);
	ss << std::put_time(&t, "%Y-%m-%d %X");

	std::string ret = "[" + ss.str() + "] ";
	for (auto& c : ret)
	{
		if (c == ':')
			c = '_';
	}
	return ret;
}

std::string	logMessageParse(const logMessage& msg)
{
	std::string logging_level;
	switch (msg.level)
	{

	case logLevel::Off:
		logging_level = " OFF ";
		break;
	case logLevel::Fatal:
		logging_level = "Fatal";
		break;
	case logLevel::Error:
		logging_level = "Error";
		break;
	case logLevel::Warning:
		logging_level = "Warning";
		break;
	case logLevel::Info:
		logging_level = "Info";
		break;
	case logLevel::Debug:
		logging_level = "Debug";
		break;
	case logLevel::All:
		logging_level = " ALL ";
		break;
	}

	std::string ret = "[";
	ret += timePointAsString(msg.time) + "] [" + (msg.file + ":" + std::to_string(msg.line)) + "] [" + logging_level + "]\t" + msg.message;

	return ret;
}

void pushLog(const std::string& _file, const int _line,
	const logLevel _level, const BYTE _writeMode, 
	const std::string _message, std::string _dir)

{
	if (_level > loggingLevel) {
		return;
	}
	logMessage* msg;
	if (_dir == "")
		msg = new logMessage(_file, _line, _level, _writeMode, _message, fileDir);
	else 
		msg = new logMessage(_file, _line, _level, _writeMode, _message, _dir);

	AcquireSRWLockExclusive(&loggerQueueLock);
	logMessageQueue.push(msg);
	ReleaseSRWLockExclusive(&loggerQueueLock);

	SetEvent(loggingEvent);
}

DWORD WINAPI loggingThreadStart(LPVOID arg)
{
	for (;;) {
		logMessage* msg = nullptr;
		int ret = WaitForSingleObject(loggingEvent, INFINITE);

		if (ret != WAIT_OBJECT_0) {
			printf("logging thread Error\n\tWaitForSingleObject return : %d\n\tGetLastError : %d", ret, GetLastError());
			return 0;
		}

		AcquireSRWLockExclusive(&loggerQueueLock);
		if (!logMessageQueue.empty()) {
			msg = logMessageQueue.front();
			logMessageQueue.pop();

			if (!logMessageQueue.empty())
				SetEvent(loggingEvent);
		}
		ReleaseSRWLockExclusive(&loggerQueueLock);

		MemoryBarrier();
		if (msg == nullptr)
		{
			continue;
		}
		else {
			saveLog(msg);
			delete msg;
		}

	}
}



bool saveLog(logMessage* msg)
{
	std::ofstream logFile;
	std::string line;
	std::string message;

	if ((msg->writeMode & LO_TXT) || (msg->writeMode & LO_CMD))
		message = logMessageParse(*msg);
	if (msg->writeMode & LO_CSV)
		CSVstringify(*msg, line);

	if (msg->writeMode & LO_TXT)
	{
		std::string fileName(msg->dir + ".txt");
		//if (filesize(msg->dir + ".txt") >= 1000000000)
		//{
		//	int count = 1;
		//	while (true) {
		//		if (filesize(msg->dir + "_" + std::to_string(count) + ".txt") < 1000000000)
		//		{
		//			fileName = msg->dir + "_" + std::to_string(count) + ".txt";
		//			break;
		//		}
		//		count++;
		//	}
		//}

		//logFile.open(fileName, std::ofstream::out | std::ofstream::app);
		//logFile << message << std::endl;
		//logFile.close();

		FILE* f;

		fopen_s(&f, fileName.c_str(), "a");
		fprintf(f, message.c_str());
		fprintf(f, "\n");
		fclose(f);
	}
	if (msg->writeMode & LO_CSV)
	{
		std::string fileName(msg->dir + ".csv");
		if (filesize(msg->dir + ".csv") >= 1000000000)
		{
			int count = 1;
			while (true) {
				if (filesize(msg->dir + "_" + std::to_string(count) + ".csv") < 1000000000)
				{
					fileName = msg->dir + "_" + std::to_string(count) + ".csv";
					break;
				}
				count++;
			}
		}

		logFile.open(fileName, std::ofstream::out | std::ofstream::app);
		logFile << line << std::endl;
		logFile.close();
	}
	if (msg->writeMode & LO_CMD)
	{
		std::cout << message << std::endl;
	}

	return true;
}

#undef _CRT_SECURE_NO_WARNINGS