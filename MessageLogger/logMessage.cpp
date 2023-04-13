#include "logMessage.h"
#include "define.h"
#include "logger.h"

#include <string>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>


int dayCounter;
unsigned long long int dayTemp;

void timeParser(const logMessage2& msg, logStrStructure& logStr, std::stringstream& ss)
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

	if (dayTemp != msTime) {
		dayTemp = msTime;
		dayCounter++;
	}

	ss << "[" << std::setw(2) << std::setfill('0') << hour + 9 <<
		":" << std::setw(2) << std::setfill('0') << min <<
		":" << std::setw(2) << std::setfill('0') << sec <<
		"." << std::setw(3) << std::setfill('0') << ms <<
		"]";

	logStr._time = ss.str();
	ss.str("");
}

void logPointParser(const logMessage2& msg, logStrStructure& logStr, std::stringstream& ss)
{
	ss << "[" << msg.logCppName << ":" << msg.line << " (threadID :" << msg.threadID << ")]";
	logStr._where = ss.str();

	ss.str("");
}

void logLevelParser(const logMessage2& msg, logStrStructure& logStr, std::stringstream& ss)
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

void logStringParser(const logMessage2& msg, logStrStructure& logStr)
{
	std::stringstream ss;

	timeParser(msg, logStr, ss);
	logPointParser(msg, logStr, ss);
	logLevelParser(msg, logStr, ss);

	logStr._message = msg.message;
}