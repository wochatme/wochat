#include "stdafx.h"
#include "resource.h"
#include "wochatypes.h"
#include "wochatdef.h"
#include "wochat.h"
//#include "til/u8u16convert.h"

struct RenderingPayload
{
    //// Parameters which are constant across backends.
    wil::com_ptr<ID2D1Factory> d2dFactory;
#if 0
    wil::com_ptr<IDWriteFactory2> dwriteFactory;
    wil::com_ptr<IDWriteFactory4> dwriteFactory4; // optional, might be nullptr
    wil::com_ptr<IDWriteFontFallback> systemFontFallback;
    wil::com_ptr<IDWriteFontFallback1> systemFontFallback1; // optional, might be nullptr
    wil::com_ptr<IDWriteTextAnalyzer1> textAnalyzer;
    std::function<void(HRESULT)> warningCallback;
    std::function<void(HANDLE)> swapChainChangedCallback;

    //// Parameters which are constant for the existence of the backend.
    struct
    {
        wil::com_ptr<IDXGIFactory2> factory;
        wil::com_ptr<IDXGIAdapter1> adapter;
        LUID adapterLuid{};
        UINT adapterFlags = 0;
    } dxgi;
    struct
    {
        wil::com_ptr<IDXGISwapChain2> swapChain;
        wil::unique_handle handle;
        wil::unique_handle frameLatencyWaitableObject;
        til::generation_t generation;
        til::generation_t targetGeneration;
        til::generation_t fontGeneration;
        u16x2 targetSize{};
        bool waitForPresentation = false;
    } swapChain;
    wil::com_ptr<ID3D11Device2> device;
    wil::com_ptr<ID3D11DeviceContext2> deviceContext;

    //// Parameters which change seldom.
    GenerationalSettings s;
    std::wstring userLocaleName;

    //// Parameters which change every frame.
    // This is the backing buffer for `rows`.
    Buffer<ShapedRow> unorderedRows;
    // This is used as a scratch buffer during scrolling.
    Buffer<ShapedRow*> rowsScratch;
    // This contains the rows in the right order from row 0 to N.
    // They get rotated around when we scroll the buffer. Technically
    // we could also implement scrolling by using a circular array.
    Buffer<ShapedRow*> rows;
    // This contains two viewport-sized bitmaps back to back, sort of like a Texture2DArray.
    // The first NxM (for instance 120x30 pixel) chunk contains background colors and the
    // second chunk contains foreground colors. The distance in u32 items between the start
    // and the begin of the foreground bitmap is equal to colorBitmapDepthStride.
    //
    // The background part is in premultiplied alpha, whereas the foreground part is in straight
    // alpha. This is mostly because of Direct2D being annoying, as the former is the only thing
    // it supports for bitmaps, whereas the latter is the only thing it supports for text.
    // Since we implement Direct2D's text blending algorithm, we're equally dependent on
    // straight alpha for BackendD3D, as straight alpha is used in the pixel shader there.
    Buffer<u32, 32> colorBitmap;
    // This exists as a convenience access to colorBitmap and
    // contains a view into the background color bitmap.
    std::span<u32> backgroundBitmap;
    // This exists as a convenience access to colorBitmap and
    // contains a view into the foreground color bitmap.
    std::span<u32> foregroundBitmap;
    // This stride of the colorBitmap is a "count" of u32 and not in bytes.
    size_t colorBitmapRowStride = 0;
    // FYI depth refers to the `colorBitmapRowStride * height` size of each bitmap contained
    // in colorBitmap. colorBitmap contains 2 bitmaps (background and foreground colors).
    size_t colorBitmapDepthStride = 0;
    // A generation of 1 ensures that the backends redraw the background on the first Present().
    // The 1st entry in this array corresponds to the background and the 2nd to the foreground bitmap.
    std::array<til::generation_t, 2> colorBitmapGenerations{ 1, 1 };
    // In columns/rows.
    til::rect cursorRect;
    // The viewport/SwapChain area to be presented. In pixel.
    // NOTE:
    //   This cannot use til::rect, because til::rect generally expects positive coordinates only
    //   (`operator!()` checks for negative values), whereas this one can go out of bounds,
    //   whenever glyphs go out of bounds. `AtlasEngine::_present()` will clamp it.
    i32r dirtyRectInPx{};
    // In rows.
    range<u16> invalidatedRows{};
    // In pixel.
    i16 scrollOffset = 0;

    void MarkAllAsDirty() noexcept
    {
        dirtyRectInPx = { 0, 0, s->targetSize.x, s->targetSize.y };
        invalidatedRows = { 0, s->viewportCellCount.y };
        scrollOffset = 0;
    }
#endif
};

struct IBackend 
{
    virtual ~IBackend() = default;
    virtual void ReleaseResources() noexcept = 0;
    virtual void Render(RenderingPayload& payload) = 0;
    virtual bool RequiresContinuousRedraw() noexcept = 0;
};

struct BackendD2D : IBackend
{

    void ReleaseResources() noexcept override;
    void Render(RenderingPayload& payload) override;
    bool RequiresContinuousRedraw() noexcept override;
#if 0
private:
    ATLAS_ATTR_COLD void _handleSettingsUpdate(const RenderingPayload& p);
    void _drawBackground(const RenderingPayload& p) noexcept;
    void _drawText(RenderingPayload& p);
    ATLAS_ATTR_COLD f32 _drawTextPrepareLineRendition(const RenderingPayload& p, const ShapedRow* row, f32 baselineY) const noexcept;
    ATLAS_ATTR_COLD void _drawTextResetLineRendition(const ShapedRow* row) const noexcept;
    ATLAS_ATTR_COLD f32r _getGlyphRunDesignBounds(const DWRITE_GLYPH_RUN& glyphRun, f32 baselineX, f32 baselineY);
    ATLAS_ATTR_COLD void _drawGridlineRow(const RenderingPayload& p, const ShapedRow* row, u16 y);
    void _drawCursorPart1(const RenderingPayload& p);
    void _drawCursorPart2(const RenderingPayload& p);
    static void _drawCursor(const RenderingPayload& p, ID2D1RenderTarget* renderTarget, D2D1_RECT_F rect, ID2D1Brush* brush) noexcept;
    void _resizeCursorBitmap(const RenderingPayload& p, til::size newSize);
    void _drawSelection(const RenderingPayload& p);
    void _debugShowDirty(const RenderingPayload& p);
    void _debugDumpRenderTarget(const RenderingPayload& p);
    ID2D1SolidColorBrush* _brushWithColor(u32 color);
    ATLAS_ATTR_COLD ID2D1SolidColorBrush* _brushWithColorUpdate(u32 color);
    void _fillRectangle(const D2D1_RECT_F& rect, u32 color);

    wil::com_ptr<ID2D1DeviceContext> _renderTarget;
    wil::com_ptr<ID2D1DeviceContext4> _renderTarget4; // Optional. Supported since Windows 10 14393.
    wil::com_ptr<ID2D1StrokeStyle> _dottedStrokeStyle;
    wil::com_ptr<ID2D1Bitmap> _backgroundBitmap;
    wil::com_ptr<ID2D1BitmapBrush> _backgroundBrush;
    til::generation_t _backgroundBitmapGeneration;

    wil::com_ptr<ID2D1Bitmap> _cursorBitmap;
    til::size _cursorBitmapSize; // in columns/rows

    wil::com_ptr<ID2D1SolidColorBrush> _emojiBrush;
    wil::com_ptr<ID2D1SolidColorBrush> _brush;
    u32 _brushColor = 0;

    Buffer<DWRITE_GLYPH_METRICS> _glyphMetrics;

    til::generation_t _generation;
    til::generation_t _fontGeneration;
    til::generation_t _cursorGeneration;
    u16x2 _viewportCellCount{};

#if ATLAS_DEBUG_SHOW_DIRTY
    i32r _presentRects[9]{};
    size_t _presentRectsPos = 0;
#endif

#if ATLAS_DEBUG_DUMP_RENDER_TARGET
    wchar_t _dumpRenderTargetBasePath[MAX_PATH]{};
    size_t _dumpRenderTargetCounter = 0;
#endif
#endif 
};

void BackendD2D::ReleaseResources() noexcept
{
#if 0
    _renderTarget.reset();
    _renderTarget4.reset();
    // Ensure _handleSettingsUpdate() is called so that _renderTarget gets recreated.
    _generation = {};
#endif
}

void BackendD2D::Render(RenderingPayload& payload)
{

}
bool BackendD2D::RequiresContinuousRedraw() noexcept
{
    return false;
}
