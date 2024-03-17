#ifndef __DUI_WINDOW0_H__
#define __DUI_WINDOW0_H__

#include "dui/dui_win.h"

#define WIN0_MODE_NORMAL	DUI_MODE0

class XWindow0 : public XWindowT <XWindow0>
{
	bool m_networkIsGood = false;
public:
	XWindow0()
	{
		m_backgroundColor = 0xFF2A2928;
		m_property |= (DUI_PROP_MOVEWIN | DUI_PROP_BTNACTIVE);
		m_message = WM_XWINDOWS00;
		m_mode = WIN0_MODE_NORMAL;
	}
	~XWindow0() {}

public:
	void UpdateControlPosition()
	{
		int sw, sh, dx, dy, T;
		int gap = 8; // pixel
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		XControl* xctl;

		xctl = m_ctlArray[0][XWIN0_BUTTON_NETWORK];
		assert(nullptr != xctl);
		sw = xctl->getWidth();
		sh = xctl->getHeight();
		dy = h - gap - sh;
		dx = (w - sw) >> 1;
		xctl->setPosition(dx, dy);
		T = dy;

		xctl = m_ctlArray[0][XWIN0_BUTTON_SETTING];
		assert(nullptr != xctl);
		sw = xctl->getWidth();
		sh = xctl->getHeight();
		dy = T - (gap>>1) - sh;
		dx = (w - sw) >> 1;
		xctl->setPosition(dx, dy);
	}

private:
	void InitBitmap()
	{
		U8 id;
		XBitmap* bmp;

		int w = 32;
		int h = 32;

		id = XWIN0_BITMAP_ME;        
		bmp = &(m_bmpArray[0][id]); bmp->id = id; 
		//bmp->data = GetUIBitmap(WT_UI_BMP_MYMSALLICON);  
		assert(g_myInfo);
		assert(g_myInfo->icon32);
		bmp->data = g_myInfo->icon32;
		bmp->w = w; bmp->h = h;

		id = XWIN0_BITMAP_TALKN;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpTalkN;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_TALKH;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpTalkH;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_TALKP;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpTalkP;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_TALKA;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpTalkA;     bmp->w = w; bmp->h = h;

		id = XWIN0_BITMAP_FRIENDN;   bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFriendN;   bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FRIENDH;   bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFriendH;   bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FRIENDP;   bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFriendP;   bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FRIENDA;   bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFriendA;   bmp->w = w; bmp->h = h;

		id = XWIN0_BITMAP_QUANN;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpQuanN;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_QUANH;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpQuanH;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_QUANP;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpQuanP;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_QUANA;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpQuanA;     bmp->w = w; bmp->h = h;

		id = XWIN0_BITMAP_SETTINGN;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpSettingN;  bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_SETTINGH;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpSettingH;  bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_SETTINGP;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpSettingP;  bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_SETTINGA;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpSettingA;  bmp->w = w; bmp->h = h;

		id = XWIN0_BITMAP_FILEN;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFileN;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FILEH;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFileH;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FILEP;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFileP;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FILEA;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFileA;     bmp->w = w; bmp->h = h;

		id = XWIN0_BITMAP_FAVORITEN; bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFavoriteN; bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FAVORITEH; bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFavoriteH; bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FAVORITEP; bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFavoriteP; bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_FAVORITEA; bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpFavoriteA; bmp->w = w; bmp->h = h;

		id = XWIN0_BITMAP_COINN;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpCoinN;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_COINH;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpCoinH;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_COINP;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpCoinP;     bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_COINA;     bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpCoinA;     bmp->w = w; bmp->h = h;

		w = 13; h = 13;
		id = XWIN0_BITMAP_NETWORKN;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpNetWorkN;  bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_NETWORKH;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpNetWorkH;  bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_NETWORKP;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpNetWorkN;  bmp->w = w; bmp->h = h;
		id = XWIN0_BITMAP_NETWORKA;  bmp = &(m_bmpArray[0][id]); bmp->id = id; bmp->data = (U32*)xbmpNetWorkN;  bmp->w = w; bmp->h = h;
	}

public:
	void InitControl()
	{
		U16 id;
		int	w = XWIN0_WIDTH;
		int left, top, sw, sh, T;
		U32 objSize;

		U8* mem;
		XBitmap* bmpN;
		XBitmap* bmpH;
		XBitmap* bmpP;
		XBitmap* bmpA;
		XButton* button;

		assert(nullptr != m_pool);

		InitBitmap(); // inital all bitmap resource

		m_maxCtl[m_mode] = 0;

		id = XWIN0_BUTTON_ME;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0ME");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_ME]);
			button->setBitmap(bmpN, bmpN, bmpN, bmpN);
			sw = button->getWidth(); sh = button->getHeight();
			left = top = (w - sw) >> 1;
			button->setPosition(left, top);
			T = top + sh + 20;
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[0][id] = button;
		}
		else return;

		id = XWIN0_BUTTON_TALK;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0TALK");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_TALKN]);
			bmpH = &(m_bmpArray[0][XWIN0_BITMAP_TALKH]);
			bmpP = &(m_bmpArray[0][XWIN0_BITMAP_TALKP]);
			bmpA = &(m_bmpArray[0][XWIN0_BITMAP_TALKA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			button->setStatus(XCONTROL_STATE_ACTIVE);
			m_ctlArray[0][id] = button;
			m_activeControl = id; // we set talk as the first active button
		}
		else return;

		id = XWIN0_BUTTON_FRIEND;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0FRIEND");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_FRIENDN]);
			bmpH = &(m_bmpArray[0][XWIN0_BITMAP_FRIENDH]);
			bmpP = &(m_bmpArray[0][XWIN0_BITMAP_FRIENDP]);
			bmpA = &(m_bmpArray[0][XWIN0_BITMAP_FRIENDA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[0][id] = button;
		}
		else return;

		id = XWIN0_BUTTON_QUAN;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0QUAN");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_QUANN]);
			bmpH = &(m_bmpArray[0][XWIN0_BITMAP_QUANH]);
			bmpP = &(m_bmpArray[0][XWIN0_BITMAP_QUANP]);
			bmpA = &(m_bmpArray[0][XWIN0_BITMAP_QUANA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[0][id] = button;
		}
		else return;

		id = XWIN0_BUTTON_COIN;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0COIN");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_COINN]);
			bmpH = &(m_bmpArray[0][XWIN0_BITMAP_COINH]);
			bmpP = &(m_bmpArray[0][XWIN0_BITMAP_COINP]);
			bmpA = &(m_bmpArray[0][XWIN0_BITMAP_COINA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[0][id] = button;
		}
		else return;

		id = XWIN0_BUTTON_FAVORITE;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0FAVORITE");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_FAVORITEN]);
			bmpH = &(m_bmpArray[0][XWIN0_BITMAP_FAVORITEH]);
			bmpP = &(m_bmpArray[0][XWIN0_BITMAP_FAVORITEP]);
			bmpA = &(m_bmpArray[0][XWIN0_BITMAP_FAVORITEA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[0][id] = button;
		}
		else return;

		id = XWIN0_BUTTON_FILE;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0FILE");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_FILEN]);
			bmpH = &(m_bmpArray[0][XWIN0_BITMAP_FILEH]);
			bmpP = &(m_bmpArray[0][XWIN0_BITMAP_FILEP]);
			bmpA = &(m_bmpArray[0][XWIN0_BITMAP_FILEA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[0][id] = button;
		}
		else return;

		id = XWIN0_BUTTON_SETTING;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0SETTING");
			bmpN = &(m_bmpArray[0][XWIN0_BITMAP_SETTINGN]);
			bmpH = &(m_bmpArray[0][XWIN0_BITMAP_SETTINGH]);
			bmpP = &(m_bmpArray[0][XWIN0_BITMAP_SETTINGP]);
			bmpA = &(m_bmpArray[0][XWIN0_BITMAP_SETTINGA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			//button->setProperty(XCONTROL_PROP_ACTIVE);
			m_ctlArray[0][id] = button;
		}
		else return;

		id = XWIN0_BUTTON_NETWORK;
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((m_mode << 8) | id), "W0NETWORK");
			if (g_NetworkStatus)
			{
				m_networkIsGood = true;
				bmpN = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKN]);
				bmpH = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKN]);
				bmpP = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKN]);
				bmpA = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKN]);
			}
			else
			{
				m_networkIsGood = false;
				bmpN = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKH]);
				bmpH = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKH]);
				bmpP = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKH]);
				bmpA = &(m_bmpArray[0][XWIN0_BITMAP_NETWORKH]);
			}
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			sw = button->getWidth(); sh = button->getHeight();
			left = (w - sw) >> 1;
			top = T;
			T = top + sh + sh;
			button->setPosition(left, top);
			button->setProperty(XCONTROL_PROP_STATIC);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[0][id] = button;
		}
		m_maxCtl[m_mode] = 9;
	}

	int Do_DUI_TIMER(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr) 
	{ 
		int r = 0;

		bool isNetworkGood = g_NetworkStatus; // take a snapshot of this global variable
		if (isNetworkGood)
		{
			if (!m_networkIsGood)
				r++;
		}
		else
		{
			if (m_networkIsGood)
				r++;
		}
		m_networkIsGood = isNetworkGood;

		if (r)
		{
			XButton* btn = (XButton*)m_ctlArray[0][XWIN0_BUTTON_NETWORK];
			XBitmap* bmp = m_networkIsGood? &(m_bmpArray[0][XWIN0_BITMAP_NETWORKN]) : &(m_bmpArray[0][XWIN0_BITMAP_NETWORKH]);
			btn->setBitmap(bmp, bmp, bmp, bmp);
		}
		return r; 
	}

	int SetButton(U8 buttonId)
	{
		if (buttonId >= XWIN0_BUTTON_TALK && buttonId <= XWIN0_BUTTON_SETTING)
		{
			if (m_activeControl != buttonId)
			{
				XControl* xctl;
				for (U8 i = XWIN0_BUTTON_TALK; i <= XWIN0_BUTTON_SETTING; i++)
				{
					xctl = m_ctlArray[0][i];
					assert(xctl);
					xctl->setStatus(XCONTROL_STATE_NORMAL);
				}
				xctl = m_ctlArray[0][buttonId];
				assert(xctl);
				xctl->setStatus(XCONTROL_STATE_ACTIVE);
				m_activeControl = buttonId;
				m_status |= DUI_STATUS_NEEDRAW;  // need to redraw this virtual window
				InvalidateDUIWindow();           // set the gloabl redraw flag so next paint routine will do the paint work
			}
		}

		return 0;
	}

	int UpdateMyIcon()
	{
		InvalidateScreen();
		return 0;
	}
};

#endif  /* __DUI_WINDOW0_H__ */

