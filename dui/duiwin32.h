#ifndef _WT_DUIWIN32_H_
#define _WT_DUIWIN32_H_

struct ID2D1HwndRenderTarget;
struct ID2D1Bitmap;
class InlineImage;
class DrawingEffect;

typedef D2D1_RECT_F RectF;

class DrawingEffect
{
protected:
    // The color is stored as BGRA, with blue in the lowest byte,
    // then green, red, alpha; which is what D2D, GDI+, and GDI DIBs use.
    // GDI's COLORREF stores red as the lowest byte.
    U32 m_color;

public:
    DrawingEffect(U32 color)
        : m_color(color)
    { }

    inline U32 GetColor() const throw()
    {
        // Returns the BGRA value for D2D.
        return m_color;
    }

    inline COLORREF GetColorRef() const throw()
    {
        // Returns color as COLORREF.
        return GetColorRef(m_color);
    }

    static inline COLORREF GetColorRef(U32 bgra) throw()
    {
        // Swaps color order (bgra <-> rgba) from D2D/GDI+'s to a COLORREF.
        // This also leaves the top byte 0, since alpha is ignored anyway.
        return RGB(GetBValue(bgra), GetGValue(bgra), GetRValue(bgra));
    }

    static inline COLORREF GetBgra(COLORREF rgb) throw()
    {
        // Swaps color order (bgra <-> rgba) from COLORREF to D2D/GDI+'s.
        // Sets alpha to full opacity.
        return RGB(GetBValue(rgb), GetGValue(rgb), GetRValue(rgb)) | 0xFF000000;
    }
};

#ifndef HINST_THISCOMPONENT
EXTERN_C IMAGE_DOS_HEADER __ImageBase;
#define HINST_THISCOMPONENT ((HINSTANCE)&__ImageBase)
#endif

// Releases a COM object and nullifies pointer.

template <typename InterfaceType>
inline void SafeRelease(InterfaceType** currentObject)
{
    if (*currentObject != NULL)
    {
        (*currentObject)->Release();
        *currentObject = NULL;
    }
}

// Acquires an additional reference, if non-null.
template <typename InterfaceType>
inline InterfaceType* SafeAcquire(InterfaceType* newObject)
{
    if (newObject != NULL)
        newObject->AddRef();

    return newObject;
}

// Sets a new COM object, releasing the old one.
template <typename InterfaceType>
inline void SafeSet(InterfaceType** currentObject, InterfaceType* newObject)
{
    SafeAcquire(newObject);
    SafeRelease(currentObject);
    *currentObject = newObject;
}

// Releases a COM object and nullifies pointer.
template <typename InterfaceType>
inline InterfaceType* SafeDetach(InterfaceType** currentObject)
{
    InterfaceType* oldObject = *currentObject;
    *currentObject = NULL;
    return oldObject;
}

// Sets a new COM object, acquiring the reference.
template <typename InterfaceType>
inline void SafeAttach(InterfaceType** currentObject, InterfaceType* newObject)
{
    SafeRelease(currentObject);
    *currentObject = newObject;
}

// Maps exceptions to equivalent HRESULTs,
inline HRESULT ExceptionToHResult() throw()
{
    try
    {
        throw;  // Rethrow previous exception.
    }
    catch (std::bad_alloc&)
    {
        return E_OUTOFMEMORY;
    }
    catch (...)
    {
        return E_FAIL;
    }
}


// Generic COM base implementation for classes, since DirectWrite uses
// callbacks for several different kinds of objects, particularly the
// text renderer and inline objects.
//
// Example:
//
//  class RenderTarget : public ComBase<QiList<IDWriteTextRenderer> >
//
template <typename InterfaceChain>
class ComBase : public InterfaceChain
{
public:
    explicit ComBase() throw()
        : refValue_(0)
    { }

    // IUnknown interface
    IFACEMETHOD(QueryInterface)(IID const& iid, OUT void** ppObject)
    {
        *ppObject = NULL;
        InterfaceChain::QueryInterfaceInternal(iid, ppObject);
        if (*ppObject == NULL)
            return E_NOINTERFACE;

        AddRef();
        return S_OK;
    }

    IFACEMETHOD_(ULONG, AddRef)()
    {
        return InterlockedIncrement(&refValue_);
    }

    IFACEMETHOD_(ULONG, Release)()
    {
        ULONG newCount = InterlockedDecrement(&refValue_);
        if (newCount == 0)
            delete this;

        return newCount;
    }

    virtual ~ComBase()
    { }

protected:
    ULONG refValue_;

private:
    // No copy construction allowed.
    ComBase(const ComBase& b);
    ComBase& operator=(ComBase const&);
};


struct QiListNil
{
};

// When the QueryInterface list refers to itself as class,
// which hasn't fully been defined yet.
template <typename InterfaceName, typename InterfaceChain>
class QiListSelf : public InterfaceChain
{
public:
    inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
    {
        if (iid != __uuidof(InterfaceName))
            return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

        *ppObject = static_cast<InterfaceName*>(this);
    }
};


// When this interface is implemented and more follow.
template <typename InterfaceName, typename InterfaceChain = QiListNil>
class QiList : public InterfaceName, public InterfaceChain
{
public:
    inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
    {
        if (iid != __uuidof(InterfaceName))
            return InterfaceChain::QueryInterfaceInternal(iid, ppObject);

        *ppObject = static_cast<InterfaceName*>(this);
    }
};


// When the this is the last implemented interface in the list.
template <typename InterfaceName>
class QiList<InterfaceName, QiListNil> : public InterfaceName
{
public:
    inline void QueryInterfaceInternal(IID const& iid, OUT void** ppObject) throw()
    {
        if (iid != __uuidof(InterfaceName))
            return;

        *ppObject = static_cast<InterfaceName*>(this);
    }
};

////////////////////////////////////////
// Helper to construct text ranges when calling setters.
//
// Example: textLayout_->SetFontWeight(DWRITE_FONT_WEIGHT_BOLD, MakeDWriteTextRange(20, 10));

struct MakeDWriteTextRange : public DWRITE_TEXT_RANGE
{
    inline MakeDWriteTextRange(U32 startPosition, U32 length)
    {
        this->startPosition = startPosition;
        this->length = length;
    }

    // Overload to extend to end of text.
    inline MakeDWriteTextRange(U32 startPosition)
    {
        this->startPosition = startPosition;
        this->length = UINT32_MAX - startPosition;
    }
};

//////////////////////////////////////////////////////////////////////////////
// Layouts were optimized for mostly static UI text, so a layout's text is
// immutable upon creation. This means that when we modify the text, we must
// create a new layout, copying the old properties over to the new one. This
// class assists with that.
class EditableLayout
{
public:
    struct CaretFormat
    {
        // The important range based properties for the current caret.
        // Note these are stored outside the layout, since the current caret
        // actually has a format, independent of the text it lies between.
        wchar_t fontFamilyName[100];
        wchar_t localeName[LOCALE_NAME_MAX_LENGTH];
        FLOAT fontSize;
        DWRITE_FONT_WEIGHT fontWeight;
        DWRITE_FONT_STRETCH fontStretch;
        DWRITE_FONT_STYLE fontStyle;
        U32 color;
        BOOL hasUnderline;
        BOOL hasStrikethrough;
    };

    EditableLayout() {}
    ~EditableLayout() {}

public:
    void SetFactory(void* f) 
    { 
        factory_ = (IDWriteFactory*)f;
    }

    IDWriteFactory* GetFactory() { return factory_; }

    /// Inserts a given string in the text layout's stored string at a certain text postion;
    HRESULT STDMETHODCALLTYPE InsertTextAt(
        IN OUT IDWriteTextLayout** currentLayout,
        IN OUT std::wstring& text,
        U32 position,
        WCHAR const* textToInsert,  // [lengthToInsert]
        U32 textToInsertLength,
        CaretFormat* caretFormat = NULL
    );

    /// Deletes a specified amount characters from the layout's stored string.
    HRESULT STDMETHODCALLTYPE RemoveTextAt(IN OUT IDWriteTextLayout** currentLayout,
        IN OUT std::wstring& text,
        U32 position,
        U32 lengthToRemove
    );

    HRESULT STDMETHODCALLTYPE Clear(IN OUT IDWriteTextLayout** currentLayout, IN OUT std::wstring& text);

private:
    HRESULT STDMETHODCALLTYPE RecreateLayout(IN OUT IDWriteTextLayout** currentLayout, const std::wstring& text)
    {
        HRESULT hr = S_OK;
        IDWriteTextLayout* newLayout = NULL;
        FLOAT maxWidth = (*currentLayout)->GetMaxWidth();
        FLOAT maxHeight = (*currentLayout)->GetMaxHeight();
        UINT32 stringLength = static_cast<UINT32>(text.length());

        hr = factory_->CreateTextLayout(
            text.c_str(),
            stringLength,
            *currentLayout,
            maxWidth,
            maxHeight,
            &newLayout
        );

        if (SUCCEEDED(hr))
            SafeAttach(currentLayout, SafeDetach(&newLayout));

        SafeRelease(&newLayout);
        return hr;
    }

    static void CopyGlobalProperties(IDWriteTextLayout* oldLayout, IDWriteTextLayout* newLayout);

    static void CopyRangedProperties(
        IDWriteTextLayout* oldLayout,
        U32 startPos,
        U32 endPos,
        U32 newLayoutTextOffset,
        IDWriteTextLayout* newLayout,
        bool isOffsetNegative = false
    );

    static void CopySinglePropertyRange(
        IDWriteTextLayout* oldLayout, U32 startPosForOld,
        IDWriteTextLayout* newLayout, U32 startPosForNew,
        U32 length, EditableLayout::CaretFormat* caretFormat = NULL
    );

public:
    IDWriteFactory* factory_ = nullptr;
};


////////////////////////////////////////////////////////////////////////////////

class RenderTarget;

// Intermediate render target for UI to draw to either a D2D or GDI surface.
class DECLSPEC_UUID("4327AC14-3172-4807-BF40-02C7475A2520") RenderTarget
    : public ComBase<
    QiListSelf<RenderTarget,
    QiList<IDWriteTextRenderer>
    > >
{
public:
    virtual ~RenderTarget() {};

    virtual void BeginDraw() = NULL;
    virtual void EndDraw() = NULL;
    virtual void Clear(U32 color) = NULL;
    virtual void Resize(UINT width, UINT height) = NULL;
    virtual void UpdateMonitor() = NULL;

    virtual void SetTransform(DWRITE_MATRIX const& transform) = NULL;
    virtual void GetTransform(DWRITE_MATRIX & transform) = NULL;
    virtual void SetAntialiasing(bool isEnabled) = NULL;


    virtual void DrawTextLayout(
        IDWriteTextLayout * textLayout,
        const RectF & rect
    ) = NULL;

    // Draws a single image, from the given coordinates, to the given coordinates.
    // If the height and width differ, they will be scaled, but mirroring must be
    // done via a matrix transform.
    virtual void DrawImage(
        IWICBitmapSource * image,
        const RectF & sourceRect,  // where in source atlas texture
        const RectF & destRect     // where on display to draw it
    ) = NULL;

    virtual void FillRectangle(
        const RectF & destRect,
        const DrawingEffect & drawingEffect
    ) = NULL;

protected:
    // This context is not persisted, only existing on the stack as it
    // is passed down through. This is mainly needed to handle cases
    // where runs where no drawing effect set, like those of an inline
    // object or trimming sign.
    struct Context
    {
        Context(RenderTarget* initialTarget, IUnknown* initialDrawingEffect)
            : target(initialTarget),
            drawingEffect(initialDrawingEffect)
        { }

        // short lived weak pointers
        RenderTarget* target;
        IUnknown* drawingEffect;
    };

    IUnknown* GetDrawingEffect(void* clientDrawingContext, IUnknown * drawingEffect)
    {
        // Callbacks use this to use a drawing effect from the client context
        // if none was passed into the callback.
        if (drawingEffect != NULL)
            return drawingEffect;

        return (reinterpret_cast<Context*>(clientDrawingContext))->drawingEffect;
    }
};

class RenderTargetD2D : public RenderTarget
{
public:
    RenderTargetD2D(ID2D1Factory* d2dFactory, IDWriteFactory* dwriteFactory, HWND hwnd);
    HRESULT static Create(ID2D1Factory* d2dFactory, IDWriteFactory* dwriteFactory, HWND hwnd, OUT RenderTarget** renderTarget);

    virtual ~RenderTargetD2D();

    virtual void BeginDraw();
    virtual void EndDraw();
    virtual void Clear(U32 color);
    virtual void Resize(UINT width, UINT height);
    virtual void UpdateMonitor();

    virtual void SetTransform(DWRITE_MATRIX const& transform);
    virtual void GetTransform(DWRITE_MATRIX& transform);
    virtual void SetAntialiasing(bool isEnabled);

    virtual void DrawTextLayout(
        IDWriteTextLayout* textLayout,
        const RectF& rect
    );

    virtual void DrawImage(
        IWICBitmapSource* image,
        const RectF& sourceRect,  // where in source atlas texture
        const RectF& destRect     // where on display to draw it
    );

    void FillRectangle(
        const RectF& destRect,
        const DrawingEffect& drawingEffect
    );

    // IDWriteTextRenderer implementation 

    IFACEMETHOD(DrawGlyphRun)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        const DWRITE_GLYPH_RUN* glyphRun,
        const DWRITE_GLYPH_RUN_DESCRIPTION* glyphRunDescription,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(DrawUnderline)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        const DWRITE_UNDERLINE* underline,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(DrawStrikethrough)(
        void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        const DWRITE_STRIKETHROUGH* strikethrough,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(DrawInlineObject)(
        void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown* clientDrawingEffect
        );

    IFACEMETHOD(IsPixelSnappingDisabled)(
        void* clientDrawingContext,
        OUT BOOL* isDisabled
        );

    IFACEMETHOD(GetCurrentTransform)(
        void* clientDrawingContext,
        OUT DWRITE_MATRIX* transform
        );

    IFACEMETHOD(GetPixelsPerDip)(
        void* clientDrawingContext,
        OUT FLOAT* pixelsPerDip
        );

public:
    // For cached images, to avoid needing to recreate the textures each draw call.
    struct ImageCacheEntry
    {
        ImageCacheEntry(IWICBitmapSource* initialOriginal, ID2D1Bitmap* initialConverted)
            : original(SafeAcquire(initialOriginal)),
            converted(SafeAcquire(initialConverted))
        { }

        ImageCacheEntry(const ImageCacheEntry& b)
        {
            original = SafeAcquire(b.original);
            converted = SafeAcquire(b.converted);
        }

        ImageCacheEntry& operator=(const ImageCacheEntry& b)
        {
            if (this != &b)
            {
                // Define assignment operator in terms of destructor and
                // placement new constructor, paying heed to self assignment.
                this->~ImageCacheEntry();
                new(this) ImageCacheEntry(b);
            }
            return *this;
        }

        ~ImageCacheEntry()
        {
            SafeRelease(&original);
            SafeRelease(&converted);
        }

        IWICBitmapSource* original;
        ID2D1Bitmap* converted;
    };

protected:
    HRESULT CreateTarget();
    ID2D1Bitmap* GetCachedImage(IWICBitmapSource* image);
    ID2D1Brush* GetCachedBrush(const DrawingEffect* effect);

protected:
    IDWriteFactory* dwriteFactory_;
    ID2D1Factory* d2dFactory_;
    ID2D1HwndRenderTarget* target_;     // D2D render target
    ID2D1SolidColorBrush* brush_;       // reusable scratch brush for current color

    std::vector<ImageCacheEntry> imageCache_;

    HWND hwnd_;
    HMONITOR hmonitor_;

};

struct IWICBitmapSource;


class DECLSPEC_UUID("1DE84D4E-1AD2-40ec-82B3-1B5B93471C65") InlineImage
    : public ComBase<
    QiListSelf<InlineImage,
    QiList<IDWriteInlineObject
    > > >
{
public:
    InlineImage(
        IWICBitmapSource * image,
        unsigned int index = ~0
    );

    ~InlineImage()
    {
        SafeRelease(&image_);
    }

    IFACEMETHOD(Draw)(
        void* clientDrawingContext,
        IDWriteTextRenderer * renderer,
        FLOAT originX,
        FLOAT originY,
        BOOL isSideways,
        BOOL isRightToLeft,
        IUnknown * clientDrawingEffect
        );

    IFACEMETHOD(GetMetrics)(
        OUT DWRITE_INLINE_OBJECT_METRICS * metrics
        );

    IFACEMETHOD(GetOverhangMetrics)(
        OUT DWRITE_OVERHANG_METRICS * overhangs
        );

    IFACEMETHOD(GetBreakConditions)(
        OUT DWRITE_BREAK_CONDITION * breakConditionBefore,
        OUT DWRITE_BREAK_CONDITION * breakConditionAfter
        );

    static HRESULT LoadImageFromResource(
        const wchar_t* resourceName,
        const wchar_t* resourceType,
        IWICImagingFactory * wicFactory,
        OUT IWICBitmapSource * *bitmap
    );

    static HRESULT LoadImageFromFile(
        const wchar_t* fileName,
        IWICImagingFactory * wicFactory,
        OUT IWICBitmapSource * *bitmap
    );

protected:
    IWICBitmapSource* image_;
    RectF rect_; // coordinates in image, similar to index of HIMAGE_LIST
    float baseline_;
};


#endif // _WT_DUIWIN32_H_