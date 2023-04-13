#include "define.h"
#include "logger.h"
#include "logMessage.h"
#include "MessageLogger.h"

#include <Windows.h>
#include <deque>
#include <string>
#include <chrono>
#include <sstream>
#include <iostream>
#include <iomanip>

#include "MemoryPool/MemoryPool/MemoryPool.h"
#include "customDataStructure/customDataStructure/queue_MemoryRecycle.h"
#include "customDataStructure/customDataStructure/queue_Sync.h"
#ifdef _DEBUG
#pragma comment(lib, "MemoryPoolD")

#else
#pragma comment(lib, "MemoryPool")

#endif

using namespace std;




#include <fstream>
#include <chrono>




















