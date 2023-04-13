#pragma once
#include "define.h"

#include <chrono>
#include <string>

struct logStrStructure {
	std::string _time;
	std::string _where;
	std::string _level;
	std::string _message;
};

struct logMessage2 {
	logMessage2() {};
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



void logStringParser(const logMessage2& msg, logStrStructure& logStr);