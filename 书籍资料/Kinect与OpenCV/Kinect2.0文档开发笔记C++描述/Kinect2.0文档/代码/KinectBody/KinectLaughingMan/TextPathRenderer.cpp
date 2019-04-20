#include "stdafx.h"
#include "included.h"

// IDWritePixelSnapping::GetCurrentTransform ��ȡ�ĵ�λ����
const DWRITE_MATRIX identityTransform = { 1, 0, 0, 1, 0, 0 };

// <PathTextRenderer>�����ı�·����Ⱦ��
void PathTextRenderer::CreatePathTextRenderer(FLOAT pixelsPerDip, PathTextRenderer **ppTextRenderer){
    *ppTextRenderer = nullptr;
    PathTextRenderer *newRenderer = new PathTextRenderer(pixelsPerDip);
    newRenderer->AddRef();
    *ppTextRenderer = newRenderer;
    newRenderer = nullptr;
}

// <PathTextRenderer> ���캯��
PathTextRenderer::PathTextRenderer(FLOAT pixelsPerDip) :m_pixelsPerDip(pixelsPerDip), m_ref(0){
}

// �̻�����
HRESULT PathTextRenderer::DrawGlyphRun(
    void* pClientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    DWRITE_MEASURING_MODE measuringMode,
    DWRITE_GLYPH_RUN const* glyphRun,
    DWRITE_GLYPH_RUN_DESCRIPTION const* glyphRunDescription,
    IUnknown* clientDrawingEffect
    )
{
    HRESULT hr = S_OK;
    // ��������
    if (!pClientDrawingContext) return hr;
    // ��ȡ����
    PathTextDrawingContext* pDC = reinterpret_cast<PathTextDrawingContext*>(pClientDrawingContext);
    // ��ȡ��ǰԶת������
    D2D1_MATRIX_3X2_F originalTransform;
    pDC->pRenderTarget->GetTransform(&originalTransform);
    // ���㼯���峤��
    FLOAT maxLength = 0.f;
    if (SUCCEEDED(hr)){
        hr = pDC->pGeometry->ComputeLength(nullptr, &maxLength);
    }
    // ����һ�����ǿ����޸ĵľֲ�����
    DWRITE_GLYPH_RUN partialGlyphRun = *glyphRun;
    // ����Ǵ������һ��Ǵ�������
    BOOL leftToRight = (glyphRun->bidiLevel % 2 == 0);
    // ����·�����ó�ʼ����
    FLOAT length = baselineOriginX;
    // �ڵ�ǰ���μ�Ⱥ���õ�һ�����ε�����λ��
    UINT firstGlyphIdx = 0;
    // ��ѭ��
    while (firstGlyphIdx < glyphRun->glyphCount){
        // ���������������Ⱥ�������ͼ�Ⱥ���ܿ��
        UINT numGlyphsInCluster = 0;
        UINT i = firstGlyphIdx;
        FLOAT clusterWidth = 0;

        while (glyphRunDescription->clusterMap[i] == glyphRunDescription->clusterMap[firstGlyphIdx] && i < glyphRun->glyphCount){
            clusterWidth += glyphRun->glyphAdvances[i];
            i++;
            numGlyphsInCluster++;
        }

        // ����·�����㼯Ⱥ���е�
        FLOAT midpoint = leftToRight ? (length + (clusterWidth / 2)) : (length - (clusterWidth / 2));

        // ��·������Ⱦ�����Ⱥ
        if (midpoint < maxLength && SUCCEEDED(hr)) {
            // �����е�ƫ��ֵ
            D2D1_POINT_2F offset;
            D2D1_POINT_2F tangent;
            hr = pDC->pGeometry->ComputePointAtLength(midpoint, D2D1::IdentityMatrix(), &offset, &tangent);

            D2D1_MATRIX_3X2_F rotation = D2D1::Matrix3x2F(
                tangent.x,
                tangent.y,
                -tangent.y,
                tangent.x,
                (offset.x * (1.0f - tangent.x) + offset.y * tangent.y),
                (offset.y * (1.0f - tangent.x) - offset.x * tangent.y)
                );

            // �����߷��򴴽���Ⱦ����
            D2D1_MATRIX_3X2_F translation = leftToRight ?
                D2D1::Matrix3x2F::Translation(-clusterWidth / 2, 0) : // LTR --> nudge it left
                D2D1::Matrix3x2F::Translation(clusterWidth / 2, 0); // RTL --> nudge it right

            // ����ת������
            pDC->pRenderTarget->SetTransform(translation * rotation * originalTransform);

            // ��Ⱦ����
            partialGlyphRun.glyphCount = numGlyphsInCluster;
            pDC->pRenderTarget->DrawGlyphRun(
                D2D1::Point2F(offset.x, offset.y),
                &partialGlyphRun,
                pDC->pBrush
                );
        }


        // �ƽ�����һ��
        length = leftToRight ? (length + clusterWidth) : (length - clusterWidth);
        partialGlyphRun.glyphIndices += numGlyphsInCluster;
        partialGlyphRun.glyphAdvances += numGlyphsInCluster;

        if (partialGlyphRun.glyphOffsets != nullptr)
        {
            partialGlyphRun.glyphOffsets += numGlyphsInCluster;
        }

        firstGlyphIdx += numGlyphsInCluster;
    }

    // ��ԭת������
    pDC->pRenderTarget->SetTransform(originalTransform);

    return S_OK;
}

// �̻��»���
HRESULT PathTextRenderer::DrawUnderline(
    _In_opt_ void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    _In_ DWRITE_UNDERLINE const* underline,
    _In_opt_ IUnknown* clientDrawingEffect
    )
{
    // ����Ҫ������д
    return E_NOTIMPL;
}

// �̻���ɾ����
HRESULT PathTextRenderer::DrawStrikethrough(
    _In_opt_ void* clientDrawingContext,
    FLOAT baselineOriginX,
    FLOAT baselineOriginY,
    _In_ DWRITE_STRIKETHROUGH const* strikethrough,
    _In_opt_ IUnknown* clientDrawingEffect
    )
{
    // ����Ҫ������д
    return E_NOTIMPL;
}

// �̻���������
HRESULT PathTextRenderer::DrawInlineObject(
    _In_opt_ void* clientDrawingContext,
    FLOAT originX,
    FLOAT originY,
    IDWriteInlineObject* inlineObject,
    BOOL isSideways,
    BOOL isRightToLeft,
    _In_opt_ IUnknown* clientDrawingEffect
    )
{
    // ����Ҫ������д
    return E_NOTIMPL;
}

//
// IDWritePixelSnapping ����
//
HRESULT PathTextRenderer::IsPixelSnappingDisabled(
    _In_opt_ void* clientDrawingContext,
    _Out_ BOOL* isDisabled
    )
{
    *isDisabled = FALSE;
    return S_OK;
}

HRESULT PathTextRenderer::GetCurrentTransform(
    _In_opt_ void* clientDrawingContext,
    _Out_ DWRITE_MATRIX* transform
    )
{
    *transform = identityTransform;
    return S_OK;
}

HRESULT PathTextRenderer::GetPixelsPerDip(
    _In_opt_ void* clientDrawingContext,
    _Out_ FLOAT* pixelsPerDip
    )
{
    *pixelsPerDip = m_pixelsPerDip;
    return S_OK;
}

// IUnknown ����
HRESULT PathTextRenderer::QueryInterface(
    REFIID riid,
    _Outptr_ void** object
    )
{
    *object = nullptr;
    return E_NOTIMPL;
}

ULONG PathTextRenderer::AddRef()
{
    m_ref++;

    return m_ref;
}

ULONG PathTextRenderer::Release()
{
    m_ref--;

    if (m_ref == 0)
    {
        delete this;
        return 0;
    }

    return m_ref;
}
