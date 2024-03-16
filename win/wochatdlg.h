#ifndef __WOCHAT_DLG_H__
#define __WOCHAT_DLG_H__

static const wchar_t txtItIsBad[] = { 0x53d1, 0x751f, 0x9519, 0x8bef, 0 };
static const wchar_t txtSearchAdd[] = { 0x6dfb, 0x52a0, 0x597d, 0x53cb, 0 };
static const wchar_t txtSearch[] = { 0x641c, 0 };
static const wchar_t txtAdd[] = { 0x6dfb, 0x52a0, 0 };
static const wchar_t txtCancel[] = { 0x53d6, 0x6d88, 0 };
static wchar_t txtInvalidPubKey[] = {
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

static U8 friend_pubkey[67] = { 0 };

class CAddFriendDlg : public CDialogImpl<CAddFriendDlg>
{
public:
	enum { IDD = IDD_SEARCHADD };

	BEGIN_MSG_MAP(CAddFriendDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnAddFriend)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		SetWindowTextW((LPCWSTR)txtSearchAdd);
		HWND hWndCtl = GetDlgItem(IDOK);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtAdd);
		hWndCtl = GetDlgItem(IDCANCEL);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtCancel);

		CenterWindow(GetParent());
		return TRUE;
	}

	bool PublicKeyIsValid(wchar_t* str)
	{
		bool bRet = false;
		if (str)
		{
			if (str[0] != L'0')
				return false;

			if ((str[1] == L'2') || (str[1] == L'3')) // the first two bytes is '02' or '03' for public key
			{
				U8 i;
				wchar_t oneChar;
				for (i = 2; i < 66; i++)
				{
					oneChar = str[i];
					if (oneChar >= L'0' && oneChar <= L'9')
						continue;
					if (oneChar >= L'A' && oneChar <= L'F')
						continue;
					break;
				}
				if (66 == i)
					bRet = true;
			}
		}
		return bRet;
	}

	LRESULT OnAddFriend(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		HWND hWndCtl = GetDlgItem(IDC_EDIT_SEARCH);
		if (hWndCtl)
		{
			wchar_t wpubkey[67] = { 0 };
			int num = ::GetWindowTextW(hWndCtl, wpubkey, 67);
			if (num == 66 && PublicKeyIsValid(wpubkey))
			{
				if (wt_UTF16ToUTF8((U16*)wpubkey, 66, friend_pubkey, nullptr) == WT_OK)
				{
					HWND hWndParent = GetParent();
					if (hWndParent)
						::PostMessage(hWndParent, WM_ALLOTHER_MESSAGE, DLG_ADD_FRIEND, (LPARAM)friend_pubkey);
				}
				EndDialog(wID);
			}
			else if (num)
				{
					MessageBox(txtInvalidPubKey, txtItIsBad, MB_OK);
					::SetWindowTextW(hWndCtl, nullptr);
				}
		}
		return 0;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};

static const wchar_t txtUpdateMyInfo[] = { 0x66f4,0x65b0,0x6211,0x7684,0x4fe1,0x606f,0 };
static const wchar_t txtUpdateMyInfo_Name[] = { 0x59d3,0x540d,0 };
static const wchar_t txtUpdateMyInfo_Motto[] = { 0x5ea7,0x53f3,0x94ed,0 };
static const wchar_t txtUpdateMyInfo_Area[] = { 0x6240,0x5728,0x5730,0x533a,0 };
static const wchar_t txtUpdateMyInfo_OK[] = { 0x66f4,0x65b0,0 };
static const wchar_t txtUpdateMyInfo_Cancel[] = { 0x53d6,0x6d88,0 };
static const wchar_t txtUpdateMyInfo_Sex[] = { 0x6027,0x522b,0 };
static const wchar_t txtUpdateMyInfo_Male[] = { 0x7537,0 };
static const wchar_t txtUpdateMyInfo_Female[] = { 0x5973,0 };
static const wchar_t txtUpdateMyInfo_Unkown[] = { 0x62d2,0x7edd,0x56de,0x7b54,0 };
static const wchar_t txtUpdateMyInfo_DOB[] = { 0x751f,0x65e5,0 };

class CEditMyInfoDlg : public CDialogImpl<CEditMyInfoDlg>
{
public:
	enum { IDD = IDD_DIALOG_MYINFO };

	BEGIN_MSG_MAP(CEditMyInfoDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDCANCEL, OnCloseCmd)
	END_MSG_MAP()

	// Handler prototypes (uncomment arguments if needed):
	//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)
	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		int i;
		wchar_t* p;
		wchar_t editName[WT_CHAR_NAME_MAX_LEN + 1] = { 0 };
		wchar_t editMotto[WT_CHAR_MOTTO_MAX_LEN + 1] = { 0 };
		wchar_t editArea[WT_CHAR_AREA_MAX_LEN + 1] = { 0 };

		SetWindowTextW((LPCWSTR)txtUpdateMyInfo);
		HWND hWndCtl = GetDlgItem(IDOK);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_OK);

		hWndCtl = GetDlgItem(IDCANCEL);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Cancel);

		hWndCtl = GetDlgItem(IDC_STATIC_NAME);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Name);

		hWndCtl = GetDlgItem(IDC_STATIC_MOTTO);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Motto);

		hWndCtl = GetDlgItem(IDC_STATIC_AREA);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Area);

		hWndCtl = GetDlgItem(IDC_STATIC_SEX);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Sex);

		hWndCtl = GetDlgItem(IDC_RADIO_MALE);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Male);

		hWndCtl = GetDlgItem(IDC_RADIO_FEMALE);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Female);

		hWndCtl = GetDlgItem(IDC_RADIO_UNKNOWN);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_Unkown);

		hWndCtl = GetDlgItem(IDC_STATIC_DOB);
		if (hWndCtl)
			::SetWindowTextW(hWndCtl, (LPCWSTR)txtUpdateMyInfo_DOB);

		hWndCtl = GetDlgItem(IDC_EDIT_NAME);
		if (hWndCtl)
		{
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_NAME_MAX_LEN, 0);
			if (g_myInfo->name_length <= WT_CHAR_NAME_MAX_LEN)
			{
				p = (wchar_t*)g_myInfo->name;
				for (i = 0; i < g_myInfo->name_length; i++) editName[i] = p[i];
				::SetWindowTextW(hWndCtl, editName);
			}
		}

		hWndCtl = GetDlgItem(IDC_EDIT_MOTTO);
		if (hWndCtl)
		{
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_MOTTO_MAX_LEN, 0);
			if (g_myInfo->motto_length <= WT_CHAR_MOTTO_MAX_LEN)
			{
				p = (wchar_t*)g_myInfo->motto;
				for (i = 0; i < g_myInfo->motto_length; i++) editMotto[i] = p[i];
				::SetWindowTextW(hWndCtl, editMotto);
			}
		}

		hWndCtl = GetDlgItem(IDC_EDIT_AREA);
		if (hWndCtl)
		{
			::SendMessage(hWndCtl, EM_LIMITTEXT, WT_CHAR_AREA_MAX_LEN, 0);
			if (g_myInfo->area_length <= WT_CHAR_AREA_MAX_LEN)
			{
				p = (wchar_t*)g_myInfo->area;
				for (i = 0; i < g_myInfo->area_length; i++) editArea[i] = p[i];
				::SetWindowTextW(hWndCtl, editArea);
			}
		}
		switch (g_myInfo->sex)
		{
		case WT_MALE:
			CheckRadioButton(IDC_RADIO_MALE, IDC_RADIO_UNKNOWN, IDC_RADIO_MALE);
			break;
		case WT_FEMALE:
			CheckRadioButton(IDC_RADIO_MALE, IDC_RADIO_UNKNOWN, IDC_RADIO_FEMALE);
			break;
		default:
			CheckRadioButton(IDC_RADIO_MALE, IDC_RADIO_UNKNOWN, IDC_RADIO_UNKNOWN);
			break;
		}

		CenterWindow(GetParent());
		return TRUE;
	}

	LRESULT OnCloseCmd(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
};

#endif //__WOCHAT_DLG_H__