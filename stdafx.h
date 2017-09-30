// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>

// C 运行时头文件
#include <stdlib.h>
#include <cstdio>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <commdlg.h>
#include <io.h>
#include <direct.h>
#include <wininet.h>
#include <TlHelp32.h>
#include <shellapi.h>
#include <CommCtrl.h>
#pragma comment(lib,"Wininet.lib")
#pragma comment(lib,"comctl32.lib")

// TODO: 在此处引用程序需要的其他头文件
#include <string>
#include <fstream>