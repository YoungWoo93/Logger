#pragma once
#include <sstream>
#include <string>

#define LO_NONE          0x00
#define LO_TXT           0x01
#define LO_CSV           0x02
#define LO_DBS           0x04
#define LO_CMD           0x08

enum class logLevel {
    Off,
    Fatal,
    Error,
    Warning,
    Info,
    Debug,
    All,
};

// const logLevel _level
// const BYTE _writeMode = LO_TXT | LO_CMD | LO_CSV | LO_DBS
// const std::string _message
// const std::string _dir = ""

#define LOG2(...)							writeReq(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__);
#define LOG(...)                            writeReq(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __VA_ARGS__);

#define LOGOUT(level, mode)                 do { logLevel __level = level; char __MODE = mode;  std::string __FILENAME = ""; std::stringstream __ss; __ss
#define LOGOUT_EX(level, mode, fileName)    do {logLevel __level = level; char __MODE = mode;  std::string __FILENAME = fileName; std::stringstream __ss; __ss

#define LOGEND                              ""; writeReq(strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__, __LINE__, __level, __MODE, __ss.str(), __FILENAME); } while(0)
