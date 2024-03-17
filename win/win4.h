#ifndef __DUI_WINDOW4_H__
#define __DUI_WINDOW4_H__

#include "dui/dui_win.h"

#define XWIN4_OFFSET	  10

class XWindow4 : public XWindowT <XWindow4>
{
	enum 
	{
		WIN4_GAP_TOP4    = 40,
		WIN4_GAP_BOTTOM4 = 10,
		WIN4_GAP_LEFT4   = 10,
		WIN4_GAP_RIGHT4  = 10
	};

	WTChatGroup* m_chatGroup = nullptr;

	IDWriteTextLayout* m_cacheTL[1024];
	FLOAT m_offsetX[1024];
	FLOAT m_offsetY[1024];
	U16  m_idxTL = 0;

public:
	XWindow4()
	{
		m_backgroundColor = 0xFFF5F5F5;
		m_property |= (DUI_PROP_HASVSCROLL | DUI_PROP_HANDLEWHEEL | DUI_PROP_HANDLETEXT | DUI_PROP_LARGEMEMPOOL);
		m_message = WM_XWINDOWS04;
	}

	~XWindow4() 
	{
		for (U16 i = 0; i < m_idxTL; i++)
		{
			assert(m_cacheTL[i]);
			ReleaseUnknown(m_cacheTL[i]);
		}
		m_idxTL = 0;
	}

public:
	bool IsPublicKeyMatched(U8* pk)
	{
		bool matched = false;
		if (m_chatGroup && pk)
		{
			U8 i = 0;
			WTFriend* p = m_chatGroup->people;
			assert(p);
			for (i = 0; i < PUBLIC_KEY_SIZE; i++)
			{
				if (pk[i] != p->pubkey[i]) 
					break;
			}
			if (i == PUBLIC_KEY_SIZE)
				matched = true;
		}
		return matched;
	}

	U32 GetPublicKey(U8* pk) // public key is 33 bytes in raw
	{
		U32 r = WT_FAIL;
		if (m_chatGroup && pk)
		{
			WTFriend* p = m_chatGroup->people;
			assert(p);
			for (int i = 0; i < PUBLIC_KEY_SIZE; i++) pk[i] = p->pubkey[i];
			r = WT_OK;
		}
		return r;
	}

	int PostUpdateSize() 
	{ 
		if(m_area.right > m_area.left)
			g_win4Width = m_area.right - m_area.left; // update this import global width

		if(m_area.bottom > m_area.top)
			g_win4Height = m_area.bottom - m_area.top;

		return 0; 
	}

	int ReLayoutText(int width, int height)
	{
		int r = 0;
#if 0
		if (width && m_chatGroup)
		{
			m_sizeAll.cy = 0;
			WTChatGroup* p = m_chatGroup->headMessage;
			while (nullptr != p)
			{
				{  // determine the height of the text layout
					IDWriteTextLayout* pTextLayout = nullptr;
					IDWriteTextFormat* pTextFormat = GetTextFormat(WT_TEXTFORMAT_MAINTEXT);
					FLOAT Wf = static_cast<FLOAT>(width * 2 / 3); // the text width is half of the window
					g_pDWriteFactory->CreateTextLayout(
						p->message,
						p->msgLen,
						pTextFormat,
						Wf,
						static_cast<FLOAT>(1),
						(IDWriteTextLayout**)(&pTextLayout));

					if (pTextLayout)
					{
						pTextLayout->GetMetrics(&(p->tm));
						p->height = static_cast<int>(p->tm.height) + 1 + WT_CHATWINDOW_GAP;
						p->width = static_cast<int>(p->tm.width) + 1;
						
						m_sizeAll.cy += p->height;
					}
					SafeRelease(&pTextLayout);
				}
				p = p->next;
			}
			m_ptOffset.y = (m_sizeAll.cy > height) ? (m_sizeAll.cy - height) : 0;

			r++;
		}
#endif
		return r;
	}

	int SetChatGroup(WTChatGroup* cg)
	{
		int r = 0;
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		assert(cg);

		WTChatGroup* prev = m_chatGroup;
		m_chatGroup = cg;

		if (h > 0 && w > 0) // this window is visible
		{
			m_sizeAll.cy = m_chatGroup->height + (m_chatGroup->total * WT_CHATWINDOW_GAP);
			m_ptOffset.y = m_sizeAll.cy - h;
			if (m_ptOffset.y < 0)
				m_ptOffset.y = 0;
		}

		if(m_chatGroup->width != w)
		{
			m_chatGroup->width = w;
			ReLayoutText(w, h);
		}
		if (prev != m_chatGroup)
		{
			m_status |= DUI_STATUS_NEEDRAW;
			InvalidateDUIWindow();
			r++;
		}

		return r;
	}

	int UpdateMessageConfirmation(U8* pk, U8* hash)
	{
		int i = 0, r = 0;
#if 0
		XMessage* xm;

		if (nullptr == m_chatGroup)
			return 0;

		for (i = 0; i < 33; i++) // check the public key is the same or not
		{
			if (m_chatGroup->pubkey[i] != pk[i])
				break;
		}
		if (i != 33) // if the public key is not the same, something is wrong
			return 0;

		xm = (XMessage*)hash_search(g_messageHTAB, hash, HASH_FIND, NULL);
		if (xm)
		{
			WTChatGroup* p = xm->pointer;
			if (p)
			{
				p->state |= XMESSAGE_CONFIRMATION;
				r++;
			}
		}
		if (r)
			InvalidateScreen();
#endif 
		return r;
	}

	void UpdateControlPosition() // the user has changed the width of this window
	{
		if (m_chatGroup)
		{
			int w = m_area.right - m_area.left;
			int h = m_area.bottom - m_area.top;
			if (m_chatGroup->width != w)
			{
				m_chatGroup->width = w;
				ReLayoutText(w, h);
			}
		}
	}

	int Do_DUI_CREATE(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr) 
	{ 
		U16 h = 18;
		GetTextFormatAndHeight(WT_TEXTFORMAT_MAINTEXT, &h);
		m_sizeLine.cy = h;
		return 0; 
	}

	int AddNewMessage(MessageTask* mt, bool isMe=true)
	{
		int r = 0;
		assert(mt);
		assert(mt->message);

		if (m_chatGroup)
		{
			int w = m_area.right - m_area.left;
			int h = m_area.bottom - m_area.top;
			WTFriend* people = m_chatGroup->people;
			assert(people);

			if (memcmp(people->pubkey, mt->pubkey, PUBLIC_KEY_SIZE) == 0)
			{
				if ('T' == mt->type)
				{
					WTChatMessage* p = (WTChatMessage*)wt_palloc0(m_pool, sizeof(WTChatMessage));
					if (p)
					{
						int txtW = 0, txtH = 0;
						if (GetTextWidthAndHeight(WT_TEXTFORMAT_MAINTEXT, (U16*)mt->message, mt->msgLen, (w * 2 / 3), &txtW, &txtH) == WT_OK)
						{
							p->height = txtH;
							p->width = txtW;
							p->message = (U16*)mt->message;
							p->message_length = mt->msgLen;
							m_chatGroup->total++;
							m_chatGroup->height += p->height;
							if(isMe) 
								p->state = WT_MESSAGE_FROM_ME;
							if (nullptr == m_chatGroup->msgHead)
							{
								assert(m_chatGroup->msgTail == nullptr);
								m_chatGroup->msgHead = p;
								m_chatGroup->msgTail = p;
							}
							else
							{
								assert(m_chatGroup->msgTail);
								m_chatGroup->msgTail->next = p;
								p->prev = m_chatGroup->msgTail;
								m_chatGroup->msgTail = p;
							}
							m_sizeAll.cy = m_chatGroup->height + (m_chatGroup->total * WT_CHATWINDOW_GAP);
							if (w > 0 && h > 0)
							{
								m_ptOffset.y = m_sizeAll.cy - h;
								if (m_ptOffset.y < 0)
									m_ptOffset.y = 0;
							}
							InterlockedIncrement(&(mt->state));
							r++;
							InvalidateScreen();
						}
						else
						{
							wt_pfree(p);
							return 0;
						}
					}
				}
			}
		}
		return r;
	}

	int Do_DUI_PAINT(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr)
	{
		if (m_chatGroup && m_chatGroup->total)
		{
			int dx, dy, H;
			int w = m_area.right - m_area.left;
			int h = m_area.bottom - m_area.top;
			U32* icon = nullptr;
			WTChatMessage* p = m_chatGroup->msgHead;
			WTFriend* people = m_chatGroup->people;
			assert(people);

			int pos = XWIN4_OFFSET;
			while (p)
			{
				H = p->height + WT_CHATWINDOW_GAP; // p->height is only text height
				if (pos + H > m_ptOffset.y && pos < m_ptOffset.y + h) // this node is in the visible area, we need to draw it
				{
					dy = pos - m_ptOffset.y;
					if (WT_MESSAGE_FROM_ME & p->state)
					{
						icon = g_myInfo->icon32;
						dx = w - m_scrollWidth - WT_SMALL_ICON_WIDTH - XWIN4_OFFSET;
					}
					else
					{
						icon = people->icon;
						assert(icon);
						dx = XWIN4_OFFSET;
					}
					DUI_ScreenDrawRectRound(m_screen, w, h, icon, WT_SMALL_ICON_WIDTH, WT_SMALL_ICON_WIDTH, dx, dy, m_backgroundColor, m_backgroundColor);
#if 0
					if (WT_MESSAGE_FROM_CONFIRM & p->state)
					{
						DUI_ScreenDrawRect(m_screen, w, h, (U32*)xbmpOK, 10, 10, x-12, dy + 2);
					}
#endif
				}
				pos += H;
				if (pos >= m_ptOffset.y + h) // out of the scope of the drawing area
					break;
				p = p->next;
			}
		}
		return 0;
	}

	int DoDrawText(DUI_Surface surface, DUI_Brush brushText, DUI_Brush brushSelText, DUI_Brush brushCaret, DUI_Brush brushBkg0, DUI_Brush brushBkg1)
	{ 
		if (m_chatGroup && m_chatGroup->total) // we have message to display
		{
			HRESULT hr;
			U32 color;
			bool isMe;
			int dx, dy, H;
			IDWriteTextFormat* pTextFormat = GetTextFormatAndHeight(WT_TEXTFORMAT_MAINTEXT);
			ID2D1HwndRenderTarget* pD2DRenderTarget = static_cast<ID2D1HwndRenderTarget*>(surface);
			ID2D1SolidColorBrush* pTextBrush = static_cast<ID2D1SolidColorBrush*>(brushText);
			ID2D1SolidColorBrush* pBkgBrush0 = static_cast<ID2D1SolidColorBrush*>(brushBkg0);
			ID2D1SolidColorBrush* pBkgBrush1 = static_cast<ID2D1SolidColorBrush*>(brushBkg1);
			D2D1_POINT_2F orgin;
			D2D1_RECT_F bkgarea;

			int w = m_area.right - m_area.left;
			int h = m_area.bottom - m_area.top;
			FLOAT Wf = static_cast<FLOAT>(w * 2 / 3);

			for (U16 i = 0; i < m_idxTL; i++)
			{
				assert(m_cacheTL[i]);
				ReleaseUnknown(m_cacheTL[i]);
			}
			m_idxTL = 0;

			int pos = XWIN4_OFFSET;
			WTChatMessage* p = m_chatGroup->msgHead;
			while (p)
			{
				isMe = (WT_MESSAGE_FROM_ME & p->state);
				H = p->height + WT_CHATWINDOW_GAP;
				if (pos + H > m_ptOffset.y && pos < m_ptOffset.y + h) // we are in the visible area
				{
					dy = pos - m_ptOffset.y;
					dx = (XWIN4_OFFSET<<2) + WT_SMALL_ICON_WIDTH;
					if (isMe) // this message is from me
						dx = w - p->width - 38 - m_scrollWidth - XWIN4_OFFSET - XWIN4_OFFSET;

					hr = g_pDWriteFactory->CreateTextLayout(
						(wchar_t*)p->message, p->message_length, 
						pTextFormat, Wf, 1.f, (IDWriteTextLayout**)(&m_cacheTL[m_idxTL]));

					if (S_OK == hr && m_cacheTL[m_idxTL])
					{
						bkgarea.left = static_cast<FLOAT>(m_area.left + dx) - XWIN4_OFFSET - XWIN4_OFFSET;
						bkgarea.top = static_cast<FLOAT>(m_area.top + dy);
						bkgarea.right = bkgarea.left + static_cast<FLOAT>(p->width) + XWIN4_OFFSET + XWIN4_OFFSET;
						bkgarea.bottom = bkgarea.top + static_cast<FLOAT>(p->height) + XWIN4_OFFSET + XWIN4_OFFSET;
						if (isMe)
							pD2DRenderTarget->FillRectangle(bkgarea, pBkgBrush0);
						else
							pD2DRenderTarget->FillRectangle(bkgarea, pBkgBrush1);

						orgin.x = static_cast<FLOAT>(dx + m_area.left - XWIN4_OFFSET);
						orgin.y = static_cast<FLOAT>(dy + m_area.top + XWIN4_OFFSET);

						m_offsetX[m_idxTL] = orgin.x;
						m_offsetY[m_idxTL] = orgin.y;

						pD2DRenderTarget->DrawTextLayout(orgin, m_cacheTL[m_idxTL], pTextBrush);
						m_idxTL++;
						if (m_idxTL >= 1024) // it is impossible
						{
							assert(0);
						}
					}
				}
				pos += H;
				if (pos >= m_ptOffset.y + h) // out of the scope of the drawing area
					break;
				p = p->next;
			}
		}
		return 0; 
	}

	int Do_DUI_MOUSEMOVE(U32 uMsg, U64 wParam, U64 lParam, void* lpData = nullptr) 
	{ 
		int i, idxHit, r = 0;
		FLOAT xPos = static_cast<FLOAT>(GET_X_LPARAM(lParam));
		FLOAT yPos = static_cast<FLOAT>(GET_Y_LPARAM(lParam));
		int w = m_area.right - m_area.left;
		int h = m_area.bottom - m_area.top;

		HRESULT hr;
		BOOL isTrailingHit = FALSE;
		BOOL isInside = FALSE;
		DWRITE_HIT_TEST_METRICS htm = { 0 };
		FLOAT dx, dy;

		idxHit = -1;
		for (i = 0; i < m_idxTL; i++)
		{
			assert(m_cacheTL[i]);
			dx = xPos - m_offsetX[i];
			dy = yPos - m_offsetY[i];
			hr = m_cacheTL[i]->HitTestPoint(dx, dy, &isTrailingHit, &isInside, &htm);
			if (S_OK == hr && isInside)
			{
				::SetCursor(dui_hCursorIBeam);
				SetDUIWindowCursor();
				idxHit = i;
				break;
			}
		}
		return r; 
	}
};

#endif  /* __DUI_WINDOW4_H__ */

