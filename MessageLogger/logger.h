#pragma once

#include "define.h"
#include "logMessage.h"

#include <string>
#include <Windows.h>

#include "customDataStructure/customDataStructure/queue_MemoryRecycle.h"
#include "MemoryPool/MemoryPool/MemoryPool.h"

class logger {
public:
	logger();
	~logger();

	void			logging(const logMessage2& msg);

	bool			setLoggingDir(const std::string& logFileDir);
	std::string		getLoggingDir();

	//bool			setLoggingDBA(const std::string logDBA);
	//std::string	getLoggingDBA();

	bool			setLoggingLevel(const logLevel _levelThreshold);
	logLevel		getLoggingLevel();

	void			loggingPause();
	void			loggingResume();


public:
	string logFileDir;
	logLevel levelThreshold;
	bool pauseFlag;
	bool runFlag;
	HANDLE hThread;
	HANDLE loggingEvent;

	queue_MemoryRecycle<logMessage2*> logMessageQueue;
	ObjectPool_sigle<logMessage2> logMessagePool;

	CRITICAL_SECTION loggerCS;
	CRITICAL_SECTION messagePoolCS;
};
