#define _USE_LOGGER_
#define _USE_PROFILE_


#ifdef _DEBUG
#pragma comment(lib, "MessageLoggerD")
#pragma comment(lib, "ProfilerD")

#else
#pragma comment(lib, "MessageLogger")
#pragma comment(lib, "Profiler")

#endif

#include "../MessageLogger/MessageLogger.h"
#include "MessageLogger/MessageLogger/MessageLoggerLegacy.h"
#include "Profiler/Profiler/Profiler.h"

#include <iostream>
#include <fstream>

#include <stdio.h>
#include <Windows.h>
#include <thread>
using namespace std;

void asyncIOtest(const char* str)
{
	auto fileHandle = CreateFile(
		L"FILE_by_overlapped.txt",
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE,
		NULL,
		OPEN_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (fileHandle == INVALID_HANDLE_VALUE)
	{
		cout << GetLastError() << endl;
		return;
	}

	OVERLAPPED overlap = { 0 };
	overlap.hEvent = CreateEvent(NULL, true, true, NULL);
	overlap.Offset = -1;
	overlap.OffsetHigh = -1;

	WriteFile(fileHandle, str, strlen(str), NULL, &overlap);

	DWORD writeSize;
	GetOverlappedResult(fileHandle, &overlap, &writeSize, false);


}

void test1M()
{
	for (int c = 0; c < 10; c++)
	{
		cout << c << endl;
		//PROFILE_START("FILE*, fprintf");
		{
			scopeProfiler s("FILE*, fprintf");
			for (int i = 0; i < 1024; i++)
			{
				//LOG(logLevel::All, LO_TXT, "테스트 로그 ALL 레벨 " + to_string(i));
				//while (saveLog());

				FILE* f;

				fopen_s(&f, "FILE.txt", "a");
				fprintf(f, "___----------____________------------------____\n");
				fclose(f);
			}
		}
		//PROFILE_END("FILE*, fprintf");

		//PROFILE_START("textOut");
		{
			scopeProfiler s("textOut");
			for (int i = 0; i < 1024; i++)
			{
				LOG(logLevel::All, LO_TXT, to_string(i));
				//while (saveLog());
			}
		}
		//PROFILE_END("textOut");


		//PROFILE_START("std out");
		{
			scopeProfiler s("std out");
			for (int i = 0; i < 1024; i++)
			{
				LOG(logLevel::All, LO_CMD, to_string(i));
				//while (saveLog());
			}
		}
		//PROFILE_END("std out");

		//PROFILE_START("overlapped out");
		{
			scopeProfiler s("overlapped out");
			for (int i = 0; i < 1024; i++)
			{
				asyncIOtest("___----------____________------------------____\n");
			}
		}
		//PROFILE_END("overlapped out");
	}
}

void loggingLevelTest() 
{
	int i = 100;

	LOG(logLevel::All, LO_CMD | LO_TXT, to_string(i)); // +5
	LOG(logLevel::Debug, LO_CMD | LO_TXT, to_string(i)); // +0
	LOG(logLevel::Info, LO_CMD | LO_TXT, to_string(i)); // +0
	LOG(logLevel::Warning, LO_CMD | LO_TXT, to_string(i)); // +8
	LOG(logLevel::Error, LO_CMD | LO_TXT, to_string(i)); // +17
	LOG(logLevel::Fatal, LO_CMD | LO_TXT, to_string(i)); // +9
	LOG(logLevel::Off, LO_CMD | LO_TXT, to_string(i)); // -1
}



#define threadCount 8
unsigned logger1Proc(void* arg)
{
	for (int i = 0; i < 1024; i++)
	{
		LOG_LEGACY(logLevel::Fatal, LO_TXT, "test" + to_string(i) + "\t" + to_string(123));
		//LOGOUT(logLevel::Fatal, LO_TXT) << "test" << i << "\t" << 123 << LOGEND;
		
		//while (saveLog());
	}

	return 0;
}
void logger1_test()
{
	HANDLE ts[threadCount];
	for (int i = 0; i < threadCount; i++)
	{
		ts[i] = (HANDLE)_beginthreadex(nullptr, 0, logger1Proc, nullptr, CREATE_SUSPENDED, nullptr);
	}
	scopeProfiler s("lagacy logger1");
	for (int i = 0; i < threadCount; i++)
	{
		ResumeThread(ts[i]);
	}
	WaitForMultipleObjects(threadCount, ts, true, INFINITE);
	for (int i = 0; i < threadCount; i++)
	{
		CloseHandle(ts[i]);
	}
}

unsigned logger2Proc(void* arg)
{
	for (int i = 0; i < 1024; i++)
	{
		//LOG(logLevel::Fatal, LO_TXT, "test" + to_string(i) + "\t" + to_string(123));
		LOG2(logLevel::Fatal, LO_TXT, "test" + to_string(i) + "\t" + to_string(123));
		//LOGOUT(logLevel::Fatal, LO_TXT) << "test" << i << "\t" << 123 << LOGEND;
		//LOGOUT_EX(logLevel::Fatal, LO_TXT, "tergetFile") << "test" << i << "\t" << 123 << LOGEND;
	}

	return 0;
}
void logger2_test()
{
	HANDLE ts[threadCount];
	for (int i = 0; i < threadCount; i++)
	{
		ts[i] = (HANDLE)_beginthreadex(nullptr, 0, logger2Proc, nullptr, CREATE_SUSPENDED, nullptr);
	}
	scopeProfiler s("remake stream logger2");
	for (int i = 0; i < threadCount; i++)
	{
		ResumeThread(ts[i]);
	}
	WaitForMultipleObjects(threadCount, ts, true, INFINITE);
	for (int i = 0; i < threadCount; i++)
	{
		CloseHandle(ts[i]);
	}
}

unsigned logger3Proc(void* arg)
{
	for (int i = 0; i < 1024; i++)
	{
		//LOG(logLevel::Fatal, LO_TXT, "test" + to_string(i) + "\t" + to_string(123));
		//LOG2(logLevel::Fatal, LO_TXT, "test" + to_string(i) + "\t" + to_string(123));
		LOGOUT(logLevel::Fatal, LO_TXT) << "test" << i << "\t" << 123 << LOGEND;
		//LOGOUT_EX(logLevel::Fatal, LO_TXT, "tergetFile") 
		//	<< "test" 
		//	<< i 
		//	<< "\t" 
		//	<< 123 
		//	<< LOGEND;
	}

	return 0;
}
void logger3_test()
{
	HANDLE ts[threadCount];
	for (int i = 0; i < threadCount; i++)
	{
		ts[i] = (HANDLE)_beginthreadex(nullptr, 0, logger2Proc, nullptr, CREATE_SUSPENDED, nullptr);
	}
	scopeProfiler s("remake fast logger3");
	for (int i = 0; i < threadCount; i++)
	{
		ResumeThread(ts[i]);
	}
	WaitForMultipleObjects(threadCount, ts, true, INFINITE);
	for (int i = 0; i < threadCount; i++)
	{
		CloseHandle(ts[i]);
	}
}

#include <sstream>

void main()
{
	string s = "123456789123456789123456789123456789123456789123456789";
	auto c = __FILE__;
	int t = __LINE__;
	std::cout << sizeof(__FILE__);
	loggerInit("level test, Warning", logLevel::All);
	
	for (int i = 0; i < 1; i++)
	{
		logger1_test();
		logger2_test();
		Sleep(10000);
		//logger3_test();
		//Sleep(10000);
	}
	for (int i=0; i<10; i++)
	{
		cout << i << endl;
		{
			logger1_test();
		}
		Sleep(5000);
		{
			logger2_test();
		}
		Sleep(5000);
		{
			//logger3_test();
		}
		//Sleep(10000);
	}
}