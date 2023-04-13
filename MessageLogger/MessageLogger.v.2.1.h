#pragma once
#include "define.h"

#include <chrono>
#include <string>
#include <mutex>
#include <sstream>
#include <Windows.h>





struct logMessage3;



void			writeReq3(const std::string& _logCppName, const int _line, const logLevel _level, const BYTE _writeMode = LO_TXT, const std::string _logWriteFileName = "", const std::string _message = "");
void			logging3(logMessage3* msg);



struct logMessage3 {
	logMessage3() {};
	logMessage3(const std::string& _logCppName, const int _line, const logLevel _level,
		const BYTE _writeMode = LO_TXT, const std::string _message = "", const std::string _logWriteFileName = "")
		:level(_level), writeMode(_writeMode), line(_line), threadID(GetCurrentThreadId()), time(std::chrono::system_clock::now()),
		logCppName(_logCppName), message(_message), logWriteFileName(_logWriteFileName) {
	}

	~logMessage3() {
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



