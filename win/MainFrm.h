// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once
#include "wochatdlg.h"
#include "win0.h"
#include "win1.h"
#include "win2.h"
#include "win3.h"
#include "win4.h"
#include "win5.h"

/************************************************************************************************
*  The layout of the Main Window
*
* +------+------------------------+-----------------------------------------------------+
* |      |         Win1           |                       Win3                          |
* |      +------------------------+-----------------------------------------------------+
* |      |                        |                                                     |
* |      |                        |                                                     |
* |      |                        |                       Win4                          |
* | Win0 |                        |                                                     |
* |      |         Win2           |                                                     |
* |      |                        +-----------------------------------------------------|
* |      |                        |                                                     |
* |      |                        |                       Win5                          |
* |      |                        |                                                     |
* +------+------------------------+-----------------------------------------------------+
*
*
* We have one vertical bar and three horizonal bars.
*
*************************************************************************************************
*/

class CMainFrame : 
	public CFrameWindowImpl<CMainFrame>, 
	public CUpdateUI<CMainFrame>,
	public CMessageFilter, public CIdleHandler
{
	enum
	{
		STEPXY = 1,
		SPLITLINE_WIDTH = 1,
		MOVESTEP = 1
	};

	enum class DrapMode { dragModeNone, dragModeV, dragModeH };
	DrapMode m_dragMode = DrapMode::dragModeNone;

	enum class AppMode
	{
		ModeMe,
		ModeTalk,
		ModeFriend,
		ModeQuan,
		ModeCoin,
		ModeFavorite,
		ModeFile,
		ModeSetting
	};

	AppMode m_mode = AppMode::ModeTalk;

	U32* m_screenBuff = nullptr;
	U32  m_screenSize = 0;

	RECT m_rectClient = { 0 };

	int m_splitterVPos = -1;               // splitter bar position
	int m_splitterVPosOld = -1;            // keep align value
	int m_splitterHPos = -1;
	int m_splitterHPosOld = -1;

	int m_cxyDragOffset = 0;		// internal

	int m_splitterVPosToLeft = 300;       // the minum pixel to the left of the client area.
	int m_splitterVPosToRight = 400;       // the minum pixel to the right of the client area.
	int m_splitterHPosToTop = (XWIN3_HEIGHT + XWIN4_HEIGHT);        // the minum pixel to the top of the client area.
	int m_splitterHPosToBottom = XWIN5_HEIGHT;       // the minum pixel to the right of the client area.

	int m_marginLeft = 64;
	int m_marginRight = 0;

	int m_splitterHPosfix0 = XWIN1_HEIGHT;
	int m_splitterHPosfix1 = XWIN3_HEIGHT;

	UINT m_nDPI = 96;
	float m_deviceScaleFactor = 1.f;
	U8 m_isInThisWindow = 0;

	U8 m_loadingPercent = 0;

	XWindow0 m_win0;
	XWindow1 m_win1;
	XWindow2 m_win2;
	XWindow3 m_win3;
	XWindow4 m_win4;
	XWindow5 m_win5;

	int m0 = 0;
	int m1 = 0;
	int m2 = 0;
	int m3 = 0;
	int m4 = 0;
	int m5 = 0;
	int mdrag = 0;

	ID2D1HwndRenderTarget* m_pD2DRenderTarget = nullptr;
	ID2D1Bitmap* m_pixelBitmap0 = nullptr;
	ID2D1Bitmap* m_pixelBitmap1 = nullptr;
	ID2D1Bitmap* m_pixelBitmap2 = nullptr;
	ID2D1SolidColorBrush* m_pTextBrushSel = nullptr;
	ID2D1SolidColorBrush* m_pTextBrush0 = nullptr;
	ID2D1SolidColorBrush* m_pTextBrush1 = nullptr;
	ID2D1SolidColorBrush* m_pCaretBrush = nullptr;
	ID2D1SolidColorBrush* m_pBkgBrush0 = nullptr;
	ID2D1SolidColorBrush* m_pBkgBrush1 = nullptr;

public:
	/* DECLARE_FRAME_WND_CLASS(NULL, IDR_MAINFRAME) */
	DECLARE_FRAME_WND_CLASS_EX(NULL, IDR_MAINFRAME,CS_DBLCLKS, COLOR_WINDOW) // to support double-click

	CMainFrame()
	{
		m_rectClient.left = m_rectClient.right = m_rectClient.top = m_rectClient.bottom = 0;
		m_win0.SetWindowId((const U8*)"DUIWin0", 7);
		m_win1.SetWindowId((const U8*)"DUIWin1", 7);
		m_win2.SetWindowId((const U8*)"DUIWin2", 7);
		m_win3.SetWindowId((const U8*)"DUIWin3", 7);
		m_win4.SetWindowId((const U8*)"DUIWin4", 7);
		m_win5.SetWindowId((const U8*)"DUIWin5", 7);
	}

	~CMainFrame()
	{
		if (nullptr != m_screenBuff)
			VirtualFree(m_screenBuff, 0, MEM_RELEASE);

		m_screenBuff = nullptr;
		m_screenSize = 0;
	}

	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return CFrameWindowImpl<CMainFrame>::PreTranslateMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	U32 GetCurrentPublicKey(U8* pk)
	{
		return m_win4.GetPublicKey(pk);
	}

	BEGIN_UPDATE_UI_MAP(CMainFrame)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainFrame)
		MESSAGE_HANDLER_DUIWINDOW(DUI_ALLMESSAGE, OnDUIWindowMessage)
		MESSAGE_HANDLER(WM_ERASEBKGND, OnEraseBkgnd)
		MESSAGE_HANDLER(WM_PAINT, OnPaint)
		MESSAGE_HANDLER(WM_TIMER, OnTimer)
		MESSAGE_HANDLER(WM_MOUSEMOVE, OnMouseMove)
		MESSAGE_HANDLER(WM_MOUSEWHEEL, OnMouseWheel)
		MESSAGE_HANDLER(WM_MOUSELEAVE, OnMouseLeave)
		MESSAGE_HANDLER(WM_MOUSEHOVER, OnMouseHover)
		MESSAGE_HANDLER(WM_LBUTTONDOWN, OnLButtonDown)
		MESSAGE_HANDLER(WM_LBUTTONUP, OnLButtonUp)
		MESSAGE_HANDLER(WM_LBUTTONDBLCLK, OnLButtonDoubleClick)
		MESSAGE_HANDLER(WM_KEYDOWN, OnKeyDown)
		MESSAGE_HANDLER(WM_SIZE, OnSize)
		MESSAGE_HANDLER(WM_SETCURSOR, OnSetCursor)
		MESSAGE_HANDLER(WM_LOADPERCENTMESSAGE, OnLoading)
		MESSAGE_HANDLER(WM_XWINDOWS00, OnWin0Message)
		MESSAGE_HANDLER(WM_XWINDOWS01, OnWin1Message)
		MESSAGE_HANDLER(WM_XWINDOWS02, OnWin2Message)
		MESSAGE_HANDLER(WM_XWINDOWS03, OnWin3Message)
		MESSAGE_HANDLER(WM_XWINDOWS04, OnWin4Message)
		MESSAGE_HANDLER(WM_XWINDOWS05, OnWin5Message)
		MESSAGE_HANDLER(WM_ALLOTHER_MESSAGE, OnOtherMessage)
		MESSAGE_HANDLER(WM_GETMINMAXINFO, OnGetMinMaxInfo)
		MESSAGE_HANDLER(WM_CREATE, OnCreate)
		MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
		COMMAND_ID_HANDLER(ID_ADD_FRIEND, OnAddFriend)
		COMMAND_ID_HANDLER(ID_EDIT_MYINFO, OnEditMyInfo)
		COMMAND_ID_HANDLER(ID_QUIT_WOCHAT, OnQuitWoChat)
		CHAIN_MSG_MAP(CUpdateUI<CMainFrame>)
		CHAIN_MSG_MAP(CFrameWindowImpl<CMainFrame>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnEraseBkgnd(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		return 0; // don't want flicker
	}

	// this is the routine to process all window messages for win0/1/2/3/4/5
	LRESULT OnDUIWindowMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		WPARAM wp = wParam;
		LPARAM lp = lParam;

		ClearDUIWindowReDraw(); // reset the bit to indicate the redraw before we handle the messages

		switch (uMsg)
		{
		case WM_MOUSEWHEEL:
		{
			POINT pt;
			pt.x = GET_X_LPARAM(lParam);
			pt.y = GET_Y_LPARAM(lParam);
			ScreenToClient(&pt);
			lp = MAKELONG(pt.x, pt.y);
		}
		break;
		case WM_CREATE:
			wp = (WPARAM)m_hWnd;
			break;
		default:
			break;
		}

		m_win0.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_win1.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_win2.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_win3.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_win4.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);
		m_win5.HandleOSMessage((U32)uMsg, (U64)wp, (U64)lp, this);

		if (DUIWindowNeedReDraw()) // after all 5 virtual windows procced the window message, the whole window may need to fresh
		{
			// U64 s = DUIWindowNeedReDraw();
			Invalidate(); // send out the WM_PAINT messsage to the window to redraw
		}
		// to allow the host window to continue to handle the windows message
		bHandled = FALSE;
		return 0;
	}

	LRESULT OnCreate(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		DWORD dwThreadID;
		HANDLE hThread = nullptr;
		wchar_t hexPK[67] = { 0 };
		wchar_t title[128 + 1] = { 0 };
		// register object for message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		m_loadingPercent = 0;
		hThread = ::CreateThread(NULL, 0, LoadingDataThread, m_hWnd, 0, &dwThreadID);
		if (nullptr == hThread)
		{
			MessageBox(TEXT("Loading thread creation is failed!"), TEXT("WoChat Error"), MB_OK);
			PostMessage(WM_CLOSE, 0, 0);
			return 0;
		}
		m_nDPI = GetDpiForWindow(m_hWnd);
		UINT_PTR id = SetTimer(XWIN_CARET_TIMER, 666);

		//swprintf((wchar_t*)title, 256, L"WoChat - %s[%s]", (wchar_t*)g_myInfo->name, ng_strMyPubKey);
		assert(g_myInfo);
		wt_Raw2HexStringW(g_myInfo->pubkey, PUBLIC_KEY_SIZE, (wchar_t*)hexPK, nullptr);
		swprintf((wchar_t*)title, 128, L"WoChat - [%s]", (wchar_t*)hexPK);
		SetWindowTextW(title);

		return 0;
	}

	LRESULT OnDestroy(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& bHandled)
	{
		// tell all threads to quit
		InterlockedIncrement(&g_Quit);
		SetEvent(g_MQTTPubEvent); // tell MQTT pub thread to quit gracefully

		// unregister message filtering and idle updates
		CMessageLoop* pLoop = _Module.GetMessageLoop();
		ATLASSERT(pLoop != NULL);
		pLoop->RemoveMessageFilter(this);
		pLoop->RemoveIdleHandler(this);

		KillTimer(XWIN_CARET_TIMER);

		ReleaseUnknown(m_pBkgBrush0);
		ReleaseUnknown(m_pBkgBrush1);
		ReleaseUnknown(m_pCaretBrush);
		ReleaseUnknown(m_pTextBrushSel);
		ReleaseUnknown(m_pTextBrush0);
		ReleaseUnknown(m_pTextBrush1);
		ReleaseUnknown(m_pixelBitmap0);
		ReleaseUnknown(m_pixelBitmap1);
		ReleaseUnknown(m_pixelBitmap2);
		ReleaseUnknown(m_pD2DRenderTarget);

		bHandled = FALSE;
		return 1;
	}

	LRESULT OnGetMinMaxInfo(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM lParam, BOOL& bHandled)
	{
		LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
		lpMMI->ptMinTrackSize.x = 800;
		lpMMI->ptMinTrackSize.y = 600;
		return 0;
	}

	bool IsOverSplitterRect(int x, int y) const
	{
		// -1 == don't check
		return (((x == -1) || ((x >= m_rectClient.left) && (x < m_rectClient.right))) &&
			((y == -1) || ((y >= m_rectClient.top) && (y < m_rectClient.bottom))));
	}

	DrapMode IsOverSplitterBar(int x, int y) const
	{
		if (m_splitterVPos > 0)
		{
			int width = SPLITLINE_WIDTH << 1;
			if (!IsOverSplitterRect(x, y))
				return DrapMode::dragModeNone;

			if ((x >= m_splitterVPos) && (x < (m_splitterVPos + width)))
				return DrapMode::dragModeV;

			if (m_splitterHPos > 0)
			{
				if ((x >= m_splitterVPos + width) && (y >= m_splitterHPos) && (y < (m_splitterHPos + width)))
					return DrapMode::dragModeH;
			}
		}

		return DrapMode::dragModeNone;
	}

	LRESULT OnSetCursor(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = DUIWindowCursorIsSet() ? TRUE : FALSE;

		ClearDUIWindowCursor();

		if (((HWND)wParam == m_hWnd) && (LOWORD(lParam) == HTCLIENT) && m_loadingPercent > 100)
		{
			DWORD dwPos = ::GetMessagePos();

			POINT ptPos = { GET_X_LPARAM(dwPos), GET_Y_LPARAM(dwPos) };

			ScreenToClient(&ptPos);

			DrapMode mode = IsOverSplitterBar(ptPos.x, ptPos.y);
			if (DrapMode::dragModeNone != mode)
				bHandled = TRUE;
		}

		return 0;
	}

	LRESULT OnMouseWheel(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnMouseLeave(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		m_isInThisWindow = 0;
		return 0;
	}

	LRESULT OnMouseHover(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnLButtonDown(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (m_loadingPercent <= 100)
			return 0;

		if (DUIWindowInDragMode())
		{
			mdrag = 1;
			ATLASSERT(DrapMode::dragModeNone == m_dragMode);

			if (::GetCapture() != m_hWnd)
				SetCapture();
		}
		else
		{
			m_dragMode = IsOverSplitterBar(xPos, yPos);
			switch (m_dragMode)
			{
			case DrapMode::dragModeV:
				m_cxyDragOffset = xPos - m_splitterVPos;
				break;
			case DrapMode::dragModeH:
				m_cxyDragOffset = yPos - m_splitterHPos;
				break;
			default:
				if (::GetCapture() == m_hWnd)
					::ReleaseCapture();
				return 0;
			}

			if (::GetCapture() != m_hWnd)
			{
				ClearDUIWindowLBtnDown();
				SetCapture();
			}
		}
		return 0;
	}

	LRESULT OnLButtonUp(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_loadingPercent <= 100)
			return 0;

		ClearDUIWindowLBtnDown();
		if (::GetCapture() == m_hWnd)
		{
			mdrag = 0;
			::ReleaseCapture();
		}
		return 0;
	}

	LRESULT OnLButtonDoubleClick(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		return 0;
	}

	LRESULT OnKeyDown(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		if (m_loadingPercent <= 100)
			return 0;

		if (::GetCapture() == m_hWnd)
		{
			bool bChanged = false;
			POINT pt = { 0 };
			int xyPos;

			switch (wParam)
			{
#if 0
			case VK_RETURN:
			case VK_ESCAPE:
				::ReleaseCapture();
				break;
#endif
			case VK_LEFT:
			case VK_RIGHT:
			case VK_UP:
			case VK_DOWN:
				if (DrapMode::dragModeV == m_dragMode)
				{
					::GetCursorPos(&pt);
					int xyPos = m_splitterVPos + ((wParam == VK_LEFT) ? -MOVESTEP : MOVESTEP);
					if (xyPos >= m_splitterVPosToLeft && xyPos < (m_rectClient.right - m_splitterVPosToRight))
					{
						pt.x += (xyPos - m_splitterVPos);
						::SetCursorPos(pt.x, pt.y);
						bChanged = true;
					}
				}
				if (DrapMode::dragModeH == m_dragMode)
				{
					::GetCursorPos(&pt);
					int xyPos = m_splitterHPos + ((wParam == VK_UP) ? -MOVESTEP : MOVESTEP);

					if (xyPos >= m_splitterHPosToTop && xyPos < (m_rectClient.bottom - m_splitterHPosToBottom))
					{
						pt.y += (xyPos - m_splitterHPos);
						::SetCursorPos(pt.x, pt.y);
						bChanged = true;
					}
				}

				if (bChanged)
					Invalidate();

				break;
			default:
				break;
			}

		}
		return 0;
	}

	// When the virtial or horizonal bar is changed, We need to change the position of window 1/2/3/4/5
	// window 0 is fixed
	void AdjustDUIWindowPosition()
	{
		U32  size;
		XRECT area;
		XRECT* r = &area;
		U32* dst = m_screenBuff;

		assert(m_screenBuff);
		XRECT* xr = m_win0.GetWindowArea(); // windows 0 is always in fixed size
		r->left = xr->left; r->top = xr->top; r->right = xr->right; r->bottom = xr->bottom;
		size = (U32)((r->right - r->left) * (r->bottom - r->top));
		dst += size;

		switch (m_mode)
		{
		case AppMode::ModeTalk:
			// windows 1
			r->left = r->right; r->right = m_splitterVPos; r->top = m_rectClient.top; r->bottom = m_splitterHPosfix0;
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			m_win1.UpdateSize(r, dst);
			dst += size;
			// windows 2
			r->top = m_splitterHPosfix0 + SPLITLINE_WIDTH; r->bottom = m_rectClient.bottom;
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			m_win2.UpdateSize(r, dst);
			// windows 3
			dst += size;
			r->left = m_splitterVPos + SPLITLINE_WIDTH; r->right = m_rectClient.right; r->top = m_rectClient.top; r->bottom = m_splitterHPosfix1;
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			m_win3.UpdateSize(r, dst);
			// windows 4
			dst += size;
			r->top = m_splitterHPosfix1 + SPLITLINE_WIDTH; r->bottom = m_splitterHPos;
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			m_win4.UpdateSize(r, dst);
			// windows 5
			dst += size;
			r->top = m_splitterHPos + SPLITLINE_WIDTH; r->bottom = m_rectClient.bottom;
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			m_win5.UpdateSize(r, dst);
			break;
		case AppMode::ModeFriend:
			// windows 1
			r->left = r->right; r->right = m_splitterVPos; r->top = m_rectClient.top; r->bottom = m_splitterHPosfix0;
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			m_win1.UpdateSize(r, dst);
			dst += size;
			// windows 2
			r->top = m_splitterHPosfix0 + SPLITLINE_WIDTH; r->bottom = m_rectClient.bottom;
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			m_win2.UpdateSize(r, dst);
			// windows 3
			dst += size;
			r->left = m_splitterVPos + SPLITLINE_WIDTH; r->right = m_rectClient.right; r->top = m_rectClient.top; r->bottom = m_rectClient.bottom;
			m_win3.UpdateSize(r, dst);
			m_win4.UpdateSize(nullptr);
			m_win5.UpdateSize(nullptr);
			break;
		case AppMode::ModeSetting:
			m_win1.UpdateSize(nullptr);
			// windows 2
			r->left = r->right; r->right = m_splitterVPos; r->top = m_rectClient.top; r->bottom = m_rectClient.bottom;
			m_win2.UpdateSize(r, dst);
			size = (U32)((r->right - r->left) * (r->bottom - r->top));
			dst += size;
			// windows 3
			r->left = m_splitterVPos + SPLITLINE_WIDTH; r->right = m_rectClient.right; r->top = m_rectClient.top; r->bottom = m_rectClient.bottom;
			m_win3.UpdateSize(r, dst);
			m_win4.UpdateSize(nullptr);
			m_win5.UpdateSize(nullptr);
			break;
		case AppMode::ModeQuan:
		case AppMode::ModeCoin:
		case AppMode::ModeFavorite:
		case AppMode::ModeFile:
			// windows 1
			r->left = r->right; r->right = m_rectClient.right; r->top = m_rectClient.top; r->bottom = m_rectClient.bottom;
			m_win1.UpdateSize(r, dst);
			m_win2.UpdateSize(nullptr);
			m_win3.UpdateSize(nullptr);
			m_win4.UpdateSize(nullptr);
			m_win5.UpdateSize(nullptr);
			break;
		case AppMode::ModeMe:
			// windows 1
			r->left = r->right; r->right = m_rectClient.right; r->top = m_rectClient.top; r->bottom = m_rectClient.bottom;
			m_win1.UpdateSize(r, dst);
			m_win2.UpdateSize(nullptr);
			m_win3.UpdateSize(nullptr);
			m_win4.UpdateSize(nullptr);
			m_win5.UpdateSize(nullptr);
			break;
		default:
			ATLASSERT(0);
			break;
		}
	}

	LRESULT OnMouseMove(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);

		if (!m_isInThisWindow)
		{
			m_isInThisWindow = 1;

			TRACKMOUSEEVENT tme = { sizeof(TRACKMOUSEEVENT) };
			tme.dwFlags = TME_LEAVE | TME_HOVER;
			tme.hwndTrack = m_hWnd;
			tme.dwHoverTime = 3000; // in ms
			TrackMouseEvent(&tme);
		}

		if (m_loadingPercent <= 100) // the loading phase is not completed yet
			return 0;

		if (!DUIWindowInDragMode() && !DUIWindowLButtonDown())
		{
			bool bChanged = false;
			if (::GetCapture() == m_hWnd)
			{
				int	newSplitPos;
				switch (m_dragMode)
				{
				case DrapMode::dragModeV:
				{
					newSplitPos = xPos - m_cxyDragOffset;
					if (m_splitterVPos != newSplitPos) // the position is changed
					{
						if (newSplitPos >= m_splitterVPosToLeft && newSplitPos < (m_rectClient.right - m_rectClient.left - m_splitterVPosToRight))
						{
							m_splitterVPos = newSplitPos;
							m_splitterVPosOld = m_splitterVPos - m_rectClient.left;
							bChanged = true;
						}
					}
				}
				break;
				case DrapMode::dragModeH:
				{
					newSplitPos = yPos - m_cxyDragOffset;
					if (m_splitterHPos != newSplitPos)
					{
						if (newSplitPos >= m_splitterHPosToTop && newSplitPos < (m_rectClient.bottom - m_rectClient.top - m_splitterHPosToBottom))
						{
							m_splitterHPos = newSplitPos;
							m_splitterHPosOld = m_rectClient.bottom - m_splitterHPos;
							bChanged = true;
						}
					}
				}
				break;
				default:
					ATLASSERT(0);
					break;
				}
				if (bChanged)
					AdjustDUIWindowPosition();
			}
			else
			{
				DrapMode mode = IsOverSplitterBar(xPos, yPos);
				switch (mode)
				{
				case DrapMode::dragModeV:
					ATLASSERT(nullptr != g_hCursorWE);
					::SetCursor(g_hCursorWE);
					break;
				case DrapMode::dragModeH:
					ATLASSERT(nullptr != g_hCursorNS);
					::SetCursor(g_hCursorNS);
					break;
				default:
					break;
				}
			}
			if (bChanged)
				Invalidate();
		}

		return 0;
	}

	LRESULT OnOtherMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (DLG_ADD_FRIEND == wParam)
		{
			U8* hexPK = (U8*)lParam;
			if (hexPK != nullptr)
			{
				U32 status;
				U8 pubkey[PUBLIC_KEY_SIZE];
				assert(g_myInfo);
				status = wt_HexString2Raw(hexPK, 66, pubkey, nullptr);
				assert(status == WT_OK);
				U8* pkRobot = GetRobotPublicKey();
				assert(pkRobot);
				// this pubkey is not robot or myself
				if (memcmp(pkRobot, pubkey, PUBLIC_KEY_SIZE) && memcmp(g_myInfo->pubkey, pubkey, PUBLIC_KEY_SIZE))
				{
					WTFriend* people = FindFriend(pubkey);
					if (people == nullptr) // we cannot find this PK in our friend list
					{
						U8* blob = TryToAddNewFriend(WT_FRIEND_SOURCE_ADDMANUALLY, pubkey);
						if (blob)
						{
							status = m_win2.AddNewFriend(blob, WT_BLOB_LEN, (U8*)txtSourceManualAdd, 12);
							free(blob);
							QueryFriendInformation(pubkey);
							if (WT_OK == status)
								Invalidate();
						}
					}
				}
			}
		}
		return 0;
	}

	LRESULT OnWin0Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bool bChanged = true;

		if (m_loadingPercent <= 100) // the loading phase is not completed yet
			return 0;

		if (wParam == 0)
		{
			U16 ctlId = (U16)lParam;
			U16 mode = m_win0.GetMode();

			ctlId &= 0xFF;
			switch (ctlId)
			{
			case XWIN0_BUTTON_ME:
				m_mode = AppMode::ModeMe;
				DoMeModeLayOut();
				break;
			case XWIN0_BUTTON_TALK:
				DoTalkModeLayOut();
				break;
			case XWIN0_BUTTON_FRIEND:
				DoFriendModeLayOut();
				break;
			case XWIN0_BUTTON_SETTING:
				m_mode = AppMode::ModeSetting;
				DoSettingModeLayOut();
				break;
			case XWIN0_BUTTON_QUAN:
			case XWIN0_BUTTON_COIN:
			case XWIN0_BUTTON_FAVORITE:
			case XWIN0_BUTTON_FILE:
				m_mode = AppMode::ModeCoin;
				DoTBDModeLayOut();
				break;
			default:
				bChanged = false;
				break;
			}
		}

		if (bChanged)
			Invalidate();

		return 0;
	}

	LRESULT OnWin1Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_loadingPercent > 100) // the loading phase is completed
		{
			if (wParam == 0)
			{
				U32 status;
				U16 ctlId = (U16)lParam;
				U16 mode = ctlId >> 8;
				ctlId &= 0xFF;

				U16 m1 = m_win1.GetMode();
				ATLASSERT(m1 == mode);

				switch (mode)
				{
				case WIN1_MODE_TALK:
#if 0
					if (ctlId == XWIN1_TALK_BUTTON_SEARCH)
					{
						if (g_dwMainThreadID)
							::PostThreadMessage(g_dwMainThreadID, WM_WIN_SEARCHALLTHREAD, 0, 0L);

					}
#endif 
					break;
				case WIN1_MODE_ME:
					switch (ctlId)
					{
					case XWIN1_ME_BUTTON_MYICON:
						{
							int r = m_win1.SelectIconFile();
							if (r)
							{
								m_win0.UpdateMyIcon();
								status = SaveMyInformation();
								ATLASSERT(status == WT_OK);
								Invalidate();
							}
						}
						break;
					case XWIN1_ME_BUTTON_PUBLICKEY:
						assert(g_myInfo);
						CopyPublicKey(g_myInfo->pubkey);
						break;
					case XWIN1_ME_BUTTON_EDIT:
						{
							CEditMyInfoDlg dlg;
							if (IDOK == dlg.DoModal())
							{
								int r = m_win1.RefreshMyInfomation();
								status = SaveMyInformation();
								ATLASSERT(status == WT_OK);
								if (r) 
									Invalidate();
							}
						}
						break;
					default:
						break;
					}
					break;
				case WIN1_MODE_FRIEND:
					if (ctlId == XWIN1_FRIEND_BUTTON_SEARCH)
					{
						CAddFriendDlg dlg;
						dlg.DoModal();
					}
					break;
				case WIN1_MODE_TBD:
					break;
				default:
					ATLASSERT(0);
					break;
				}
			}
		}
		return 0;
	}

	LRESULT OnWin2Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		U16 mode = (U16)wParam;

		switch (mode)
		{
		case WIN2_MODE_TALK:
			{
				WTChatGroup* cg = (WTChatGroup*)lParam;
				if (cg)
				{
					m_mode = AppMode::ModeTalk;
					DoTalkModeLayOut();
					m_win3.SetChatGroup(cg);
					m_win4.SetChatGroup(cg);
					Invalidate();
				}
			}
			break;
		case WIN2_MODE_FRIEND:
			{
				WTFriend* people = (WTFriend*)lParam;
				if (people)
				{
					int r = m_win3.UpdateFriendInformation(people);
					if (r)
						Invalidate();
				}
			}
			break;
		default:
			break;
		}
		return 0;
	}

	LRESULT OnWin3Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		U16 mode = m_win3.GetMode();
		U8 id;
		WTFriend* p;
		switch (mode)
		{
		case WIN3_MODE_FRIEND:
			id = (U8)lParam;
			p = m_win3.GetFriend();
			if (id == XWIN3_FRIEND_BUTTON_CHAT)
			{
				if (p)
				{
					WTChatGroup* cg = p->chatgroup;
				}
			}
			else if (id == XWIN3_FRIEND_BUTTON_PUBKEY)
			{
				if (p)	CopyPublicKey(p->pubkey);
			}
			break;
		default:
			break;
		}
		return 0;
	}

	LRESULT OnWin4Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{

		return 0;
	}

	LRESULT OnWin5Message(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (m_loadingPercent > 100) // the loading phase is completed
		{
			switch (wParam)
			{
			case WIN5_UPDATE_MESSAGE:
				{
					MessageTask* mt = (MessageTask*)lParam;
					if (mt)
					{
						int r = m_win4.AddNewMessage(mt);
						if (r) Invalidate();
					}
				}
			break;
			default:
				break;
			}
		}

		return 0;
	}

	LRESULT OnLoading(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		static bool mqtt_started = false;
		m_loadingPercent = (U8)wParam;
		Invalidate();  // redraw the screen

		if (m_loadingPercent > 100 && !mqtt_started)
		{
			MessageTask* task;
			DWORD dwThreadID;
			HANDLE hThread = nullptr;
			mqtt_started = true;

			m_win2.SetFriendAndChatList(g_myInfo->people, g_myInfo->people_count, g_myInfo->chatgroup, g_myInfo->chatgroup_count);
			WTChatGroup* cg = m_win2.GetSelectedChatGroup();
			if (cg)
			{
				m_win3.SetChatGroup(cg);
				m_win4.SetChatGroup(cg);
			}

			hThread = ::CreateThread(NULL, 0, MQTTSubThread, m_hWnd, 0, &dwThreadID);
			hThread = ::CreateThread(NULL, 0, MQTTPubThread, m_hWnd, 0, &dwThreadID);

			/* we send out a reqeust to the broker to query whether my information is uploaded into the backend database */
			task = (MessageTask*)wt_palloc0(g_myInfo->pool, sizeof(MessageTask));
			if (task)
			{
				task->message = (U8*)wt_palloc0(g_myInfo->pool, PUBLIC_KEY_SIZE);
				if (task->message)
				{
					int i;
					U8* pkRobot = GetRobotPublicKey();
					ATLASSERT(pkRobot);
					for (i = 0; i < PUBLIC_KEY_SIZE; i++) task->pubkey[i] = pkRobot[i];
					assert(g_myInfo);
					for (i = 0; i < PUBLIC_KEY_SIZE; i++) task->message[i] = g_myInfo->pubkey[i];
					task->msgLen = PUBLIC_KEY_SIZE;
					task->state = MESSAGE_TASK_STATE_ONE;
					task->type = 'Q';
					PushTaskIntoSendMessageQueue(task);
				}
				else wt_pfree(task);
			}
		}
		return 0;
	}

	LRESULT OnTimer(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		int r = 0;
		bool isEqual = false, found = false;
		U8 op, i;
		U16* utf16Msg = nullptr;
		U8* binMsg = nullptr;
		U32 value32, utf16Len = 0;
		U8 pubkey[PUBLIC_KEY_SIZE];
		U8* pkRobot = GetRobotPublicKey();

		/* check the in coming queue frequently, only pick up one task */
		EnterCriticalSection(&g_csMQTTSub);
		MessageTask* p = g_SubQueue;
		while (p)
		{
			if (MESSAGE_TASK_STATE_NULL == p->state)
			{
				op = p->type;
				switch (p->type)
				{
				case 'T':
					for (i = 0; i < PUBLIC_KEY_SIZE; i++) pubkey[i] = p->pubkey[i];
					if (wt_UTF8ToUTF16(p->message, p->msgLen, nullptr, &utf16Len) == WT_OK)
					{
						utf16Msg = (U16*)wt_palloc(g_myInfo->pool, utf16Len * sizeof(U16));
						if (utf16Msg)
						{
							wt_UTF8ToUTF16(p->message, p->msgLen, utf16Msg, nullptr);
							found = true;
							r++;
							InterlockedIncrement(&(p->state));
						}
					}
					break;
				case 'R':
					value32 = p->value32;
					for (i = 0; i < PUBLIC_KEY_SIZE; i++) pubkey[i] = p->pubkey[i];
					InterlockedIncrement(&(p->state));
					found = true;
					break;
				case 'F':
					if (memcmp(p->pubkey, pkRobot, PUBLIC_KEY_SIZE) == 0)
					{
						binMsg = (U8*)wt_palloc(g_myInfo->pool, p->msgLen);
						if (binMsg)
						{
							memcpy(binMsg, p->message, p->msgLen);
							value32 = p->msgLen;
							InterlockedIncrement(&(p->state));
							found = true;
						}
					}
					break;
				default:
					break;
				}

				if (found)
					break;
			}
			p = p->next;
		}
		LeaveCriticalSection(&g_csMQTTSub);

		if (found)
		{
			switch (op)
			{
			case 'T':
				assert(utf16Msg);
				if (m_win4.IsPublicKeyMatched(pubkey)) // we first look for the pubkey in window 4
				{
					MessageTask task = { 0 };
					MessageTask* mt = &task;
					mt->message = (U8*)utf16Msg;
					mt->msgLen = utf16Len;
					mt->type = 'T';
					for (i = 0; i < PUBLIC_KEY_SIZE; i++) mt->pubkey[i] = pubkey[i];
					m_win4.AddNewMessage(mt, false);
					r++;
				}
				else // this message is not belongint to win4, look for other chat group
				{
					r = m_win2.ProcessTextMessage(pubkey, utf16Msg, utf16Len);
				}
				break;
			case 'R':
				WT_MEMCMP(g_myInfo->pubkey, pubkey, PUBLIC_KEY_SIZE, isEqual);
				if (isEqual)
				{
					if (value32 != g_myInfo->version) // upload my information to the server
					{
						MessageTask* mt = CreateMyInfoMessage();
						PushTaskIntoSendMessageQueue(mt);
					}
					//else InterlockedIncrement(&g_UpdateMyInfo); // we have completed the upload
				}
				break;
			case 'F':
				if (binMsg)
				{
					bool found = false;
					WTFriend* people = m_win2.UpdateFrinedInfo(binMsg, value32, &found);
					if (found) r++;
					wt_pfree(binMsg);
					binMsg = nullptr;
					if (people)
					{
						m_win3.UpdateFriendInformation(people);
					}
				}
				break;
			default:
				break;
			}
		}

		if (r)	Invalidate();
		return 0;
	}

	U32	DoMeModeLayOut()
	{
		ATLASSERT(AppMode::ModeMe == m_mode);

		m_splitterHPosfix0 = 0;
		m_splitterHPosfix1 = 0;
		m_splitterVPos = 0;
		m_splitterHPos = 0;

		m_win1.SetMode(WIN1_MODE_ME);
		AdjustDUIWindowPosition();

		return WT_OK;
	}

	U32	DoTalkModeLayOut()
	{
		m_mode = AppMode::ModeTalk;

		m_splitterHPosfix0 = XWIN1_HEIGHT;
		m_splitterHPosfix1 = XWIN3_HEIGHT;
		m_splitterVPos = m_splitterVPosOld;
		m_splitterHPos = (m_rectClient.bottom - m_rectClient.top) - m_splitterHPosOld;
		ATLASSERT(m_splitterVPos > 0);
		ATLASSERT(m_splitterHPos > 0);

		m_win0.SetButton(XWIN0_BUTTON_TALK);
		m_win1.SetMode(WIN1_MODE_TALK);
		m_win2.SetMode(WIN2_MODE_TALK);
		m_win3.SetMode(WIN3_MODE_TALK);
		AdjustDUIWindowPosition();

		return WT_OK;
	}

	U32	DoFriendModeLayOut()
	{
		m_mode = AppMode::ModeFriend;

		m_splitterHPosfix0 = XWIN1_HEIGHT;
		m_splitterHPosfix1 = 0;
		m_splitterVPos = m_splitterVPosOld;
		m_splitterHPos = 0;
		ATLASSERT(m_splitterVPos > 0);

		m_win1.SetMode(WIN1_MODE_FRIEND);
		m_win2.SetMode(WIN2_MODE_FRIEND);
		m_win3.SetMode(WIN3_MODE_FRIEND);
		WTFriend* people = m_win2.GetCurrentFriend();
		if (people)
			m_win3.UpdateFriendInformation(people);

		AdjustDUIWindowPosition();

		return WT_OK;
	}

	U32	DoTBDModeLayOut()
	{
		ATLASSERT(AppMode::ModeQuan == m_mode || AppMode::ModeCoin == m_mode || AppMode::ModeFavorite == m_mode || AppMode::ModeFile == m_mode);

		m_splitterHPosfix0 = 0;
		m_splitterHPosfix1 = 0;
		m_splitterVPos = 0;
		m_splitterHPos = 0;

		m_win1.SetMode(WIN1_MODE_TBD);
		AdjustDUIWindowPosition();

		return WT_OK;
	}

	U32	DoSettingModeLayOut()
	{
		ATLASSERT(AppMode::ModeSetting == m_mode);

		m_splitterHPosfix0 = 0;
		m_splitterHPosfix1 = 0;
		m_splitterVPos = m_splitterVPosOld;
		m_splitterHPos = 0;
		ATLASSERT(m_splitterVPos > 0);

		m_win2.SetMode(WIN2_MODE_SETTING);
		m_win3.SetMode(WIN3_MODE_SETTING);
		AdjustDUIWindowPosition();

		return WT_OK;
	}

	LRESULT OnSize(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		if (SIZE_MINIMIZED != wParam)
		{
			ReleaseUnknown(m_pD2DRenderTarget);

			GetClientRect(&m_rectClient);
			ATLASSERT(0 == m_rectClient.left);
			ATLASSERT(0 == m_rectClient.top);
			ATLASSERT(m_rectClient.right > 0);
			ATLASSERT(m_rectClient.bottom > 0);

			U32 w = (U32)(m_rectClient.right - m_rectClient.left);
			U32 h = (U32)(m_rectClient.bottom - m_rectClient.top);

			if (nullptr != m_screenBuff)
			{
				VirtualFree(m_screenBuff, 0, MEM_RELEASE);
				m_screenBuff = nullptr;
				m_screenSize = 0;
			}

			m_screenSize = DUI_ALIGN_PAGE(w * h * sizeof(U32));
			ATLASSERT(m_screenSize >= (w * h * sizeof(U32)));

			m_screenBuff = (U32*)VirtualAlloc(NULL, m_screenSize, MEM_COMMIT, PAGE_READWRITE);

			if (nullptr == m_screenBuff)
			{
				m_win0.UpdateSize(nullptr, nullptr);
				m_win1.UpdateSize(nullptr, nullptr);
				m_win2.UpdateSize(nullptr, nullptr);
				m_win3.UpdateSize(nullptr, nullptr);
				m_win4.UpdateSize(nullptr, nullptr);
				m_win5.UpdateSize(nullptr, nullptr);
				Invalidate();
				return 0;
			}

			if (m_splitterVPos < 0)
			{
				m_splitterVPos = (XWIN0_WIDTH + XWIN1_WIDTH);
				if (m_splitterVPos < m_splitterVPosToLeft)
					m_splitterVPos = m_splitterVPosToLeft;

				if (m_splitterVPos > (m_rectClient.right - m_rectClient.left - m_splitterVPosToRight))
				{
					m_splitterVPos = (m_rectClient.right - m_rectClient.left - m_splitterVPosToRight);
					ATLASSERT(m_splitterVPos > m_splitterVPosToLeft);
				}
				m_splitterVPosOld = m_splitterVPos;
			}

			if (m_splitterHPos < 0)
			{
				m_splitterHPos = m_rectClient.bottom - m_rectClient.top - m_splitterHPosToBottom;
				if (m_splitterHPos < m_splitterHPosToTop)
					m_splitterHPos = m_splitterHPosToTop;

				if (m_splitterHPos > (m_rectClient.bottom - m_rectClient.top - m_splitterHPosToBottom))
				{
					m_splitterHPos = m_rectClient.bottom - m_rectClient.top - m_splitterHPosToBottom;
					ATLASSERT(m_splitterHPos > m_splitterHPosToTop);
				}
				m_splitterHPosOld = (m_rectClient.bottom - m_rectClient.top) - m_splitterHPos;
			}

			{ // set  windows 0 buffer
				U32  size;
				XRECT area;
				XRECT* r = &area;
				U32* dst = m_screenBuff;

				r->left = m_rectClient.left;
				r->right = XWIN0_WIDTH;
				r->top = m_rectClient.top;
				r->bottom = m_rectClient.bottom;
				m_win0.UpdateSize(r, dst);
			}

			switch (m_mode)
			{
			case AppMode::ModeTalk:
				DoTalkModeLayOut();
				break;
			case AppMode::ModeFriend:
				DoFriendModeLayOut();
				break;
			case AppMode::ModeQuan:
			case AppMode::ModeCoin:
			case AppMode::ModeFavorite:
			case AppMode::ModeFile:
				DoTBDModeLayOut();
				break;
			case AppMode::ModeSetting:
				DoSettingModeLayOut();
				break;
			case AppMode::ModeMe:
				DoMeModeLayOut();
				break;
			default:
				ATLASSERT(0);
				break;
			}
			Invalidate();
		}

		return 0;
	}

	int GetFirstIntegralMultipleDeviceScaleFactor() const noexcept
	{
		return static_cast<int>(std::ceil(m_deviceScaleFactor));
	}

	D2D1_SIZE_U GetSizeUFromRect(const RECT& rc, const int scaleFactor) noexcept
	{
		const long width = rc.right - rc.left;
		const long height = rc.bottom - rc.top;
		const UINT32 scaledWidth = width * scaleFactor;
		const UINT32 scaledHeight = height * scaleFactor;
		return D2D1::SizeU(scaledWidth, scaledHeight);
	}

	HRESULT CreateDeviceDependantResource(int left, int top, int right, int bottom)
	{
		U8 result = 0;
		HRESULT hr = S_OK;
		if (nullptr == m_pD2DRenderTarget)  // Create a Direct2D render target.
		{
			RECT rc;
			const int integralDeviceScaleFactor = GetFirstIntegralMultipleDeviceScaleFactor();
			D2D1_RENDER_TARGET_PROPERTIES drtp{};
			drtp.type = D2D1_RENDER_TARGET_TYPE_DEFAULT;
			drtp.usage = D2D1_RENDER_TARGET_USAGE_NONE;
			drtp.minLevel = D2D1_FEATURE_LEVEL_DEFAULT;
			drtp.dpiX = 96.f * integralDeviceScaleFactor;
			drtp.dpiY = 96.f * integralDeviceScaleFactor;
			// drtp.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_UNKNOWN);
			drtp.pixelFormat = D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_IGNORE);

			rc.left = left; rc.top = top; rc.right = right; rc.bottom = bottom;
			D2D1_HWND_RENDER_TARGET_PROPERTIES dhrtp{};
			dhrtp.hwnd = m_hWnd;
			dhrtp.pixelSize = GetSizeUFromRect(rc, integralDeviceScaleFactor);
			dhrtp.presentOptions = D2D1_PRESENT_OPTIONS_NONE;

			ATLASSERT(nullptr != g_pD2DFactory);

			//hr = g_pD2DFactory->CreateHwndRenderTarget(renderTargetProperties, hwndRenderTragetproperties, &m_pD2DRenderTarget);
			hr = g_pD2DFactory->CreateHwndRenderTarget(drtp, dhrtp, &m_pD2DRenderTarget);

			if (S_OK == hr && nullptr != m_pD2DRenderTarget)
			{
				ReleaseUnknown(m_pBkgBrush0);
				ReleaseUnknown(m_pBkgBrush1);
				ReleaseUnknown(m_pCaretBrush);
				ReleaseUnknown(m_pTextBrushSel);
				ReleaseUnknown(m_pTextBrush0);
				ReleaseUnknown(m_pTextBrush1);
				ReleaseUnknown(m_pixelBitmap0);
				ReleaseUnknown(m_pixelBitmap1);
				//ReleaseUnknown(m_pixelBitmap2);

				U32 pixel[1] = { 0xFFEEEEEE };
				hr = m_pD2DRenderTarget->CreateBitmap(
					D2D1::SizeU(1, 1), pixel, 4, D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
					&m_pixelBitmap0);
				if (S_OK == hr && nullptr != m_pixelBitmap0)
				{
					pixel[0] = 0xFF056608;
					hr = m_pD2DRenderTarget->CreateBitmap(
						D2D1::SizeU(1, 1), pixel, 4, D2D1::BitmapProperties(D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)),
						&m_pixelBitmap1);
					if (S_OK == hr && nullptr != m_pixelBitmap1)
					{
						hr = m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000), &m_pTextBrush0);
						if (S_OK == hr && nullptr != m_pTextBrush0)
						{
							hr = m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000), &m_pTextBrush1);
							if (S_OK == hr && nullptr != m_pTextBrush1)
							{
								hr = m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x87CEFA), &m_pTextBrushSel);
								if (S_OK == hr && nullptr != m_pTextBrushSel)
								{
									hr = m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x000000), &m_pCaretBrush);
									if (S_OK == hr && nullptr != m_pCaretBrush)
									{
										hr = m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0x95EC6A), &m_pBkgBrush0);
										if (S_OK == hr && nullptr != m_pBkgBrush0)
											hr = m_pD2DRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF), &m_pBkgBrush1);
									}
								}
							}
						}
					}
				}
			}
		}
		return hr;
	}

	void ShowLoadingProgress(int left, int top, int right, int bottom)
	{
		int w = right - left;
		int h = bottom - top;

		ATLASSERT(w > 0 && h > 0);

		if (w > 500 && h > 100)
		{
			int offset = m_loadingPercent * 5;
			int L = (w - 500) >> 1;
			int T = (h - 8) >> 1;
			int R = L + 500;
			int B = T + 8;

			D2D1_RECT_F r1 = D2D1::RectF(static_cast<FLOAT>(L), static_cast<FLOAT>(T), static_cast<FLOAT>(R), static_cast<FLOAT>(T + 1));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r1);

			D2D1_RECT_F r2 = D2D1::RectF(static_cast<FLOAT>(L), static_cast<FLOAT>(B), static_cast<FLOAT>(R), static_cast<FLOAT>(B + 1));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r2);

			D2D1_RECT_F r3 = D2D1::RectF(static_cast<FLOAT>(L), static_cast<FLOAT>(T), static_cast<FLOAT>(L + 1), static_cast<FLOAT>(B));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r3);

			D2D1_RECT_F r4 = D2D1::RectF(static_cast<FLOAT>(R), static_cast<FLOAT>(T), static_cast<FLOAT>(R + 1), static_cast<FLOAT>(B));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r4);

			D2D1_RECT_F r5 = D2D1::RectF(static_cast<FLOAT>(L), static_cast<FLOAT>(T), static_cast<FLOAT>(L + offset), static_cast<FLOAT>(B));
			m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap1, &r5);
		}
	}

	void DrawSeperatedLines()
	{
		if (nullptr != m_pixelBitmap0)
		{
			if (m_splitterVPos > 0)
			{
				D2D1_RECT_F rect = D2D1::RectF(
					static_cast<FLOAT>(m_splitterVPos),
					static_cast<FLOAT>(m_rectClient.top),
					static_cast<FLOAT>(m_splitterVPos + SPLITLINE_WIDTH), // m_cxySplitBar),
					static_cast<FLOAT>(m_rectClient.bottom)
				);
				m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap0, &rect);
			}
			if (m_splitterHPosfix0 > 0)
			{
				D2D1_RECT_F rect = D2D1::RectF(
					static_cast<FLOAT>(XWIN0_WIDTH),
					static_cast<FLOAT>(m_splitterHPosfix0),
					static_cast<FLOAT>(m_splitterVPos),
					static_cast<FLOAT>(m_splitterHPosfix0 + SPLITLINE_WIDTH)
				);
				m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap0, &rect);
			}
			if (m_splitterHPosfix1 > 0)
			{
				D2D1_RECT_F rect = D2D1::RectF(
					static_cast<FLOAT>(m_splitterVPos + 2),
					static_cast<FLOAT>(m_splitterHPosfix1),
					static_cast<FLOAT>(m_rectClient.right),
					static_cast<FLOAT>(m_splitterHPosfix1 + SPLITLINE_WIDTH)
				);
				m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap0, &rect);
			}
			if (m_splitterHPos > 0)
			{
				D2D1_RECT_F rect = D2D1::RectF(
					static_cast<FLOAT>(m_splitterVPos + SPLITLINE_WIDTH),
					static_cast<FLOAT>(m_splitterHPos),
					static_cast<FLOAT>(m_rectClient.right),
					static_cast<FLOAT>(m_splitterHPos + SPLITLINE_WIDTH)
				);
				m_pD2DRenderTarget->DrawBitmap(m_pixelBitmap0, &rect);
			}
		}
	}

	LRESULT OnPaint(UINT /*uMsg*/, WPARAM wParam, LPARAM /*lParam*/, BOOL& bHandled)
	{
		PAINTSTRUCT ps;
		BeginPaint(&ps);

		RECT rc;
		GetClientRect(&rc);
		HRESULT hr = CreateDeviceDependantResource(rc.left, rc.top, rc.right, rc.bottom);

		if (S_OK == hr && nullptr != m_pD2DRenderTarget
			&& nullptr != m_pixelBitmap0
			&& nullptr != m_pixelBitmap1
			&& nullptr != m_pTextBrush0
			&& nullptr != m_pTextBrush1
			&& nullptr != m_pTextBrushSel
			&& nullptr != m_pCaretBrush
			&& nullptr != m_pBkgBrush0
			&& nullptr != m_pBkgBrush1)
		{
			m_pD2DRenderTarget->BeginDraw();
			////////////////////////////////////////////////////////////////////////////////////////////////////
			//m_pD2DRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
			if (m_loadingPercent > 100)
			{
				//m_pD2DRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
				DrawSeperatedLines(); // draw seperator lines
				// draw five windows
				m0 += m_win0.Draw(m_pD2DRenderTarget, m_pTextBrush0, m_pTextBrushSel, m_pCaretBrush, m_pBkgBrush0, m_pBkgBrush1);
				m1 += m_win1.Draw(m_pD2DRenderTarget, m_pTextBrush0, m_pTextBrushSel, m_pCaretBrush, m_pBkgBrush0, m_pBkgBrush1);
				m2 += m_win2.Draw(m_pD2DRenderTarget, m_pTextBrush0, m_pTextBrushSel, m_pCaretBrush, m_pBkgBrush0, m_pBkgBrush1);
				m3 += m_win3.Draw(m_pD2DRenderTarget, m_pTextBrush0, m_pTextBrushSel, m_pCaretBrush, m_pBkgBrush0, m_pBkgBrush1);
				m4 += m_win4.Draw(m_pD2DRenderTarget, m_pTextBrush0, m_pTextBrushSel, m_pCaretBrush, m_pBkgBrush0, m_pBkgBrush1);
				m5 += m_win5.Draw(m_pD2DRenderTarget, m_pTextBrush0, m_pTextBrushSel, m_pCaretBrush, m_pBkgBrush0, m_pBkgBrush1);
			}
			else
			{
				m_pD2DRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::White));
				ShowLoadingProgress(rc.left, rc.top, rc.right, rc.bottom);
			}

			hr = m_pD2DRenderTarget->EndDraw();
			////////////////////////////////////////////////////////////////////////////////////////////////////
			if (FAILED(hr) || D2DERR_RECREATE_TARGET == hr)
			{
				ReleaseUnknown(m_pD2DRenderTarget);
			}
		}

		EndPaint(&ps);
		return 0;
	}

	LRESULT OnQuitWoChat(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		PostMessage(WM_CLOSE);
		return 0;
	}

	LRESULT OnEditMyInfo(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CEditMyInfoDlg dlg;
		if (IDOK == dlg.DoModal())
		{
			U32 status;
			m_win1.RefreshMyInfomation();
			status = SaveMyInformation();
			ATLASSERT(status == WT_OK);
			Invalidate();
		}
		return 0;
	}

	LRESULT OnAddFriend(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CAddFriendDlg dlg;
		dlg.DoModal();
		return 0;
	}

#if 0
	LRESULT OnFileNewWindow(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		::PostThreadMessage(_Module.m_dwMainThreadID, WM_USER, 0, 0L);
		return 0;
	}
#endif    
};
