#pragma once
#include "define.h"

#include <chrono>
#include <string>
#include <iostream>
#include <queue>

#include <Windows.h>


struct	logMessage;
extern std::queue<logMessage*> logMessageQueue;

//#define LOG(...)							Logger(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__)


// const logLevel _level
// const BYTE _writeMode = LO_TXT | LO_CMD | LO_CSV
// const std::string _message
// const std::string _dir = ""
//#define LOG(...)							pushLog(new logMessage(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__));
#define LOG_LEGACY(...)							pushLog(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__);


std::string		getLoggerFileName();
std::string		logMessageParse(const logMessage& msg);
void			loggerInit(const char* fileName = "", logLevel level = logLevel::All, const char* dir = "");
DWORD WINAPI	loggingThreadStart(LPVOID arg);

void			pushLog(const std::string& _file, const int _line, const logLevel _level, const BYTE _writeMode, const std::string _message, const std::string _dir = "");
bool			saveLog(logMessage* msg);



//class Logger {
//public:
//	Logger()
//		: Logger(logLevel::All, LO_TXT, "") {
//		fileName = getLoggerFileName();
//	};
//
//	Logger(const logLevel _level, const BYTE _mode, const std::string& _saveDir = "")
//		:loggerLevel(_level), loggerWriteMode(_mode), loggerDir(_saveDir) {
//		fileName = getLoggerFileName();
//	};
//
//	~Logger() {
//	};
//
//	logLevel		loggerLevel;
//	BYTE			loggerWriteMode;
//	std::string		loggerDir;
//};

struct logMessage {
	logMessage(const std::string& _file, const int _line, const logLevel _level,
		const BYTE _writeMode = LO_TXT, const std::string _message = "", const std::string _dir = "")
		:time(std::chrono::system_clock::now()), file(_file), line(_line), level(_level), message(_message),
		writeMode(_writeMode), dir(_dir) {
	}

	~logMessage() {
	}

public:
	std::chrono::system_clock::time_point	time;
	std::string								file;
	size_t									line;
	logLevel								level;
	std::string								message;
	BYTE									writeMode;
	std::string								dir;
};




