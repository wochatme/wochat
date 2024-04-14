#include "stdafx.h"
#include "resource.h"
#include "wochatypes.h"
#include "wochatdef.h"
#include "wochat.h"
//#include "til/u8u16convert.h"

namespace Atlas
{
#define ATLAS_POD_OPS(type)                                    \
    constexpr bool operator==(const type& rhs) const noexcept  \
    {                                                          \
        return __builtin_memcmp(this, &rhs, sizeof(rhs)) == 0; \
    }                                                          \
                                                               \
    constexpr bool operator!=(const type& rhs) const noexcept  \
    {                                                          \
        return !(*this == rhs);                                \
    }

    template<typename T>
    struct vec2
    {
        // These members aren't zero-initialized to make these trivial types,
        // and allow the compiler to quickly memset() allocations, etc.
        T x;
        T y;

        ATLAS_POD_OPS(vec2)
    };

    template<typename T>
    struct vec4
    {
        // These members aren't zero-initialized to make these trivial types,
        // and allow the compiler to quickly memset() allocations, etc.
        T x;
        T y;
        T z;
        T w;

        ATLAS_POD_OPS(vec4)
    };

    template<typename T>
    struct rect
    {
        // These members aren't zero-initialized to make these trivial types,
        // and allow the compiler to quickly memset() allocations, etc.
        T left;
        T top;
        T right;
        T bottom;

        ATLAS_POD_OPS(rect)

        constexpr bool empty() const noexcept
        {
            return left >= right || top >= bottom;
        }

        constexpr bool non_empty() const noexcept
        {
            return left < right&& top < bottom;
        }
    };

    template<typename T>
    struct range
    {
        T start;
        T end;

        ATLAS_POD_OPS(range)

        constexpr bool empty() const noexcept
        {
            return start >= end;
        }

        constexpr bool non_empty() const noexcept
        {
            return start < end;
        }

        constexpr bool contains(T v) const noexcept
        {
            return v >= start && v < end;
        }
    };

    using u8 = uint8_t;
    using u8x2 = vec2<u8>;

    using u16 = uint16_t;
    using u16x2 = vec2<u16>;
    using u16r = rect<u16>;

    using i16 = int16_t;
    using i16x2 = vec2<i16>;
    using i16x4 = vec4<i16>;
    using i16r = rect<i16>;

    using u32 = uint32_t;
    using u32x2 = vec2<u32>;
    using u32x4 = vec4<u32>;
    using u32r = rect<u32>;

    using i32 = int32_t;
    using i32x2 = vec2<i32>;
    using i32x4 = vec4<i32>;
    using i32r = rect<i32>;

    using f32 = float;
    using f32x2 = vec2<f32>;
    using f32x4 = vec4<f32>;
    using f32r = rect<f32>;

    template<typename T = D2D1_COLOR_F>
    constexpr T colorFromU32(u32 rgba)
    {
        const auto r = static_cast<f32>((rgba >> 0)  & 0xff) / 255.0f;
        const auto g = static_cast<f32>((rgba >> 8)  & 0xff) / 255.0f;
        const auto b = static_cast<f32>((rgba >> 16) & 0xff) / 255.0f;
        const auto a = static_cast<f32>((rgba >> 24) & 0xff) / 255.0f;
        return { r, g, b, a };
    }

    template<typename T = D2D1_COLOR_F>
    constexpr T colorFromU32Premultiply(u32 rgba)
    {
        const auto r = static_cast<f32>((rgba >> 0) & 0xff) / 255.0f;
        const auto g = static_cast<f32>((rgba >> 8) & 0xff) / 255.0f;
        const auto b = static_cast<f32>((rgba >> 16) & 0xff) / 255.0f;
        const auto a = static_cast<f32>((rgba >> 24) & 0xff) / 255.0f;
        return { r * a, g * a, b * a, a };
    }

    constexpr u32 u32ColorPremultiply(u32 rgba)
    {
        auto rb = rgba & 0x00ff00ff;
        auto g = rgba & 0x0000ff00;
        const auto a = rgba & 0xff000000;

        const auto m = rgba >> 24;
        rb = (rb * m / 0xff) & 0x00ff00ff;
        g = (g * m / 0xff) & 0x0000ff00;

        return rb | g | a;
    }

    // MSVC STL (version 22000) implements std::clamp<T>(T, T, T) in terms of the generic
    // std::clamp<T, Predicate>(T, T, T, Predicate) with std::less{} as the argument,
    // which introduces branching. While not perfect, this is still better than std::clamp.
    template<typename T>
    constexpr T clamp(T val, T min, T max) noexcept
    {
        return val < min ? min : (max < val ? max : val);
    }

    template<typename T>
    constexpr T alignForward(T val, T alignment) noexcept
    {
        assert((alignment & (alignment - 1)) == 0); // alignment should be a power of 2
        return (val + alignment - 1) & ~(alignment - 1);
    }
#if 0
    void ColorGlyphRunDraw(ID2D1DeviceContext4* d2dRenderTarget4, ID2D1SolidColorBrush* emojiBrush,
        ID2D1SolidColorBrush* foregroundBrush, const DWRITE_COLOR_GLYPH_RUN1* colorGlyphRun) noexcept
    {

    }
#endif
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
        wil::com_ptr<ID2D1DeviceContext> _renderTarget;

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
        ID2D1DeviceContext* t = _renderTarget.get();
    }
    bool BackendD2D::RequiresContinuousRedraw() noexcept
    {
        return false;
    }
}