// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//  are changed infrequently
//

#pragma once
// Change these values to use different versions
//#define WINVER		0x0601
#define _WIN32_WINNT	0x0A00 
//#define _WIN32_IE	0x0700
//#define _RICHEDIT_VER	0x0500

#define WIN32_LEAN_AND_MEAN 
/*
 * The defined WIN32_NO_STATUS macro disables return code definitions in
 * windows.h, which avoids "macro redefinition" MSVC warnings in ntstatus.h.
 */
#define WIN32_NO_STATUS
#include <windows.h>
#undef WIN32_NO_STATUS

#include <stdio.h>
#include <stdint.h>
#include <io.h> 
#include <fcntl.h> 
#include <ntstatus.h>
#include <bcrypt.h>
//#include <commdlg.h>
//#include <commctrl.h>
#include <stringapiset.h>
#include <uxtheme.h>
#include <vssym32.h> 

#include <atlbase.h>
#include <atlapp.h>

extern CAppModule _Module;

#include <atlwin.h>

#include <wil/com.h>
#include <mutex>
#include <cmath>

#include <d2d1.h>
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib, "bcrypt.lib")
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "Uxtheme.lib")
#pragma comment(lib, "Delayimp.lib")

#if defined _M_IX86
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
  #pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
