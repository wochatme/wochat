#include "dui.h"
#include "duiwin32.h"

// when the mouse is over this control, show the cursor of this control
int XControl::ShowCursor(bool inner)
{
    int ret = 0;

    if (!(XCONTROL_PROP_STATIC & m_property))
    {
        if (inner)
        {
            if (nullptr != m_Cursor1)
            {
                ::SetCursor((HCURSOR)m_Cursor1);
                SetDUIWindowCursor();
                ret = 1;
            }
        }
        else
        {
            if (nullptr != m_Cursor2)
            {
                ::SetCursor((HCURSOR)m_Cursor2);
                SetDUIWindowCursor();
                ret = 1;
            }
        }
    }

    return ret;
}

int XLabel::DrawText(int dx, int dy, DUI_Surface surface, DUI_Brush brush, DUI_Brush brushSel, DUI_Brush brushCaret, U64 flag)
{
    ID2D1HwndRenderTarget* pD2DRenderTarget = (ID2D1HwndRenderTarget*)surface;
    ID2D1SolidColorBrush* pTextBrush        = (ID2D1SolidColorBrush*)brush;
    ID2D1SolidColorBrush* pTextBrushSel     = (ID2D1SolidColorBrush*)brushSel;
    IDWriteTextLayout* pTextLayout          = (IDWriteTextLayout*)m_pTextLayout;

    float X = static_cast<FLOAT>(dx + m_left);
    float Y = static_cast<FLOAT>(dy + m_top);

    if (pTextLayout && pD2DRenderTarget && pTextBrush && pTextBrushSel)
    {
        D2D1_POINT_2F orgin;
        orgin.x = X;
        orgin.y = Y;
#if 0
        int w = m_right - m_left;
        if (w > (m_parentW * 7 / 8))
        {
            // Set trimming to ellipsis
            DWRITE_TRIMMING trimming = { DWRITE_TRIMMING_GRANULARITY_CHARACTER, 0, 0 };
            IDWriteInlineObject* pEllipsisInlineObject;
            IDWriteFactory* pTextFactory = static_cast<IDWriteFactory*>(m_pTextFactory);
            IDWriteTextFormat* pTextFormat = static_cast<IDWriteTextFormat*>(m_pTextFormat);

            HRESULT hr = pTextFactory->CreateEllipsisTrimmingSign(pTextFormat, &pEllipsisInlineObject);
            if (SUCCEEDED(hr))
            {
                trimming.granularity = DWRITE_TRIMMING_GRANULARITY_CHARACTER;
                trimming.delimiter = 0;
                trimming.delimiterCount = 0;
                pTextLayout->SetTrimming(&trimming, pEllipsisInlineObject);
                pEllipsisInlineObject->Release();
            }
        }
#endif
        pD2DRenderTarget->DrawTextLayout(orgin, pTextLayout, pTextBrush);
    }

    return 0;
}

void XLabel::setText(U16* text, U16 len)
{
    if (len > 0)
    {
        m_TextLen = (len <= DUI_MAX_LABEL_STRING) ? len : DUI_MAX_LABEL_STRING;
        for (U16 i = 0; i < m_TextLen; i++) m_Text[i] = text[i];
        m_Text[m_TextLen] = L'\0';

        SafeRelease(&m_pTextLayout);

        assert(m_pTextFactory);
        assert(m_pTextFormat);
        m_pTextFactory->CreateTextLayout((const wchar_t*)m_Text, m_TextLen, 
            m_pTextFormat, (FLOAT)1, (FLOAT)1, &m_pTextLayout);
        if (m_pTextLayout)
        {
            DWRITE_TEXT_METRICS tm;
            m_pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            m_pTextLayout->GetMetrics(&tm);
            m_left = 0;
            m_right = static_cast<int>(tm.width) + 1;
            m_top = 0;
            m_bottom = static_cast<int>(tm.height) + 1;
        }
    }
}

int XEditBoxLine::InitControl(void* ptr0, void* ptr1)
{
    m_Cursor1 = dui_hCursorIBeam;
    m_Cursor2 = dui_hCursorArrow;

    m_pTextFactory = ptr0;
    m_pTextFormat = ptr1;
    m_TextLen = 0;
    m_Text[0] = L'\0';

    IDWriteFactory* pTextFactory = static_cast<IDWriteFactory*>(m_pTextFactory);
    IDWriteTextFormat* pTextFormat = static_cast<IDWriteTextFormat*>(m_pTextFormat);
    if (pTextFactory && pTextFormat)
    {
        SafeRelease((IDWriteTextLayout**)(&m_pTextLayout));

        HRESULT hr = pTextFactory->CreateTextLayout(
            m_Text,
            m_TextLen,
            pTextFormat,
            1000.f,
            1.f,
            (IDWriteTextLayout**)(&m_pTextLayout));

        if (S_OK == hr && m_pTextLayout)
        {
            DWRITE_TEXT_METRICS tm;
            IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
            pTextLayout->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
            pTextLayout->GetMetrics(&tm);
            m_Height = static_cast<int>(tm.height) + 1;
        }
    }
    return m_pTextLayout ? 0 : 1;
}


int XEditBoxLine::DrawText(int dx, int dy, DUI_Surface surface,DUI_Brush brush, DUI_Brush brushSel, DUI_Brush brushCaret, U64 flag)
{
    float W, H;
    float X = static_cast<FLOAT>(dx + m_left + 5);
    float Y = static_cast<FLOAT>(dy + m_top + ((m_bottom - m_top - m_Height)>>1));

    ID2D1HwndRenderTarget* pD2DRenderTarget = static_cast<ID2D1HwndRenderTarget*>(surface);
    ID2D1SolidColorBrush* pTextBrush = static_cast<ID2D1SolidColorBrush*>(brush);
    ID2D1SolidColorBrush* pTextBrushSel = static_cast<ID2D1SolidColorBrush*>(brushSel);
    ID2D1SolidColorBrush* pTextBrushCaret = static_cast<ID2D1SolidColorBrush*>(brushCaret);
    IDWriteTextLayout* pTextLayout = m_pTextLayout;

    if (pTextLayout && pD2DRenderTarget && pTextBrush && pTextBrushSel && brushCaret)
    {
        D2D1_POINT_2F orgin;
        D2D1_RECT_F clipRect;
        DWRITE_TEXT_RANGE caretRange;

        if (m_caret && (m_property & XCONTROL_PROP_FOCUS)) // if the caret is visible and this editbox has the focus, we draw the caret
        {
            RectF caretRect = {};
            DWRITE_HIT_TEST_METRICS caretMetrics;
            float caretX, caretY;
            pTextLayout->HitTestTextPosition(m_caretPosition, m_caretPositionOffset > 0, &caretX, &caretY, &caretMetrics);
            DWORD caretIntThickness = 2;
            SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretIntThickness, FALSE); // get the width of the caret
            const float caretThickness = float(caretIntThickness);
            
            caretRect.left = caretX - caretThickness / 2.0f;
            caretRect.right = caretRect.left + caretThickness;
            caretRect.top = caretY;
            caretRect.bottom = caretY + caretMetrics.height;

            W = caretRect.right - caretRect.left;
            H = caretRect.bottom - caretRect.top;

            caretRect.left = X + caretRect.left;
            caretRect.top = Y + caretRect.top;
            caretRect.right = caretRect.left + W;
            caretRect.bottom = caretRect.top + H;
            if (caretRect.top < static_cast<FLOAT>(dy + m_top))
            {
                m_Offset = static_cast<int>(caretRect.top) - (dy + m_top) - 1;
            }

            if (caretRect.bottom > static_cast<FLOAT>(dy + m_bottom))
            {
                m_Offset = static_cast<int>(caretRect.bottom) + 1 - (dy + m_bottom);
            }

            pD2DRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
            pD2DRenderTarget->FillRectangle(caretRect, pTextBrushCaret);
            pD2DRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }
    }

    return 0;
}

void XEditBox::Term()
{
    //IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
    //SafeRelease(&m_pTextLayout);
}

U32 XEditBox::GetInputUTF16Message(U16* utf16buf, U32 max_len)
{
    U32 len = 0;

    if (utf16buf)
    {
        wchar_t* p = (wchar_t*)utf16buf;
        len = (U32)m_Text.size();
        if (len > max_len)
            len = max_len;
        for (U32 i = 0; i < len; i++)
            p[i] = m_Text[i];
    }
    return len;
}

U32 XEditBox::ClearText()
{
    if (m_Text.size() > 0)
    {
        m_Text.clear();
        IDWriteFactory* pTextFactory = static_cast<IDWriteFactory*>(m_pTextFactory);
        IDWriteTextFormat* pTextFormat = static_cast<IDWriteTextFormat*>(m_pTextFormat);
        if (pTextFactory && pTextFormat)
        {
            SafeRelease((IDWriteTextLayout**)(&m_pTextLayout));
            pTextFactory->CreateTextLayout(
                m_Text.c_str(),
                m_Text.length(),
                pTextFormat,
                static_cast<FLOAT>(m_right - m_left),
                static_cast<FLOAT>(m_bottom - m_top),
                (IDWriteTextLayout**)(&m_pTextLayout));
            // Set the initial text layout and update caret properties accordingly.
            if (m_pTextLayout)
                UpdateCaretFormatting();
        }
    }
    return 0;
}

// when the position is changed, the width may change, so we need to re-create the textlayout
int XEditBox::AfterPositionIsChanged(bool inner)
{
    IDWriteFactory* pTextFactory = static_cast<IDWriteFactory*>(m_pTextFactory);
    IDWriteTextFormat* pTextFormat = static_cast<IDWriteTextFormat*>(m_pTextFormat);

    assert(m_right >= m_left);
    assert(m_bottom >= m_top);

    if (pTextFactory && pTextFormat && inner)
    {
        SafeRelease((IDWriteTextLayout**)(&m_pTextLayout));

        pTextFactory->CreateTextLayout(
            m_Text.c_str(),
            m_Text.length(),
            pTextFormat,
            static_cast<FLOAT>(m_right - m_left),
            static_cast<FLOAT>(m_bottom - m_top),
            (IDWriteTextLayout**)(&m_pTextLayout));

        // Set the initial text layout and update caret properties accordingly.
        if (m_pTextLayout)
            UpdateCaretFormatting();
    }
    return 0;
}

int XEditBox::Draw(int dx, int dy)
{
    int ret = 0;

    IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);

    assert(m_parentBuf);
    assert(m_bottom > m_top);
    assert(m_bottom2 > m_top2);

    if (pTextLayout)
    {
        DWRITE_TEXT_METRICS textMetrics;
        pTextLayout->GetMetrics(&textMetrics);

        m_Height = static_cast<int>(textMetrics.height) + 1;

        int H = m_bottom2 - m_top2;
        if (m_Height > (m_bottom - m_top)) // we need to draw the vertical bar
        {
            // Draw the vertical scroll bar
            DUI_ScreenFillRect(m_parentBuf, m_parentR - m_parentL, m_parentB - m_parentT, 0xFF333333, 8, H, m_right2 - 8, m_top2);
            //DUI_ScreenFillRectRound(m_screen, w, h, m_thumbColor, thumb_width, thumb_height, w - m_scrollWidth + 1, thumb_start, m_scrollbarColor, 0xFFD6D3D2);
        }
    }
    return ret;
}

void XEditBox::GetLineMetrics(OUT std::vector<DWRITE_LINE_METRICS>& lineMetrics)
{
    // Retrieves the line metrics, used for caret navigation, up/down and home/end.
    IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
    DWRITE_TEXT_METRICS textMetrics;
    pTextLayout->GetMetrics(&textMetrics);

    lineMetrics.resize(textMetrics.lineCount);
    pTextLayout->GetLineMetrics(&lineMetrics.front(), textMetrics.lineCount, &textMetrics.lineCount);
}

// get the rect area of caret
void XEditBox::GetCaretRect(RectF& rect) 
{
    RectF zeroRect = {};
    rect = zeroRect;
    IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
    if (pTextLayout)
    {
        // Translate text character offset to point x,y.
        DWRITE_HIT_TEST_METRICS caretMetrics;
        float caretX, caretY;
        pTextLayout->HitTestTextPosition(m_caretPosition, m_caretPositionOffset > 0, &caretX, &caretY, &caretMetrics);

        // The default thickness of 1 pixel is almost _too_ thin on modern large monitors,
        DWORD caretIntThickness = 2;
        SystemParametersInfo(SPI_GETCARETWIDTH, 0, &caretIntThickness, FALSE); // get the width of the caret
#if 0
        if (caretIntThickness < 2)
            caretIntThickness = 2;
#endif
        const float caretThickness = float(caretIntThickness);

        // Return the caret rect, untransformed.
        rect.left = caretX - caretThickness / 2.0f;
        rect.right = rect.left + caretThickness;
        rect.top = caretY;
        rect.bottom = caretY + caretMetrics.height;
    }
}

// Returns a valid range of the current selection, regardless of whether the caret or anchor is first.
DWRITE_TEXT_RANGE XEditBox::GetSelectionRange()
{
    UINT32 caretBegin = m_caretAnchor;
    UINT32 caretEnd = m_caretPosition + m_caretPositionOffset;

    if (caretBegin > caretEnd)
        std::swap(caretBegin, caretEnd);

    // Limit to actual text length.
    UINT32 textLength = static_cast<UINT32>(m_Text.size());
    caretBegin = std::min(caretBegin, textLength);
    caretEnd = std::min(caretEnd, textLength);

    DWRITE_TEXT_RANGE textRange = { caretBegin, caretEnd - caretBegin };
    return textRange;
}

// Deletes selection.
int XEditBox::DeleteSelection()
{
    int ret = 0;

    DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();

    if (selectionRange.length > 0) // there is some selected text
    {
        m_layoutEditor.RemoveTextAt((IDWriteTextLayout**)(&m_pTextLayout), m_Text, selectionRange.startPosition, selectionRange.length);

        SetSelection(SetSelectionModeAbsoluteLeading, selectionRange.startPosition, false);

        ret = 1;
    }

    return ret;
}

// Given the line metrics, determines the current line and starting text
// position of that line by summing up the lengths. When the starting
// line position is beyond the given text position, we have our line.
void XEditBox::GetLineFromPosition(const DWRITE_LINE_METRICS* lineMetrics, // [lineCount]
    UINT32 lineCount, UINT32 textPosition, OUT UINT32* lineOut, OUT UINT32* linePositionOut)
{
    UINT32 line = 0;
    UINT32 linePosition = 0;
    UINT32 nextLinePosition = 0;
    for (; line < lineCount; ++line)
    {
        linePosition = nextLinePosition;
        nextLinePosition = linePosition + lineMetrics[line].length;
        if (nextLinePosition > textPosition)
        {
            // The next line is beyond the desired text position,
            // so it must be in the current line.
            break;
        }
    }
    *linePositionOut = linePosition;
    *lineOut = std::min(line, lineCount - 1);
}

// Moves the system caret to a new position.
// Although we don't actually use the system caret (drawing our own
// instead), this is important for accessibility, so the magnifier
// can follow text we type. The reason we draw our own directly
// is because intermixing DirectX and GDI content (the caret) reduces
// performance.
void XEditBox::UpdateSystemCaret(const RectF& rect)
{
#if 0
    // Gets the current caret position (in untransformed space).

    if (GetFocus() != hwnd_) // Only update if we have focus.
        return;

    D2D1::Matrix3x2F pageTransform;
    GetViewMatrix(&Cast(pageTransform));

    // Transform caret top/left and size according to current scale and origin.
    D2D1_POINT_2F caretPoint = pageTransform.TransformPoint(D2D1::Point2F(rect.left, rect.top));

    float width = (rect.right - rect.left);
    float height = (rect.bottom - rect.top);
    float transformedWidth = width * pageTransform._11 + height * pageTransform._21;
    float transformedHeight = width * pageTransform._12 + height * pageTransform._22;

    // Update the caret's location, rounding to nearest integer so that
    // it lines up with the text selection.

    int intX = RoundToInt(caretPoint.x);
    int intY = RoundToInt(caretPoint.y);
    int intWidth = RoundToInt(transformedWidth);
    int intHeight = RoundToInt(caretPoint.y + transformedHeight) - intY;

    CreateCaret(hwnd_, NULL, intWidth, intHeight);
    SetCaretPos(intX, intY);

    // Don't actually call ShowCaret. It's enough to just set its position.
#endif 

}

void XEditBox::UpdateCaretFormatting()
{
    UINT32 currentPos = m_caretPosition + m_caretPositionOffset;

    if (currentPos > 0)
    {
        --currentPos; // Always adopt the trailing properties.
    }

    IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
    // Get the family name
    m_caretFormat.fontFamilyName[0] = L'\0';
    pTextLayout->GetFontFamilyName(currentPos, &m_caretFormat.fontFamilyName[0], ARRAYSIZE(m_caretFormat.fontFamilyName));

    // Get the locale
    m_caretFormat.localeName[0] = L'\0';
    pTextLayout->GetLocaleName(currentPos, &m_caretFormat.localeName[0], ARRAYSIZE(m_caretFormat.localeName));

    // Get the remaining attributes...
    pTextLayout->GetFontWeight(currentPos, &m_caretFormat.fontWeight);
    pTextLayout->GetFontStyle(currentPos, &m_caretFormat.fontStyle);
    pTextLayout->GetFontStretch(currentPos, &m_caretFormat.fontStretch);
    pTextLayout->GetFontSize(currentPos, &m_caretFormat.fontSize);
    pTextLayout->GetUnderline(currentPos, &m_caretFormat.hasUnderline);
    pTextLayout->GetStrikethrough(currentPos, &m_caretFormat.hasStrikethrough);

    // Get the current color.
    IUnknown* drawingEffect = NULL;
    pTextLayout->GetDrawingEffect(currentPos, &drawingEffect);
    m_caretFormat.color = 0;
    if (drawingEffect != NULL)
    {
        DrawingEffect& effect = *reinterpret_cast<DrawingEffect*>(drawingEffect);
        m_caretFormat.color = effect.GetColor();
    }

    SafeRelease(&drawingEffect);
}

// Uses hit-testing to align the current caret position to a whole cluster,
// rather than residing in the middle of a base character + diacritic,
// surrogate pair, or character + UVS.
void XEditBox::AlignCaretToNearestCluster(bool isTrailingHit, bool skipZeroWidth)
{
    IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
    if (pTextLayout)
    {
        DWRITE_HIT_TEST_METRICS hitTestMetrics;
        float caretX, caretY;
        pTextLayout->HitTestTextPosition(
            m_caretPosition,
            false,
            &caretX,
            &caretY,
            &hitTestMetrics
        );

        // The caret position itself is always the leading edge.
        // An additional offset indicates a trailing edge when non-zero.
        // This offset comes from the number of code-units in the
        // selected cluster or surrogate pair.
        m_caretPosition = hitTestMetrics.textPosition;
        m_caretPositionOffset = (isTrailingHit) ? hitTestMetrics.length : 0;

        // For invisible, zero-width characters (like line breaks
        // and formatting characters), force leading edge of the
        // next position.
        if (skipZeroWidth && hitTestMetrics.width == 0)
        {
            m_caretPosition += m_caretPositionOffset;
            m_caretPositionOffset = 0;
        }
    }
}

// Moves the caret relatively or absolutely, optionally extending the
// selection range (for example, when shift is held).
bool XEditBox::SetSelection(SetSelectionMode moveMode, UINT32 advance, bool extendSelection, bool updateCaretFormat)
{
    UINT32 line = UINT32_MAX; // current line number, needed by a few modes
    UINT32 absolutePosition = m_caretPosition + m_caretPositionOffset;
    UINT32 oldAbsolutePosition = absolutePosition;
    UINT32 oldCaretAnchor = m_caretAnchor;
    IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
    switch (moveMode)
    {
    case SetSelectionModeLeft:
        m_caretPosition += m_caretPositionOffset;
        if (m_caretPosition > 0)
        {
            --m_caretPosition;
            AlignCaretToNearestCluster(false, true);

            // special check for CR/LF pair
            absolutePosition = m_caretPosition + m_caretPositionOffset;
            if (absolutePosition >= 1
                && absolutePosition < m_Text.size()
                && m_Text[absolutePosition - 1] == '\r'
                && m_Text[absolutePosition] == '\n')
            {
                m_caretPosition = absolutePosition - 1;
                AlignCaretToNearestCluster(false, true);
            }
        }
        break;

    case SetSelectionModeRight:
        m_caretPosition = absolutePosition;
        AlignCaretToNearestCluster(true, true);

        // special check for CR/LF pair
        absolutePosition = m_caretPosition + m_caretPositionOffset;
        if (absolutePosition >= 1
            && absolutePosition < m_Text.size()
            && m_Text[absolutePosition - 1] == '\r'
            && m_Text[absolutePosition] == '\n')
        {
            m_caretPosition = absolutePosition + 1;
            AlignCaretToNearestCluster(false, true);
        }
        break;

    case SetSelectionModeLeftChar:
        m_caretPosition = absolutePosition;
        m_caretPosition -= std::min(advance, absolutePosition);
        m_caretPositionOffset = 0;
        break;

    case SetSelectionModeRightChar:
        m_caretPosition = absolutePosition + advance;
        m_caretPositionOffset = 0;
        {
            // Use hit-testing to limit text position.
            DWRITE_HIT_TEST_METRICS hitTestMetrics;
            float caretX, caretY;
            pTextLayout->HitTestTextPosition(
                m_caretPosition,
                false,
                &caretX,
                &caretY,
                &hitTestMetrics
            );
            m_caretPosition = std::min(m_caretPosition, hitTestMetrics.textPosition + hitTestMetrics.length);
        }
        break;

    case SetSelectionModeUp:
    case SetSelectionModeDown:
    {
        // Retrieve the line metrics to figure out what line we are on.
        std::vector<DWRITE_LINE_METRICS> lineMetrics;
        GetLineMetrics(lineMetrics);

        UINT32 linePosition;
        GetLineFromPosition(
            &lineMetrics.front(),
            static_cast<UINT32>(lineMetrics.size()),
            m_caretPosition,
            &line,
            &linePosition
        );

        // Move up a line or down
        if (moveMode == SetSelectionModeUp)
        {
            if (line <= 0)
                break; // already top line
            line--;
            linePosition -= lineMetrics[line].length;
        }
        else
        {
            linePosition += lineMetrics[line].length;
            line++;
            if (line >= lineMetrics.size())
                break; // already bottom line
        }

        // To move up or down, we need three hit-testing calls to determine:
        // 1. The x of where we currently are.
        // 2. The y of the new line.
        // 3. New text position from the determined x and y.
        // This is because the characters are variable size.

        DWRITE_HIT_TEST_METRICS hitTestMetrics;
        float caretX, caretY, dummyX;
        
        // Get x of current text position
        pTextLayout->HitTestTextPosition(
            m_caretPosition,
            m_caretPositionOffset > 0, // trailing if nonzero, else leading edge
            &caretX,
            &caretY,
            &hitTestMetrics
        );

        // Get y of new position
        pTextLayout->HitTestTextPosition(
            linePosition,
            false, // leading edge
            &dummyX,
            &caretY,
            &hitTestMetrics
        );

        // Now get text position of new x,y.
        BOOL isInside, isTrailingHit;
        pTextLayout->HitTestPoint(
            caretX,
            caretY,
            &isTrailingHit,
            &isInside,
            &hitTestMetrics
        );

        m_caretPosition = hitTestMetrics.textPosition;
        m_caretPositionOffset = isTrailingHit ? (hitTestMetrics.length > 0) : 0;
    }
    break;

    case SetSelectionModeLeftWord:
    case SetSelectionModeRightWord:
    {
        // To navigate by whole words, we look for the canWrapLineAfter
        // flag in the cluster metrics.

        // First need to know how many clusters there are.
        std::vector<DWRITE_CLUSTER_METRICS> clusterMetrics;
        UINT32 clusterCount;
        pTextLayout->GetClusterMetrics(NULL, 0, &clusterCount);

        if (clusterCount == 0)
            break;

        // Now we actually read them.
        clusterMetrics.resize(clusterCount);
        pTextLayout->GetClusterMetrics(&clusterMetrics.front(), clusterCount, &clusterCount);

        m_caretPosition = absolutePosition;

        UINT32 clusterPosition = 0;
        UINT32 oldCaretPosition = m_caretPosition;

        if (moveMode == SetSelectionModeLeftWord)
        {
            // Read through the clusters, keeping track of the farthest valid
            // stopping point just before the old position.
            m_caretPosition = 0;
            m_caretPositionOffset = 0; // leading edge
            for (UINT32 cluster = 0; cluster < clusterCount; ++cluster)
            {
                clusterPosition += clusterMetrics[cluster].length;
                if (clusterMetrics[cluster].canWrapLineAfter)
                {
                    if (clusterPosition >= oldCaretPosition)
                        break;

                    // Update in case we pass this point next loop.
                    m_caretPosition = clusterPosition;
                }
            }
        }
        else // SetSelectionModeRightWord
        {
            // Read through the clusters, looking for the first stopping point
            // after the old position.
            for (UINT32 cluster = 0; cluster < clusterCount; ++cluster)
            {
                UINT32 clusterLength = clusterMetrics[cluster].length;
                m_caretPosition = clusterPosition;
                m_caretPositionOffset = clusterLength; // trailing edge
                if (clusterPosition >= oldCaretPosition && clusterMetrics[cluster].canWrapLineAfter)
                    break; // first stopping point after old position.

                clusterPosition += clusterLength;
            }
        }
    }
    break;

    case SetSelectionModeHome:
    case SetSelectionModeEnd:
    {
        // Retrieve the line metrics to know first and last position
        // on the current line.
        std::vector<DWRITE_LINE_METRICS> lineMetrics;
        GetLineMetrics(lineMetrics);

        GetLineFromPosition(
            &lineMetrics.front(),
            static_cast<UINT32>(lineMetrics.size()),
            m_caretPosition,
            &line,
            &m_caretPosition
        );

        m_caretPositionOffset = 0;
        if (moveMode == SetSelectionModeEnd)
        {
            // Place the caret at the last character on the line,
            // excluding line breaks. In the case of wrapped lines,
            // newlineLength will be 0.
            UINT32 lineLength = lineMetrics[line].length - lineMetrics[line].newlineLength;
            m_caretPositionOffset = std::min(lineLength, 1u);
            m_caretPosition += lineLength - m_caretPositionOffset;
            AlignCaretToNearestCluster(true);
        }
    }
    break;

    case SetSelectionModeFirst:
        m_caretPosition = 0;
        m_caretPositionOffset = 0;
        break;

    case SetSelectionModeAll:
        m_caretAnchor = 0;
        extendSelection = true;
        __fallthrough;

    case SetSelectionModeLast:
        m_caretPosition = UINT32_MAX;
        m_caretPositionOffset = 0;
        AlignCaretToNearestCluster(true);
        break;

    case SetSelectionModeAbsoluteLeading:
        m_caretPosition = advance;
        m_caretPositionOffset = 0;
        break;

    case SetSelectionModeAbsoluteTrailing:
        m_caretPosition = advance;
        AlignCaretToNearestCluster(true);
        break;
    }

    absolutePosition = m_caretPosition + m_caretPositionOffset;

    if (!extendSelection)
        m_caretAnchor = absolutePosition;

    bool caretMoved = (absolutePosition != oldAbsolutePosition) || (m_caretAnchor != oldCaretAnchor);

    if (caretMoved)
    {
        // update the caret formatting attributes
        if (updateCaretFormat)
            UpdateCaretFormatting();

        //PostRedraw();

        RectF rect;
        GetCaretRect(rect);
        UpdateSystemCaret(rect);
    }
    return caretMoved;


}

// Pastes text from clipboard at current caret position.
int XEditBox::PasteFromClipboard()
{
    int r = 0;
    UINT32 characterCount = 0;

    if (!(m_property & XCONTROL_PROP_FOCUS))
        return 0;

    DeleteSelection();

    if (OpenClipboard(NULL))
    {
        HGLOBAL hClipboardData = GetClipboardData(CF_UNICODETEXT);
        if (hClipboardData != NULL)
        {
            // Get text and size of text.
            size_t byteSize = GlobalSize(hClipboardData);
            void* memory = GlobalLock(hClipboardData); // [byteSize] in bytes
            if (memory != NULL)
            {
                const wchar_t* text = reinterpret_cast<const wchar_t*>(memory);
                characterCount = static_cast<UINT32>(wcsnlen(text, byteSize / sizeof(wchar_t)));
                // Insert the text at the current position.
                m_layoutEditor.InsertTextAt((IDWriteTextLayout**)&m_pTextLayout,
                    m_Text,
                    m_caretPosition + m_caretPositionOffset,
                    text,
                    characterCount
                );
                GlobalUnlock(hClipboardData);
                if (byteSize > 0)
                    r++;
            }
        }
        CloseClipboard();
    }
    
    SetSelection(SetSelectionModeRightChar, characterCount, false);
    //SetSelection(SetSelectionModeRightChar, characterCount, true);
    return r;
}

// Copies selected text to clipboard.
void XEditBox::CopyToClipboard()
{
    DWRITE_TEXT_RANGE selectionRange = GetSelectionRange();
    if (selectionRange.length > 0)
    {
        // Open and empty existing contents.
        if (OpenClipboard(nullptr))
        {
            if (EmptyClipboard())
            {
                // Allocate room for the text
                size_t byteSize = sizeof(wchar_t) * (selectionRange.length + 1);
                HGLOBAL hClipboardData = GlobalAlloc(GMEM_DDESHARE | GMEM_ZEROINIT, byteSize);
                if (hClipboardData != NULL)
                {
                    void* memory = GlobalLock(hClipboardData);  // [byteSize] in bytes
                    if (memory != NULL)
                    {
                        // Copy text to memory block.
                        const wchar_t* text = m_Text.c_str();
                        memcpy(memory, &text[selectionRange.startPosition], byteSize);
                        GlobalUnlock(hClipboardData);

                        if (SetClipboardData(CF_UNICODETEXT, hClipboardData) != NULL)
                        {
                            hClipboardData = NULL; // system now owns the clipboard, so don't touch it.
                        }
                    }
                    GlobalFree(hClipboardData); // free if failed
                }
            }
            CloseClipboard();
        }
    }
}

int XEditBox::OnKeyBoard(UINT32 msg, U64 wparam, U64 lparam)
{
    int ret = 0;

    if (m_property & XCONTROL_PROP_FOCUS) // if this control does not have focus, we do nothing
    {
        UINT32 keyCode = static_cast<UINT32>(wparam);

        bool heldShift = (GetKeyState(VK_SHIFT) & 0x80) != 0;
        bool heldControl = (GetKeyState(VK_CONTROL) & 0x80) != 0;

        UINT32 absolutePosition = m_caretPosition + m_caretPositionOffset;

        if (DUI_CHAR == msg)
        {
            if (keyCode >= 0x20 || keyCode == 9)
            {
                // Replace any existing selection.
                DeleteSelection();
                // Convert the UTF32 character code from the Window message to UTF16,
                // yielding 1-2 code-units. Then advance the caret position by how
                // many code-units were inserted.
                UINT32 charsLength = 1;
                wchar_t chars[2] = { static_cast<wchar_t>(keyCode), 0 };

                // If above the basic multi-lingual plane, split into
                // leading and trailing surrogates.
                if (keyCode > 0xFFFF)
                {
                    // From http://unicode.org/faq/utf_bom.html#35
                    chars[0] = wchar_t(0xD800 + (keyCode >> 10) - (0x10000 >> 10));
                    chars[1] = wchar_t(0xDC00 + (keyCode & 0x3FF));
                    charsLength++;
                }
                m_layoutEditor.InsertTextAt((IDWriteTextLayout**)&m_pTextLayout, 
                    m_Text, m_caretPosition + m_caretPositionOffset, chars, charsLength, &m_caretFormat);
                
                SetSelection(SetSelectionModeRight, charsLength, false, false);

                ret = 1;
            }
            return ret;
        }
        else if (DUI_KEYDOWN == msg)
        {
            ret = 1;
            switch (keyCode)
            {
            case VK_RETURN:
                if (heldControl)
                {
                    // Insert CR/LF pair
                    DeleteSelection();
                    m_layoutEditor.InsertTextAt((IDWriteTextLayout**)&m_pTextLayout, m_Text, absolutePosition, L"\n", 2, &m_caretFormat);
                    SetSelection(SetSelectionModeAbsoluteLeading, absolutePosition + 2, false, false);
                }
                else
                    ret = 0;
                break;
            case VK_BACK:
                // Erase back one character (less than a character though).
                // Since layout's hit-testing always returns a whole cluster,
                // we do the surrogate pair detection here directly. Otherwise
                // there would be no way to delete just the diacritic following
                // a base character.
                if (absolutePosition != m_caretAnchor)
                {
                    // delete the selected text
                    DeleteSelection();
                }
                else if (absolutePosition > 0)
                {
                    UINT32 count = 1;
                    // Need special case for surrogate pairs and CR/LF pair.
                    if (absolutePosition >= 2
                        && absolutePosition <= m_Text.size())
                    {
                        wchar_t charBackOne = m_Text[absolutePosition - 1];
                        wchar_t charBackTwo = m_Text[absolutePosition - 2];
                        if ((IsLowSurrogate(charBackOne) && IsHighSurrogate(charBackTwo))
                            || (charBackOne == '\n' && charBackTwo == '\r'))
                        {
                            count = 2;
                        }
                    }
                    SetSelection(SetSelectionModeLeftChar, count, false);
                    m_layoutEditor.RemoveTextAt((IDWriteTextLayout**)&m_pTextLayout, m_Text, m_caretPosition, count);
                }
                break;

            case VK_DELETE:
                // Delete following cluster.
                if (absolutePosition != m_caretAnchor)
                {
                    // Delete all the selected text.
                    DeleteSelection();
                }
                else
                {
                    DWRITE_HIT_TEST_METRICS hitTestMetrics;
                    float caretX, caretY;

                    IDWriteTextLayout* pTextLayout = static_cast<IDWriteTextLayout*>(m_pTextLayout);
                    // Get the size of the following cluster.
                    pTextLayout->HitTestTextPosition(
                        absolutePosition,
                        false,
                        &caretX,
                        &caretY,
                        &hitTestMetrics
                    );
                    m_layoutEditor.RemoveTextAt((IDWriteTextLayout**)&m_pTextLayout, m_Text, hitTestMetrics.textPosition, hitTestMetrics.length);
                    SetSelection(SetSelectionModeAbsoluteLeading, hitTestMetrics.textPosition, false);
                }
                break;
            case VK_TAB:
                break;
            case VK_LEFT:
                SetSelection(heldControl ? SetSelectionModeLeftWord : SetSelectionModeLeft, 1, heldShift);
                break;
            case VK_RIGHT:
                SetSelection(heldControl ? SetSelectionModeRightWord : SetSelectionModeRight, 1, heldShift);
                break;
            case VK_UP:
                SetSelection(SetSelectionModeUp, 1, heldShift);
                break;
            case VK_DOWN:
                SetSelection(SetSelectionModeDown, 1, heldShift);
                break;
            case VK_HOME: // beginning of line
                SetSelection(heldControl ? SetSelectionModeFirst : SetSelectionModeHome, 0, heldShift);
                break;
            case VK_END: // end of line
                SetSelection(heldControl ? SetSelectionModeLast : SetSelectionModeEnd, 0, heldShift);
                break;
            case 'C':
                if (heldControl)
                {
                    CopyToClipboard();
                }
                break;
            case VK_INSERT:
                break;
            case 'V':
                if (heldControl)
                {
                    PasteFromClipboard();
                }
                break;
            case 'X':
                if (heldControl)
                {
                    CopyToClipboard();
                    DeleteSelection();
                }
                break;
            case 'A':
                if (heldControl)
                {
                    SetSelection(SetSelectionModeAll, 0, true);
                }
                break;
            default:
                ret = 0;
                break;
            }
        }
    }
    return ret;
}

// do the real draw on the screen
int XEditBox::DrawText(int dx, int dy, DUI_Surface surface, DUI_Brush brush,DUI_Brush brushSel, DUI_Brush brushCaret, U64 flag)
{
    float W, H;
    float X = static_cast<FLOAT>(dx + m_left);
    float Y = static_cast<FLOAT>(dy + m_top);

    ID2D1HwndRenderTarget* pD2DRenderTarget = static_cast<ID2D1HwndRenderTarget*>(surface);
    ID2D1SolidColorBrush* pTextBrush        = static_cast<ID2D1SolidColorBrush*>(brush);
    ID2D1SolidColorBrush* pTextBrushSel     = static_cast<ID2D1SolidColorBrush*>(brushSel);
    ID2D1SolidColorBrush* pTextBrushCaret   = static_cast<ID2D1SolidColorBrush*>(brushCaret);
    IDWriteTextLayout* pTextLayout          = static_cast<IDWriteTextLayout*>(m_pTextLayout);

    if (pTextLayout && pD2DRenderTarget && pTextBrush && pTextBrushSel && brushCaret)
    {
        D2D1_POINT_2F orgin;
        D2D1_RECT_F clipRect;
        DWRITE_TEXT_RANGE caretRange;

        clipRect.left = static_cast<FLOAT>(dx + m_left2);
        clipRect.right = clipRect.left + static_cast<FLOAT>(m_right2 - m_left2);
        clipRect.top = static_cast<FLOAT>(dy + m_top);;
        clipRect.bottom = clipRect.top + static_cast<FLOAT>(m_bottom2 - m_top2);

        DWRITE_TEXT_METRICS tm;
        pTextLayout->GetMetrics(&tm);
        m_Height = static_cast<int>(tm.height) + 1;

        pD2DRenderTarget->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        //pD2DRenderTarget->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_ALIASED);

        if (m_caret && (m_property & XCONTROL_PROP_FOCUS)) // if the caret is visible and this editbox has the focus, we draw the caret
        {
            RectF caretRect;
            GetCaretRect(caretRect);
            W = caretRect.right - caretRect.left;
            H = caretRect.bottom - caretRect.top;

            caretRect.left = X + caretRect.left;
            caretRect.top = Y + caretRect.top;
            caretRect.right = caretRect.left + W;
            caretRect.bottom = caretRect.top + H;
            if (caretRect.top < static_cast<FLOAT>(dy + m_top))
            {
                m_Offset = static_cast<int>(caretRect.top) - (dy + m_top) - 1;
            }

            if (caretRect.bottom > static_cast<FLOAT>(dy + m_bottom))
            {
                m_Offset = static_cast<int>(caretRect.bottom) + 1 - (dy + m_bottom);
            }

            pD2DRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
            pD2DRenderTarget->FillRectangle(caretRect, pTextBrushCaret);
            pD2DRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
        }

        caretRange = GetSelectionRange();
        if (caretRange.length > 0)
        {
            // Determine actual number of hit-test ranges
            UINT32 actualHitTestCount = 0;
            pTextLayout->HitTestTextRange(caretRange.startPosition, caretRange.length, 0, 0, NULL, 0, &actualHitTestCount);

            std::vector<DWRITE_HIT_TEST_METRICS> hitTestMetrics(actualHitTestCount);

            if (caretRange.length > 0)
            {
                pTextLayout->HitTestTextRange(
                    caretRange.startPosition,
                    caretRange.length,
                    0, // x
                    0, // y
                    &hitTestMetrics[0],
                    static_cast<UINT32>(hitTestMetrics.size()),
                    &actualHitTestCount
                );
            }
            // Draw the selection ranges behind the text.
            if (actualHitTestCount > 0)
            {
                pD2DRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_ALIASED);
                for (size_t i = 0; i < actualHitTestCount; ++i)
                {
                    const DWRITE_HIT_TEST_METRICS& htm = hitTestMetrics[i];
                    RectF highlightRect = { htm.left + X, htm.top + Y, (htm.left + X + htm.width), (htm.top + Y + htm.height) };
                    pD2DRenderTarget->FillRectangle(highlightRect, pTextBrushSel);
                }
                pD2DRenderTarget->SetAntialiasMode(D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
            }
        }

        orgin.x = X;
        orgin.y = Y;
        pD2DRenderTarget->DrawTextLayout(orgin, pTextLayout, pTextBrush);

        pD2DRenderTarget->PopAxisAlignedClip();
    }
    return 0;
}

// Copies a single range of similar properties, from one old layout to a new one.
void EditableLayout::CopySinglePropertyRange(IDWriteTextLayout* oldLayout, UINT32 startPosForOld, IDWriteTextLayout* newLayout, UINT32 startPosForNew,
    UINT32 length, EditableLayout::CaretFormat* caretFormat)
{
    DWRITE_TEXT_RANGE range = { startPosForNew, std::min(length, UINT32_MAX - startPosForNew) };

    // font collection
    IDWriteFontCollection* fontCollection = NULL;
    oldLayout->GetFontCollection(startPosForOld, &fontCollection);
    newLayout->SetFontCollection(fontCollection, range);
    SafeRelease(&fontCollection);

    if (caretFormat != NULL)
    {
        newLayout->SetFontFamilyName(caretFormat->fontFamilyName, range);
        newLayout->SetLocaleName(caretFormat->localeName, range);
        newLayout->SetFontWeight(caretFormat->fontWeight, range);
        newLayout->SetFontStyle(caretFormat->fontStyle, range);
        newLayout->SetFontStretch(caretFormat->fontStretch, range);
        newLayout->SetFontSize(caretFormat->fontSize, range);
        newLayout->SetUnderline(caretFormat->hasUnderline, range);
        newLayout->SetStrikethrough(caretFormat->hasStrikethrough, range);
    }
    else
    {
        // font family
        wchar_t fontFamilyName[100];
        fontFamilyName[0] = '\0';
        oldLayout->GetFontFamilyName(startPosForOld, &fontFamilyName[0], ARRAYSIZE(fontFamilyName));
        newLayout->SetFontFamilyName(fontFamilyName, range);

        // weight/width/slope
        DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;
        DWRITE_FONT_STYLE style = DWRITE_FONT_STYLE_NORMAL;
        DWRITE_FONT_STRETCH stretch = DWRITE_FONT_STRETCH_NORMAL;
        oldLayout->GetFontWeight(startPosForOld, &weight);
        oldLayout->GetFontStyle(startPosForOld, &style);
        oldLayout->GetFontStretch(startPosForOld, &stretch);

        newLayout->SetFontWeight(weight, range);
        newLayout->SetFontStyle(style, range);
        newLayout->SetFontStretch(stretch, range);

        // font size
        FLOAT fontSize = 12.0;
        oldLayout->GetFontSize(startPosForOld, &fontSize);
        newLayout->SetFontSize(fontSize, range);

        // underline and strikethrough
        BOOL value = FALSE;
        oldLayout->GetUnderline(startPosForOld, &value);
        newLayout->SetUnderline(value, range);
        oldLayout->GetStrikethrough(startPosForOld, &value);
        newLayout->SetStrikethrough(value, range);

        // locale
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
        localeName[0] = '\0';
        oldLayout->GetLocaleName(startPosForOld, &localeName[0], ARRAYSIZE(localeName));
        newLayout->SetLocaleName(localeName, range);
    }

    // drawing effect
    IUnknown* drawingEffect = NULL;
    oldLayout->GetDrawingEffect(startPosForOld, &drawingEffect);
    newLayout->SetDrawingEffect(drawingEffect, range);
    SafeRelease(&drawingEffect);

    // inline object
    IDWriteInlineObject* inlineObject = NULL;
    oldLayout->GetInlineObject(startPosForOld, &inlineObject);
    newLayout->SetInlineObject(inlineObject, range);
    SafeRelease(&inlineObject);

    // typography
    IDWriteTypography* typography = NULL;
    oldLayout->GetTypography(startPosForOld, &typography);
    newLayout->SetTypography(typography, range);
    SafeRelease(&typography);
}

// Determines the length of a block of similarly formatted properties.
UINT32 CalculateRangeLengthAt(IDWriteTextLayout* layout,  UINT32 pos)
{
    // Use the first getter to get the range to increment the current position.
    DWRITE_TEXT_RANGE incrementAmount = { pos, 1 };
    DWRITE_FONT_WEIGHT weight = DWRITE_FONT_WEIGHT_NORMAL;

    layout->GetFontWeight(pos, &weight, &incrementAmount);

    UINT32 rangeLength = incrementAmount.length - (pos - incrementAmount.startPosition);
    return rangeLength;
}

// Copies properties that set on ranges.
void EditableLayout::CopyRangedProperties(IDWriteTextLayout* oldLayout, UINT32 startPos,  UINT32 endPos, // an STL-like one-past position.
    UINT32 newLayoutTextOffset, IDWriteTextLayout* newLayout, bool isOffsetNegative)
{
    UINT32 currentPos = startPos;
    UINT32 rangeLength;
    while (currentPos < endPos)
    {
        rangeLength = CalculateRangeLengthAt(oldLayout, currentPos);
        rangeLength = std::min(rangeLength, endPos - currentPos);
        if (isOffsetNegative)
        {
            CopySinglePropertyRange(oldLayout, currentPos, newLayout, currentPos - newLayoutTextOffset, rangeLength);
        }
        else
        {
            CopySinglePropertyRange(oldLayout, currentPos, newLayout, currentPos + newLayoutTextOffset, rangeLength);
        }
        currentPos += rangeLength;
    }
}

// Inserts text and shifts all formatting.
STDMETHODIMP EditableLayout::InsertTextAt(IN OUT IDWriteTextLayout** currentLayout,
    IN OUT std::wstring& text,   UINT32 position,
    WCHAR const* textToInsert,                  // [lengthToInsert]
    UINT32 textToInsertLength,
    CaretFormat* caretFormat)
{
    HRESULT hr = S_OK;

    // The inserted string gets all the properties of the character right before position.
    // If there is no text right before position, so use the properties of the character right after position.

    // Copy all the old formatting.
    IDWriteTextLayout* oldLayout = SafeAcquire(*currentLayout);
    UINT32 oldTextLength = static_cast<UINT32>(text.length());
    position = std::min(position, static_cast<UINT32>(text.size()));

    try
    {
        // Insert the new text and recreate the new text layout. 
        text.insert(position, textToInsert, textToInsertLength);
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    if (SUCCEEDED(hr))
    {
        hr = RecreateLayout(currentLayout, text);
    }

    IDWriteTextLayout* newLayout = *currentLayout;

    if (SUCCEEDED(hr))
    {
        CopyGlobalProperties(oldLayout, newLayout);

        // For each property, get the position range and apply it to the old text.
        if (position == 0)
        {
            // Inserted text
            CopySinglePropertyRange(oldLayout, 0, newLayout, 0, textToInsertLength);

            // The rest of the text
            CopyRangedProperties(oldLayout, 0, oldTextLength, textToInsertLength, newLayout);
        }
        else
        {
            // 1st block
            CopyRangedProperties(oldLayout, 0, position, 0, newLayout);

            // Inserted text
            CopySinglePropertyRange(oldLayout, position - 1, newLayout, position, textToInsertLength, caretFormat);

            // Last block (if it exists)
            CopyRangedProperties(oldLayout, position, oldTextLength, textToInsertLength, newLayout);
        }
        // Copy trailing end.
        CopySinglePropertyRange(oldLayout, oldTextLength, newLayout, static_cast<UINT32>(text.length()), UINT32_MAX);
    }
    SafeRelease(&oldLayout);

    return S_OK;
}

// Removes text and shifts all formatting.
STDMETHODIMP EditableLayout::RemoveTextAt(IN OUT IDWriteTextLayout** currentLayout, IN OUT std::wstring& text, UINT32 position, UINT32 lengthToRemove)
{
    HRESULT hr = S_OK;
    // copy all the old formatting.
    IDWriteTextLayout* oldLayout = SafeAcquire(*currentLayout);
    UINT32 oldTextLength = static_cast<UINT32>(text.length());

    try
    {
        // Remove the old text and recreate the new text layout.
        text.erase(position, lengthToRemove);
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    if (SUCCEEDED(hr))
    {
        RecreateLayout(currentLayout, text);
    }

    IDWriteTextLayout* newLayout = *currentLayout;

    if (SUCCEEDED(hr))
    {
        CopyGlobalProperties(oldLayout, newLayout);

        if (position == 0)
        {
            // The rest of the text
            CopyRangedProperties(oldLayout, lengthToRemove, oldTextLength, lengthToRemove, newLayout, true);
        }
        else
        {
            // 1st block
            CopyRangedProperties(oldLayout, 0, position, 0, newLayout, true);

            // Last block (if it exists, we increment past the deleted text)
            CopyRangedProperties(oldLayout, position + lengthToRemove, oldTextLength, lengthToRemove, newLayout, true);
        }
        CopySinglePropertyRange(oldLayout, oldTextLength, newLayout, static_cast<UINT32>(text.length()), UINT32_MAX);
    }

    SafeRelease(&oldLayout);

    return S_OK;
}

STDMETHODIMP EditableLayout::Clear(IN OUT IDWriteTextLayout** currentLayout, IN OUT std::wstring& text)
{
    HRESULT hr = S_OK;
    try
    {
        text.clear();
    }
    catch (...)
    {
        hr = ExceptionToHResult();
    }

    if (SUCCEEDED(hr))
    {
        hr = RecreateLayout(currentLayout, text);
    }
    return hr;
}

// Copies global properties that are not range based.
void EditableLayout::CopyGlobalProperties(IDWriteTextLayout* oldLayout, IDWriteTextLayout* newLayout)
{
    newLayout->SetTextAlignment(oldLayout->GetTextAlignment());
    newLayout->SetParagraphAlignment(oldLayout->GetParagraphAlignment());
    newLayout->SetWordWrapping(oldLayout->GetWordWrapping());
    newLayout->SetReadingDirection(oldLayout->GetReadingDirection());
    newLayout->SetFlowDirection(oldLayout->GetFlowDirection());
    newLayout->SetIncrementalTabStop(oldLayout->GetIncrementalTabStop());

    DWRITE_TRIMMING trimmingOptions = {};
    IDWriteInlineObject* inlineObject = NULL;
    oldLayout->GetTrimming(&trimmingOptions, &inlineObject);
    newLayout->SetTrimming(&trimmingOptions, inlineObject);
    SafeRelease(&inlineObject);

    DWRITE_LINE_SPACING_METHOD lineSpacingMethod = DWRITE_LINE_SPACING_METHOD_DEFAULT;
    FLOAT lineSpacing = 0;
    FLOAT baseline = 0;
    oldLayout->GetLineSpacing(&lineSpacingMethod, &lineSpacing, &baseline);
    newLayout->SetLineSpacing(lineSpacingMethod, lineSpacing, baseline);
}

inline bool operator== (const RenderTargetD2D::ImageCacheEntry& entry, const IWICBitmapSource* original)
{
    return entry.original == original;
}

HRESULT RenderTargetD2D::Create(ID2D1Factory* d2dFactory, IDWriteFactory* dwriteFactory, HWND hwnd, OUT RenderTarget** renderTarget)
{
    *renderTarget = NULL;
    HRESULT hr = S_OK;

    RenderTargetD2D* newRenderTarget = SafeAcquire(new(std::nothrow) RenderTargetD2D(d2dFactory, dwriteFactory, hwnd));
    if (newRenderTarget == NULL)
    {
        return E_OUTOFMEMORY;
    }

    hr = newRenderTarget->CreateTarget();
    if (FAILED(hr))
        SafeRelease(&newRenderTarget);

    *renderTarget = SafeDetach(&newRenderTarget);

    return hr;
}

RenderTargetD2D::RenderTargetD2D(ID2D1Factory* d2dFactory, IDWriteFactory* dwriteFactory, HWND hwnd)
    : hwnd_(hwnd),
    hmonitor_(NULL),
    d2dFactory_(SafeAcquire(d2dFactory)),
    dwriteFactory_(SafeAcquire(dwriteFactory)),
    target_(),
    brush_()
{
}

RenderTargetD2D::~RenderTargetD2D()
{
    SafeRelease(&brush_);
    SafeRelease(&target_);
    SafeRelease(&d2dFactory_);
    SafeRelease(&dwriteFactory_);
}

HRESULT RenderTargetD2D::CreateTarget()
{
    // Creates a D2D render target set on the HWND.

    HRESULT hr = S_OK;

    // Get the window's pixel size.
    RECT rect = {};
    GetClientRect(hwnd_, &rect);
    D2D1_SIZE_U d2dSize = D2D1::SizeU(rect.right, rect.bottom);

    // Create a D2D render target.
    ID2D1HwndRenderTarget* target = NULL;

    hr = d2dFactory_->CreateHwndRenderTarget(
        D2D1::RenderTargetProperties(),
        D2D1::HwndRenderTargetProperties(hwnd_, d2dSize),
        &target
    );

    if (SUCCEEDED(hr))
    {
        SafeSet(&target_, target);

        // Any scaling will be combined into matrix transforms rather than an
        // additional DPI scaling. This simplifies the logic for rendering
        // and hit-testing. If an application does not use matrices, then
        // using the scaling factor directly is simpler.
        target->SetDpi(96.0, 96.0);

        // Create a reusable scratch brush, rather than allocating one for
        // each new color.
        SafeRelease(&brush_);
        hr = target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &brush_);
    }

    if (SUCCEEDED(hr))
    {
        // Update the initial monitor rendering parameters.
        UpdateMonitor();
    }

    SafeRelease(&target);

    return hr;
}


void RenderTargetD2D::Resize(UINT width, UINT height)
{
    D2D1_SIZE_U size;
    size.width = width;
    size.height = height;
    target_->Resize(size);
}

// Updates rendering parameters according to current monitor.
void RenderTargetD2D::UpdateMonitor()
{
    HMONITOR monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONEAREST);
    if (monitor != hmonitor_)
    {
        // Create based on monitor settings, rather than the defaults of
        // gamma=1.8, contrast=.5, and clearTypeLevel=.5

        IDWriteRenderingParams* renderingParams = NULL;

        dwriteFactory_->CreateMonitorRenderingParams(
            monitor,
            &renderingParams
        );
        target_->SetTextRenderingParams(renderingParams);

        hmonitor_ = monitor;
        InvalidateRect(hwnd_, NULL, FALSE);

        SafeRelease(&renderingParams);
    }
}


void RenderTargetD2D::BeginDraw()
{
    target_->BeginDraw();
    target_->SetTransform(D2D1::Matrix3x2F::Identity());
}


void RenderTargetD2D::EndDraw()
{
    HRESULT hr = target_->EndDraw();

    // If the device is lost for any reason, we need to recreate it.
    if (hr == D2DERR_RECREATE_TARGET)
    {
        // Flush resources and recreate them.
        // This is very rare for a device to be lost,
        // but it can occur when connecting via Remote Desktop.
        imageCache_.clear();
        hmonitor_ = NULL;

        CreateTarget();
    }
}


void RenderTargetD2D::Clear(UINT32 color)
{
    target_->Clear(D2D1::ColorF(color));
}


ID2D1Bitmap* RenderTargetD2D::GetCachedImage(IWICBitmapSource* image)
{
    // Maps a WIC image source to an aready cached D2D bitmap.
    // If not already cached, it creates the D2D bitmap from WIC.

    if (image == NULL)
        return NULL;

    // Find an existing match
    std::vector<ImageCacheEntry>::iterator match = std::find(imageCache_.begin(), imageCache_.end(), image);
    if (match != imageCache_.end())
        return match->converted; // already cached

    // Convert the WIC image to a ready-to-use device-dependent D2D bitmap.
    // This avoids needing to recreate a new texture every draw call, but
    // allows easy reconstruction of textures if the device changes and
    // resources need recreation (also lets callers be D2D agnostic).

    ID2D1Bitmap* bitmap = NULL;
    target_->CreateBitmapFromWicBitmap(image, NULL, &bitmap);
    if (bitmap == NULL)
        return NULL;

    // Save for later calls.
    try
    {
        imageCache_.push_back(ImageCacheEntry(image, bitmap));
    }
    catch (...)
    {
        // Out of memory
        SafeRelease(&bitmap);
        return NULL;
    }

    // Release it locally and return the pointer.
    // The bitmap is now referenced by the bitmap cache.
    bitmap->Release();
    return bitmap;
}


void RenderTargetD2D::FillRectangle(
    const RectF& destRect,
    const DrawingEffect& drawingEffect
)
{
    ID2D1Brush* brush = GetCachedBrush(&drawingEffect);
    if (brush == NULL)
        return;

    // We will always get a strikethrough as a LTR rectangle
    // with the baseline origin snapped.
    target_->FillRectangle(destRect, brush);
}


void RenderTargetD2D::DrawImage(
    IWICBitmapSource* image,
    const RectF& sourceRect,  // where in source atlas texture
    const RectF& destRect     // where on display to draw it
)
{
    // Ignore zero size source rects.
    // Draw nothing if the destination is zero size.
    if (&sourceRect == NULL
        || sourceRect.left >= sourceRect.right
        || sourceRect.top >= sourceRect.bottom
        || destRect.left >= destRect.right
        || destRect.top >= destRect.bottom)
    {
        return;
    }

    ID2D1Bitmap* bitmap = GetCachedImage(image);
    if (bitmap == NULL)
        return;

    target_->DrawBitmap(
        bitmap,
        destRect,
        1.0, // opacity
        D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
        sourceRect
    );
}

void RenderTargetD2D::DrawTextLayout(IDWriteTextLayout* textLayout, const RectF& rect)
{
    if (textLayout)
    {
        Context context(this, NULL);
        textLayout->Draw(
            &context,
            this,
            rect.left,
            rect.top
        );
    }
}

ID2D1Brush* RenderTargetD2D::GetCachedBrush(const DrawingEffect* effect)
{
    if (effect && brush_)
    {
        UINT32 bgra = effect->GetColor();
        float alpha = (bgra >> 24) / 255.0f;
        brush_->SetColor(D2D1::ColorF(bgra, alpha));
        return brush_;
    }

    return nullptr;
}

void RenderTargetD2D::SetTransform(DWRITE_MATRIX const& transform)
{
    target_->SetTransform(reinterpret_cast<const D2D1_MATRIX_3X2_F*>(&transform));
}


void RenderTargetD2D::GetTransform(DWRITE_MATRIX& transform)
{
    target_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(&transform));
}


void RenderTargetD2D::SetAntialiasing(bool isEnabled)
{
    target_->SetAntialiasMode(isEnabled ? D2D1_ANTIALIAS_MODE_PER_PRIMITIVE : D2D1_ANTIALIAS_MODE_ALIASED);
}

HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawGlyphRun(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    const DWRITE_GLYPH_RUN* glyphRun,
    const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
    IUnknown* clientDrawingEffect
)
{
    // If no drawing effect is applied to run, but a clientDrawingContext
    // is passed, use the one from that instead. This is useful for trimming
    // signs, where they don't have a color of their own.
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    // Since we use our own custom renderer and explicitly set the effect
    // on the layout, we know exactly what the parameter is and can
    // safely cast it directly.
    DrawingEffect* effect = reinterpret_cast<DrawingEffect*>(clientDrawingEffect);
    ID2D1Brush* brush = GetCachedBrush(effect);
    if (brush == NULL)
        return E_FAIL;

    target_->DrawGlyphRun(
        D2D1::Point2(baselineOriginX, baselineOriginY),
        glyphRun,
        brush,
        measuringMode
    );

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawUnderline(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_UNDERLINE* underline,
    IUnknown* clientDrawingEffect
)
{
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    DrawingEffect* effect = reinterpret_cast<DrawingEffect*>(clientDrawingEffect);
    ID2D1Brush* brush = GetCachedBrush(effect);
    if (brush == NULL)
        return E_FAIL;

    // We will always get a strikethrough as a LTR rectangle
    // with the baseline origin snapped.
    D2D1_RECT_F rectangle =
    {
        baselineOriginX,
        baselineOriginY + underline->offset,
        baselineOriginX + underline->width,
        baselineOriginY + underline->offset + underline->thickness
    };

    // Draw this as a rectangle, rather than a line.
    target_->FillRectangle(&rectangle, brush);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawStrikethrough(
    void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    const DWRITE_STRIKETHROUGH* strikethrough,
    IUnknown* clientDrawingEffect
)
{
    clientDrawingEffect = GetDrawingEffect(clientDrawingContext, clientDrawingEffect);

    DrawingEffect* effect = reinterpret_cast<DrawingEffect*>(clientDrawingEffect);
    ID2D1Brush* brush = GetCachedBrush(effect);
    if (brush == NULL)
        return E_FAIL;

    // We will always get an underline as a LTR rectangle
    // with the baseline origin snapped.
    D2D1_RECT_F rectangle =
    {
        baselineOriginX,
        baselineOriginY + strikethrough->offset,
        baselineOriginX + strikethrough->width,
        baselineOriginY + strikethrough->offset + strikethrough->thickness
    };

    // Draw this as a rectangle, rather than a line.
    target_->FillRectangle(&rectangle, brush);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::DrawInlineObject(
    void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
)
{
    // Inline objects inherit the drawing effect of the text
    // they are in, so we should pass it down (if none is set
    // on this range, use the drawing context's effect instead).
    Context subContext(*reinterpret_cast<RenderTarget::Context*>(clientDrawingContext));

    if (clientDrawingEffect != NULL)
        subContext.drawingEffect = clientDrawingEffect;

    inlineObject->Draw(
        &subContext,
        this,
        originX,
        originY,
        false,
        false,
        subContext.drawingEffect
    );

    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::IsPixelSnappingDisabled(
    void* clientDrawingContext,
    OUT BOOL* isDisabled
)
{
    // Enable pixel snapping of the text baselines,
    // since we're not animating and don't want blurry text.
    *isDisabled = FALSE;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::GetCurrentTransform(
    void* clientDrawingContext,
    OUT DWRITE_MATRIX* transform
)
{
    // Simply forward what the real renderer holds onto.
    target_->GetTransform(reinterpret_cast<D2D1_MATRIX_3X2_F*>(transform));
    return S_OK;
}


HRESULT STDMETHODCALLTYPE RenderTargetD2D::GetPixelsPerDip(
    void* clientDrawingContext,
    OUT FLOAT* pixelsPerDip
)
{
    // Any scaling will be combined into matrix transforms rather than an
    // additional DPI scaling. This simplifies the logic for rendering
    // and hit-testing. If an application does not use matrices, then
    // using the scaling factor directly is simpler.
    *pixelsPerDip = 1;
    return S_OK;
}

InlineImage::InlineImage(
    IWICBitmapSource* image,
    unsigned int index
)
    : image_(SafeAcquire(image))
{
    // Pass the index of the image in the sequence of concatenated sequence
    // (just like toolbar images).
    UINT imageWidth = 0, imageHeight = 0;

    if (image != NULL)
        image->GetSize(&imageWidth, &imageHeight);

    if (index == ~0)
    {
        // No index. Use entire image.
        rect_.left = 0;
        rect_.top = 0;
        rect_.right = float(imageWidth);
        rect_.bottom = float(imageHeight);
    }
    else
    {
        // Use index.
        float size = float(imageHeight);
        float offset = index * size;
        rect_.left = offset;
        rect_.top = 0;
        rect_.right = offset + size;
        rect_.bottom = size;
    }

    baseline_ = float(imageHeight);
}


HRESULT STDMETHODCALLTYPE InlineImage::Draw(
    void* clientDrawingContext,
    IDWriteTextRenderer* renderer,
    FLOAT originX,
    FLOAT originY,
    BOOL isSideways,
    BOOL isRightToLeft,
    IUnknown* clientDrawingEffect
)
{
    // Go from the text renderer interface back to the actual render target.
    RenderTarget* renderTarget = NULL;
    HRESULT hr = renderer->QueryInterface(__uuidof(RenderTarget), (IID_PPV_ARGS(&renderTarget)));
    if (FAILED(hr))
        return hr;

    float height = rect_.bottom - rect_.top;
    float width = rect_.right - rect_.left;
    RectF destRect = { originX, originY, originX + width, originY + height };

    renderTarget->DrawImage(image_, rect_, destRect);

    SafeRelease(&renderTarget);

    return S_OK;
}


HRESULT STDMETHODCALLTYPE InlineImage::GetMetrics(
    OUT DWRITE_INLINE_OBJECT_METRICS* metrics
)
{
    DWRITE_INLINE_OBJECT_METRICS inlineMetrics = {};
    inlineMetrics.width = rect_.right - rect_.left;
    inlineMetrics.height = rect_.bottom - rect_.top;
    inlineMetrics.baseline = baseline_;
    *metrics = inlineMetrics;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE InlineImage::GetOverhangMetrics(
    OUT DWRITE_OVERHANG_METRICS* overhangs
)
{
    overhangs->left = 0;
    overhangs->top = 0;
    overhangs->right = 0;
    overhangs->bottom = 0;
    return S_OK;
}


HRESULT STDMETHODCALLTYPE InlineImage::GetBreakConditions(
    OUT DWRITE_BREAK_CONDITION* breakConditionBefore,
    OUT DWRITE_BREAK_CONDITION* breakConditionAfter
)
{
    *breakConditionBefore = DWRITE_BREAK_CONDITION_NEUTRAL;
    *breakConditionAfter = DWRITE_BREAK_CONDITION_NEUTRAL;
    return S_OK;
}


namespace
{
    HRESULT LoadAndLockResource(
        const wchar_t* resourceName,
        const wchar_t* resourceType,
        OUT UINT8** fileData,
        OUT DWORD* fileSize
    )
    {
        HRSRC resourceHandle = NULL;
        HGLOBAL resourceDataHandle = NULL;
        *fileData = NULL;
        *fileSize = 0;

        // Locate the resource handle in our DLL.
        resourceHandle = FindResourceW(
            HINST_THISCOMPONENT,
            resourceName,
            resourceType
        );
        if (resourceHandle == NULL)
            return E_FAIL;

        // Load the resource.
        resourceDataHandle = LoadResource(
            HINST_THISCOMPONENT,
            resourceHandle);
        if (resourceDataHandle == NULL)
            return E_FAIL;

        // Lock it to get a system memory pointer.
        *fileData = (BYTE*)LockResource(resourceDataHandle);
        if (*fileData == NULL)
            return E_FAIL;

        // Calculate the size.
        *fileSize = SizeofResource(HINST_THISCOMPONENT, resourceHandle);
        if (*fileSize == 0)
            return E_FAIL;

        return S_OK;
    }
}

HRESULT InlineImage::LoadImageFromResource(const wchar_t* resourceName, const wchar_t* resourceType, IWICImagingFactory* wicFactory, OUT IWICBitmapSource** bitmap)
{
    // Loads an image from a resource into the given bitmap.
    HRESULT hr = S_OK;

    DWORD fileSize;
    UINT8* fileData;    // [fileSize]

    IWICStream* stream = NULL;
    IWICBitmapDecoder* decoder = NULL;
    IWICBitmapFrameDecode* source = NULL;
    IWICFormatConverter* converter = NULL;

    hr = LoadAndLockResource(resourceName, resourceType, &fileData, &fileSize);

    // Create a WIC stream to map onto the memory.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateStream(&stream);
    }

    // Initialize the stream with the memory pointer and size.
    if (SUCCEEDED(hr))
    {
        hr = stream->InitializeFromMemory(reinterpret_cast<BYTE*>(fileData), fileSize);
    }

    // Create a decoder for the stream.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateDecoderFromStream(
            stream,
            NULL,
            WICDecodeMetadataCacheOnLoad,
            &decoder
        );
    }

    // Create the initial frame.
    if (SUCCEEDED(hr))
    {
        hr = decoder->GetFrame(0, &source);
    }

    // Convert format to 32bppPBGRA - which D2D expects.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateFormatConverter(&converter);
    }

    if (SUCCEEDED(hr))
    {
        hr = converter->Initialize(
            source,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.f,
            WICBitmapPaletteTypeMedianCut
        );
    }

    if (SUCCEEDED(hr))
    {
        *bitmap = SafeDetach(&converter);
    }

    SafeRelease(&converter);
    SafeRelease(&source);
    SafeRelease(&decoder);
    SafeRelease(&stream);

    return hr;
}


HRESULT InlineImage::LoadImageFromFile(
    const wchar_t* fileName,
    IWICImagingFactory* wicFactory,
    OUT IWICBitmapSource** bitmap
)
{
    // Loads an image from a file into the given bitmap.

    HRESULT hr = S_OK;

    // create a decoder for the stream
    IWICBitmapDecoder* decoder = NULL;
    IWICBitmapFrameDecode* source = NULL;
    IWICFormatConverter* converter = NULL;

    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateDecoderFromFilename(
            fileName,
            NULL,
            GENERIC_READ,
            WICDecodeMetadataCacheOnLoad,
            &decoder
        );
    }

    // Create the initial frame.
    if (SUCCEEDED(hr))
    {
        hr = decoder->GetFrame(0, &source);
    }

    // Convert format to 32bppPBGRA - which D2D expects.
    if (SUCCEEDED(hr))
    {
        hr = wicFactory->CreateFormatConverter(&converter);
    }

    if (SUCCEEDED(hr))
    {
        hr = converter->Initialize(
            source,
            GUID_WICPixelFormat32bppPBGRA,
            WICBitmapDitherTypeNone,
            NULL,
            0.f,
            WICBitmapPaletteTypeMedianCut
        );
    }

    if (SUCCEEDED(hr))
    {
        *bitmap = SafeDetach(&converter);
    }

    SafeRelease(&converter);
    SafeRelease(&source);
    SafeRelease(&decoder);

    return hr;
}
