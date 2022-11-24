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
#include "Profiler/Profiler/Profiler.h"

#include <iostream>
#include <fstream>

#include <stdio.h>
#include <Windows.h>

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

void main()
{
	loggerInit("level test, Warning", logLevel::Warning);
	for (;;)
	{
		loggingLevelTest();
	}
	Sleep(110);
	
}