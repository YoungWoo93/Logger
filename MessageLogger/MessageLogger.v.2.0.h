#pragma once
#include "define.h"

#include <chrono>
#include <string>
#include <mutex>
#include <sstream>
#include <Windows.h>








struct logMessage2 {
	logMessage2() {};
	logMessage2(const std::string& _logCppName, const int _line, const logLevel _level,
		const BYTE _writeMode = LO_TXT, const std::string _message = "", const std::string _logWriteFileName = "")
		:level(_level), writeMode(_writeMode), line(_line), threadID(GetCurrentThreadId()), time(std::chrono::system_clock::now()),
		logCppName(_logCppName), message(_message), logWriteFileName(_logWriteFileName) {
	}

	~logMessage2() {
	}


public:
	//
	// 1. �ϴܷ����� ���� ���� Ȯ���ؾ���, �׷��� �α׸� ������ ���� ���� �Ǵܰ���
	//		- level
	// 2. �״��� ��带 Ȯ���ؾ���, �׷��� ������� ���ڿ��� ������ ������
	//		- writeMode
	// 3. �״��� ���ڿ��� ��������, CMD �ϰ�� �׳� message��, text, (CSV, DBS) �ϰ�쿡�� �׿� �°�
	//		-���ڿ� ����µ� �ʿ��� ����
	//			- logCppName(CPP ���ϸ�)
	//			- line
	//			- time
	//			- threadID
	//			- �޽���
	// 
	// 4. ���������� file path �� ��������, ���Ͽ� ���� ����ϰ�쿡�� �ش�
	//
	logLevel								level;				//	4

	unsigned char							writeMode;			//	1 (4 + 1)

	unsigned int							line;				//	4
	unsigned int							threadID;			//	4
	std::chrono::system_clock::time_point	time;				//	8
	std::string								logCppName;			//	32
	std::string								message;			//	32

	std::string								logWriteFileName;	//	32
	//										DB ������ ���� �ĺ���
};

bool			setLoggingDir(const std::string logFileDir);
std::string		getLoggingDir();

//bool			setLoggingDBA(const std::string logDBA);
//std::string	getLoggingDBA();

bool			setLoggingLevel(const logLevel _levelThreshold);
logLevel		getLoggingLevel();

void			loggingPause();
void			loggingResume();

void			writeReq(const std::string& _logCppName, const int _line, const logLevel _level, const char _writeMode = LO_TXT, const std::string _message = "", const std::string _logWriteFileName = "");
void			logging(logMessage2* msg);
