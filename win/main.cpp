// WTL10 Application Wizard1.cpp : main source file for WTL10 Application Wizard1.exe
//

#include "stdafx.h"
#include <atlframe.h>
#include <atlctrls.h>
#include <atldlgs.h>

#include "resource.h"
#include "xbitmapdata.h"
#include "wochatypes.h"
#include "wochatdef.h"
#include "wochat.h"
#include "MainFrm.h"

////////////////////////////////////////////////////////////////
// global variables
////////////////////////////////////////////////////////////////
volatile LONG		g_threadCount = 0;
volatile LONG		g_Quit = 0;
volatile LONG       g_NetworkStatus = 0;
volatile LONG       g_messageId = 0;
DWORD               g_dwMainThreadID = 0;

int					g_win4Width = 0;
int					g_win4Height = 0;

WTFriend*           g_friendRoot = nullptr;
WTChatGroup*        g_chatgroupRoot = nullptr;
U32					g_friendTotal = 0;
U32					g_chatgroupTotal = 0;

HTAB*               g_peopleHTAB = nullptr;
MemoryPoolContext   g_messageMemPool = nullptr;
MemoryPoolContext   g_topMemPool = nullptr;

WTMyInfo*           g_myInfo = nullptr;

U8*                 g_MQTTPubClientId = nullptr;
U8*                 g_MQTTSubClientId = nullptr;

HINSTANCE			g_hInstance = nullptr;
HANDLE				g_MQTTPubEvent = nullptr;

CRITICAL_SECTION    g_csMQTTSub;
CRITICAL_SECTION    g_csMQTTPub;
CRITICAL_SECTION    g_csSQLiteDB;

MessageTask*        g_PubQueue = nullptr;
MessageTask*        g_SubQueue = nullptr;

ID2D1Factory*       g_pD2DFactory = nullptr;
IDWriteFactory*     g_pDWriteFactory = nullptr;

HCURSOR				g_hCursorWE = nullptr;
HCURSOR				g_hCursorNS = nullptr;
HCURSOR				g_hCursorHand = nullptr;
HCURSOR				g_hCursorIBeam = nullptr;
wchar_t				g_AppPath[MAX_PATH + 1] = { 0 };
wchar_t				g_DBPath[MAX_PATH + 1] = { 0 };

////////////////////////////////////////////////////////////////
// local static variables
////////////////////////////////////////////////////////////////
static IDWriteTextFormat*     ng_pTextFormat[WT_TEXTFORMAT_TOTAL] = { 0 };
static U16                    ng_fontHeight[WT_TEXTFORMAT_TOTAL] = { 0 };

static const U8* ng_hexPKRobot = (const U8*)"03339A1C8FDB6AFF46845E49D110E0400021E16146341858585C2E25CA399C01CA";
static U8 ng_pkRobot[PUBLIC_KEY_SIZE] = { 0 };
static U32* ng_bitmapArray[WT_UI_BITMAP_MAX] = { 0 };
static U32  ng_bitmapWidth[WT_UI_BITMAP_MAX] = { 0 };
static U32  ng_bitmapHeight[WT_UI_BITMAP_MAX] = { 0 };

static wchar_t ng_strMyPubKey[67] = { 0 };

static D2D1_DRAW_TEXT_OPTIONS d2dDrawTextOptions = D2D1_DRAW_TEXT_OPTIONS_NONE;
static HMODULE hDLLD2D{};
static HMODULE hDLLDWrite{};

CAppModule _Module;
CMainFrame wndFrame;

U8* GetRobotPublicKey()
{
	return ng_pkRobot;
}

wchar_t* GetMyPublicKeyString()
{
	return ng_strMyPubKey;
}

IDWriteTextFormat* GetTextFormatAndHeight(U8 idx, U16* height)
{
	IDWriteTextFormat* pTxtFormat = nullptr;

	if (idx < WT_TEXTFORMAT_TOTAL)
	{
		if (height)
			*height = ng_fontHeight[idx];

		ATLASSERT(ng_pTextFormat[idx]);
		pTxtFormat = ng_pTextFormat[idx];
	}
	return pTxtFormat;
}

U32 GetTextWidthAndHeight(U8 fontIdx, U16* text, U32 length, int width, int* w, int* h)
{
	U32 ret = WT_FAIL;
	IDWriteTextLayout* pTextLayout = nullptr;
	IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(fontIdx);
	assert(pTextFormat);

	HRESULT hr = g_pDWriteFactory->CreateTextLayout((wchar_t*)text, length, pTextFormat, static_cast<FLOAT>(width), 1.f, &pTextLayout);
	if (hr == S_OK && pTextLayout)
	{
		DWRITE_TEXT_METRICS tm;
		if (pTextLayout->GetMetrics(&tm) == S_OK)
		{
			int txtH = static_cast<int>(tm.height) + 1;
			int txtW = static_cast<int>(tm.width) + 1;
			if (w) *w = txtW;
			if (h) *h = txtH;
			ret = WT_OK;
		}
		ReleaseUnknown(pTextLayout);
	}
	return ret;
}

U32* GetUIBitmap(U8 idx, U32* width, U32* height)
{
	U32* bmp = nullptr;
	if (idx < WT_UI_BITMAP_MAX)
	{
		bmp = ng_bitmapArray[idx];
		if (width)  *width = ng_bitmapWidth[idx];
		if (height) *height = ng_bitmapHeight[idx];
	}
	return bmp;
}

// get the public key of the current receiver
U32 GetReceiverPublicKey(void* parent, U8* pk)
{
	U32 r = WT_FAIL;
	CMainFrame* pThis = static_cast<CMainFrame*>(parent);

	if (pThis && pk)
	{
		r = pThis->GetCurrentPublicKey(pk);
	}
	return r;
}

class CWoChatThreadManager
{
public:
	// thread init param
	struct _RunData
	{
		LPTSTR lpstrCmdLine;
		int nCmdShow;
	};

	// thread proc
	static DWORD WINAPI RunThread(LPVOID lpData)
	{
		RECT rw = { 0 };
		CMessageLoop theLoop;

		InterlockedIncrement(&g_threadCount);

		_Module.AddMessageLoop(&theLoop);

		_RunData* pData = (_RunData*)lpData;

		rw.left = 100; rw.top = 100; rw.right = rw.left + 1024; rw.bottom = rw.top + 768;
		if(wndFrame.CreateEx(NULL, rw) == NULL)
		{
			ATLTRACE(_T("Frame window creation failed!\n"));
			return 0;
		}

		wndFrame.ShowWindow(pData->nCmdShow);
		delete pData;

		int nRet = theLoop.Run();

		_Module.RemoveMessageLoop();

		InterlockedDecrement(&g_threadCount);
		return nRet;
	}

	DWORD m_dwCount;
	HANDLE m_arrThreadHandles[MAXIMUM_WAIT_OBJECTS - 1];

	CWoChatThreadManager() : m_dwCount(0)
	{
		for (U8 i = 0; i < MAXIMUM_WAIT_OBJECTS - 1; i++)
			m_arrThreadHandles[i] = nullptr;
	}

// Operations
	DWORD AddThread(LPTSTR lpstrCmdLine, int nCmdShow)
	{
		if(m_dwCount == (MAXIMUM_WAIT_OBJECTS - 1))
		{
			::MessageBox(NULL, _T("ERROR: Cannot create ANY MORE threads!!!"), _T("WTL10 Application Wizard1"), MB_OK);
			return 0;
		}

		_RunData* pData = new _RunData;
		pData->lpstrCmdLine = lpstrCmdLine;
		pData->nCmdShow = nCmdShow;
		DWORD dwThreadID;
		HANDLE hThread = ::CreateThread(NULL, 0, RunThread, pData, 0, &dwThreadID);
		if(hThread == NULL)
		{
			::MessageBox(NULL, _T("ERROR: Cannot create thread!!!"), _T("WTL10 Application Wizard1"), MB_OK);
			return 0;
		}

		m_arrThreadHandles[m_dwCount] = hThread;
		m_dwCount++;
		return dwThreadID;
	}

	void RemoveThread(DWORD dwIndex)
	{
		::CloseHandle(m_arrThreadHandles[dwIndex]);
		if(dwIndex != (m_dwCount - 1))
			m_arrThreadHandles[dwIndex] = m_arrThreadHandles[m_dwCount - 1];
		m_dwCount--;
	}

	int Run(LPTSTR lpstrCmdLine, int nCmdShow)
	{
		MSG msg;
		// force message queue to be created
		::PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

		AddThread(lpstrCmdLine, nCmdShow);

		int nRet = m_dwCount;
		DWORD dwRet;
		while(m_dwCount > 0)
		{
			dwRet = ::MsgWaitForMultipleObjects(m_dwCount, m_arrThreadHandles, FALSE, INFINITE, QS_ALLINPUT);

			if(dwRet == 0xFFFFFFFF)
			{
				::MessageBox(NULL, _T("ERROR: Wait for multiple objects failed!!!"), _T("WTL10 Application Wizard1"), MB_OK);
			}
			else if(dwRet >= WAIT_OBJECT_0 && dwRet <= (WAIT_OBJECT_0 + m_dwCount - 1))
			{
				RemoveThread(dwRet - WAIT_OBJECT_0);
			}
			else if(dwRet == (WAIT_OBJECT_0 + m_dwCount))
			{
				if(::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
				{
					if(msg.message == WM_USER)
						AddThread(_T(""), SW_SHOWNORMAL);
				}
			}
			else
			{
				::MessageBeep((UINT)-1);
			}
		}

		return nRet;
	}
};

static void LoadD2DOnce()
{
	DWORD loadLibraryFlags = 0;
	HMODULE kernel32 = ::GetModuleHandleW(L"kernel32.dll");

	if (kernel32)
	{
		if (::GetProcAddress(kernel32, "SetDefaultDllDirectories"))
		{
			// Availability of SetDefaultDllDirectories implies Windows 8+ or
			// that KB2533623 has been installed so LoadLibraryEx can be called
			// with LOAD_LIBRARY_SEARCH_SYSTEM32.
			loadLibraryFlags = LOAD_LIBRARY_SEARCH_SYSTEM32;
		}
	}

	typedef HRESULT(WINAPI* D2D1CFSig)(D2D1_FACTORY_TYPE factoryType, REFIID riid, CONST D2D1_FACTORY_OPTIONS* pFactoryOptions, IUnknown** factory);
	typedef HRESULT(WINAPI* DWriteCFSig)(DWRITE_FACTORY_TYPE factoryType, REFIID iid, IUnknown** factory);

	hDLLD2D = ::LoadLibraryEx(TEXT("D2D1.DLL"), 0, loadLibraryFlags);
	D2D1CFSig fnD2DCF = DLLFunction<D2D1CFSig>(hDLLD2D, "D2D1CreateFactory");

	if (fnD2DCF)
	{
		// A single threaded factory as Scintilla always draw on the GUI thread
		fnD2DCF(D2D1_FACTORY_TYPE_SINGLE_THREADED,
			__uuidof(ID2D1Factory),
			nullptr,
			reinterpret_cast<IUnknown**>(&g_pD2DFactory));
	}

	hDLLDWrite = ::LoadLibraryEx(TEXT("DWRITE.DLL"), 0, loadLibraryFlags);
	DWriteCFSig fnDWCF = DLLFunction<DWriteCFSig>(hDLLDWrite, "DWriteCreateFactory");
	if (fnDWCF)
	{
		const GUID IID_IDWriteFactory2 = // 0439fc60-ca44-4994-8dee-3a9af7b732ec
		{ 0x0439fc60, 0xca44, 0x4994, { 0x8d, 0xee, 0x3a, 0x9a, 0xf7, 0xb7, 0x32, 0xec } };

		const HRESULT hr = fnDWCF(DWRITE_FACTORY_TYPE_SHARED,
			IID_IDWriteFactory2,
			reinterpret_cast<IUnknown**>(&g_pDWriteFactory));

		if (SUCCEEDED(hr))
		{
			// D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT
			d2dDrawTextOptions = static_cast<D2D1_DRAW_TEXT_OPTIONS>(0x00000004);
		}
		else
		{
			fnDWCF(DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
				reinterpret_cast<IUnknown**>(&g_pDWriteFactory));
		}
	}
}

static bool LoadD2D()
{
	static std::once_flag once;

	ReleaseUnknown(g_pD2DFactory);
	ReleaseUnknown(g_pDWriteFactory);

	std::call_once(once, LoadD2DOnce);

	return g_pDWriteFactory && g_pD2DFactory;
}

static int InitDirectWriteTextFormat(HINSTANCE hInstance)
{
	int iRet = 0;
	U8 idx;
	HRESULT hr = S_OK;
	FLOAT fontSize;
	WCHAR* fontFamilyName;
	WCHAR const* ln = L"en-US"; // locale name
	WCHAR* testString;
	IDWriteTextLayout* pTextLayout = nullptr;
	DWRITE_TEXT_METRICS tm;
	wchar_t strChinese[2] = { 0x85cf,0 };
	wchar_t strEnglish[2] = { 0x0058,0 };

	ATLASSERT(g_pDWriteFactory);

	for (U8 i = 0; i < WT_TEXTFORMAT_TOTAL; i++)
	{
		ReleaseUnknown(ng_pTextFormat[i]);
		ng_fontHeight[i] = 0;
	}

	idx = WT_TEXTFORMAT_MAINTEXT; fontSize = 13.5f; fontFamilyName = L"Microsoft Yahei UI"; testString = strChinese;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_TITLE; fontSize = 11.5f; fontFamilyName = L"Arial"; testString = strEnglish;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_GROUPNAME; fontSize = 17.f; fontFamilyName = L"Microsoft Yahei UI"; testString = strChinese;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_PEOPLENAME; fontSize = 16.f; fontFamilyName = L"Microsoft Yahei UI"; testString = strChinese;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_OTHER0; fontSize = 21.f; fontFamilyName = L"Microsoft Yahei"; testString = strChinese;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_ENGLISHSMALL; fontSize = 13.0f; fontFamilyName = L"Arial"; testString = strEnglish;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_OTHER1; fontSize = 18.0f; fontFamilyName = L"Arial"; testString = strEnglish;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_OTHER2; fontSize = 16.f; fontFamilyName = L"Microsoft Yahei"; testString = strChinese;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	idx = WT_TEXTFORMAT_OTHER3; fontSize = 11.f; fontFamilyName = L"Microsoft Yahei"; testString = strChinese;
	hr = g_pDWriteFactory->CreateTextFormat(fontFamilyName, NULL, DWRITE_FONT_WEIGHT_NORMAL, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, ln, &(ng_pTextFormat[idx]));
	if (S_OK == hr && nullptr != ng_pTextFormat[idx])
	{
		// we determine the height of the text in this font by using a simple test string
		hr = g_pDWriteFactory->CreateTextLayout(testString, 1, ng_pTextFormat[idx], 600.f, 1.f, &pTextLayout);
		if (S_OK == hr && pTextLayout)
		{
			if (S_OK == pTextLayout->GetMetrics(&tm))
			{
				ng_fontHeight[idx] = static_cast<int>(tm.height) + 1;
			}
			else iRet = (int)(20 + idx);
		}
		else iRet = (int)(20 + idx);
		ReleaseUnknown(pTextLayout);
	}
	else iRet = (int)(20 + idx);
	if (iRet) return iRet;

	return 0;
}

static int GetApplicationPath(HINSTANCE hInstance)
{
	int iRet = 0;
	DWORD len, i = 0;

	len = GetModuleFileNameW(hInstance, g_AppPath, MAX_PATH);
	if (len > 0)
	{
		for (i = len - 1; i > 0; i--)
		{
			if (g_AppPath[i] == L'\\')
				break;
		}
		g_AppPath[i] = L'\0';
	}
	else
		g_AppPath[0] = L'\0';

	if (i == 0 || i > (MAX_PATH - 10))
		return 30;

	for (DWORD k = 0; k < i; k++)
		g_DBPath[k] = g_AppPath[k];

	g_DBPath[i] = L'\\';
	g_DBPath[i + 1] = L'w'; g_DBPath[i + 2] = L't'; g_DBPath[i + 3] = L'.'; g_DBPath[i + 4] = L'd'; g_DBPath[i + 5] = L'b';
	g_DBPath[i + 6] = L'\0';

	return iRet;
}

/*
 * MemoryContextInit
 *		Start up the memory-context subsystem.
 *
 * This must be called before creating contexts or allocating memory in
 * contexts.
 */
static U32 MemoryContextInit()
{
	U32 ret = WT_FAIL;

	ATLASSERT(g_topMemPool == nullptr);
	ATLASSERT(g_messageMemPool == nullptr);
	ATLASSERT(g_MQTTPubClientId == nullptr);
	ATLASSERT(g_MQTTSubClientId == nullptr);

	wt_HexString2Raw((U8*)ng_hexPKRobot, 66, ng_pkRobot, nullptr);

	g_topMemPool = wt_mempool_create("TopMemoryContext", 0, DUI_ALLOCSET_DEFAULT_INITSIZE, DUI_ALLOCSET_DEFAULT_MAXSIZE);
	if (g_topMemPool)
	{
		g_MQTTPubClientId = (U8*)wt_palloc0(g_topMemPool, MQTT_CLIENTID_SIZE);
		g_MQTTSubClientId = (U8*)wt_palloc0(g_topMemPool, MQTT_CLIENTID_SIZE);
		g_myInfo = (WTMyInfo*)wt_palloc0(g_topMemPool, sizeof(WTMyInfo));
		if (g_myInfo && g_MQTTPubClientId && g_MQTTSubClientId)
		{
			g_myInfo->icon32 = (U32*)wt_palloc0(g_topMemPool, WT_SMALL_ICON_HEIGHT);
			g_messageMemPool = wt_mempool_create("MessagePool", 0, DUI_ALLOCSET_DEFAULT_INITSIZE, DUI_ALLOCSET_DEFAULT_MAXSIZE);
			if (g_messageMemPool && g_myInfo->icon32)
			{
				HASHCTL hctl = { 0 };
				hctl.keysize = PUBLIC_KEY_SIZE;
				hctl.entrysize = hctl.keysize + sizeof(WTFriend*);
				g_peopleHTAB = hash_create("PeopleHTAB", 256, &hctl, HASH_ELEM | HASH_BLOBS);
				if (g_peopleHTAB)
					ret = WT_OK;
			}
		}
	}
	return ret;
}

static void MemoryContextTerm()
{
	hash_destroy(g_peopleHTAB);
	wt_mempool_destroy(g_messageMemPool);
	g_messageMemPool = nullptr;
	wt_mempool_destroy(g_topMemPool);
	g_topMemPool = nullptr;
}

static void InitUIBitmap()
{
	for (U8 i = 0; i < WT_UI_BITMAP_MAX; i++)
	{
		ng_bitmapArray[i] = nullptr;
		ng_bitmapWidth[i] = 0;
		ng_bitmapHeight[i] = 0;
	}

	ng_bitmapArray[WT_UI_BMP_MYLARGEICON] = (U32*)xbmpIcon128;
	ng_bitmapWidth[WT_UI_BMP_MYLARGEICON] = 128;
	ng_bitmapHeight[WT_UI_BMP_MYLARGEICON] = 128;

	ng_bitmapArray[WT_UI_BMP_MYSMALLICON] = (U32*)xbmpIcon32;
	ng_bitmapWidth[WT_UI_BMP_MYSMALLICON] = 32;
	ng_bitmapHeight[WT_UI_BMP_MYSMALLICON] = 32;

	ng_bitmapArray[WT_UI_BMP_AILARGEICON] = (U32*)xbmpAI128;
	ng_bitmapWidth[WT_UI_BMP_AILARGEICON] = 128;
	ng_bitmapHeight[WT_UI_BMP_AILARGEICON] = 128;

	ng_bitmapArray[WT_UI_BMP_AIMSALLICON] = (U32*)xbmpAI32;
	ng_bitmapWidth[WT_UI_BMP_AIMSALLICON] = 32;
	ng_bitmapHeight[WT_UI_BMP_AIMSALLICON] = 32;
}

static int AppInit(HINSTANCE hInstance)
{
	int iRet = 0;
	g_PubQueue = nullptr;
	g_SubQueue = nullptr;
	g_friendRoot = nullptr;
	g_chatgroupRoot = nullptr;
	g_friendTotal = 0;
	g_chatgroupTotal = 0;

	InitUIBitmap();
	DUI_Init();
	InitializeCriticalSection(&g_csMQTTSub);
	InitializeCriticalSection(&g_csMQTTPub);
	InitializeCriticalSection(&g_csSQLiteDB);

	iRet = MemoryContextInit();
	if (iRet)
		return 1;

	g_MQTTPubEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	if (NULL == g_MQTTPubEvent)
		return 2;

	g_hCursorWE = ::LoadCursor(NULL, IDC_SIZEWE);
	g_hCursorNS = ::LoadCursor(NULL, IDC_SIZENS);
	//g_hCursorHand = ::LoadCursor(nullptr, IDC_HAND);
	//g_hCursorIBeam = ::LoadCursor(NULL, IDC_IBEAM);
	if (NULL == g_hCursorWE || NULL == g_hCursorNS) // || NULL == g_hCursorHand || NULL == g_hCursorIBeam)
		return 3;

	if (!LoadD2D())
		return 4;

	iRet = InitDirectWriteTextFormat(hInstance);
	if (iRet)
		return 5;

	iRet = GetApplicationPath(hInstance);
	if (iRet)
		return 6;

	iRet = MosquittoInit();

	if (iRet)
		return 7;

	return 0;
}

static void AppTerm(HINSTANCE hInstance)
{
	UINT i, tries;

	// tell all threads to quit
	InterlockedIncrement(&g_Quit);

	// wait for all threads to quit gracefully
	tries = 20;
	while (g_threadCount && tries > 0)
	{
		Sleep(1000);
		tries--;
	}
	DUI_Term();

	ReleaseUnknown(g_pDWriteFactory);
	ReleaseUnknown(g_pD2DFactory);

	if (hDLLDWrite)
	{
		FreeLibrary(hDLLDWrite);
		hDLLDWrite = {};
	}
	if (hDLLD2D)
	{
		FreeLibrary(hDLLD2D);
		hDLLD2D = {};
	}

	CloseHandle(g_MQTTPubEvent);
	DeleteCriticalSection(&g_csMQTTSub);
	DeleteCriticalSection(&g_csMQTTPub);
	DeleteCriticalSection(&g_csSQLiteDB);

	MosquittoTerm();
	MemoryContextTerm();
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpstrCmdLine, int nCmdShow)
{
	int nRet = 0;
	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	UNREFERENCED_PARAMETER(hPrevInstance);

	g_Quit = 0;
	g_threadCount = 0;
	g_hInstance = hInstance;  // save the instance
	g_dwMainThreadID = GetCurrentThreadId();

	// The Microsoft Security Development Lifecycle recommends that all
	// applications include the following call to ensure that heap corruptions
	// do not go unnoticed and therefore do not introduce opportunities
	// for security exploits.
	HeapSetInformation(NULL, HeapEnableTerminationOnCorruption, NULL, 0);

	AtlInitCommonControls(ICC_BAR_CLASSES);	// add flags to support other controls

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	nRet = AppInit(hInstance);
	if (nRet)
	{
		wchar_t msg[MAX_PATH + 1] = { 0 };
		swprintf((wchar_t*)msg, MAX_PATH, L"AppInit() is failed. Return code:[%d]", nRet);
		MessageBoxW(NULL, (LPCWSTR)msg, L"WoChat Error", MB_OK);
		goto ExitThisApplication;
	}
	else // BLOCK: Run application
	{
		nRet = 0;
		U32 status = GetAccountNumber(&nRet); // we check if there is any account available 
		if (status == WT_OK)
		{
			bool bLoginIsSuccessful = nRet ? DoLogin() : DoRegistration();
			if (bLoginIsSuccessful)
			{
				ATLASSERT(g_myInfo);
				wt_Raw2HexStringW(g_myInfo->pubkey, PUBLIC_KEY_SIZE, ng_strMyPubKey, nullptr);

				CWoChatThreadManager mgr;
				nRet = mgr.Run(lpstrCmdLine, nCmdShow); // the main part
			}
		}
		else
		{
			wchar_t msg[MAX_PATH + 1] = { 0 };
			swprintf((wchar_t*)msg, MAX_PATH, L"Cannot access SQLite database!. Return code:[%u]", status);
			MessageBoxW(NULL, (LPCWSTR)msg, L"WoChat Error", MB_OK);
			goto ExitThisApplication;
		}
	}

ExitThisApplication:
	AppTerm(hInstance);
	_Module.Term();
	::CoUninitialize();

	return nRet;
}
