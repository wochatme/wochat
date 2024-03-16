#ifndef __WT_DUI_H__
#define __WT_DUI_H__

#include <assert.h>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>

#include "wochatypes.h"

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include "duiwin32.h"
#endif 

#include "dui_render.h"

#define DUI_DEBUG	1
#define DUI_OK      0
#define DUI_FAILED  0xFF


/* DUI_ALIGN() is only to be used to align on a power of 2 boundary */
#define DUI_ALIGN(size, boundary)   (((size) + ((boundary) -1)) & ~((boundary) - 1))
#define DUI_ALIGN_DEFAULT32(size)   DUI_ALIGN(size, 4)
#define DUI_ALIGN_DEFAULT64(size)   DUI_ALIGN(size, 8)      /** Default alignment */
#define DUI_ALIGN_PAGE(size)        DUI_ALIGN(size, (1<<16))
#define DUI_ALIGN_TRUETYPE(size)    DUI_ALIGN(size, 64)    

/*
 * Fast MOD arithmetic, assuming that y is a power of 2 !
 */
#define DUI_MOD(x,y)			 ((x) & ((y)-1))

#ifdef _MSC_VER
#define DUI_NO_VTABLE __declspec(novtable)
#else
#define DUI_NO_VTABLE
#endif

#define DUI_ALLOCSET_DEFAULT_INITSIZE   (8 * 1024)
#define DUI_ALLOCSET_DEFAULT_MAXSIZE    (8 * 1024 * 1024)
#define DUI_ALLOCSET_SMALL_INITSIZE     (1 * 1024)
#define DUI_ALLOCSET_SMALL_MAXSIZE	    (8 * 1024)

typedef struct tagXPOINT
{
    int  x;
    int  y;
} XPOINT;

typedef struct tagXSIZE
{
    int  cx;
    int  cy;
} XSIZE;

typedef struct tagXRECT
{
    int    left;
    int    top;
    int    right;
    int    bottom;
} XRECT;


// a pure 32-bit true color bitmap object
typedef struct XBitmap
{
    U8    id;
    U32* data;
    int   w;
    int   h;
} XBitmap;

// determin if one object is hitted
#define XWinPointInRect(x, y, OBJ)  \
        (((x) >= ((OBJ)->left)) && ((x) < ((OBJ)->right)) && ((y) >= ((OBJ)->top)) && ((y) < ((OBJ)->bottom)))

#define MESSAGEMAP_TABLE_SIZE       (1<<16)

extern U8 DUIMessageOSMap[MESSAGEMAP_TABLE_SIZE];

extern HCURSOR	dui_hCursorArrow;
extern HCURSOR	dui_hCursorWE;
extern HCURSOR	dui_hCursorNS;
extern HCURSOR	dui_hCursorHand;
extern HCURSOR	dui_hCursorIBeam;


#define DUI_GLOBAL_STATE_IN_NONE_MODE   0x0000000000000000
#define DUI_GLOBAL_STATE_NEED_REDRAW    0x0000000000000001
#define DUI_GLOBAL_STATE_IN_DRAG_MODE   0x0000000000000002
#define DUI_GLOBAL_STATE_SET_CURSOR     0x0000000000000004
#define DUI_GLOBAL_STATE_INIT_FAILED    0x0000000000000008
#define DUI_GLOBAL_STATE_CTRL_PRESSED   0x0000000000000010
#define DUI_GLOBAL_STATE_SHIFT_PRESSED  0x0000000000000020
#define DUI_GLOBAL_STATE_ALT_PRESSED    0x0000000000000040
#define DUI_GLOBAL_STATE_LBUTTON_DOWN   0x0000000000000080

// this gloable variable is shared by all virtual windows so they can know each other
extern U64 dui_status;

#define DUIWindowLButtonDown()           (DUI_GLOBAL_STATE_LBUTTON_DOWN & dui_status)
#define SetDUIWindowLBtnDown()          do { dui_status |= DUI_GLOBAL_STATE_LBUTTON_DOWN;  } while(0)
#define ClearDUIWindowLBtnDown()        do { dui_status &= ~DUI_GLOBAL_STATE_LBUTTON_DOWN; } while(0)

#define DUIWindowInDragMode()           (DUI_GLOBAL_STATE_IN_DRAG_MODE & dui_status)
#define SetDUIWindowDragMode()          do { dui_status |= DUI_GLOBAL_STATE_IN_DRAG_MODE;  } while(0)
#define ClearDUIWindowDragMode()        do { dui_status &= ~DUI_GLOBAL_STATE_IN_DRAG_MODE; } while(0)

#define DUIWindowNeedReDraw()           (DUI_GLOBAL_STATE_NEED_REDRAW & dui_status)
#define InvalidateDUIWindow()           do { dui_status |= DUI_GLOBAL_STATE_NEED_REDRAW; } while(0)
#define ClearDUIWindowReDraw()          do { dui_status &= ~DUI_GLOBAL_STATE_NEED_REDRAW;} while(0)

#define DUIWindowCursorIsSet()          (DUI_GLOBAL_STATE_SET_CURSOR & dui_status)
#define SetDUIWindowCursor()            do { dui_status |= DUI_GLOBAL_STATE_SET_CURSOR; } while(0)
#define ClearDUIWindowCursor()          do { dui_status &= ~DUI_GLOBAL_STATE_SET_CURSOR;} while(0)

#define DUIWindowInitFailed()           (DUI_GLOBAL_STATE_INIT_FAILED & dui_status)
#define SetDUIWindowInitFailed()        do { dui_status |= DUI_GLOBAL_STATE_INIT_FAILED; } while(0)

#define DUIShiftKeyIsPressed()          (DUI_GLOBAL_STATE_SHIFT_PRESSED & dui_status)
#define DUICtrlKeyIsPressed()           (DUI_GLOBAL_STATE_CTRL_PRESSED & dui_status)
#define DUIAltKeyIsPressed()            (DUI_GLOBAL_STATE_ALT_PRESSED & dui_status)

#define SetDUIWindowShiftKey()          do { dui_status |= DUI_GLOBAL_STATE_SHIFT_PRESSED; } while(0)
#define SetDUIWindowCtrlKey()           do { dui_status |= DUI_GLOBAL_STATE_CTRL_PRESSED; } while(0)
#define SetDUIWindowAltKey()            do { dui_status |= DUI_GLOBAL_STATE_ALT_PRESSED; } while(0)

#define ClearDUIWindowShiftKey()        do { dui_status &= ~DUI_GLOBAL_STATE_SHIFT_PRESSED; } while(0)
#define ClearDUIWindowCtrlKey()         do { dui_status &= ~DUI_GLOBAL_STATE_CTRL_PRESSED; } while(0)
#define ClearDUIWindowAltKey()          do { dui_status &= ~DUI_GLOBAL_STATE_ALT_PRESSED; } while(0)

#define IsHexLetter(c)                  (((c) >= '0' && (c) <= '9') || ((c) >= 'A' && (c) <= 'F') || ((c) >='a' && (c) <= 'f'))
#define IsAlphabet(c)		            (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))

// DUI Message that is OS independent;
#define DUI_ALLMESSAGE          0x00
#define DUI_NULL                0x00
#define DUI_CREATE              0x01
#define DUI_DESTROY             0x02
#define DUI_ERASEBKGND          0x03
#define DUI_PAINT         	    0x04
#define DUI_TIMER               0x05
#define DUI_MOUSEMOVE           0x06
#define DUI_MOUSEWHEEL          0x07
#define DUI_MOUSEFIRST          0x08
#define DUI_LBUTTONDOWN         0x0A
#define DUI_LBUTTONUP           0x0B
#define DUI_LBUTTONDBLCLK       0x0C
#define DUI_RBUTTONDOWN         0x0D
#define DUI_RBUTTONUP           0x0E
#define DUI_RBUTTONDBLCLK       0x0F
#define DUI_MBUTTONDOWN         0x10
#define DUI_MBUTTONUP           0x11
#define DUI_MBUTTONDBLCLK       0x12
#define DUI_CAPTURECHANGED      0x13
#define DUI_KEYFIRST            0x14
#define DUI_KEYDOWN             0x15
#define DUI_KEYUP               0x16
#define DUI_CHAR                0x17
#define DUI_DEADCHAR            0x18
#define DUI_SYSKEYDOWN          0x19
#define DUI_SYSKEYUP            0x1A
#define DUI_SYSCHAR             0x1B
#define DUI_SYSDEADCHAR         0x1C
#define DUI_GETMINMAXINFO       0x1D
#define DUI_SIZE                0x1E
#define DUI_SETCURSOR           0x1F
#define DUI_SETFOCUS            0x20
#define DUI_MOUSEACTIVATE       0x21
#define DUI_MOUSELEAVE          0x22
#define DUI_MOUSEHOVER          0x23
#define DUI_NCLBUTTONDOWN       0x24


// User defined message
#define DUI_XWINDOWS00          0xFF
#define DUI_XWINDOWS01          0xFE
#define DUI_XWINDOWS02          0xFD
#define DUI_XWINDOWS03          0xFC
#define DUI_XWINDOWS04          0xFB
#define DUI_XWINDOWS05          0xFA
#define DUI_XWINDOWS06          0xF9
#define DUI_XWINDOWS07          0xF8
#define DUI_XWINDOWS08          0xF7
#define DUI_XWINDOWS09          0xF6

#ifdef _WIN32
#define WM_XWINDOWSXX       (WM_USER + DUI_NULL)
#define WM_XWINDOWS00       (WM_USER + DUI_XWINDOWS00)
#define WM_XWINDOWS01       (WM_USER + DUI_XWINDOWS01)
#define WM_XWINDOWS02       (WM_USER + DUI_XWINDOWS02)
#define WM_XWINDOWS03       (WM_USER + DUI_XWINDOWS03)
#define WM_XWINDOWS04       (WM_USER + DUI_XWINDOWS04)
#define WM_XWINDOWS05       (WM_USER + DUI_XWINDOWS05)
#define WM_XWINDOWS06       (WM_USER + DUI_XWINDOWS06)
#define WM_XWINDOWS07       (WM_USER + DUI_XWINDOWS07)
#define WM_XWINDOWS08       (WM_USER + DUI_XWINDOWS08)
#define WM_XWINDOWS09       (WM_USER + DUI_XWINDOWS09)
#define WM_XWINDOWSPAINT    (WM_USER + DUI_PAINT)
#endif 

#define DUI_KEY_RETURN      0x0D

int DUI_Init();
void DUI_Term();

#define XMOUSE_NULL         0
#define XMOUSE_MOVE         1
#define XMOUSE_LBDOWN       2
#define XMOUSE_LBUP         3
#define XMOUSE_RBDOWN       4
#define XMOUSE_RBUP         5

#define XKEYBOARD_NORMAL    0x0000
#define XKEYBOARD_CTRL      0x0001
#define XKEYBOARD_SHIF      0x0002
#define XKEYBOARD_ALT       0x0004

enum XControlProperty
{
    XCONTROL_PROP_NONE     = 0x00000000,
    XCONTROL_PROP_ROUND    = 0x00000001,
    XCONTROL_PROP_STATIC   = 0x00000002,
    //XCONTROL_PROP_LBDOWN   = 0x00000004,
    XCONTROL_PROP_KEYBOARD = 0x00000008,
    XCONTROL_PROP_EDITBOX  = 0x00000010,
    XCONTROL_PROP_TEXT     = 0x00000020,
    XCONTROL_PROP_HIDDEN   = 0x00000040,
    XCONTROL_PROP_FOCUS    = 0x00000080,
    XCONTROL_PROP_RBDOWN   = 0x00000100,
    XCONTROL_PROP_RBUP     = 0x00000200
};

enum XControlState
{
    XCONTROL_STATE_HIDDEN = 0,
    XCONTROL_STATE_NORMAL,
    XCONTROL_STATE_HOVERED,
    XCONTROL_STATE_PRESSED,
    XCONTROL_STATE_ACTIVE
};

typedef void* DUI_Surface;
typedef void* DUI_TextFactory;
typedef void* DUI_TextFormat;
typedef void* DUI_Brush;

typedef struct DUI_TEXT_RANGE 
{
    U32 startPosition;
    U32 length;
} DUI_TEXT_RANGE;

// the generic control class
class XControl
{
public:
    U16   m_Id = 0;
protected:
    U8   m_Name[16+1] = { 0 };

    // each control has two sizes: inner and outer.
    // for Button/Label, they are the same.
    // for Editbox with vertical scrollbar, they are not the same
    // inner size
    int  m_left   = 0;
    int  m_top    = 0;
    int  m_right  = 0;
    int  m_bottom = 0;
    // outer size
    int  m_left2   = 0;
    int  m_top2    = 0;
    int  m_right2  = 0;
    int  m_bottom2 = 0;

    U32  m_property = XCONTROL_PROP_NONE;
    U32  m_status = XCONTROL_STATE_NORMAL;
    U32  m_statusPrev = XCONTROL_STATE_NORMAL;

    U32* m_parentBuf = nullptr; // the draw screen buffer of the parent
    int  m_parentL = 0; // the left position of the parent
    int  m_parentT = 0; // the top position of the parent
    int  m_parentR = 0; // the right position of the parent
    int  m_parentB = 0; // the bottom position of the parent

    U32  m_Color0 = 0xFFFFFFFF;
    U32  m_Color1 = 0xFFFFFFFF;

    wchar_t  m_tipMessage[32];

    void* m_Cursor1 = nullptr;  // the cursor when the mouse is over the inner part
    void* m_Cursor2 = nullptr;  // the cursor when the mousr is over the outer part

public:
    int DoMouseMove(int x, int y, int idxActive);
    int DoMouseLBClickDown(int x, int y, int* idxActive = nullptr);
    int DoMouseLBClickUp(int x, int y, int* idxActive = nullptr);
    int DoMouseRBClickDown(int x, int y, int* idxActive = nullptr);
    int DoMouseRBClickUp(int x, int y, int absX, int absY, HWND hWnd);
    int DoMouseLeave();
    int DoMouseHover(int x, int y);
    int ShowCursor(bool inner = true);

    void setRoundColor(U32 c0, U32 c1)
    {
        m_Color0 = c0;
        m_Color1 = c1;
    }

    int getLeft(bool inner = true)
    {
        assert(m_left >= 0);
        assert(m_left2 >= 0);
        return (inner ? m_left : m_left2);
    }

    int getRight(bool inner = true)
    {
        assert(m_right >= 0);
        assert(m_right2 >= 0);
        return (inner ? m_right : m_right2);
    }

    int getTop(bool inner = true)
    {
        assert(m_top >= 0);
        assert(m_top2 >= 0);

        return (inner ? m_top : m_top2);
    }

    int getBottom(bool inner = true)
    {
        assert(m_bottom >= 0);
        assert(m_bottom2 >= 0);

        return (inner ? m_bottom : m_bottom2);
    }

    int getWidth(bool inner = true)
    {
        assert(m_right >= m_left);
        assert(m_right2 >= m_left2);

        return (inner ? (m_right - m_left) : (m_right2 - m_left2));
    }

    int getHeight(bool inner = true)
    {
        assert(m_bottom >= m_top);
        assert(m_bottom2 >= m_top2);

        return (inner ? (m_bottom - m_top) : (m_bottom2 - m_top2));
    }

    void setSize(int width, int height, bool inner = true)
    {
        assert(width >= 0);
        assert(height >= 0);
        if (inner)
        {
            m_left = m_top = 0;
            m_right = width;
            m_bottom = height;
        }
        else
        {
            m_left2 = m_top2 = 0;
            m_right2 = width;
            m_bottom2 = height;
        }
    }

    void setPosition(int left_, int top_, int right_, int bottom_, bool inner = true)
    {
#if 0
        assert(left_ >= 0);
        assert(top_ >= 0);
        assert(right_ >= left_);
        assert(bottom_ >= top_);
#endif 
        if (inner)
        {
            m_left = left_;
            m_top = top_;
            m_right = right_;
            m_bottom = bottom_;
        }
        else
        {
            m_left2 = left_;
            m_top2 = top_;
            m_right2 = right_;
            m_bottom2 = bottom_;
        }
        AfterPositionIsChanged(inner);
    }

    void setPosition(int left_, int top_, bool inner = true)
    {
        int w, h;
#if 0
        assert(left_ >= 0);
        assert(top_ >= 0);
#endif
        if (inner)
        {
            w = m_right - m_left;
            h = m_bottom - m_top;

            assert(w >= 0);
            assert(h >= 0);

            m_left = left_;
            m_top = top_;
            m_right = m_left + w;
            m_bottom = m_top + h;
        }
        else
        {
            w = m_right2 - m_left2;
            h = m_bottom2 - m_top2;

            assert(w >= 0);
            assert(h >= 0);

            m_left2 = left_;
            m_top2 = top_;
            m_right2 = m_left2 + w;
            m_bottom2 = m_top2 + h;
        }

        AfterPositionIsChanged(inner);
    }

    void setProperty(U32 property)
    {
        m_property |= property;
    }

    int clearProperty(U32 property)
    {
        int r = 0;

        r = (m_property & property) ? 1 : 0; // if property is exist, then we need to redraw

        m_property &= (~property);

        return r;
    }

    U32 getProperty()
    {
        return m_property;
    }

    void AttachParent(U32* parentBuf, int left, int top, int right, int bottom)
    {
        m_parentBuf = parentBuf;
        m_parentL = left;
        m_parentT = top;
        m_parentR = right;
        m_parentB = bottom;
    }

    bool IsVisible()
    {
        return (0 == (XCONTROL_PROP_HIDDEN & m_property));
    }

    void HideMe(bool hide = true)
    {
        if(hide)
            m_property |= XCONTROL_PROP_HIDDEN;
        else
            m_property &= ~XCONTROL_PROP_HIDDEN;
    }

    bool IsOverMe(int xPos, int yPos, bool* inner = nullptr)
    {
        bool bRet = false;
        if(inner)
            *inner = false;
        if (!(XCONTROL_PROP_STATIC & m_property)) // this is not a static control
        {
            if (xPos >= m_left && xPos < m_right && yPos >= m_top && yPos < m_bottom)
            {
                bRet = true;
                if (inner)
                    *inner = true;
            }
            else if (xPos >= m_left2 && xPos < m_right2 && yPos >= m_top2 && yPos < m_bottom2)
            {
                bRet = true;
                if (inner)
                    *inner = false;
            }
        }
        return bRet;
    }

    int setStatus(U32 newStatus, U8 mouse_event = XMOUSE_NULL)
    {
        int r = 0;

        if (!(XCONTROL_PROP_STATIC & m_property)) // the static control cannot change the status
        {
            r = (m_status != newStatus) ? 1 : 0;
            m_statusPrev = m_status;
            m_status = newStatus;
        }
        return r;
    }

    U32 getStatus()
    {
        return m_status;
    }

    int Init(U16 id, const char* name, void* ptr0 = nullptr, void* ptr1 = nullptr)
    {
        size_t len = strlen(name);

        m_Id = id;

        if (len > 16)
            len = 16;

        if (len > 0)
        {
            size_t i;
            char* p = (char*)name;
            for (i = 0; i < len; i++)
                m_Name[i] = *p++;
            m_Name[i] = '\0';
        }
        else m_Name[0] = '\0';

        return InitControl(ptr0, ptr1);
    }

    virtual int OnTimer() { return 0; }
    virtual int AfterPositionIsChanged(bool inner = true) { return 0; }
    virtual void Term() {}
    virtual int Draw(int dx = 0, int dy = 0) { return 0; }
    virtual int OnKeyBoard(U32 msg, U64 wparam = 0, U64 lparam = 0) { return 0; }

    virtual int DrawText(int dx, int dy,
        DUI_Surface surface = nullptr, 
        DUI_Brush brush = nullptr, 
        DUI_Brush brushSel = nullptr, 
        DUI_Brush brushCaret = nullptr, U64 flag = 0) 
    {
        return 0;
    };

    virtual int InitControl(void* ptr0 = nullptr, void* ptr1 = nullptr) = 0;

    int (*pfAction) (void* obj, U32 uMsg, U64 wParam, U64 lParam);
};

class XButton : public XControl
{
public:
    // all XBitmpas should have extactly the same size
    XBitmap* imgNormal;
    XBitmap* imgHover;
    XBitmap* imgPress;
    XBitmap* imgActive;
    int Draw(int dx = 0, int dy = 0);

    int InitControl(void* ptr0 = nullptr, void* ptr1 = nullptr)
    {
        m_Cursor1 = dui_hCursorHand;
        m_Cursor2 = dui_hCursorArrow;
        return 0;
    }

    void setBitmap(XBitmap* n, XBitmap* h, XBitmap* p, XBitmap* a)
    {
        assert(nullptr != n);
        assert(nullptr != h);
        assert(nullptr != p);
        assert(nullptr != a);
        imgNormal = n; imgHover = h; imgPress = p; imgActive = a;

        if(0 == m_left && 0 == m_top && 0 == m_right && 0 == m_bottom)
            setSize(imgNormal->w, imgNormal->h);
    }

};

#define DUI_MAX_LABEL_STRING    128
class XLabel : public XControl
{
private:
    U32  m_bkgColor = 0xFFFFFFFF;
    U32  m_caretAnchor = 0;
    U32  m_caretPosition = 0;
    U32  m_caretPositionOffset = 0;
    U16  m_Text[DUI_MAX_LABEL_STRING + 1] = { 0 };
    U16  m_TextLen = 0;
    IDWriteFactory*      m_pTextFactory = nullptr;
    IDWriteTextFormat*   m_pTextFormat = nullptr;
    IDWriteTextLayout*   m_pTextLayout = nullptr;
public:
    XLabel()
    {
        m_property = XCONTROL_PROP_TEXT;
    }

    int DrawText(int dx, int dy,
        DUI_Surface surface = nullptr,
        DUI_Brush brush = nullptr,
        DUI_Brush brushSel = nullptr,
        DUI_Brush brushCaret = nullptr, U64 flag = 0);

    int InitControl(void* ptr0 = nullptr, void* ptr1 = nullptr)
    {
        m_Cursor1 = dui_hCursorIBeam;
        m_Cursor2 = dui_hCursorIBeam;
        m_pTextFactory = static_cast<IDWriteFactory*>(ptr0);
        m_pTextFormat  = static_cast<IDWriteTextFormat*>(ptr1);
        return 0;
    }

    void setText(U16* text, U16 len = 0);

};

enum SetSelectionMode
{
    SetSelectionModeLeft,               // cluster left
    SetSelectionModeRight,              // cluster right
    SetSelectionModeUp,                 // line up
    SetSelectionModeDown,               // line down
    SetSelectionModeLeftChar,           // single character left (backspace uses it)
    SetSelectionModeRightChar,          // single character right
    SetSelectionModeLeftWord,           // single word left
    SetSelectionModeRightWord,          // single word right
    SetSelectionModeHome,               // front of line
    SetSelectionModeEnd,                // back of line
    SetSelectionModeFirst,              // very first position
    SetSelectionModeLast,               // very last position
    SetSelectionModeAbsoluteLeading,    // explicit position (for mouse click)
    SetSelectionModeAbsoluteTrailing,   // explicit position, trailing edge
    SetSelectionModeAll                 // select all text
};

#define DUI_EDITBOX_MAX_LENGTH		(1<<16)

inline bool IsLowSurrogate(U32 ch) throw()
{
    // 0xDC00 <= ch <= 0xDFFF
    return (ch & 0xFC00) == 0xDC00;
}

inline bool IsHighSurrogate(U32 ch) throw()
{
    // 0xD800 <= ch <= 0xDBFF
    return (ch & 0xFC00) == 0xD800;
}

#define DUI_EDITBOXLINE_MAX_LENGTH      128
class XEditBoxLine : public XControl  // one line edit box
{
private:
    bool m_caret = false;
    U32  m_bkgColor = 0xFFFFFFFF;
    U32  m_caretAnchor = 0;
    U32  m_caretPosition = 0;
    U32  m_caretPositionOffset = 0;
    int  m_Height = 0;  // the total height of the text in pixel
    int  m_Offset = 0;  // the offset of the vertical scrollbar
    //int  m_Width = 0;
    wchar_t  m_Text[DUI_EDITBOXLINE_MAX_LENGTH + 1] = { 0 };
    U32  m_TextLen = 0;
public:
    void* m_pTextFactory = nullptr;
    void* m_pTextFormat = nullptr;
    IDWriteTextLayout* m_pTextLayout = nullptr;

    XEditBoxLine()
    {
        m_property = XCONTROL_PROP_EDITBOX;
    }

    int OnTimer()
    {
        m_caret = !m_caret;
        // if this editbox has focus, we need to redraw the caret on timer
        return (m_property & XCONTROL_PROP_FOCUS) ? 1 : 0;
    }

    int InitControl(void* ptr0 = nullptr, void* ptr1 = nullptr);
    int Draw(int dx = 0, int dy = 0);
    int DrawText(int dx, int dy,
        DUI_Surface surface = nullptr,
        DUI_Brush brush = nullptr,
        DUI_Brush brushSel = nullptr,
        DUI_Brush brushCaret = nullptr, U64 flag = 0);

};

class XEditBox : public XControl
{
private:
    bool m_caret = false;
    U32  m_bkgColor = 0xFFFFFFFF;
    U32  m_caretAnchor = 0;
    U32  m_caretPosition = 0;
    U32  m_caretPositionOffset = 0;
    int  m_Height = 0;  // the total height of the text in pixel
    int  m_Offset = 0;  // the offset of the vertical scrollbar
    //int  m_Width = 0;
    //U16  m_Text[DUI_EDITBOX_MAX_LENGTH + 1] = { 0 };
    U32  m_TextLen = 0;
    std::wstring m_Text;

    void* m_pTextFactory = nullptr;
    void* m_pTextFormat = nullptr;
    IDWriteTextLayout* m_pTextLayout = nullptr;

    EditableLayout m_layoutEditor;
    EditableLayout::CaretFormat m_caretFormat;

    DrawingEffect* m_pageBackgroundEffect = nullptr;
    DrawingEffect* m_textSelectionEffect = nullptr;
    DrawingEffect* m_imageSelectionEffect = nullptr;
    DrawingEffect* m_caretBackgroundEffect = nullptr;

public:
    XEditBox()
    {
        m_property = XCONTROL_PROP_EDITBOX;
    }

    int OnTimer() 
    { 
        m_caret = !m_caret;
        // if this editbox has focus, we need to redraw the caret on timer
        return (m_property & XCONTROL_PROP_FOCUS) ? 1 : 0; 
    }

    int Draw(int dx = 0, int dy = 0);

    int DrawText(int dx, int dy,
        DUI_Surface surface = nullptr,
        DUI_Brush brush = nullptr,
        DUI_Brush brushSel = nullptr,
        DUI_Brush brushCaret = nullptr, U64 flag = 0);

    int InitControl(void* ptr0 = nullptr, void* ptr1 = nullptr)
    {
        m_Cursor1 = dui_hCursorIBeam;
        m_Cursor2 = dui_hCursorArrow;

        m_pTextFactory = ptr0;
        m_pTextFormat =  ptr1;

        m_layoutEditor.SetFactory(m_pTextFactory);
        //m_Text[0] = 0x5B57;
        m_TextLen = 0;
        m_Text.assign(L"");
#if 0
        m_pageBackgroundEffect  = SafeAcquire(new(std::nothrow) DrawingEffect(0xFF000000 | D2D1::ColorF::White));
        m_textSelectionEffect   = SafeAcquire(new(std::nothrow) DrawingEffect(0xFF000000 | D2D1::ColorF::LightSkyBlue));
        m_imageSelectionEffect  = SafeAcquire(new(std::nothrow) DrawingEffect(0x80000000 | D2D1::ColorF::LightSkyBlue));
        m_caretBackgroundEffect = SafeAcquire(new(std::nothrow) DrawingEffect(0xFF000000 | D2D1::ColorF::Black));
#endif
        return 0;
    }

    U32 GetInputUTF16MessageLength()
    {
        return (m_Text.size());
    }

    U32 GetInputUTF16Message(U16* utf16buf, U32 max_len);

    U32 ClearText();

    int AfterPositionIsChanged(bool inner = true);
    
    void Term();
  
    void GetCaretRect(RectF& rect);

    int OnKeyBoard(U32 msg, U64 wparam = 0, U64 lparam = 0);
    
    int PasteFromClipboard();
    
    int DeleteSelection();
    
    DWRITE_TEXT_RANGE GetSelectionRange();

    bool SetSelection(SetSelectionMode moveMode, U32 advance, bool extendSelection, bool updateCaretFormat = true);
    void AlignCaretToNearestCluster(bool isTrailingHit = false, bool skipZeroWidth = false);

    void GetLineFromPosition(
        const DWRITE_LINE_METRICS* lineMetrics,   // [lineCount]
        U32 lineCount,
        U32 textPosition,
        OUT U32* lineOut,
        OUT U32* linePositionOut
    );

    void GetLineMetrics(OUT std::vector<DWRITE_LINE_METRICS>& lineMetrics);
    void UpdateSystemCaret(const RectF& rect);
    void UpdateCaretFormatting();
    void CopyToClipboard();
};


#endif // __WT_DUI_H__