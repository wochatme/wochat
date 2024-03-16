#ifndef __DUI_WINDOW2_H__
#define __DUI_WINDOW2_H__

#include "dui/dui_win.h"

static const U16 txtGeneralSetting[] = { 0x901a,0x7528,0x8bbe,0x7f6e,0 };
static const U16 txtAIRobotSetting[] = { 0x9ec4,0x5927,0x4ed9,0x8bbe,0x7f6e,0 };
static const U16 txtNetworkSetting[] = { 0x7f51,0x7edc,0x8bbe,0x7f6e,0 };
static const U16 txtAboutSetting[]  =  { 0x5173,0x4e8e,0x672c,0x8f6f,0x4ef6,0 };
static const U16 txtNameUnknown[]    = { 0x59d3,0x540d,0x672a,0x77e5,0 };

static const wchar_t Win2LastMSG[] = {
	0x66fe, 0x7ecf, 0x6709, 0x4e00, 0x4efd, 0x771f, 0x8bda, 0x7684,
	0x7231, 0x60c5, 0x6446, 0x5728, 0x6211, 0x7684, 0x9762, 0x524d, 0 };

#define WIN2_MODE_TALK		DUI_MODE0
#define WIN2_MODE_FRIEND	DUI_MODE1
#define WIN2_MODE_SETTING	DUI_MODE2

class XWindow2 : public XWindowT <XWindow2>
{
private:
	enum {
		SELECTED_COLOR = 0xFFC6C6C6,
		HOVERED_COLOR = 0xFFDADADA,
		DEFAULT_COLOR = 0xFFE5E5E5
	};

	enum {
		ITEM_HEIGHT = 64,
		ICON_HEIGHT = 32,
		ICON_HEIGHTF = 32,
		ITEM_MARGIN = ((ITEM_HEIGHT - ICON_HEIGHT) >> 1),
		ITEM_MARGINF = ((ITEM_HEIGHT - ICON_HEIGHTF) >> 1)
	};

	int		m_lineHeight0 = 0;
	int		m_lineHeight1 = 0;

	WTChatGroup* m_chatgroupRoot = nullptr;
	WTChatGroup* m_chatgroupSelected = nullptr;
	WTChatGroup* m_chatgroupSelectPrev = nullptr;
	WTChatGroup* m_chatgroupHovered = nullptr;
	U32 m_chatgroupTotal = 0;

	WTFriend* m_friendRoot = nullptr;
	WTFriend* m_friendSelected = nullptr;
	WTFriend* m_friendSelectPrev = nullptr;
	WTFriend* m_friendHovered = nullptr;
	U32 m_friendTotal = 0;

	WTSetting* m_settingRoot = nullptr;
	WTSetting* m_settingSelected = nullptr;
	WTSetting* m_settingSelectPrev = nullptr;
	WTSetting* m_settingHovered = nullptr;
	U32 m_settingTotal = 0;

	void InitBitmap()
	{
	}

public:
	XWindow2()
	{
		m_backgroundColor = DEFAULT_COLOR;
		m_property |= (DUI_PROP_HASVSCROLL | DUI_PROP_HANDLEWHEEL | DUI_PROP_LARGEMEMPOOL | DUI_PROP_HANDLETEXT);
		m_message = WM_XWINDOWS02;
	}
	~XWindow2() 
	{
	}

	WTChatGroup* GetSelectedChatGroup()
	{
		return m_chatgroupSelected;
	}

	void AfterSetMode()
	{
		switch (m_mode)
		{
		case WIN2_MODE_TALK:
			m_sizeAll.cy = m_chatgroupTotal * ITEM_HEIGHT;
			break;
		case WIN2_MODE_FRIEND:
			m_sizeAll.cy = m_friendTotal * ITEM_HEIGHT;
			break;
		case WIN2_MODE_SETTING:
			m_sizeAll.cy = m_settingTotal * ITEM_HEIGHT;
			break;
		default:
			assert(0);
			break;
		}
	}

	int UpdateFrinedInfo(U8* blob, U32 blen)
	{
		int r = 0;
		bool found = false;

		assert(blob);
		assert(blen == WT_BLOB_LEN);

		U8* pubkey = blob + 4;
		WTGuy* guy = (WTGuy*)hash_search(g_peopleHTAB, pubkey, HASH_FIND, &found);
		if (found)
		{
			assert(guy);
			if (guy->people)
			{
				PopluateFriendInfo(guy->people, blob, blen);
				m_status |= DUI_STATUS_NEEDRAW;  // need to redraw this virtual window
				InvalidateDUIWindow();           // set the gloabl redraw flag so next paint routine will do the paint work
				r++;
			}
		}
		return r;
	}

	U32 AddNewFriend(U8* blob, U32 blen, U8* icon)
	{
		U32 ret = WT_FAIL;
		assert(blob);
		assert(blen == WT_BLOB_LEN);

		WTFriend* p = nullptr;
		WTFriend* q = nullptr;

		if (m_friendRoot == nullptr)
		{
			m_friendRoot = (WTFriend*)wt_palloc0(m_pool, sizeof(WTFriend));
			assert(m_friendRoot);
			q = m_friendRoot;
		}
		else
		{
			p = m_friendRoot;
			while (p)
			{
				if (p->next == nullptr)
					break;
				p = p->next;
			}
			q = (WTFriend*)wt_palloc0(m_pool, sizeof(WTFriend));
			p->next = q;
			q->prev = p;
		}

		if (q)
		{
			bool found = false;
			U8* pubkey = blob + 4;
			WTGuy*  guy = (WTGuy*)hash_search(g_peopleHTAB, pubkey, HASH_ENTER, &found);
			assert(guy);
			if (!found)
			{
				guy->people = q;
				PopluateFriendInfo(guy->people, blob, blen);
				ret = WT_OK;
				m_status |= DUI_STATUS_NEEDRAW;  // need to redraw this virtual window
				InvalidateDUIWindow();           // set the gloabl redraw flag so next paint routine will do the paint work
			}
		}
		return ret;
	}

	U32 ProcessTextMessage(U8* pubkey, U16* text, U32 text_length)
	{
		WTFriend* people = nullptr;
		bool found = false;
		int txtH = 0, txtW = 0;
		
		if (GetTextWidthAndHeight(WT_TEXTFORMAT_MAINTEXT, text, text_length, (g_win4Width * 2 / 3), &txtW, &txtH) != WT_OK)
			return WT_DWRITE_METRIC_ERROR;

		assert(pubkey);
		WTGuy* guy = (WTGuy*)hash_search(g_peopleHTAB, pubkey, HASH_ENTER, &found);
		if (!found) // the sender of this message is not in my friend list
		{
			int i;
			U16* p16;
			assert(guy);
			WTFriend* p = (WTFriend*)wt_palloc0(g_messageMemPool, sizeof(WTFriend));
			if (p)
			{
				guy->people = p;
				people = p;
				p16 = (U16*)p->name;
				for (i = 0; i < 4; i++) p16[i] = txtNameUnknown[i];
				p->name_legnth = 4;

				for (i = 0; i < PUBLIC_KEY_SIZE; i++) p->pubkey[i] = pubkey[i];
				p->hLarge = p->wLarge = 128;
				p->iconLarge = GetUIBitmap(WT_UI_BMP_MYLARGEICON);
				wt_Resize128To32Bmp(p->iconLarge, p->icon);

				QueryFriendInformation(pubkey);
			}
		}
		else
		{
			assert(guy);
			people = guy->people;
		}
	
		if (people)
		{
			WTChatGroup* cg = people->chatgroup;
			if (cg)
			{
				assert(cg->people == people);
				WTChatMessage* xmsg = (WTChatMessage*)wt_palloc0(m_pool, sizeof(WTChatMessage));
				assert(xmsg);
				xmsg->message = text;
				xmsg->message_length = text_length;
				xmsg->height = txtH;
				xmsg->width = txtW;
				if (cg->msgHead == nullptr)
				{
					assert(cg->msgTail == nullptr);
					cg->msgHead = xmsg;
					cg->msgTail = xmsg;
				}
				else
				{
					assert(cg->msgTail);
					cg->msgTail->next = xmsg;
					xmsg->prev = cg->msgTail;
					cg->msgTail = xmsg;
				}
				cg->total++;
				cg->unread++;
				cg->height += xmsg->height;
				//NotifyParent(m_message, WIN2_MODE_TALK, (LPARAM)cg);
			}
			else
			{
				cg = (WTChatGroup*)wt_palloc0(m_pool, sizeof(WTChatGroup));
				assert(cg);
				cg->people = people;
				people->chatgroup = cg;
				cg->width = g_win4Width;
				m_chatgroupTotal++;
				// put cg in the head of the double-link
				cg->next = m_chatgroupRoot;
				if(m_chatgroupRoot)
					m_chatgroupRoot->prev = cg;
				m_chatgroupRoot = cg;
				m_chatgroupSelected = cg;

				WTChatMessage* xmsg = (WTChatMessage*)wt_palloc0(m_pool, sizeof(WTChatMessage));
				assert(xmsg);
				xmsg->message = text;
				xmsg->message_length = text_length;
				xmsg->height = txtH;
				xmsg->width = txtW;
				cg->msgHead = cg->msgTail = xmsg;
				cg->total = 1;
				NotifyParent(m_message, WIN2_MODE_TALK, (LPARAM)cg);
			}
		}
		return WT_OK;
	}

#if 0
	U32 LoadFriends()
	{
		int rc;
		sqlite3* db;
		sqlite3_stmt* stmt = NULL;

		assert(nullptr == m_friendRoot);
		assert(nullptr != m_pool);

		m_friendSelected = nullptr;
		m_friendTotal = 0;

		rc = sqlite3_open16(g_DBPath, &db);
		if (SQLITE_OK == rc)
		{
			char sql[256] = { 0 };
			U8 hexPK0[67] = { 0 };
			wt_Raw2HexString(g_pMyInformation->pubkey, PUBLIC_KEY_SIZE, hexPK0, nullptr);

			sprintf_s((char*)sql, 256, "SELECT pk,dt,sr,ub,si FROM p WHERE at<>0 AND (id=1 OR me='%s') ORDER BY id", hexPK0);
			rc = sqlite3_prepare_v2(db, (const char*)sql, -1, &stmt, NULL);
			if (SQLITE_OK == rc)
			{
				U8* hexPK;
				S64 dt;
				U8* sr;
				U8* ub;
				U8* si;
				U32 ub_len, si_len;
				WTFriend* p;
				WTFriend* q;
				bool found = false;
				WTGuy* guy;

				while (SQLITE_ROW == sqlite3_step(stmt))
				{
					hexPK  = (U8*)sqlite3_column_text(stmt, 0);
					dt     = (U32)sqlite3_column_int64(stmt, 1);
					sr     = (U8*)sqlite3_column_text(stmt, 2);
					ub     = (U8*)sqlite3_column_blob(stmt, 3);
					ub_len = (U32)sqlite3_column_bytes(stmt, 3);
					si     = (U8*)sqlite3_column_blob(stmt, 4);
					si_len = (U32)sqlite3_column_bytes(stmt, 4);

					if (ub_len != WT_BLOB_LEN || si_len != WT_SMALL_ICON_SIZE)
						continue; // this is not a valid record

					if (!m_friendRoot)
					{
						m_friendRoot = (WTFriend*)wt_palloc0(m_pool, sizeof(WTFriend));
						assert(m_friendRoot);
						m_friendSelected = m_friendRoot;
						p = m_friendRoot;
					}
					else
					{
						q = (WTFriend*)wt_palloc0(m_pool, sizeof(WTFriend));
						assert(q);
						assert(p);
						p->next = q;
						q->prev = p;
						p = q;
					}
					m_friendTotal++;
					PopluateFriendInfo(p, ub, ub_len, si, si_len);

					found = false;
					guy = (WTGuy*)hash_search(m_peopleHTAB, p->pubkey, HASH_ENTER, &found);
					assert(guy);
					if (!found)
					{
						guy->people = p;
					}
				}
				sqlite3_finalize(stmt);
			}
			sqlite3_close(db);
		}
		return WT_OK;
	}
#endif
	U32 InitSettings()
	{
		U16 h = 20;
		assert(nullptr == m_settingRoot);
		assert(nullptr != m_pool);

		IDWriteTextLayout* pTextLayout = nullptr;
		IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_MAINTEXT);
		HRESULT hr = g_pDWriteFactory->CreateTextLayout((wchar_t*)txtGeneralSetting, 4, pTextFormat, 600.f, 1.f, &pTextLayout);

		if (S_OK == hr && pTextLayout)
		{
			DWRITE_TEXT_METRICS tm;
			pTextLayout->GetMetrics(&tm);
			h = static_cast<int>(tm.height) + 1;
		}
		SafeRelease(&pTextLayout);

		m_settingTotal = 0;
		m_settingRoot = (WTSetting*)wt_palloc0(m_pool, sizeof(WTSetting));
		if (m_settingRoot)
		{
			m_settingTotal++;
			m_settingSelected = m_settingRoot;
			WTSetting* p = m_settingRoot;
			p->id = SETTING_GENERAL;
			p->height = h;
			p->name = (U16*)txtGeneralSetting;
			p->nameLen = 4;

			WTSetting* q = (WTSetting*)wt_palloc0(m_pool, sizeof(WTSetting));
			if (q)
			{
				m_settingTotal++;
				q->id = SETTING_NETWORK;
				p->height = h;
				q->name = (U16*)txtNetworkSetting;
				q->nameLen = 4;
				p->next = q;
				p = q;
				q = (WTSetting*)wt_palloc0(m_pool, sizeof(WTSetting));
				if (q)
				{
					m_settingTotal++;
					q->id = SETTING_AIROBOT;
					p->height = h;
					q->name = (U16*)txtAIRobotSetting;
					q->nameLen = 5;
					p->next = q;
					p = q;
					q = (WTSetting*)wt_palloc0(m_pool, sizeof(WTSetting));
					if (q)
					{
						m_settingTotal++;
						q->id = SETTING_ABOUT;
						p->height = h;
						q->name = (U16*)txtAboutSetting;
						q->nameLen = 5;
						p->next = q;
						p = q;
					}
				}
			}
		}
		return WT_OK;
	}
#if 0
	int LoadChatGroupList()
	{
		assert(m_chatgroupRoot == nullptr);

		if (m_friendSelected)
		{
			m_chatgroupRoot = (WTChatGroup*)wt_palloc0(m_pool, sizeof(WTChatGroup));
			if (nullptr != m_chatgroupRoot)
			{
				WTChatGroup* p;
				m_chatgroupSelected = m_chatgroupRoot;
				p = m_chatgroupRoot;
				p->people = m_friendSelected;
				m_friendSelected->chatgroup = p;
				p->width = g_win4Width;
				m_chatgroupTotal++;
			}
		}
		return 0;
	}
#endif
	void SetFriendAndChatList(WTFriend* people, U32 peopelTotal, WTChatGroup* chatgroup, U32 chatgroupTotal)
	{
		assert(people);
		assert(chatgroup);
		m_friendRoot = people;
		m_chatgroupRoot = chatgroup;
		m_chatgroupSelected = chatgroup;
		m_friendTotal = peopelTotal;
		m_chatgroupTotal = chatgroupTotal;
	}

	int Do_DUI_CREATE(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int ret = 0;
		InitSettings();
		//LoadChatGroupList();
		return ret;
	}

	int Do_DUI_DESTROY(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		return 0;
	}


public:
	int DoTalkDrawText(DUI_Surface surface, DUI_Brush brushText, DUI_Brush brushSelText, DUI_Brush brushCaret, DUI_Brush brushBkg0, DUI_Brush brushBkg1)
	{
		U16 lineH0 = 0, lineH1 = 0;
		HRESULT hr;
		IDWriteTextFormat* pTextFormat0;
		IDWriteTextFormat* pTextFormat1;
		ID2D1HwndRenderTarget* pD2DRenderTarget = static_cast<ID2D1HwndRenderTarget*>(surface);
		ID2D1SolidColorBrush* pTextBrush = static_cast<ID2D1SolidColorBrush*>(brushText);
		ID2D1SolidColorBrush* pBkgBrush0 = static_cast<ID2D1SolidColorBrush*>(brushBkg0);
		ID2D1SolidColorBrush* pBkgBrush1 = static_cast<ID2D1SolidColorBrush*>(brushBkg1);
		IDWriteTextLayout* pTextLayout0 = nullptr;
		IDWriteTextLayout* pTextLayout1 = nullptr;
		IDWriteTextLayout* pTextLayout2 = nullptr;

		pTextFormat0 = GetTextFormatAndHeight(WT_TEXTFORMAT_PEOPLENAME, &lineH0);
		pTextFormat1 = GetTextFormatAndHeight(WT_TEXTFORMAT_ENGLISHSMALL, &lineH1);
		assert(pTextFormat0);
		assert(pTextFormat1);

		int dx, dy, pos;
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		int margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		D2D1_POINT_2F orgin;
		dx = ITEM_MARGIN;
		WTFriend* people;
		WTChatGroup* p = m_chatgroupRoot;
		pos = 0;
		while (nullptr != p)
		{
			if (pos + ITEM_HEIGHT > m_ptOffset.y)
			{
				people = p->people;
				assert(people);

				dx = ITEM_MARGIN + ICON_HEIGHT + ITEM_MARGIN;
				dy = pos - m_ptOffset.y + 12;
				hr = g_pDWriteFactory->CreateTextLayout((wchar_t*)people->name, people->name_legnth, pTextFormat0, static_cast<FLOAT>(w), static_cast<FLOAT>(1), &pTextLayout0);
				if (S_OK == hr)
				{
					orgin.x = static_cast<FLOAT>(dx + m_area.left);
					orgin.y = static_cast<FLOAT>(dy + m_area.top);
					pD2DRenderTarget->DrawTextLayout(orgin, pTextLayout0, pTextBrush);
				}
				SafeRelease(&pTextLayout0);
#if 0
				hr = g_pDWriteFactory->CreateTextLayout(p->lastmsg, p->lastmsgLen, pTextFormat1, static_cast<FLOAT>(W), static_cast<FLOAT>(1), &pTextLayout1);
				if (S_OK == hr)
				{
					orgin.x = static_cast<FLOAT>(dx + m_area.left);
					orgin.y = static_cast<FLOAT>(dy + m_area.top + ITEM_MARGIN + ITEM_MARGIN);
					pD2DRenderTarget->DrawTextLayout(orgin, pTextLayout1, pTextBrush);
				}
				SafeRelease(&pTextLayout1);
#endif
			}
			p = p->next;
			pos += ITEM_HEIGHT;
			if (pos >= (m_ptOffset.y + h))
				break;
		}
		return 0;
	}

	int DoSettingDrawText(DUI_Surface surface, DUI_Brush brushText, DUI_Brush brushSelText, DUI_Brush brushCaret, DUI_Brush brushBkg0, DUI_Brush brushBkg1)
	{
		HRESULT hr;
		IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_MAINTEXT);
		ID2D1HwndRenderTarget* pD2DRenderTarget = static_cast<ID2D1HwndRenderTarget*>(surface);
		ID2D1SolidColorBrush* pTextBrush = static_cast<ID2D1SolidColorBrush*>(brushText);
		ID2D1SolidColorBrush* pBkgBrush0 = static_cast<ID2D1SolidColorBrush*>(brushBkg0);
		ID2D1SolidColorBrush* pBkgBrush1 = static_cast<ID2D1SolidColorBrush*>(brushBkg1);
		IDWriteTextLayout* pTextLayout = nullptr;

		assert(pTextFormat);

		int dx, dy, pos;
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		int margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		D2D1_POINT_2F orgin;
		dx = ITEM_MARGIN;

		WTSetting* p = m_settingRoot;
		pos = 0;
		while (nullptr != p)
		{
			if (pos + ITEM_HEIGHT > m_ptOffset.y)
			{
				dy = pos - m_ptOffset.y + ((ITEM_HEIGHT - p->height)>>1);
				hr = g_pDWriteFactory->CreateTextLayout((wchar_t*)p->name, p->nameLen, pTextFormat, static_cast<FLOAT>(w), static_cast<FLOAT>(1), &pTextLayout);
				if (S_OK == hr)
				{
					orgin.x = static_cast<FLOAT>(dx + m_area.left);
					orgin.y = static_cast<FLOAT>(dy + m_area.top);
					pD2DRenderTarget->DrawTextLayout(orgin, pTextLayout, pTextBrush);
				}
				SafeRelease(&pTextLayout);
			}
			p = p->next;
			pos += ITEM_HEIGHT;
			if (pos >= (m_ptOffset.y + h))
				break;
		}
		return 0;
	}

	int DoFriendDrawText(DUI_Surface surface, DUI_Brush brushText, DUI_Brush brushSelText, DUI_Brush brushCaret, DUI_Brush brushBkg0, DUI_Brush brushBkg1)
	{
		U16 lineH0 = 0, lineH1 = 0;
		HRESULT hr;
		IDWriteTextFormat* pTextFormat0;
		IDWriteTextFormat* pTextFormat1;
		ID2D1HwndRenderTarget* pD2DRenderTarget = static_cast<ID2D1HwndRenderTarget*>(surface);
		ID2D1SolidColorBrush* pTextBrush = static_cast<ID2D1SolidColorBrush*>(brushText);
		ID2D1SolidColorBrush* pBkgBrush0 = static_cast<ID2D1SolidColorBrush*>(brushBkg0);
		ID2D1SolidColorBrush* pBkgBrush1 = static_cast<ID2D1SolidColorBrush*>(brushBkg1);
		IDWriteTextLayout* pTextLayout = nullptr;

		pTextFormat0 = GetTextFormatAndHeight(WT_TEXTFORMAT_PEOPLENAME, &lineH0);
		pTextFormat1 = GetTextFormatAndHeight(WT_TEXTFORMAT_ENGLISHSMALL, &lineH1);
		assert(pTextFormat0);
		assert(pTextFormat1);

		int dx, dy, pos;
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		int margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		D2D1_POINT_2F orgin;
		wchar_t hexPK[67] = { 0 };

		WTFriend* p = m_friendRoot;
		pos = 0;
		while (nullptr != p)
		{
			if (pos + ITEM_HEIGHT > m_ptOffset.y)
			{
				dx = ITEM_MARGINF + ICON_HEIGHTF + ITEM_MARGINF;
				dy = pos - m_ptOffset.y + 12;
				hr = g_pDWriteFactory->CreateTextLayout((wchar_t*)p->name, p->name_legnth, pTextFormat0, static_cast<FLOAT>(w), static_cast<FLOAT>(1), &pTextLayout);
				if (S_OK == hr)
				{
					orgin.x = static_cast<FLOAT>(dx + m_area.left);
					orgin.y = static_cast<FLOAT>(dy + m_area.top);
					pD2DRenderTarget->DrawTextLayout(orgin, pTextLayout, pTextBrush);
				}
				SafeRelease(&pTextLayout);

				dy = pos - m_ptOffset.y + 14 + lineH0;
				wt_Raw2HexStringW(p->pubkey, PUBLIC_KEY_SIZE, hexPK, nullptr);
				hexPK[6] = hexPK[7] = hexPK[8] = L'.';
				hexPK[9] = hexPK[62]; hexPK[10] = hexPK[63]; hexPK[11] = hexPK[64]; hexPK[12] = hexPK[65];
				hr = g_pDWriteFactory->CreateTextLayout(hexPK, 13, pTextFormat1, static_cast<FLOAT>(w), static_cast<FLOAT>(1), &pTextLayout);
				if (S_OK == hr)
				{
					orgin.x = static_cast<FLOAT>(dx + m_area.left);
					orgin.y = static_cast<FLOAT>(dy + m_area.top);
					pD2DRenderTarget->DrawTextLayout(orgin, pTextLayout, pTextBrush);
				}
				SafeRelease(&pTextLayout);
			}
			p = p->next;
			pos += ITEM_HEIGHT;
			if (pos >= (m_ptOffset.y + h))
				break;
		}
		return 0;
	}

	int DoDrawText(DUI_Surface surface, DUI_Brush brushText, DUI_Brush brushSelText, DUI_Brush brushCaret, DUI_Brush brushBkg0, DUI_Brush brushBkg1)
	{
		int r = 0;
		switch (m_mode)
		{
		case WIN2_MODE_TALK:
			r = DoTalkDrawText(surface, brushText, brushSelText, brushCaret, brushBkg0, brushBkg1);
			break;
		case WIN2_MODE_FRIEND:
			r = DoFriendDrawText(surface, brushText, brushSelText, brushCaret, brushBkg0, brushBkg1);
			break;
		case WIN2_MODE_SETTING:
			r = DoSettingDrawText(surface, brushText, brushSelText, brushCaret, brushBkg0, brushBkg1);
			break;
		default:
			assert(0);
			break;
		}
		return r;
	}

	int TalkPaint(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		U32 color;
		int dx, dy, pos;
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		int margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		WTFriend* people;
		dx = ITEM_MARGIN;
		WTChatGroup* p = m_chatgroupRoot;
		pos = 0;
		while (nullptr != p)
		{
			if (pos + ITEM_HEIGHT > m_ptOffset.y)
			{
				people = p->people;
				assert(people);

				if (p == m_chatgroupSelected)
					color = SELECTED_COLOR;
				else if (p == m_chatgroupHovered)
					color = HOVERED_COLOR;
				else
					color = DEFAULT_COLOR;
				dy = pos - m_ptOffset.y;
				DUI_ScreenFillRect(m_screen, w, h, color, w - margin, ITEM_HEIGHT, 0, dy);
				DUI_ScreenDrawRectRound(m_screen, w, h, people->icon, WT_SMALL_ICON_WIDTH, WT_SMALL_ICON_HEIGHT, dx, dy + ITEM_MARGIN, color, color);
			}
			p = p->next;
			pos += ITEM_HEIGHT;
			if (pos >= (m_ptOffset.y + h))
				break;
		}
		return 0;
	}

	int FriendPaint(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int r = 0;

		U32 color;
		int dx, dy, pos;
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		int margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		int H = ITEM_HEIGHT;

		dx = ITEM_MARGINF;
		WTFriend* p = m_friendRoot;
		pos = 0;
		while (nullptr != p)
		{
			if (pos + ITEM_HEIGHT > m_ptOffset.y)
			{
				if (p == m_friendSelected)
					color = SELECTED_COLOR;
				else if (p == m_friendHovered)
					color = HOVERED_COLOR;
				else
					color = DEFAULT_COLOR;

				dy = pos - m_ptOffset.y;
				DUI_ScreenFillRect(m_screen, w, h, color, w - margin, ITEM_HEIGHT, 0, dy);
				DUI_ScreenDrawRectRound(m_screen, w, h, p->icon, WT_SMALL_ICON_WIDTH, WT_SMALL_ICON_HEIGHT, dx, dy + ITEM_MARGINF, color, color);
			}
			p = p->next;
			pos += ITEM_HEIGHT;
			if (pos >= (m_ptOffset.y + h))
				break;
		}
		return r;
	}

	int SettingPaint(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int r = 0;
		U32 color;
		int dx, dy, pos;
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;
		int margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;

		WTSetting* p = m_settingRoot;
		pos = 0;
		p = m_settingRoot;
		while (nullptr != p)
		{
			if (pos + ITEM_HEIGHT > m_ptOffset.y)
			{
				if (p == m_settingSelected)
					color = SELECTED_COLOR;
				else if (p == m_settingHovered)
					color = HOVERED_COLOR;
				else
					color = DEFAULT_COLOR;
				dy = pos - m_ptOffset.y;
				DUI_ScreenFillRect(m_screen, w, h, color, w - margin, ITEM_HEIGHT, 0, dy);
			}
			p = p->next;
			pos += ITEM_HEIGHT;
			if (pos >= (m_ptOffset.y + h))
				break;
		}
		return r;
	}

	int Do_DUI_PAINT(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int r = 0;
		switch (m_mode)
		{
		case WIN2_MODE_TALK:
			r = TalkPaint(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_FRIEND:
			r = FriendPaint(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_SETTING:
			r = SettingPaint(uMsg, wParam, lParam, lpData);
			break;
		default:
			assert(0);
			break;
		}
		return r;
	}

	int TalkMouseMove(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		WTChatGroup* hovered = m_chatgroupHovered;
		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;

			assert(xPos >= 0);
			assert(yPos >= 0);

			WTChatGroup* p = m_chatgroupRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_chatgroupHovered = p;
					hit++;
					if (hovered != m_chatgroupHovered) // The hovered item is changed, we need to redraw
						ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_chatgroupHovered)
			{
				m_chatgroupHovered = nullptr;
				ret++;
			}
		}
		return ret;
	}

	int FriendMouseMove(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		WTFriend* hovered = m_friendHovered;
		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;

			assert(xPos >= 0);
			assert(yPos >= 0);

			WTFriend* p = m_friendRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_friendHovered = p;
					hit++;
					if (hovered != m_friendHovered) // The hovered item is changed, we need to redraw
						ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_friendHovered)
			{
				m_friendHovered = nullptr;
				ret++;
			}
		}
		return ret;
	}

	int SettingMouseMove(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		WTSetting* hovered = m_settingHovered;
		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;

			assert(xPos >= 0);
			assert(yPos >= 0);

			WTSetting* p = m_settingRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_settingHovered = p;
					hit++;
					if (hovered != m_settingHovered) // The hovered item is changed, we need to redraw
						ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_settingHovered)
			{
				m_settingHovered = nullptr;
				ret++;
			}
		}
		return ret;
	}

	int Do_DUI_MOUSEMOVE(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int r = 0;
		switch (m_mode)
		{
		case WIN2_MODE_TALK:
			r = TalkMouseMove(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_FRIEND:
			r = FriendMouseMove(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_SETTING:
			r = SettingMouseMove(uMsg, wParam, lParam, lpData);
			break;
		default:
			assert(0);
			break;
		}
		return r;
	}

	int TalkLButtonDown(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;
			WTChatGroup* p = m_chatgroupRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_chatgroupHovered = p;
					m_chatgroupSelected = p;
					hit++;
					ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_chatgroupHovered)
			{
				m_chatgroupHovered = nullptr;
				ret++;;
			}
		}
		else
		{
			assert(m_chatgroupSelected);
			if (m_chatgroupSelectPrev != m_chatgroupSelected)
			{
				m_chatgroupSelectPrev = m_chatgroupSelected;
				ret++;
				NotifyParent(m_message, (U64)m_mode, (U64)m_chatgroupSelected);
			}
		}

		return ret;
	}

	int FriendLButtonDown(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;
			WTFriend* p = m_friendRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_friendHovered = p;
					m_friendSelected = p;
					hit++;
					ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_friendHovered)
			{
				m_friendHovered = nullptr;
				ret++;;
			}
		}
		return ret;
	}

	int SettingLButtonDown(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;
			WTSetting* p = m_settingRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_settingHovered = p;
					m_settingSelected = p;
					hit++;
					ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_settingHovered)
			{
				m_settingHovered = nullptr;
				ret++;;
			}
		}
		return ret;

		return 0;
	}

	int Do_DUI_LBUTTONDOWN(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int r = 0;
		switch (m_mode)
		{
		case WIN2_MODE_TALK:
			r = TalkLButtonDown(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_FRIEND:
			r = FriendLButtonDown(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_SETTING:
			r = SettingLButtonDown(uMsg, wParam, lParam, lpData);
			break;
		default:
			assert(0);
			break;
		}
		return r;
	}

	int TalkLButtonUp(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		return 0;
	}

	int FriendLButtonUp(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;

			WTFriend* p = m_friendRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_friendHovered = p;
					m_friendSelected = p;
					hit++;
					ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_friendHovered)
			{
				m_friendHovered = nullptr;
				ret++;
			}
		}
		else // we hit some item
		{
			assert(nullptr != m_friendSelected);
			if (m_friendSelectPrev != m_friendSelected)
			{
				// do something here
				NotifyParent(m_message, (U64)m_mode, (U64)m_friendSelected);
			}
			m_friendSelectPrev = m_friendSelected; // prevent double selection
		}
		return ret;
	}

	int SettingLButtonUp(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;

			WTSetting* p = m_settingRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_settingHovered = p;
					m_settingSelected = p;
					hit++;
					ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_settingHovered)
			{
				m_settingHovered = nullptr;
				ret++;
			}
		}
		else // we hit some item
		{
			assert(nullptr != m_settingSelected);
			if (m_settingSelectPrev != m_settingSelected)
			{
				// do something here
				//NotifyParent(m_message, (U64)m_mode, (U64)m_friendSelected);
			}
			m_settingSelectPrev = m_settingSelected; // prevent double selection
		}
		return ret;
	}

	int Do_DUI_LBUTTONUP(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int r = 0;
		switch (m_mode)
		{
		case WIN2_MODE_TALK:
			r = TalkLButtonUp(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_FRIEND:
			r = FriendLButtonUp(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_SETTING:
			r = SettingLButtonUp(uMsg, wParam, lParam, lpData);
			break;
		default:
			assert(0);
			break;
		}
		return r;
	}

	int TalkLButtonDBClick(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		return 0;
	}

	int FriendLButtonDBClick(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		int hit = -1, margin;
		int ret = 0;
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		RECT area = { 0 };
		RECT* r = &area;
		margin = (DUI_STATUS_VSCROLL & m_status) ? m_scrollWidth : 0;
		r->right = w - margin;
		r->left = 0;

		if (XWinPointInRect(xPos, yPos, &m_area))
		{
			int pos = 0;
			// transfer the coordination from screen to local virutal window
			xPos -= m_area.left;
			yPos -= m_area.top;
			WTFriend* p = m_friendRoot;
			while (nullptr != p)
			{
				r->top = pos; r->bottom = r->top + ITEM_HEIGHT;
				if (XWinPointInRect(xPos, yPos + m_ptOffset.y, r))
				{
					m_friendHovered = p;
					m_friendSelected = p;
					hit++;
					ret++;
					break;
				}
				p = p->next;
				pos += ITEM_HEIGHT;
				if (pos >= (m_ptOffset.y + h))
					break;
			}
		}
		if (-1 == hit)
		{
			if (nullptr != m_friendHovered)
			{
				m_friendHovered = nullptr;
				ret++;;
			}
		}
		else
		{
			if (m_friendSelected)
			{
				WTChatGroup* cg = m_friendSelected->chatgroup;
				if (cg == nullptr)
				{
					cg = (WTChatGroup*)wt_palloc0(m_pool, sizeof(WTChatGroup));
					assert(cg);
					cg->people = m_friendSelected;
					m_friendSelected->chatgroup = cg;
					cg->width = g_win4Width;
					m_chatgroupTotal++;
				}
				assert(cg);
				if (cg != m_chatgroupRoot)
				{
					// remove cg from the double-link 
					if (cg->prev)
						cg->prev->next = cg->next;
					if (cg->next)
						cg->next->prev = cg->prev;
					// put cg in the head of the double-link
					cg->prev = nullptr;
					cg->next = m_chatgroupRoot;
					m_chatgroupRoot->prev = cg;
					m_chatgroupRoot = cg;
					m_chatgroupSelected = cg;
				}
				NotifyParent(m_message, WIN2_MODE_TALK, (LPARAM)cg);
			}
		}
		return ret;
	}

	int SettingLButtonDBClick(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		return 0;
	}

	int Do_DUI_LBUTTONDBLCLK(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr) 
	{ 
		int r = 0;
		switch (m_mode)
		{
		case WIN2_MODE_TALK:
			r = TalkLButtonDBClick(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_FRIEND:
			r = FriendLButtonDBClick(uMsg, wParam, lParam, lpData);
			break;
		case WIN2_MODE_SETTING:
			r = SettingLButtonDBClick(uMsg, wParam, lParam, lpData);
			break;
		default:
			assert(0);
			break;
		}
		return r;
	}

};

#endif  /* __DUI_WINDOW2_H__ */

