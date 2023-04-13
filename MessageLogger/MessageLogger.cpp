#include "MessageLogger.h"
#include "logger.h"
#include "define.h"

extern logger g_logger;

bool setLoggingDir(const std::string& logFileDir, logger* logger)
{
	return logger->setLoggingDir(logFileDir);
}
std::string	getLoggingDir(logger* logger)
{
	return logger->getLoggingDir();
}

//bool			setLoggingDBA(const std::string logDBA);
//std::string	getLoggingDBA();

bool setLoggingLevel(const logLevel _levelThreshold, logger* logger)
{
	return logger->setLoggingLevel(_levelThreshold);
}
logLevel getLoggingLevel(logger* logger)
{
	return logger->getLoggingLevel();
}

void loggingPause(logger* logger)
{
	logger->loggingPause();
}
void loggingResume(logger* logger)
{
	logger->loggingResume();
}

void writeReq(const std::string& _logCppName, 
	const int _line, 
	const logLevel _level, 
	const char _writeMode, 
	const std::string _message, 
	const std::string _logWriteFileName, 
	logger* logger)
{
	if ((int)logger->levelThreshold < (int)_level)
		return;

	EnterCriticalSection(&logger->messagePoolCS);
	logMessage2* msg = logger->logMessagePool.Alloc();
	LeaveCriticalSection(&logger->messagePoolCS);

	msg->level = _level;
	msg->writeMode = _writeMode;
	msg->line = _line;
	msg->threadID = GetCurrentThreadId();
	msg->time = std::chrono::system_clock::now();
	msg->logCppName = _logCppName;
	msg->message = _message;
	msg->logWriteFileName = _logWriteFileName;

	EnterCriticalSection(&logger->loggerCS);
	logger->logMessageQueue.push(msg);
	LeaveCriticalSection(&logger->loggerCS);

	SetEvent(logger->loggingEvent);
}

