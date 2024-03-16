#ifndef __DUI_WINDOW3_H__
#define __DUI_WINDOW3_H__

#include "dui/dui_win.h"

#define WIN3_MODE_TALK		DUI_MODE0
#define WIN3_MODE_FRIEND	DUI_MODE1
#define WIN3_MODE_SETTING	DUI_MODE2

class XWindow3 : public XWindowT <XWindow3>
{
	enum
	{
		GAP_TOP3 = 40,
		GAP_BOTTOM3 = 10,
		GAP_LEFT3 = 6,
		GAP_RIGHT3 = 10
	};

	WTFriend* m_friend = nullptr;
public:
	XWindow3()
	{
		m_backgroundColor = 0xFFF5F5F5;
		m_property |= (DUI_PROP_MOVEWIN | DUI_PROP_HANDLETEXT);
		m_message = WM_XWINDOWS03;
		m_mode = WIN3_MODE_TALK;
	}
	~XWindow3() {}

	void SetFriendPointer(WTFriend* people)
	{
#if 0
		m_friend = people;
		if (m_friend)
		{
			U16 mode = WIN3_MODE_FRIEND;
			U16 id = XWIN3_FRIEND_LABEL_NAME;
			wchar_t hexPK[67];
			XLabel* lb;
			XButton* btn;
			XBitmap* bmp;
			lb  = (XLabel*)m_ctlArray[mode][id];
			assert(lb);
			lb->setText((wchar_t*)m_friend->name, m_friend->nameLen);

			id = XWIN3_FRIEND_LABEL_PUBKEY;
			lb = (XLabel*)m_ctlArray[mode][id];
			assert(lb);
			wt_Raw2HexStringW(m_friend->pubkey, 33, hexPK, nullptr);
			lb->setText((wchar_t*)hexPK, 66);
#if 0
			id = XWIN3_FRIEND_BUTTON_ICON;
			btn = (XButton*)m_ctlArray[mode][id];
			assert(btn);
			bmp = btn->imgNormal;
			bmp->data = m_friend->iconLarge;
			bmp->h = m_friend->hLarge;
			bmp->w = m_friend->wLarge;
			bmp = btn->imgHover;
			bmp->data = m_friend->iconLarge;
			bmp->h = m_friend->hLarge;
			bmp->w = m_friend->wLarge;
			bmp = btn->imgPress;
			bmp->data = m_friend->iconLarge;
			bmp->h = m_friend->hLarge;
			bmp->w = m_friend->wLarge;
			bmp = btn->imgActive;
			bmp->data = m_friend->iconLarge;
			bmp->h = m_friend->hLarge;
			bmp->w = m_friend->wLarge;
#endif 
		}
#endif
		m_status |= DUI_STATUS_NEEDRAW;  // need to redraw this virtual window
		InvalidateDUIWindow();           // set the gloabl redraw flag so next paint routine will do the paint work
	}

	void AfterSetMode()
	{
		switch (m_mode)
		{
		case WIN3_MODE_TALK:
			break;
		case WIN3_MODE_FRIEND:
			break;
		case WIN3_MODE_SETTING:
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
		int dx, dy, sw, sh, gap = 10; // pixel
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		if (w > 0 && h > 0)
		{
			switch (m_mode)
			{
			case WIN3_MODE_TALK:
				id = XWIN3_TALK_BUTTON_DOT;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				sw = xctl->getWidth();
				sh = xctl->getHeight();
				if (w > (sw + GAP_RIGHT3) && h > (sh + GAP_BOTTOM3))
				{
					dx = w - sw - GAP_RIGHT3;
					dy = h - sh - GAP_BOTTOM3;
					xctl->setPosition(dx, dy);
				}

				id = XWIN3_TALK_LABEL_PUBLICKEY;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				sh = xctl->getHeight();
				assert(h > sh);
				dx = GAP_LEFT3;
				dy = (h - sh) - 3;
				xctl->setPosition(dx, dy);

				id = XWIN3_TALK_LABEL_NAME;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);

				sh = xctl->getHeight();
				assert(dy > sh);
				dx = GAP_LEFT3;
				dy = dy - sh - 3;
				xctl->setPosition(dx, dy);

				break;
			case WIN3_MODE_FRIEND:
				id = XWIN3_FRIEND_BUTTON_ICON;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy = GAP_LEFT3;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_LABEL_NAME;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_LABEL_MOTTO;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_BUTTON_PUBKEY;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_LABEL_PUBKEY;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_BUTTON_DOB;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_LABEL_DOB;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_BUTTON_SEX;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_LABEL_SEX;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_BUTTON_AREA;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_LABEL_AREA;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_BUTTON_SOURCE;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_LABEL_SOURCE;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);

				id = XWIN3_FRIEND_BUTTON_CHAT;
				xctl = m_ctlArray[m_mode][id];
				assert(nullptr != xctl);
				dx = GAP_LEFT3;
				dy += 20;
				xctl->setPosition(dx, dy);
				break;
			case WIN3_MODE_SETTING:
				break;
			default:
				assert(0);
				break;
			}
		}
	}

private:
	void InitBitmap()
	{
		int w, h;
		U8 id, mode;
		XBitmap* bmp;

		mode = WIN3_MODE_TALK;
		w = 19;  h = 7;
		id = XWIN3_BITMAP_TALK_DOTN; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
		id = XWIN3_BITMAP_TALK_DOTH; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotH; bmp->w = w; bmp->h = h;
		id = XWIN3_BITMAP_TALK_DOTP; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotP; bmp->w = w; bmp->h = h;
		id = XWIN3_BITMAP_TALK_DOTA; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotH; bmp->w = w; bmp->h = h;

		mode = WIN3_MODE_FRIEND;
		w = 19;  h = 7; 
		id = XWIN3_BITMAP_FRIEND_ICON;   bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
		w = 19;  h = 7;
		id = XWIN3_BITMAP_FRIEND_PUBKEY; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
		w = 19;  h = 7;
		id = XWIN3_BITMAP_FRIEND_DOB;    bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
		w = 19;  h = 7;
		id = XWIN3_BITMAP_FRIEND_SEX;    bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
		w = 19;  h = 7;
		id = XWIN3_BITMAP_FRIEND_AREA;   bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
		w = 19;  h = 7;
		id = XWIN3_BITMAP_FRIEND_SOURCE; bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
		w = 19;  h = 7;
		id = XWIN3_BITMAP_FRIEND_CHAT;   bmp = &(m_bmpArray[mode][id]); bmp->id = id; bmp->data = (U32*)xbmpDotN; bmp->w = w; bmp->h = h;
	}

public:
	void InitControl()
	{
		U16 id, mode;
		U32 objSize;
		U8* mem;

		int lineheight;

		assert(nullptr != m_pool);

		InitBitmap(); // inital all bitmap resource

		mode = WIN3_MODE_TALK;
		//////////////////////////////////////////////////
		id = XWIN3_TALK_BUTTON_DOT;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmpN = &(m_bmpArray[mode][XWIN3_BITMAP_TALK_DOTN]);
			XBitmap* bmpH = &(m_bmpArray[mode][XWIN3_BITMAP_TALK_DOTH]);
			XBitmap* bmpP = &(m_bmpArray[mode][XWIN3_BITMAP_TALK_DOTP]);
			XBitmap* bmpA = &(m_bmpArray[mode][XWIN3_BITMAP_TALK_DOTA]);
			button->setBitmap(bmpN, bmpH, bmpP, bmpA);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN3_TALK_LABEL_PUBLICKEY;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_TITLE);
			lb->Init(((mode << 8) | id), "W3TITLE", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_TALK_LABEL_NAME;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		m_maxCtl[mode] = id + 1;

		mode = WIN3_MODE_FRIEND;
		//////////////////////////////////////////////////
		id = XWIN3_FRIEND_BUTTON_ICON;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN3_BITMAP_FRIEND_ICON]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN3_FRIEND_LABEL_NAME;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_FRIEND_LABEL_MOTTO;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_FRIEND_BUTTON_PUBKEY;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN3_BITMAP_FRIEND_PUBKEY]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN3_FRIEND_LABEL_PUBKEY;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_FRIEND_BUTTON_DOB;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN3_BITMAP_FRIEND_DOB]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN3_FRIEND_LABEL_DOB;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_FRIEND_BUTTON_SEX;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN3_BITMAP_FRIEND_SEX]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN3_FRIEND_LABEL_SEX;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_FRIEND_BUTTON_AREA;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN3_BITMAP_FRIEND_AREA]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN3_FRIEND_LABEL_AREA;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_FRIEND_BUTTON_SOURCE;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN3_BITMAP_FRIEND_SOURCE]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		id = XWIN3_FRIEND_LABEL_SOURCE;
		objSize = sizeof(XLabel);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XLabel* lb = new(mem)XLabel;
			assert(nullptr != lb);
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_GROUPNAME);
			assert(pTextFormat);
			lb->Init(((mode << 8) | id), "W3NAME", g_pDWriteFactory, pTextFormat);
			lb->setText((U16*)L"X", 1);
			m_ctlArray[mode][id] = lb;
		}
		else return;

		id = XWIN3_FRIEND_BUTTON_CHAT;
		objSize = sizeof(XButton);
		mem = (U8*)wt_palloc(m_pool, objSize);
		if (NULL != mem)
		{
			XButton* button = new(mem)XButton;
			assert(nullptr != button);
			button->Init(((mode << 8) | id), "W3DOT");
			XBitmap* bmp = &(m_bmpArray[mode][XWIN3_BITMAP_FRIEND_CHAT]);
			button->setBitmap(bmp, bmp, bmp, bmp);
			button->setRoundColor(m_backgroundColor, m_backgroundColor);
			m_ctlArray[mode][id] = button;
		}
		else return;

		m_maxCtl[mode] = id + 1;
	}

	U32 SetChatGroup(WTChatGroup* cg)
	{
		XLabel* lb;
		wchar_t hexPK[67];

		assert(WIN3_MODE_TALK == m_mode);
		assert(cg);

		WTFriend* p = cg->people;
		assert(p);

		lb = (XLabel*)m_ctlArray[m_mode][XWIN3_TALK_LABEL_NAME];
		lb->setText((U16*)p->name, p->name_legnth);

		wt_Raw2HexStringW(p->pubkey, PUBLIC_KEY_SIZE, hexPK, nullptr);
		lb = (XLabel*)m_ctlArray[m_mode][XWIN3_TALK_LABEL_PUBLICKEY];
		lb->setText((U16*)hexPK, PUBLIC_KEY_SIZE + PUBLIC_KEY_SIZE);

		UpdateControlPosition();
		InvalidateScreen();
		return WT_OK;
	}
};

#endif  /* __DUI_WINDOW3_H__ */

