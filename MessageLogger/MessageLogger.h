#pragma once

#include <chrono>
#include <string>
#include <iostream>
#include <queue>

#include <Windows.h>

#define LO_NONE			0x0
#define LO_TXT			0x1
#define LO_CSV			0x2
#define LO_CMD			0x4

struct	logMessage;
extern std::queue<logMessage*> logMessageQueue;

//#define LOG(...)							Logger(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__)

#ifdef _USE_LOGGER_
// const logLevel _level
// const BYTE _writeMode = LO_TXT | LO_CMD | LO_CSV
// const std::string _message
// const std::string _dir = ""
//#define LOG(...)							pushLog(new logMessage(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__));
#define LOG(...)							pushLog(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__);
#else
#define LOG(...)
#endif

enum class logLevel {
	Off,
	Fatal,
	Error,
	Warning,
	Info,
	Debug,
	All,
};



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




