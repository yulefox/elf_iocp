#ifndef ELF_DEF_WINCRT_H
#define ELF_DEF_WINCRT_H

#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN		// 从 Windows 头中排除极少使用的资料
#endif

#ifndef _WIN32_WINNT		// 允许使用特定于 Windows XP 或更高版本的功能。
#define _WIN32_WINNT 0x0501	// 将此值更改为相应的值，以适用于 Windows 的其他版本。
#endif

// Windows 头文件
#include <objbase.h>
#include <mmsystem.h>
#include <windows.h>
#include <Commdlg.h>
#include <commctrl.h>

// C 运行时头文件
#include <assert.h>
#include <fcntl.h>
#include <process.h>
#include <malloc.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <time.h>
#include <crtdbg.h>
#include <io.h>

#endif /* !ELF_DEF_WINCRT_H */
