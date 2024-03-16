#ifndef __WT_WOCHATDEF_H__
#define __WT_WOCHATDEF_H__

#define SQL_STMT_MAX_LEN	256

#define	XWIN_CARET_TIMER	666

#define IDM_EDITBOX_COPY			1000
#define IDM_EDITBOX_CUT				1001
#define IDM_EDITBOX_PASTE			1002

#define DEFAULT_BACKGROUND_COLOR	0xFFF9F3F1
#define DEFAULT_SEPERATELINE_COLOR	0xFFECECEC

#define XWIN0_WIDTH		64
#define XWIN1_WIDTH		300
#define XWIN1_HEIGHT	48
#define XWIN3_WIDTH		400
#define XWIN3_HEIGHT	XWIN1_HEIGHT
#define XWIN4_HEIGHT	100
#define XWIN5_HEIGHT	160

#define XWIN_DEFAULT_COLOR			RGB(241, 243, 249)

#if 0
#define DECLARE_XWND_CLASS(WndClassName, uIcon, uIconSmall) \
static ATL::CWndClassInfo& GetWndClassInfo() \
{ \
	static ATL::CWndClassInfo wc = \
	{ \
		{ sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS, StartWindowProc, \
		  0, 0, NULL, NULL, NULL, (HBRUSH)(COLOR_WINDOW + 1), NULL, WndClassName, NULL }, \
		NULL, NULL, IDC_ARROW, TRUE, 0, _T("") \
	}; \
	if (0 != uIcon) \
		wc.m_wc.hIcon = LoadIcon(ATL::_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(uIcon)); \
	if (0 != uIconSmall) \
		wc.m_wc.hIconSm = LoadIcon(ATL::_AtlBaseModule.GetModuleInstance(), MAKEINTRESOURCE(uIconSmall)); \
	return wc; \
}
#endif 

#define MESSAGE_HANDLER_DUIWINDOW(msg, func) \
	if(true) \
	{ \
		bHandled = TRUE; \
		lResult = func(uMsg, wParam, lParam, bHandled); \
		if(bHandled) \
			return TRUE; \
	}

/// Find a function in a DLL and convert to a function pointer.
/// This avoids undefined and conditionally defined behaviour.
template<typename T>
inline T DLLFunction(HMODULE hModule, LPCSTR lpProcName) noexcept 
{
	if (!hModule) 
	{
		return nullptr;
	}
	FARPROC function = ::GetProcAddress(hModule, lpProcName);
	static_assert(sizeof(T) == sizeof(function));
	
	T fp{};
	memcpy(&fp, &function, sizeof(T));
	
	return fp;
}

// Release an IUnknown* and set to nullptr.
// While IUnknown::Release must be noexcept, it isn't marked as such so produces
// warnings which are avoided by the catch.
template <class T>
inline void ReleaseUnknown(T*& ppUnknown) noexcept 
{
	if (ppUnknown) 
	{
		try 
		{
			ppUnknown->Release();
		}
		catch (...) 
		{
			// Never occurs
		}
		ppUnknown = nullptr;
	}
}

#endif // __WT_WOCHATDEF_H__