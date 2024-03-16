#include "stdafx.h"
#include "secp256k1.h"
#include "resource.h"
#include "wochatypes.h"
#include "wochatdef.h"
#include "wochat.h"

static bool isLoginRegSuccessful = false;
static const wchar_t txtVeryBad[] = { 0x53d1, 0x751f, 0x9519, 0x8bef, 0 };
static const wchar_t txtVeryGood[] = { 0x4e00, 0x5207, 0x6b63, 0x5e38, 0 };
static const wchar_t txtImportData[] = { 0x5bfc,0x5165, 0 };

static const wchar_t titleRegistration[] = { 0x6ce8, 0x518c, 0x8d26,0x53f7, 0 };
static const wchar_t titleLogin[] = { 0x767b, 0x5f55, 0 };
static const wchar_t txtBtnCancel[] = { 0x53d6, 0x6d88, 0 };
static const wchar_t txtBtnRegistration[] = { 0x6ce8, 0x518c, 0 };
static const wchar_t txtStaticName[] = { 0x7528, 0x6237, 0x540d, 0xff1a, 0 };
static const wchar_t txtStaticPWD0[] = { 0x5bc6, 0x7801, 0xff1a, 0 };
static const wchar_t txtHide[] = { 0x9690, 0x85cf ,0 };

static const wchar_t txtInvalidPubKey[] = {
	0x8d26, 0x53f7, 0x683c, 0x5f0f, 0x4e0d, 0x5408, 0x683c, 0xff01,
	0x5408, 0x683c, 0x7684, 0x8d26, 0x53f7, 0x7c7b, 0x4f3c, 0x5982,
	0x4e0b, 0x683c, 0x5f0f, 0xff1a, 0x000a, 0x0030, 0x0033, 0x0033,
	0x0033, 0x0039, 0x0041, 0x0031, 0x0043, 0x0038, 0x0046, 0x0044,
	0x0042, 0x0036, 0x0041, 0x0046, 0x0046, 0x0034, 0x0036, 0x0038,
	0x0034, 0x0035, 0x0045, 0x0034, 0x0039, 0x0044, 0x0031, 0x0031,
	0x0030, 0x0045, 0x0030, 0x0034, 0x0030, 0x0030, 0x0030, 0x0032,
	0x0031, 0x0045, 0x0031, 0x0036, 0x0031, 0x0034, 0x0036, 0x0033,
	0x0034, 0x0031, 0x0038, 0x0035, 0x0038, 0x0035, 0x0038, 0x0035,
	0x0043, 0x0032, 0x0045, 0x0032, 0x0035, 0x0043, 0x0041, 0x0033,
	0x0039, 0x0039, 0x0043, 0x0030, 0x0031, 0x0043, 0x0041, 0 };

static const wchar_t txtCannotOpenSK[] = { 0x65e0,0x6cd5,0x6253,0x5f00,0x79c1,0x94a5,0xff01,0 };
static const wchar_t txtCannotCreateSK[] = {
	0x65e0, 0x6cd5, 0x521b, 0x5efa, 0x8d26, 0x53f7, 0xff0c, 0x8bf7, 0x8054, 0x7cfb, 0x5ba2, 0x670d, 0x4eba, 0x5458, 0xff01, 0
};
static const wchar_t txtCreateGood[] = { 0x8d26, 0x53f7, 0x521b, 0x5efa, 0x6210, 0x529f, 0x0021, 0 };
static const wchar_t txtCreateBad[] = { 0x8d26, 0x53f7, 0x521b, 0x5efa, 0x5931, 0x8d25, 0x0021, 0x0020, 0x4f60, 0x53ef, 0x4ee5, 0x91cd, 0x65b0, 0x521b, 0x5efa, 0x3002, 0x003a, 0x002d, 0x0029, 0 };
static const wchar_t txtPWDIsNotSame[] = {
0x4e24, 0x6b21, 0x8f93, 0x5165, 0x7684, 0x5bc6, 0x7801, 0x4e0d, 0x76f8, 0x540c, 0xff0c, 0x8bf7, 0x91cd, 0x65b0, 0x8bbe, 0x7f6e, 0x5bc6, 0x7801, 0xff01, 0 };

static const wchar_t txtNameEmpty[] = { 0x7528, 0x6237, 0x540d, 0x4e0d, 0x80fd, 0x4e3a, 0x7a7a, 0xff01, 0 };
static const wchar_t txtPWDEmpty[] = { 0x5bc6, 0x7801, 0x4e0d, 0x80fd, 0x4e3a, 0x7a7a, 0xff01, 0 };

static const wchar_t txtN1[] = { 0x6625, 0x7533, 0x95e8, 0x4e0b, 0x4e00, 0x5200, 0x5ba2, 0 };
static const wchar_t txtN2[] = { 0x57ce, 0x5357, 0x5c0f, 0x675c, 0 };

static BOOL	g_fThemeApiAvailable = FALSE;
static int WidthofLoginDlg = 0;
static int HeightofLoginDlg = 0;

static HTHEME _OpenThemeData(HWND hwnd, LPCWSTR pszClassList)
{
	if (g_fThemeApiAvailable)
		return OpenThemeData(hwnd, pszClassList);
	else
		return NULL;
}

static HRESULT _CloseThemeData(HTHEME hTheme)
{
	if (g_fThemeApiAvailable)
		return CloseThemeData(hTheme);
	else
		return E_FAIL;
}

//
//	Subclass procedure for an owner-drawn button.
//  All this does is to re-enable double-click behaviour for
//  an owner-drawn button.
//
static LRESULT CALLBACK BBProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC oldproc = (WNDPROC)GetWindowLongPtr(hwnd, GWLP_USERDATA);
	TRACKMOUSEEVENT tme = { sizeof(tme) };

	static BOOL mouseOver = FALSE;
	POINT pt;
	RECT  rect;

	switch (msg)
	{
	case WM_LBUTTONDBLCLK:
		msg = WM_LBUTTONDOWN;
		break;

	case WM_MOUSEMOVE:

		if (!mouseOver)
		{
			SetTimer(hwnd, 0, 15, 0);
			mouseOver = FALSE;
		}
		break;

	case WM_TIMER:

		GetCursorPos(&pt);
		ScreenToClient(hwnd, &pt);
		GetClientRect(hwnd, &rect);

		if (PtInRect(&rect, pt))
		{
			if (!mouseOver)
			{
				mouseOver = TRUE;
				InvalidateRect(hwnd, 0, 0);
			}
		}
		else
		{
			mouseOver = FALSE;
			KillTimer(hwnd, 0);
			InvalidateRect(hwnd, 0, 0);
		}

		return 0;

		// Under Win2000 / XP, Windows sends a strange message
		// to dialog controls, whenever the ALT key is pressed
		// for the first time (i.e. to show focus rect / & prefixes etc).
		// msg = 0x0128, wParam = 0x00030003, lParam = 0
	case 0x0128:
		InvalidateRect(hwnd, 0, 0);
		break;
	}

	return CallWindowProc(oldproc, hwnd, msg, wParam, lParam);
}

//
//	Convert the specified button into an owner-drawn button.
//  The button does NOT need owner-draw or icon styles set
//  in the resource editor - this function sets these
//  styles automatically
//
static void MakeBitmapButton(HWND hwnd, UINT uIconId)
{
	WNDPROC oldproc;
	DWORD   dwStyle;

	if (GetModuleHandle(_T("uxtheme.dll")))
		g_fThemeApiAvailable = TRUE;
	else
		g_fThemeApiAvailable = FALSE;

	HICON hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(uIconId), IMAGE_ICON, 16, 16, 0);

	// Add on BS_ICON and BS_OWNERDRAW styles
	dwStyle = GetWindowLong(hwnd, GWL_STYLE);
	SetWindowLong(hwnd, GWL_STYLE, dwStyle | BS_ICON | BS_OWNERDRAW);

	// Assign icon to the button
	SendMessage(hwnd, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);

	// Subclass (to reenable double-clicks)
	oldproc = (WNDPROC)SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)BBProc);

	// Store old procedure
	SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)oldproc);

	if (g_fThemeApiAvailable)
		SetWindowTheme(hwnd, L"explorer", NULL);
}

static U32 LoginDialogAddUserName(HWND hWnd)
{
	sqlite3* db;
	int rc = sqlite3_open16(g_DBPath, &db);
	if (SQLITE_OK == rc)
	{
		sqlite3_stmt* stmt = NULL;
		rc = sqlite3_prepare_v2(db, (const char*)"SELECT id,ub FROM k WHERE pt=0 AND at<>0 ORDER BY id", -1, &stmt, NULL);
		if (SQLITE_OK == rc)
		{
			int  id;
			U8* blob;
			U32 blen, utf16len, status, rowCount = 0;
			U8* utf8Name;
			U8* s;
			U8  utf8len;
			U16 utf16[WT_NAME_MAX_LEN] = { 0 };

			while (SQLITE_ROW == sqlite3_step(stmt))
			{
				id = sqlite3_column_int(stmt, 0);
				blob = (U8*)sqlite3_column_blob(stmt, 1);
				blen = (U32)sqlite3_column_bytes(stmt, 1);

				if (blob && blen == WT_BLOB_LEN)
				{
					utf8Name = blob + 4 + PUBLIC_KEY_SIZE + 4;
					utf8len = 0;
					s = utf8Name;
					while (*s++)
					{
						utf8len++;
						if (utf8len == WT_NAME_MAX_LEN)
							break;
					}
					status = wt_UTF8ToUTF16(utf8Name, utf8len, nullptr, &utf16len);
					if (WT_OK == status && utf16len < (WT_NAME_MAX_LEN >> 1))
					{
						wt_UTF8ToUTF16(utf8Name, utf8len, utf16, nullptr);
						utf16[utf16len] = 0;
						SendMessage(hWnd, CB_ADDSTRING, 0, (LPARAM)utf16);
						SendMessage(hWnd, CB_SETITEMDATA, rowCount, (LPARAM)(id));
						rowCount++;
					}
				}
			}
			if (rowCount)
				SendMessage(hWnd, CB_SETCURSEL, 0, 0);
		}
		sqlite3_finalize(stmt);
	}
	sqlite3_close(db);

	return WT_OK;
}

class CRegistrationDlg : public CDialogImpl<CRegistrationDlg>
{
public:
	enum { IDD = IDD_REGISTER };

	BEGIN_MSG_MAP(CRegistrationDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER(IDB_CREATE, OnRegistration)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDCANCEL);
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		HICON hIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
		if (hIcon)
			SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		HWND hWndCtl;
		SetWindowTextW((LPCWSTR)titleRegistration);
		hWndCtl = GetDlgItem(IDB_CANCEL);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtBtnCancel);
		hWndCtl = GetDlgItem(IDB_CREATE);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtBtnRegistration);
		hWndCtl = GetDlgItem(IDB_IMPORT);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtImportData);
		hWndCtl = GetDlgItem(IDC_STATIC_NAME);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtStaticName);
		hWndCtl = GetDlgItem(IDC_STATIC_PWD0);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtStaticPWD0);
		hWndCtl = GetDlgItem(IDC_STATIC_PWD1);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtStaticPWD0);
		/* set the input limitation */
		hWndCtl = GetDlgItem(IDC_EDIT_NAME);
		if (hWndCtl)
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_NAME_MAX_LEN, 0);
		hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
		if (hWndCtl)
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_PASSWD_MAX_LEN, 0);
		hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
		if (hWndCtl)
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_PASSWD_MAX_LEN, 0);

		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnRegistration(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		bool checkIsGood = true;
		int len, len0, len1;
		wchar_t name[WT_CHAR_NAME_MAX_LEN + 1] = { 0 };
		wchar_t pwd0[WT_CHAR_PASSWD_MAX_LEN + 1] = { 0 };
		wchar_t pwd1[WT_CHAR_PASSWD_MAX_LEN + 1] = { 0 };

		HWND hWndCtl = GetDlgItem(IDC_EDIT_NAME);
		len = ::GetWindowTextW(hWndCtl, name, WT_CHAR_NAME_MAX_LEN);
		if (0 == len)
		{
			MessageBoxW(txtNameEmpty, txtVeryBad, MB_OK);
			checkIsGood = false;
		}
		else
		{
			hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
			len0 = ::GetWindowTextW(hWndCtl, pwd0, WT_CHAR_PASSWD_MAX_LEN);
			hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
			len1 = ::GetWindowTextW(hWndCtl, pwd1, WT_CHAR_PASSWD_MAX_LEN);
			if (len0 == 0 && len1 == 0)
			{
				MessageBox(txtPWDEmpty, txtVeryBad, MB_OK);
				checkIsGood = false;
			}
			else
			{
				if (len0 != len1)
				{
					MessageBox(txtPWDIsNotSame, txtVeryBad, MB_OK);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
					::SetWindowTextW(hWndCtl, nullptr);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
					::SetWindowTextW(hWndCtl, nullptr);
					checkIsGood = false;
				}
				else if (wmemcmp(pwd0, pwd1, len0))
				{
					MessageBox(txtPWDIsNotSame, txtVeryBad, MB_OK);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
					::SetWindowTextW(hWndCtl, nullptr);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
					::SetWindowTextW(hWndCtl, nullptr);
					checkIsGood = false;
				}
			}
		}

		if (checkIsGood)
		{
			U32 idxSk = 0xFFFFFFFF;
			isLoginRegSuccessful = false;
			if (WT_OK == CreateNewAccount((U16*)name, len, (U16*)pwd0, len0, &idxSk))
			{
				MessageBox(txtCreateGood, txtVeryGood, MB_OK);
				if (WT_OK == OpenAccount(idxSk, (U16*)pwd0, (U32)len0))
					isLoginRegSuccessful = true;
				else
					MessageBox(txtCannotOpenSK, txtVeryBad, MB_OK);
			}
			else
				MessageBox(txtCannotCreateSK, txtVeryBad, MB_OK);

			EndDialog(wID);
		}
		return 0;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};

#define X_ICON_BORDER	3	// 3 pixels

class CLoginDlg : public CDialogImpl<CLoginDlg>
{
public:
	enum { IDD = IDD_LOGIN };

	BEGIN_MSG_MAP(CLoginDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		MESSAGE_HANDLER(WM_DRAWITEM, DrawBitmapButton)
		MESSAGE_HANDLER(WM_CLOSE, OnClose)
		COMMAND_ID_HANDLER(IDOK, OnLogin)
		COMMAND_ID_HANDLER(IDB_REGISTER, OnRegistration)
		COMMAND_ID_HANDLER(IDB_CREATE, OnNewAccount)
		COMMAND_ID_HANDLER(IDB_CANCEL, OnCloseCmd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnClose(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		EndDialog(IDCANCEL);
		return 0;
	}

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		RECT rcDlg = { 0 };
		HWND hWndCtl;
		HICON hIcon = (HICON)LoadImageW(g_hInstance, MAKEINTRESOURCE(IDR_MAINFRAME), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR | LR_DEFAULTSIZE);
		if (hIcon)
			SendMessage(WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
		SetWindowTextW((LPCWSTR)titleLogin);
		hWndCtl = GetDlgItem(IDC_STATIC_NAME);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtStaticName);
		hWndCtl = GetDlgItem(IDC_STATIC_NNAME);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtStaticName);

		hWndCtl = GetDlgItem(IDC_STATIC_PWD0);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtStaticPWD0);
		hWndCtl = GetDlgItem(IDC_STATIC_NPWD);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtStaticPWD0);

		hWndCtl = GetDlgItem(IDB_CANCEL);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtBtnCancel);
		hWndCtl = GetDlgItem(IDOK);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)titleLogin);
		hWndCtl = GetDlgItem(IDB_CREATE);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtBtnRegistration);

		hWndCtl = GetDlgItem(IDB_REGISTER);
		if (hWndCtl)
		{
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtBtnRegistration);
			MakeBitmapButton(hWndCtl, IDI_ICON16);
		}

		hWndCtl = GetDlgItem(IDC_COMBO_NAME);
		if (hWndCtl)
		{
			LoginDialogAddUserName(hWndCtl);
		}

		hWndCtl = GetDlgItem(IDC_EDIT_NAME);
		if (hWndCtl)
		{
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_NAME_MAX_LEN, 0);
			::EnableWindow(hWndCtl, FALSE);
		}
		hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
		if (hWndCtl)
		{
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_PASSWD_MAX_LEN, 0);
			::EnableWindow(hWndCtl, FALSE);
		}
		hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
		if (hWndCtl)
		{
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_PASSWD_MAX_LEN, 0);
			::EnableWindow(hWndCtl, FALSE);
		}
		hWndCtl = GetDlgItem(IDB_CREATE);
		if (hWndCtl)
			::EnableWindow(hWndCtl, FALSE);

		GetWindowRect(&rcDlg);
		WidthofLoginDlg = rcDlg.right - rcDlg.left;
		HeightofLoginDlg = rcDlg.bottom - rcDlg.top;
		SetWindowPos(0, 500, 300, rcDlg.right - rcDlg.left - 280, rcDlg.bottom - rcDlg.top, SWP_SHOWWINDOW);

		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT DrawBitmapButton(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& /*bHandled*/)
	{
		DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;
		RECT rect;			// Drawing rectangle
		POINT pt;

		int ix, iy;			// Icon offset
		int bx, by;			// border sizes
		int sxIcon, syIcon;	// Icon size
		int xoff, yoff;		// 

		TCHAR szText[100];
		int   nTextLen;

		HICON hIcon;
		DWORD dwStyle = ::GetWindowLong(dis->hwndItem, GWL_STYLE);

		DWORD dwDTflags = DT_CENTER | DT_SINGLELINE | DT_VCENTER;
		BOOL  fRightAlign;

		// XP/Vista theme support
		DWORD dwThemeFlags;
		HTHEME hTheme;
		BOOL   fDrawThemed = g_fThemeApiAvailable;

		if (dis->itemState & ODS_NOFOCUSRECT)
			dwDTflags |= DT_HIDEPREFIX;

		fRightAlign = (dwStyle & BS_RIGHT) ? TRUE : FALSE;
		fRightAlign = TRUE;

		// do the theme thing
		hTheme = _OpenThemeData(dis->hwndItem, L"Button");
		switch (dis->itemAction)
		{
			// We need to redraw the whole button, no
			// matter what DRAWITEM event we receive..
		case ODA_FOCUS:
		case ODA_SELECT:
		case ODA_DRAWENTIRE:

			// Retrieve button text
			::GetWindowText(dis->hwndItem, szText, sizeof(szText) / sizeof(TCHAR));

			nTextLen = lstrlen(szText);

			// Retrieve button icon
			hIcon = (HICON)SendMessage(dis->hwndItem, BM_GETIMAGE, IMAGE_ICON, 0);

			// Find icon dimensions
			sxIcon = 16;
			syIcon = 16;

			CopyRect(&rect, &dis->rcItem);
			GetCursorPos(&pt);
			::ScreenToClient(dis->hwndItem, &pt);

			if (PtInRect(&rect, pt))
				dis->itemState |= ODS_HOTLIGHT;

			// border dimensions
			bx = 2;
			by = 2;

			// icon offsets
			if (nTextLen == 0)
			{
				// center the image if no text
				ix = (rect.right - rect.left - sxIcon) / 2;
			}
			else
			{
				if (fRightAlign)
					ix = rect.right - bx - X_ICON_BORDER - sxIcon;
				else
					ix = rect.left + bx + X_ICON_BORDER;
			}

			// center image vertically
			iy = (rect.bottom - rect.top - syIcon) / 2;

			InflateRect(&rect, -5, -5);

			// Draw a single-line black border around the button
			if (hTheme == NULL && (dis->itemState & (ODS_FOCUS | ODS_DEFAULT)))
			{
				FrameRect(dis->hDC, &dis->rcItem, (HBRUSH)GetStockObject(BLACK_BRUSH));
				InflateRect(&dis->rcItem, -1, -1);
			}

			if (dis->itemState & ODS_FOCUS)
				dwThemeFlags = PBS_DEFAULTED;
			if (dis->itemState & ODS_DISABLED)
				dwThemeFlags = PBS_DISABLED;
			else if (dis->itemState & ODS_SELECTED)
				dwThemeFlags = PBS_PRESSED;
			else if (dis->itemState & ODS_HOTLIGHT)
				dwThemeFlags = PBS_HOT;
			else if (dis->itemState & ODS_DEFAULT)
				dwThemeFlags = PBS_DEFAULTED;
			else
				dwThemeFlags = PBS_NORMAL;

			// Button is DOWN
			if (dis->itemState & ODS_SELECTED)
			{
				// Draw a button
				if (hTheme != NULL)
					DrawThemeBackground(hTheme, dis->hDC, BP_PUSHBUTTON, dwThemeFlags, &dis->rcItem, 0);
				else
					DrawFrameControl(dis->hDC, &dis->rcItem, DFC_BUTTON, DFCS_BUTTONPUSH | DFCS_PUSHED | DFCS_FLAT);

				// Offset contents to make it look "pressed"
				if (hTheme == NULL)
				{
					OffsetRect(&rect, 1, 1);
					xoff = yoff = 1;
				}
				else
					xoff = yoff = 0;
			}
			// Button is UP
			else
			{
				if (hTheme != NULL)
					DrawThemeBackground(hTheme, dis->hDC, BP_PUSHBUTTON, dwThemeFlags, &dis->rcItem, 0);
				else
					DrawFrameControl(dis->hDC, &dis->rcItem, DFC_BUTTON, DFCS_BUTTONPUSH);

				xoff = yoff = 0;
			}

			// Draw the icon
			DrawIconEx(dis->hDC, ix + xoff, iy + yoff, hIcon, sxIcon, syIcon, 0, 0, DI_NORMAL);

			// Adjust position of window text 
			if (fRightAlign)
			{
				rect.left += bx + X_ICON_BORDER;
				rect.right -= sxIcon + bx + X_ICON_BORDER;
			}
			else
			{
				rect.right -= bx + X_ICON_BORDER;
				rect.left += sxIcon + bx + X_ICON_BORDER;
			}

			// Draw the text
			OffsetRect(&rect, 0, -1);
			SetBkMode(dis->hDC, TRANSPARENT);
			DrawText(dis->hDC, szText, -1, &rect, dwDTflags);
			OffsetRect(&rect, 0, 1);

			// Draw the focus rectangle (only if text present)
			if ((dis->itemState & ODS_FOCUS) && nTextLen > 0)
			{
				if (!(dis->itemState & ODS_NOFOCUSRECT))
				{
					// Get a "fresh" copy of the button rectangle
					CopyRect(&rect, &dis->rcItem);

					if (fRightAlign)
						rect.right -= sxIcon + bx;
					else
						rect.left += sxIcon + bx + 2;

					InflateRect(&rect, -3, -3);
					DrawFocusRect(dis->hDC, &rect);
				}
			}

			break;
		}
		_CloseThemeData(hTheme);

		return 0;
	}

	LRESULT OnRegistration(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		static BOOL bLoginOnly = TRUE;
		HICON hIcon, hOld;
		bLoginOnly = !bLoginOnly;
		int offset = bLoginOnly ? 280 : 0;
		HWND hWndCtl;
		RECT rcDlg;
		GetWindowRect(&rcDlg);

		SetWindowPos(0, rcDlg.left, rcDlg.top, WidthofLoginDlg - offset, HeightofLoginDlg, SWP_SHOWWINDOW);
		hWndCtl = GetDlgItem(IDB_REGISTER);
		if (bLoginOnly)
		{
			if (hWndCtl)
			{
				::SetWindowTextW(hWndCtl, (LPCWSTR)txtBtnRegistration);
				hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ICON16), IMAGE_ICON, 16, 16, 0);
				hOld = (HICON)SendDlgItemMessage(IDB_REGISTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				DestroyIcon(hOld);
				hWndCtl = GetDlgItem(IDC_EDIT_NAME);
				if (hWndCtl)
					::EnableWindow(hWndCtl, FALSE);
				hWndCtl = GetDlgItem(IDB_CREATE);
				if (hWndCtl)
					::EnableWindow(hWndCtl, FALSE);
				hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
				if (hWndCtl)
					::EnableWindow(hWndCtl, FALSE);
				hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
				if (hWndCtl)
					::EnableWindow(hWndCtl, FALSE);
			}
		}
		else
		{
			if (hWndCtl)
			{
				::SetWindowTextW(hWndCtl, (LPCWSTR)txtHide);
				hIcon = (HICON)LoadImage(g_hInstance, MAKEINTRESOURCE(IDI_ICON18), IMAGE_ICON, 16, 16, 0);
				hOld = (HICON)SendDlgItemMessage(IDB_REGISTER, BM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
				DestroyIcon(hOld);
				hWndCtl = GetDlgItem(IDC_EDIT_NAME);
				if (hWndCtl)
					::EnableWindow(hWndCtl, TRUE);
				hWndCtl = GetDlgItem(IDB_CREATE);
				if (hWndCtl)
					::EnableWindow(hWndCtl, TRUE);
				hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
				if (hWndCtl)
					::EnableWindow(hWndCtl, TRUE);
				hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
				if (hWndCtl)
					::EnableWindow(hWndCtl, TRUE);
			}
		}

		return 0;
	}

	LRESULT OnLogin(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		HWND hWndCtl = GetDlgItem(IDC_COMBO_NAME);
		if (hWndCtl)
		{
			wchar_t pwd[64 + 1] = { 0 };
			int selectedIndex = ::SendMessage(hWndCtl, CB_GETCURSEL, 0, 0);
			U32 skIdx = (U32)::SendMessage(hWndCtl, CB_GETITEMDATA, (WPARAM)selectedIndex, 0);

			hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD);
			int len = ::GetWindowTextW(hWndCtl,pwd, 64);
			if (len > 0)
			{
				U32 ret = OpenAccount(skIdx, (U16*)pwd, (U32)len);
				if (WT_OK == ret)
				{
					EndDialog(wID);
					isLoginRegSuccessful = true;
				}
				else
				{
					::SetWindowTextW(hWndCtl, nullptr);
					MessageBox(txtCannotOpenSK, txtVeryBad, MB_OK);
				}
			}
		}
		return 0;
	}

	LRESULT OnNewAccount(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		bool checkIsGood = true;
		int len, len0, len1;
		wchar_t name[WT_CHAR_NAME_MAX_LEN + 1] = { 0 };
		wchar_t pwd0[WT_CHAR_PASSWD_MAX_LEN + 1] = { 0 };
		wchar_t pwd1[WT_CHAR_PASSWD_MAX_LEN + 1] = { 0 };

		HWND hWndCtl = GetDlgItem(IDC_EDIT_NAME);
		len = ::GetWindowTextW(hWndCtl, name, 64);
		if (0 == len)
		{
			MessageBox(txtNameEmpty, txtVeryBad, MB_OK);
			checkIsGood = false;
		}
		else
		{
			hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
			len0 = ::GetWindowTextW(hWndCtl, pwd0, WT_CHAR_PASSWD_MAX_LEN);
			hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
			len1 = ::GetWindowTextW(hWndCtl, pwd1, WT_CHAR_PASSWD_MAX_LEN);
			if (len0 == 0 && len1 == 0)
			{
				MessageBox(txtPWDEmpty, txtVeryBad, MB_OK);
				checkIsGood = false;
			}
			else
			{
				if (len0 != len1)
				{
					MessageBox(txtPWDIsNotSame, txtVeryBad, MB_OK);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
					::SetWindowTextW(hWndCtl, nullptr);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
					::SetWindowTextW(hWndCtl, nullptr);
					checkIsGood = false;
				}
				else if (wmemcmp(pwd0, pwd1, len0))
				{
					MessageBox(txtPWDIsNotSame, txtVeryBad, MB_OK);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD0);
					::SetWindowTextW(hWndCtl, nullptr);
					hWndCtl = GetDlgItem(IDC_EDIT_PASSWORD1);
					::SetWindowTextW(hWndCtl, nullptr);
					checkIsGood = false;
				}
			}
		}
		if (checkIsGood)
		{
			U32 idxSk = 0xFFFFFFFF;
			isLoginRegSuccessful = false;
			if (WT_OK == CreateNewAccount((U16*)name, len, (U16*)pwd0, len0, &idxSk))
			{
				MessageBox(txtCreateGood, txtVeryGood, MB_OK);
				if (WT_OK == OpenAccount(idxSk, (U16*)pwd0, (U32)len0))
				{
					isLoginRegSuccessful = true;
					EndDialog(wID);
				}
				else
					MessageBox(txtCannotOpenSK, txtVeryBad, MB_OK);
			}
			else
				MessageBox(txtCannotCreateSK, txtVeryBad, MB_OK);
		}
		return 0;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};

bool DoLogin()
{
	isLoginRegSuccessful = false;
	CLoginDlg dlg;
	dlg.DoModal();
	return isLoginRegSuccessful;
}

bool DoRegistration()
{
	isLoginRegSuccessful = false;

	CRegistrationDlg dlg;
	dlg.DoModal();

	return isLoginRegSuccessful;
}

U32 OpenAccount(U32 idx, U16* pwd, U32 len)
{
	U32 ret = WT_FAIL;
	U32 utf8len;
	U8* utf8Password;

	ret = wt_UTF16ToUTF8(pwd, len, nullptr, &utf8len);
	if (WT_OK != ret)
		return WT_UTF16ToUTF8_ERR;

	ret = WT_FAIL;
	utf8Password = (U8*)wt_palloc(g_messageMemPool, utf8len);
	if (utf8Password)
	{
		int  rc;
		sqlite3* db;
		sqlite3_stmt* stmt = NULL;
		U8 hash[SHA256_HASH_SIZE];

		ret = wt_UTF16ToUTF8(pwd, len, utf8Password, nullptr);
		assert(ret == WT_OK);
		wt_sha256_hash(utf8Password, len, hash); // we use the UTF8 password to hash the key to encrypt the secret key
		wt_pfree(utf8Password);

		ret = WT_FAIL;
		rc = sqlite3_open16(g_DBPath, &db);
		if (SQLITE_OK == rc)
		{
			U8 sql[64] = { 0 };
			sprintf_s((char*)sql, 64, "SELECT sk,ub,dt FROM k WHERE id=%d AND at<>0 AND pt=0", idx);
			rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
			if (SQLITE_OK == rc)
			{
				rc = sqlite3_step(stmt);
				if (SQLITE_ROW == rc)
				{
					U8* hexSK = (U8*)sqlite3_column_text(stmt, 0);
					U8* blob = (U8*)sqlite3_column_blob(stmt, 1);
					U32 blen = (U32)sqlite3_column_bytes(stmt, 1);
					S64 dt = (S64)sqlite3_column_int64(stmt, 2);

					U8 skLen = (U8)strlen((const char*)hexSK);

					if (skLen == 64 && wt_IsHexString(hexSK, skLen) && blen == WT_BLOB_LEN)
					{
						U8 sk[SECRET_KEY_SIZE];
						U8 pk[PUBLIC_KEY_SIZE];

						assert(g_myInfo);
						wt_HexString2Raw(hexSK, 64, g_myInfo->skenc, nullptr);

						AES256_ctx ctxAES = { 0 };
						wt_AES256_init(&ctxAES, hash);
						wt_AES256_decrypt(&ctxAES, 2, sk, g_myInfo->skenc);
						if (WT_OK == GenPublicKeyFromSecretKey(sk, pk))
						{
							bool isEqual = false;
							U8* p = blob + 4;
							WT_MEMCMP(pk, p, PUBLIC_KEY_SIZE, isEqual);
							if (isEqual) // the public key is matched! so we are good
							{
								U8* s;
								U8 length;
								U32 status, utf16len;
								U32* imgL = GetUIBitmap(WT_UI_BMP_MYLARGEICON);
								assert(imgL);
								U8* utf8Name = blob + 4 + PUBLIC_KEY_SIZE + 4;
								U8* utf8Area = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN;
								U8* utf8Motto = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN;
								U8* icon = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN;
								U8* iconLarge = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN + WT_SMALL_ICON_SIZE;

								wt_sha256_hash(sk, SECRET_KEY_SIZE, hash);
								wt_Raw2HexString(hash, 11, g_MQTTPubClientId, nullptr);
								wt_Raw2HexString(hash + 16, 11, g_MQTTSubClientId, nullptr);
								g_MQTTSubClientId[0] = 'S';
								g_MQTTPubClientId[0] = 'P';

								ret = WT_OK;

								memcpy(g_myInfo->seckey, sk, SECRET_KEY_SIZE);
								memcpy(g_myInfo->pubkey, pk, PUBLIC_KEY_SIZE);
								g_myInfo->version = *((U32*)(blob));
								g_myInfo->creation_date = dt;
								//g_myInfo->dob = *((U32*)(blob + 4 + PUBLIC_KEY_SIZE + 3 + 1));
								//p = blob;
								//g_myInfo->sex = p[4 + PUBLIC_KEY_SIZE + 3];

								length = 0; s = utf8Name;
								while (*s++)
								{
									length++;
									if (length == WT_NAME_MAX_LEN)
										break;
								}
								status = wt_UTF8ToUTF16(utf8Name, length, nullptr, &utf16len);
								if (WT_OK == status && utf16len < (WT_NAME_MAX_LEN >> 1))
								{
									wt_UTF8ToUTF16(utf8Name, length, (U16*)g_myInfo->name, nullptr);
									g_myInfo->name_length = utf16len;
								}
								length = 0; s = utf8Area;
								while (*s++)
								{
									length++;
									if (length == WT_AREA_MAX_LEN)
										break;
								}
								status = wt_UTF8ToUTF16(utf8Area, length, nullptr, &utf16len);
								if (WT_OK == status && utf16len < (WT_AREA_MAX_LEN >> 1))
								{
									wt_UTF8ToUTF16(utf8Area, length, (U16*)g_myInfo->area, nullptr);
									g_myInfo->area_length = utf16len;
								}
								length = 0; s = utf8Motto;
								while (*s++)
								{
									length++;
									if (length == WT_MOTTO_MAX_LEN)
										break;
								}
								status = wt_UTF8ToUTF16(utf8Motto, length, nullptr, &utf16len);
								if (WT_OK == status && utf16len < (WT_MOTTO_MAX_LEN >> 1))
								{
									wt_UTF8ToUTF16(utf8Motto, length, (U16*)g_myInfo->motto, nullptr);
									g_myInfo->motto_length = utf16len;
								}
								memcpy(g_myInfo->icon, icon, WT_SMALL_ICON_SIZE);

								g_myInfo->iconLargeW = g_myInfo->iconLargeH = 128;
								if (memcmp(imgL, iconLarge, WT_SMALL_ICON_SIZE) == 0)
								{
									g_myInfo->iconLarge = imgL; // the default large icon
								}
								else
								{
									g_myInfo->property |= WT_MYINFO_LARGEICON_ALLOC; // indicate that iconLarge is freeable
									g_myInfo->iconLarge = (U32*)wt_palloc(g_topMemPool, WT_LARGE_ICON_SIZE);
									memcpy(g_myInfo->iconLarge, iconLarge, WT_LARGE_ICON_SIZE);
								}
							}
						}
					}
				}
				sqlite3_finalize(stmt);
			}
			sqlite3_close(db);
		}
	}
	return ret;
}

static const U8 defaultTextName[]   = { 0xE5,0xA7,0x93,0xE5,0x90,0x8D,0xE6,0x9C,0xAA,0xE7,0x9F,0xA5,0 };
static const U8 defaultTextMotto[]  = { 0xE6,0x97,0xA0,0xE5,0xBA,0xA7,0xE5,0x8F,0xB3,0xE9,0x93,0xAD,0 };
static const U8 defaultTextArea[]   = { 0xE4,0xB8,0x8D,0xE6,0x98,0x8E,0xE5,0x9C,0xB0,0xE5,0x8C,0xBA,0 };
static const U8 defaultTextSource[] = { 0xE6,0x89,0x8B,0xE5,0x8A,0xA8,0xE6,0xB7,0xBB,0xE5,0x8A,0xA0,0 };

U32 CreateNewAccount(U16* name, U8 nlen, U16* pwd, U8 plen, U32* skIdx)
{
	int rc;
	U32 ret = WT_FAIL;
	U8  randomize[32];
	U8  sk[SECRET_KEY_SIZE] = { 0 };
	U8  pk[PUBLIC_KEY_SIZE] = { 0 };
	AES256_ctx ctxAES = { 0 };

	secp256k1_context* ctx = secp256k1_context_create(SECP256K1_CONTEXT_NONE);
	if (ctx)
	{
		U8 tries = 255;
		/* Randomizing the context is recommended to protect against side-channel
		 * leakage See `secp256k1_context_randomize` in secp256k1.h for more
		 * information about it. This should never fail. */
		if (WT_OK == wt_GenerateRandom32Bytes(randomize))
		{
			rc = secp256k1_context_randomize(ctx, randomize);
			assert(rc);
		}
		else
		{
			secp256k1_context_destroy(ctx);
			return WT_RANDOM_NUMBER_ERROR;
		}
		// we try to generate a new secret key
		while (tries)
		{
			ret = wt_GenerateRandom32Bytes(sk);
			rc = secp256k1_ec_seckey_verify(ctx, sk);
			if (WT_OK == ret && 1 == rc)
				break;
			tries--;
		}

		if (0 == tries) // we cannot generate the 32 bytes secret key
		{
			secp256k1_context_destroy(ctx);
			return WT_SK_GENERATE_ERR;
		}

		ret = WT_FAIL;  // now we get the SK, we need to generate the PK
		{
			secp256k1_pubkey pubkey;
			size_t pklen = 33;
			rc = secp256k1_ec_pubkey_create(ctx, &pubkey, sk);
			if (1 == rc)
			{
				rc = secp256k1_ec_pubkey_serialize(ctx, pk, &pklen, &pubkey, SECP256K1_EC_COMPRESSED);
				if (pklen == 33 && rc == 1)
					ret = WT_OK;
			}
		}
		/* This will clear everything from the context and free the memory */
		secp256k1_context_destroy(ctx);
	}
	else return WT_SECP256K1_CTX_ERROR;

	if (WT_OK == ret)
	{
		U8 hash[SHA256_HASH_SIZE];
		U8* blob;
		U32 blen, utf8len = 0;
		U8  skEncrypted[SECRET_KEY_SIZE] = { 0 };
		U8 utf8Name[WT_NAME_MAX_LEN] = { 0 };
		U8 hexSK[65] = { 0 };

		ret = wt_UTF16ToUTF8((U16*)pwd, plen, nullptr, &utf8len);
		if (WT_OK == ret)
		{
			U8* utf8Password = (U8*)wt_palloc(g_messageMemPool, utf8len);
			if (utf8Password)
			{
				ret = wt_UTF16ToUTF8((U16*)pwd, plen, utf8Password, nullptr);
				assert(ret == WT_OK);
				wt_sha256_hash(utf8Password, utf8len, hash); // we use the UTF8 password to hash the key to encrypt the secret key
				wt_pfree(utf8Password);
			}
			else return WT_MEMORY_ALLOC_ERR;
		}

		ret = wt_UTF16ToUTF8((U16*)name, nlen, nullptr, &utf8len);
		if (WT_OK != ret || utf8len > WT_NAME_MAX_LEN - 1)
			return WT_UTF16ToUTF8_ERR;

		ret = wt_UTF16ToUTF8((U16*)name, nlen, utf8Name, nullptr);
		assert(ret == WT_OK);

		wt_AES256_init(&ctxAES, hash);
		wt_AES256_encrypt(&ctxAES, 2, skEncrypted, sk);
		wt_Raw2HexString(skEncrypted, SECRET_KEY_SIZE, hexSK, nullptr);

		ret = WT_FAIL;
		blen = WT_BLOB_LEN;
		blob = (U8*)malloc(blen);
		if (blob)
		{
			sqlite3* db;
			rc = sqlite3_open16(g_DBPath, &db);
			if (SQLITE_OK == rc)
			{
				int i;
				U32* imgMeLarge = nullptr;
				U32* imgMeSmall = nullptr;
				U32 crc32, Wl = 0, Hl = 0, Ws = 0, Hs = 0;
				sqlite3_stmt* stmt = NULL;
				U8 sql[SQL_STMT_MAX_LEN] = { 0 };
				U8* p = blob + 4;

				for (i = 0; i < PUBLIC_KEY_SIZE; i++) p[i] = pk[i]; // save the public key

				p = blob + 4 + PUBLIC_KEY_SIZE;
				for (i = 0; i < (4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN); i++) p[i] = 0;

				p = blob + 4 + PUBLIC_KEY_SIZE + 4; /* now point to the name field */
				for (i = 0; i < utf8len; i++) p[i] = utf8Name[i];
				/* now point to the area field */
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN;
				for (i = 0; i < 12; i++) p[i] = defaultTextArea[i];
				/* now point to the motto field */
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN;
				for (i = 0; i < 12; i++) p[i] = defaultTextMotto[i];
				/* now point to the small icon field */
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN;
				imgMeSmall = GetUIBitmap(WT_UI_BMP_MYSMALLICON, &Ws, &Hs);
				assert(imgMeSmall);
				assert(Ws * Hs * 4 == WT_SMALL_ICON_SIZE);
				U8* q = (U8*)imgMeSmall;
				for (i = 0; i < WT_SMALL_ICON_SIZE; i++) p[i] = q[i];

				/* now point to the small icon field */
				p = blob + 4 + PUBLIC_KEY_SIZE + 4 + WT_NAME_MAX_LEN + WT_AREA_MAX_LEN + WT_MOTTO_MAX_LEN + WT_SMALL_ICON_SIZE;
				imgMeLarge = GetUIBitmap(WT_UI_BMP_MYLARGEICON, &Wl, &Hl);
				assert(imgMeLarge);
				assert(Wl * Hl * 4 == WT_LARGE_ICON_SIZE);
				q = (U8*)imgMeLarge;
				for (i = 0; i < WT_LARGE_ICON_SIZE; i++) p[i] = q[i];

				crc32 = wt_GenCRC32(blob + 4 + PUBLIC_KEY_SIZE, blen - 4 - PUBLIC_KEY_SIZE);
				*((U32*)blob) = crc32; // save the CRC32 value

				sprintf_s((char*)sql, SQL_STMT_MAX_LEN,	"INSERT INTO k(pt,at,sk,dt,ub) VALUES(0,1,'%s',(?),(?))", hexSK);
				rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
				if (SQLITE_OK == rc)
				{
					U8 result = 0;
					S64 dt = GetCurrentUTCTime64();
					rc = sqlite3_bind_int64(stmt, 1, dt);
					if (SQLITE_OK != rc) result++;
					rc = sqlite3_bind_blob(stmt, 2, blob, blen, SQLITE_TRANSIENT);
					if (SQLITE_OK != rc) result++;
					if (result == 0)
					{
						rc = sqlite3_step(stmt);
						if (SQLITE_DONE == rc)
							ret = WT_OK;
					}
					sqlite3_finalize(stmt);

					if (skIdx)
					{
						sprintf_s((char*)sql, SQL_STMT_MAX_LEN, "SELECT id FROM k WHERE sk='%s'", hexSK);
						rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
						if (SQLITE_OK == rc)
						{
							rc = sqlite3_step(stmt);
							if (SQLITE_ROW == rc)
							{
								*skIdx = (U32)sqlite3_column_int(stmt, 0);
							}
							sqlite3_finalize(stmt);
						}
					}
				}
			}
			sqlite3_close(db);
			free(blob);
			blob = nullptr;
		}
	}
	return ret;
}

