#ifndef __WOCHAT_WINDOWS1_H__
#define __WOCHAT_WINDOWS1_H__

#include "dui/dui_win.h"

static const wchar_t txtWin1PK[] = { 0x516c,0x94a5,0x0020,0x003a,0x0020,0 };
static const wchar_t txtWin1DT[] = { 0x521b,0x5efa,0x65f6,0x95f4,0xff1a,0 };
static const wchar_t txtWin1TBD[] = 
{ 0x672c,0x529f,0x80fd,0x6b63,0x5728,0x5f00,0x53d1,0x4e2d,0xff0c,0x8bf7,0x8010,0x5fc3,0x7b49,0x5f85,0xff01,0x003a,0x002d,0x0029,0 };

#define WIN1_MODE_TALK		DUI_MODE0
#define WIN1_MODE_ME		DUI_MODE1
#define WIN1_MODE_FRIEND	DUI_MODE2
#define WIN1_MODE_TBD		DUI_MODE3

class XWindow1 : public XWindowT <XWindow1>
{
public:
	XWindow1()
	{
		m_backgroundColor = 0xFFEAECED;
		m_property |= (DUI_PROP_MOVEWIN | DUI_PROP_HANDLETIMER | DUI_PROP_HANDLEKEYBOARD | DUI_PROP_HANDLETEXT);;
		m_message = WM_XWINDOWS01;
	}

	~XWindow1()	{}

	void InitBitmap()
	{
		U8 id, mode;
		int w, h;
		XBitmap* bmp;

		mode = WIN1_MODE_TALK;
		w = 27; h = 27;
		id = XWIN1_BITMAP_TALK_SEARCHN; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSouAllN; bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_TALK_SEARCHH; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSouAllH; bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_TALK_SEARCHP; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSouAllH; bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_TALK_SEARCHA; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSouAllH; bmp->w = w; bmp->h = h;

		mode = WIN1_MODE_ME;
		assert(g_myInfo);
		assert(g_myInfo->icon128);
		w = WT_LARGE_ICON_WIDTH;  h = WT_LARGE_ICON_HEIGHT;
		id = XWIN1_BITMAP_ME_ICON;      bmp = &(m_bmpArray[mode][id]);
		bmp->id = id; 
		bmp->data = (U32*)g_myInfo->icon128;
		bmp->w = w; bmp->h = h;

		w = 32; h = 32;
		id = XWIN1_BITMAP_ME_PUBLICKEY; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpKey;      bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_ME_DOB;       bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDOB;      bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_ME_SEX;       bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpGender;   bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_ME_AREA;      bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpLocation; bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_ME_EDIT;      bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpEdit;     bmp->w = w; bmp->h = h;

		mode = WIN1_MODE_FRIEND;
		w = 27; h = 27;
		id = XWIN1_BITMAP_FRIEND_SEARCHN; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSerachN; bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_FRIEND_SEARCHH; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSerachH; bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_FRIEND_SEARCHP; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSerachP; bmp->w = w; bmp->h = h;
		id = XWIN1_BITMAP_FRIEND_SEARCHA; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpSerachP; bmp->w = w; bmp->h = h;
	}

	void InitControl()
	{
		U16 id, mode;
		U32 objSize;
		U8* mem;

		assert(nullptr != m_pool);

		InitBitmap(); // inital all bitmap resource

		mode = WIN1_MODE_TALK;
		/////////////////////////////////////////////////////
		id = XWIN1_TALK_EDITBOX_SEARCH;
		objSize = sizeof(XEditBoxLine);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XEditBoxLine* eb = new(mem)XEditBoxLine;
			assert(nullptr != eb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_MAINTEXT);
			eb->Init(((mode <<8) | id), "W1EDITSEARCH", g_pDWriteFactory, pTextFormat);
			eb->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = eb;
		}
		else return;

		objSize = sizeof(XButton);
		id = XWIN1_TALK_BUTTON_SEARCH;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNSEARCH");
			XBitmap* bmpN = &(m_bmpArray[mode][XWIN1_BITMAP_TALK_SEARCHN]);
			XBitmap* bmpH = &(m_bmpArray[mode][XWIN1_BITMAP_TALK_SEARCHH]);
			XBitmap* bmpP = &(m_bmpArray[mode][XWIN1_BITMAP_TALK_SEARCHP]);
			XBitmap* bmpA = &(m_bmpArray[mode][XWIN1_BITMAP_TALK_SEARCHA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
			m_activeControl = id; 
		}
		else return;

		m_maxCtl[mode] = id + 1;

		mode = WIN1_MODE_ME;
		/////////////////////////////////////////////////////
		id = XWIN1_ME_BUTTON_MYICON;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNICON");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN1_BITMAP_ME_ICON]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN1_ME_LABEL_MYNAME;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			U32 utf16len;
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_OTHER0);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W1NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)g_myInfo->name, g_myInfo->name_length);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN1_ME_BUTTON_PUBLICKEY;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNPUBKEY");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN1_BITMAP_ME_PUBLICKEY]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN1_ME_LABEL_PUBLICKEY;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			U16 hexPK[67] = { 0 };
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_ENGLISHBIG);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W1PKValue", g_pDWriteFactory, pTextFormat);
			assert(g_myInfo);
			wt_Raw2HexStringW(g_myInfo->pubkey, PUBLIC_KEY_SIZE, (wchar_t*)hexPK, nullptr);
			lb->setText(hexPK, 66);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN1_ME_LABEL_MOTTO;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_CHINESEBIG);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W1LBLMotto", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)g_myInfo->motto, g_myInfo->motto_length);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN1_ME_BUTTON_DOB;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNDOB");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN1_BITMAP_ME_DOB]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			button->setProperty(XCONTROL_PROP_STATIC);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN1_ME_LABEL_DOB;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_CHINESEBIG);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W1DOB", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)txt_DOB_Unkown, 4);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN1_ME_BUTTON_SEX;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNICON");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN1_BITMAP_ME_SEX]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			button->setProperty(XCONTROL_PROP_STATIC);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN1_ME_LABEL_SEX;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_CHINESEBIG);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W1Sex", g_pDWriteFactory, pTextFormat);
			
			if(g_myInfo->sex == WT_MALE)
				lb->setText((U16*)txt_Sex_Male, 1);
			else if (g_myInfo->sex == WT_FEMALE)
				lb->setText((U16*)txt_Sex_Female, 1);
			else lb->setText((U16*)txt_Sex_Unkown, 4);

			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN1_ME_BUTTON_AREA;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNAREA");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN1_BITMAP_ME_AREA]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			button->setProperty(XCONTROL_PROP_STATIC);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN1_ME_LABEL_AREA;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_CHINESEBIG);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W1PKValue", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)g_myInfo->area, g_myInfo->area_length);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN1_ME_BUTTON_EDIT;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNICON");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN1_BITMAP_ME_EDIT]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		m_maxCtl[mode] = id + 1;

		mode = WIN1_MODE_FRIEND;
		/////////////////////////////////////////////////////
		id = XWIN1_FRIEND_EDITOBX_SEARCH;
		objSize = sizeof(XEditBoxLine);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XEditBoxLine* eb = new(mem)XEditBoxLine;
			assert(nullptr != eb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_MAINTEXT);
			eb->Init(((mode << 8) | id), "W1EDITSEARCH", g_pDWriteFactory, pTextFormat);
			eb->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = eb;
		}
		else return;

		objSize = sizeof(XButton);
		id = XWIN1_FRIEND_BUTTON_SEARCH;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W1BTNSEARCH");
			XBitmap* bmpN = &(m_bmpArray[mode][XWIN1_BITMAP_FRIEND_SEARCHN]);
			XBitmap* bmpH = &(m_bmpArray[mode][XWIN1_BITMAP_FRIEND_SEARCHH]);
			XBitmap* bmpP = &(m_bmpArray[mode][XWIN1_BITMAP_FRIEND_SEARCHP]);
			XBitmap* bmpA = &(m_bmpArray[mode][XWIN1_BITMAP_FRIEND_SEARCHA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
			m_activeControl = id;
		}
		else return;

		m_maxCtl[mode] = id + 1;

		mode = WIN1_MODE_TBD;
		/////////////////////////////////////////////////////
		id = XWIN1_TBD_LABEL_INFO;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_CHINESEBIG);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W1PKValue", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)txtWin1TBD, wcslen(txtWin1TBD));
			m_ctlArray[mode][id] = lb;
		}
		else return;

		m_maxCtl[mode] = id + 1;
	}

public:
	void AfterSetMode() 
	{
		switch (m_mode)
		{
		case WIN1_MODE_TALK:
		case WIN1_MODE_FRIEND:
			m_backgroundColor = 0xFFEAECED;
			break;
		case WIN1_MODE_ME:
		case WIN1_MODE_TBD:
			m_backgroundColor = 0xFFFFFFFF;
			break;
		default:
			assert(0);
			break;
		}
	}

	void UpdateControlPosition()
	{
		XControl* xctl;
		U16 id;
		int offset, gap = 10; // pixel
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		int sw, sh, dx, dy;
		int L, R, T, B;

		switch (m_mode)
		{
		case WIN1_MODE_TALK:
			id = XWIN1_TALK_BUTTON_SEARCH;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			sw = xctl->getWidth();
			sh = xctl->getHeight();
			dy = (h - sh) >> 1;
			dx = w - sw - gap;
			xctl->setPosition(dx, dy);

			xctl = m_ctlArray[m_mode][XWIN1_TALK_EDITBOX_SEARCH];
			assert(nullptr != xctl);
			L = gap;
			R = dx - (gap >> 1);
			T = dy;
			B = dy + sh;
			xctl->setPosition(L, T, R, B);
			break;
		case WIN1_MODE_ME:
			id = XWIN1_ME_BUTTON_MYICON;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			sw = xctl->getWidth();
			sh = xctl->getHeight();
			dx = gap;
			dy = gap;
			xctl->setPosition(dx, dy);
			T = dy + sh + gap + gap + 1;
			R = dy + sh;

			id = XWIN1_ME_LABEL_MYNAME;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			L = xctl->getHeight();
			offset = (sh - L) >> 1;
			dx = dx + sw + gap;
			dy = dy + offset;
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_LABEL_MOTTO;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			dy = dy + L + gap;
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_BUTTON_EDIT;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			sw = xctl->getWidth();
			sh = xctl->getHeight();
			dx = w - gap - sw;
			dy = R - sh;
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_BUTTON_PUBLICKEY;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			sw = xctl->getWidth();
			sh = xctl->getHeight();
			dx = gap;
			dy = T; 
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_BUTTON_DOB;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			dy += (sh + gap);
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_BUTTON_SEX;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			dy += (sh + gap);
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_BUTTON_AREA;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			dy += (sh + gap);
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_LABEL_PUBLICKEY;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			sh = xctl->getHeight();
			dx = gap + sw + gap;
			offset = (32 - sh) >> 1;
			dy = T + offset;
			T += (32 + gap);
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_LABEL_DOB;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			offset = (32 - sh) >> 1;
			dy = T + offset;
			T += (32 + gap);
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_LABEL_SEX;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			offset = (32 - sh) >> 1;
			dy = T + offset;
			T += (32 + gap);
			xctl->setPosition(dx, dy);

			id = XWIN1_ME_LABEL_AREA;
			xctl = m_ctlArray[m_mode][id];
			assert(nullptr != xctl);
			offset = (32 - sh) >> 1;
			dy = T + offset;
			xctl->setPosition(dx, dy);
			break;
		case WIN1_MODE_FRIEND:
			xctl = m_ctlArray[m_mode][XWIN1_FRIEND_BUTTON_SEARCH];
			assert(nullptr != xctl);
			sw = xctl->getWidth();
			sh = xctl->getHeight();
			dy = (h - sh) >> 1;
			dx = w - sw - gap;
			xctl->setPosition(dx, dy);

			xctl = m_ctlArray[m_mode][XWIN1_FRIEND_EDITOBX_SEARCH];
			assert(nullptr != xctl);
			L = gap;
			R = dx - (gap >> 1);
			T = dy;
			B = dy + sh;
			xctl->setPosition(L, T, R, B);

			break;
		case WIN1_MODE_TBD:
			xctl = m_ctlArray[m_mode][XWIN1_TBD_LABEL_INFO];
			assert(nullptr != xctl);
			sw = xctl->getWidth();
			sh = xctl->getHeight();
			dx = (w - sw) >> 1;
			dy = (h - sh) >> 1;
			xctl->setPosition(dx, dy);
			break;
		default:
			assert(0);
			break;
		}
	}

	int Do_DUI_PAINT(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr) 
	{ 
		if (WIN1_MODE_ME == m_mode)
		{
			int sw, sh, gap = 10; // pixel
			int w = m_area.right - m_area.left;
			int h = m_area.bottom - m_area.top;
			XControl* xctl;
			xctl = xctl = m_ctlArray[m_mode][XWIN1_ME_BUTTON_MYICON];
			sw = xctl->getWidth();
			sh = xctl->getHeight();
			DUI_ScreenFillRect(m_screen, w, h, 0xFFDDDDDD, w - gap - gap, 1, gap, gap + sw + gap);
		}

		return 0; 
	}

	int SelectIconFile()
	{
		int r = 0;
		OPENFILENAME ofn = { 0 };       // common dialog box structure
		WCHAR path[MAX_PATH + 1] = { 0 };

		// Initialize OPENFILENAME
		ofn.lStructSize = sizeof(OPENFILENAMEW);
		ofn.hwndOwner = m_hWnd;
		ofn.lpstrFile = path;
		//
		// Set lpstrFile[0] to '\0' so that GetOpenFileName does not 
		// use the contents of file to initialize itself.
		//
		ofn.lpstrFile[0] = _T('\0');
		ofn.nMaxFile = MAX_PATH; //sizeof(path) / sizeof(TCHAR);
		ofn.lpstrFilter = _T("24-bit bitmap files(*.bmp)\0*.bmp\0\0");
		ofn.nFilterIndex = 1;
		ofn.lpstrFileTitle = NULL;
		ofn.nMaxFileTitle = 0;
		ofn.lpstrInitialDir = NULL;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

		/* Display the Open dialog box. */
		if (GetOpenFileName(&ofn) == TRUE)
		{
			int fd = 0;
			if (_wsopen_s(&fd, path, _O_RDONLY | _O_BINARY, _SH_DENYWR, 0) == 0)
			{
				U8* bmpbuf;
				U32 size = (U32)_lseek(fd, 0, SEEK_END); /* get the file size */
				_lseek(fd, 0, SEEK_SET); /* go to the begin of the file */

				bmpbuf = (U8*)malloc(size);
				if (bmpbuf)
				{
					U32 bytes = _read(fd, bmpbuf, size);
					_close(fd);
					if (bytes == size)
					{
						WT_BITMAPFILEHEADER* fh = (WT_BITMAPFILEHEADER*)bmpbuf;
						WT_BITMAPINFOHEADER* ih = (WT_BITMAPINFOHEADER*)(bmpbuf + sizeof(WT_BITMAPFILEHEADER));
						if (ih->biWidth == WT_LARGE_ICON_WIDTH && ih->biHeight == WT_LARGE_ICON_HEIGHT && ih->biBitCount == 24 && ih->biCompression == 0)
						{
							assert(g_myInfo->icon128);
							if (0 == (g_myInfo->property & WT_MYINFO_LARGEICON_ALLOC))
							{
								g_myInfo->icon128 = (U32*)wt_palloc(g_myInfo->ctx, WT_LARGE_ICON_SIZE);
								if (g_myInfo->icon128)
									g_myInfo->property |= WT_MYINFO_LARGEICON_ALLOC;
							}
							if (g_myInfo->icon128)
							{
								int i;
								U16 lineByte = (U16)(ih->biSizeImage / ih->biHeight);
								U8 pixelBytes = (U8)(ih->biBitCount >> 3);
								U8* p = bmpbuf + sizeof(WT_BITMAPFILEHEADER) + sizeof(WT_BITMAPINFOHEADER);
								U8* q = p + lineByte * (ih->biHeight - 1);
								U8* b = (U8*)g_myInfo->icon128;
								while (q >= p)
								{
									for (i = 0; i < ih->biWidth * pixelBytes; i += pixelBytes)
									{
										*b++ = q[i + 2]; *b++ = q[i + 1]; *b++ = q[i]; *b++ = 0xFF;
									}
									q -= lineByte;
								}
								wt_Resize128To32Bmp(g_myInfo->icon128, g_myInfo->icon32);
								XBitmap* bmp = &(m_bmpArray[WIN1_MODE_ME][XWIN1_BITMAP_ME_ICON]);
								bmp->data = (U32*)g_myInfo->icon128;
								r++;
								m_status |= DUI_STATUS_NEEDRAW;  // need to redraw this virtual window
								InvalidateDUIWindow();           // set the gloabl redraw flag so next paint routine will do the paint work
							}
						}
					}
					free(bmpbuf);
				}
				else _close(fd);
			}
		}
		return r;
	}

	int RefreshMyInfomation()
	{
		int r = 0;
		U8 mode = WIN1_MODE_ME;
		U8 id;
		XLabel* lb;
		
		id = XWIN1_ME_LABEL_MYNAME;
		lb = (XLabel *)m_ctlArray[mode][id];
		lb->setText((U16*)g_myInfo->name, g_myInfo->name_length);

		id = XWIN1_ME_LABEL_MOTTO;
		lb = (XLabel*)m_ctlArray[mode][id];
		lb->setText((U16*)g_myInfo->motto, g_myInfo->motto_length);

		id = XWIN1_ME_LABEL_AREA;
		lb = (XLabel*)m_ctlArray[mode][id];
		lb->setText((U16*)g_myInfo->area, g_myInfo->area_length);

		if (m_mode == WIN1_MODE_ME)
		{
			r++;
			UpdateControlPosition();
			InvalidateScreen();
		}
		return r;
	}
};

#endif  /* __WOCHAT_WINDOWS1_H__ */

