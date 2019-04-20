// ImageRenderer�� ����ͼ��ͼ����Ⱦ

#pragma once

// �ı�·����Ⱦ���ò���
struct PathTextDrawingContext{
    ID2D1RenderTarget*          pRenderTarget;
    ID2D1Geometry*              pGeometry;
    ID2D1Brush*                 pBrush;
};


// �ı�·����Ⱦ��
class PathTextRenderer : public IDWriteTextRenderer{
public:
    // ���캯��(�豸�޹����ش�С)
    PathTextRenderer(FLOAT pixelsPerDip);
public:
    // �����ı�·����Ⱦ��
    static void CreatePathTextRenderer(FLOAT pixelsPerDip, PathTextRenderer **textRenderer);
    // �̻�����
    STDMETHOD(DrawGlyphRun)(
        _In_opt_ void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        DWRITE_MEASURING_MODE measuringMode,
        _In_ DWRITE_GLYPH_RUN const* glyphRun,
        _In_ DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
        _In_opt_ IUnknown* clientDrawingEffect
        ) override;
    // �̻��»���
    STDMETHOD(DrawUnderline)(
        _In_opt_ void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        _In_ DWRITE_UNDERLINE const* underline,
        _In_opt_ IUnknown* clientDrawingEffect
        ) override;
    // �̻���ɾ����
    STDMETHOD(DrawStrikethrough)(
        _In_opt_ void* clientDrawingContext,
        FLOAT baselineOriginX,
        FLOAT baselineOriginY,
        _In_ DWRITE_STRIKETHROUGH const* strikethrough,
        _In_opt_ IUnknown* clientDrawingEffect
        ) override;
    // �̻���������
    STDMETHOD(DrawInlineObject)(
        _In_opt_ void* clientDrawingContext,
        FLOAT originX,
        FLOAT originY,
        IDWriteInlineObject* inlineObject,
        BOOL isSideways,
        BOOL isRightToLeft,
        _In_opt_ IUnknown* clientDrawingEffect
        ) override;
    // ������ز�����
    STDMETHOD(IsPixelSnappingDisabled)(
        _In_opt_ void* clientDrawingContext,
        _Out_ BOOL* isDisabled
        ) override;
    // ��ȡ��ǰת��
    STDMETHOD(GetCurrentTransform)(
        _In_opt_ void* clientDrawingContext,
        _Out_ DWRITE_MATRIX* transform
        ) override;
    // ��ȡ�豸�޹����ش�С
    STDMETHOD(GetPixelsPerDip)(
        _In_opt_ void* clientDrawingContext,
        _Out_ FLOAT* pixelsPerDip
        ) override;
    // QueryInterface
    STDMETHOD(QueryInterface)(
        REFIID riid,
        _Outptr_ void** object
        ) override;
    // AddRef
    STDMETHOD_(ULONG, AddRef)() override;
    // Release
    STDMETHOD_(ULONG, Release)() override;
private:
    // �豸�޹����ش�С
    FLOAT m_pixelsPerDip;
    // ���ü���
    UINT m_ref;
};