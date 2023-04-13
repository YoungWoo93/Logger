#pragma once
//#include "logger.h"
#include "define.h"
#include <string>
//#include "MessageLogger.v.1.0.h"
//#include "MessageLogger.v.2.0.h"
//#include "MessageLogger.v.2.1.h"

class logger;
extern logger g_logger;

bool			setLoggingDir(const std::string logFileDir, logger* logger = &g_logger);
std::string		getLoggingDir(logger* logger = &g_logger);

//bool			setLoggingDBA(const std::string logDBA);
//std::string	getLoggingDBA();

bool			setLoggingLevel(const logLevel _levelThreshold, logger* logger = &g_logger);
logLevel		getLoggingLevel(logger* logger = &g_logger);

void			loggingPause(logger* logger = &g_logger);;
void			loggingResume(logger* logger = &g_logger);

void			writeReq(const std::string& _logCppName, const int _line, const logLevel _level, const char _writeMode = LO_TXT, const std::string _message = "", const std::string _logWriteFileName = "", logger* logger = &g_logger);

