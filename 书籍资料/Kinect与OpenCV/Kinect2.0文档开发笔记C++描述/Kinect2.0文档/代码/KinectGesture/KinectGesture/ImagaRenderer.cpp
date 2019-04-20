#include "stdafx.h"
#include "included.h"
#include <wchar.h>


static const float c_JointThickness = 3.0f;
static const float c_TrackedBoneThickness = 6.0f;
static const float c_InferredBoneThickness = 1.0f;
static const float c_HandSize = 30.0f;
static const float c_GestureThreshold = 0.9f;

// ImageRender�๹�캯��
ImageRenderer::ImageRenderer(ThisApp* app) :m_pBoss(app){
    // ����������
    m_pColorRGBX = new RGBQUAD[IMAGE_WIDTH*IMAGE_HEIGHT + DEPTH_WIDTH*DEPTH_HEIGHT];
    m_pDepthRGBX = m_pColorRGBX + IMAGE_WIDTH*IMAGE_HEIGHT;
    // ������Դ
    m_hrInit = CreateDeviceIndependentResources();
    if (!m_pColorRGBX) m_hrInit = E_OUTOFMEMORY;
    m_timer.Start();
}

// д������
void ImageRenderer::WriteBitmapData(RGBQUAD* data, int type, int width, int height){
    D2D1_RECT_U rect = { 0, 0, width, height };
    if (type == 0){
        m_pColorBitmap->CopyFromMemory(&rect, data, width*sizeof(RGBQUAD));
    }
    else if(type == 1){
        m_pDepthBitmap->CopyFromMemory(&rect, data, width*sizeof(RGBQUAD));
    }
}

// �����豸�޹���Դ
HRESULT ImageRenderer::CreateDeviceIndependentResources(){
    // ���� Direct2D ����.
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_pD2DFactory);
    // ���� WIC ����.
    if (SUCCEEDED(hr))  {
        hr = CoCreateInstance(
            CLSID_WICImagingFactory,
            NULL,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory,
            reinterpret_cast<void **>(&m_pWICFactory)
            );
    }
    // ���� DirectWrite ����.
    if (SUCCEEDED(hr)) {
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }
    // ���������ı���ʽ.
    if (SUCCEEDED(hr)) {
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Microsoft YaHei",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            13.f,
            L"", //locale
            &m_pTextFormatMain
            );
    }
    // ����
    if (SUCCEEDED(hr)){
        // �ײ����ж���
        m_pTextFormatMain->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_FAR);
        m_pTextFormatMain->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    }
    return hr;
}

// ���ļ���ȡλͼ
HRESULT ImageRenderer::LoadBitmapFromFile(
    ID2D1RenderTarget *pRenderTarget,
    IWICImagingFactory *pIWICFactory,
    PCWSTR uri,
    UINT destinationWidth,
    UINT destinationHeight,
    ID2D1Bitmap **ppBitmap
    )
{
    IWICBitmapDecoder *pDecoder = NULL;
    IWICBitmapFrameDecode *pSource = NULL;
    IWICStream *pStream = NULL;
    IWICFormatConverter *pConverter = NULL;
    IWICBitmapScaler *pScaler = NULL;

    HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
        uri,
        NULL,
        GENERIC_READ,
        WICDecodeMetadataCacheOnLoad,
        &pDecoder
        );

    if (SUCCEEDED(hr))
    {
        hr = pDecoder->GetFrame(0, &pSource);
    }
    if (SUCCEEDED(hr))
    {
        hr = pIWICFactory->CreateFormatConverter(&pConverter);
    }


    if (SUCCEEDED(hr))
    {
        if (destinationWidth != 0 || destinationHeight != 0)
        {
            UINT originalWidth, originalHeight;
            hr = pSource->GetSize(&originalWidth, &originalHeight);
            if (SUCCEEDED(hr))
            {
                if (destinationWidth == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationHeight) / static_cast<FLOAT>(originalHeight);
                    destinationWidth = static_cast<UINT>(scalar * static_cast<FLOAT>(originalWidth));
                }
                else if (destinationHeight == 0)
                {
                    FLOAT scalar = static_cast<FLOAT>(destinationWidth) / static_cast<FLOAT>(originalWidth);
                    destinationHeight = static_cast<UINT>(scalar * static_cast<FLOAT>(originalHeight));
                }

                hr = pIWICFactory->CreateBitmapScaler(&pScaler);
                if (SUCCEEDED(hr))
                {
                    hr = pScaler->Initialize(
                        pSource,
                        destinationWidth,
                        destinationHeight,
                        WICBitmapInterpolationModeCubic
                        );
                }
                if (SUCCEEDED(hr))
                {
                    hr = pConverter->Initialize(
                        pScaler,
                        GUID_WICPixelFormat32bppPBGRA,
                        WICBitmapDitherTypeNone,
                        NULL,
                        0.f,
                        WICBitmapPaletteTypeMedianCut
                        );
                }
            }
        }
        else
        {
            hr = pConverter->Initialize(
                pSource,
                GUID_WICPixelFormat32bppPBGRA,
                WICBitmapDitherTypeNone,
                NULL,
                0.f,
                WICBitmapPaletteTypeMedianCut
                );
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = pRenderTarget->CreateBitmapFromWicBitmap(
            pConverter,
            NULL,
            ppBitmap
            );
    }

    SafeRelease(pDecoder);
    SafeRelease(pSource);
    SafeRelease(pStream);
    SafeRelease(pConverter);
    SafeRelease(pScaler);

    return hr;
}

// �����豸�����Դ
HRESULT ImageRenderer::CreateDeviceResources()
{
    HRESULT hr = S_OK;
    if (!m_pRenderTarget)
    {
        RECT rc;
        GetClientRect(m_hwnd, &rc);

        D2D1_SIZE_U size = D2D1::SizeU(
            rc.right - rc.left,
            rc.bottom - rc.top
            );

        // ���� Direct2D RenderTarget.
        hr = m_pD2DFactory->CreateHwndRenderTarget(
            D2D1::RenderTargetProperties(),
            D2D1::HwndRenderTargetProperties(m_hwnd, size),
            &m_pRenderTarget
            );

        // ����λͼ
        if (SUCCEEDED(hr)){
            D2D1_BITMAP_PROPERTIES prop = { { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96.f, 96.f };
            hr = m_pRenderTarget->CreateBitmap(
                D2D1::SizeU(IMAGE_WIDTH, IMAGE_HEIGHT),
                prop,
                &m_pColorBitmap
                );
        }
        if (SUCCEEDED(hr)){
            D2D1_BITMAP_PROPERTIES prop = { { DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED }, 96.f, 96.f };
            hr = m_pRenderTarget->CreateBitmap(
                D2D1::SizeU(DEPTH_WIDTH, DEPTH_HEIGHT),
                prop,
                &m_pDepthBitmap
                );
        }

        // ������ɫ��ˢ
        if (SUCCEEDED(hr)){
            ID2D1SolidColorBrush* pSolidColorBrush = nullptr;
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pSolidColorBrush);
            m_pWhiteBrush = pSolidColorBrush;
        }

        if (SUCCEEDED(hr)){
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.27f, 0.75f, 0.27f), &m_pBrushJointTracked);
        }
       
        if (SUCCEEDED(hr)){
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow, 1.0f), &m_pBrushJointInferred);
        }

        if (SUCCEEDED(hr)){
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 1.0f), &m_pBrushBoneTracked);
        }

        if (SUCCEEDED(hr)){
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Gray, 1.0f), &m_pBrushBoneInferred);
        }

        if (SUCCEEDED(hr)){
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Red, 0.5f), &m_pBrushHandClosed);
        }

        if (SUCCEEDED(hr)){
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Green, 0.5f), &m_pBrushHandOpen);
        }

        if (SUCCEEDED(hr)){
            hr = m_pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Blue, 0.5f), &m_pBrushHandLasso);
        }
    }

    return hr;
}

// ImageRender��������
ImageRenderer::~ImageRenderer(){
    DiscardDeviceResources();
    SafeRelease(m_pD2DFactory);
    SafeRelease(m_pWICFactory);
    SafeRelease(m_pDWriteFactory);
    SafeRelease(m_pTextFormatMain);
    // �ͷ�����
    if (m_pColorRGBX){
        delete[] m_pColorRGBX;
        m_pColorRGBX = nullptr;
    }
    m_pDepthRGBX = nullptr;
}

// �����豸�����Դ
void ImageRenderer::DiscardDeviceResources(){
    SafeRelease(m_pWhiteBrush);
    SafeRelease(m_pColorBitmap);
    SafeRelease(m_pDepthBitmap);
    SafeRelease(m_pBrushJointTracked);
    SafeRelease(m_pBrushJointInferred);
    SafeRelease(m_pBrushBoneTracked);
    SafeRelease(m_pBrushBoneInferred);
    SafeRelease(m_pBrushHandOpen);
    SafeRelease(m_pBrushHandLasso);
    SafeRelease(m_pRenderTarget);
}


// ��Ⱦͼ��ͼ��
HRESULT ImageRenderer::OnRender(){
    HRESULT hr = S_OK;
    D2D1_RECT_F rect;
    D2D1_MATRIX_3X2_F matrix=D2D1::Matrix3x2F::Identity();
    // ���Դ�����Դ
    hr = CreateDeviceResources();
    if (SUCCEEDED(hr)){
        // ��ʼ
        m_pRenderTarget->BeginDraw();
        // ����
        m_pRenderTarget->Clear(D2D1::ColorF(0x0066CCFF));

        // ------------ �����Ϸ���ʾ��ɫ֡
        // ���1/3���� �߶ȱ�16:9
        m_pRenderTarget->SetTransform(matrix);
        auto size = m_pRenderTarget->GetSize();
        rect = { 0.f };
        rect.right = static_cast<float>(size.width / 3);
        rect.bottom = rect.right * (static_cast<float>(IMAGE_HEIGHT) / static_cast<float>(IMAGE_WIDTH));
        m_pRenderTarget->DrawBitmap(m_pColorBitmap, &rect);
        // ------------ �ڲ�ɫ֡�·���ʾ���֡
        matrix = D2D1::Matrix3x2F::Translation(D2D1::SizeF(0.f, rect.bottom));
        m_pRenderTarget->SetTransform(matrix);
        rect.bottom = rect.right * (static_cast<float>(DEPTH_HEIGHT) / static_cast<float>(DEPTH_WIDTH));
        m_pRenderTarget->DrawBitmap(m_pDepthBitmap, &rect);
        // ------------ �����֡�ϸ��Ƿ���ʾ����֡
        float zoom = rect.right / static_cast<float>(DEPTH_WIDTH);
        matrix = D2D1::Matrix3x2F::Scale(D2D1::SizeF(zoom, zoom));
        matrix._32 = rect.right * (static_cast<float>(IMAGE_HEIGHT) / static_cast<float>(IMAGE_WIDTH));
        m_pRenderTarget->SetTransform(matrix);
        this->DrawBody();
        // ------------ �ҷ���ʾ����
        zoom = size.width / static_cast<float>(WNDWIDTH) * 1.15f;
        matrix = D2D1::Matrix3x2F::Scale(D2D1::SizeF(zoom, zoom));
        matrix._31 = size.width * 0.35f;
        matrix._32 = 10.f;
        m_pRenderTarget->SetTransform(matrix);
        this->DrawGestureInfo();
        // �����̻�
        hr = m_pRenderTarget->EndDraw();
        // �յ��ؽ���Ϣʱ, �ͷ���Դ, �ȴ��´��Զ�����
        if (hr == D2DERR_RECREATE_TARGET) {
            DiscardDeviceResources();
            hr = S_OK;
        }
    }
    return hr;
}


// �̻�����
void ImageRenderer::DrawGestureInfo(){
    const auto ppGestures = m_pBoss->GetGestures();
    if (!ppGestures) return;
    // -------  ��Ⱦ���� ------
    // * ÿ����ʾ3��
    D2D1_RECT_F rect_base = { 0.f };
    // ��ȡ��Ϣ
    UINT size = m_pBoss->GetGestureSize();
    // �ı�����
    WCHAR buffer[1024];
    // ��Ⱦ��λ
    const float rect_unit = 100.f;
    // ѭ������
    for (UINT i = 0U; i < size; ++i){
        rect_base.left = static_cast<float>(i % 3) * 120.f;
        rect_base.top = static_cast<float>(i / 3) * 120.f + show_offset;
        rect_base.right = rect_base.left + rect_unit;
        rect_base.bottom = rect_base.top + rect_unit;
        D2D1_RECT_F rect = rect_base;
        // ��ʾ
        auto info_data = m_pBoss->GetGestureInfo(i);
        // ��ɢ
        if (info_data->type){
            // ��ʾ���ο�
            m_pRenderTarget->DrawRectangle(rect, m_pBrushHandOpen);
            const auto x_plus = rect_unit / static_cast<float>(GESTURES_SAVED * 2);
            rect.bottom -= rect_unit * 0.5f;
            // ��Ⱦ���Ŷ�
            for (UINT j = 0U; j < GESTURES_SAVED; ++j){
                auto index = (j + info_data->index + 1) % GESTURES_SAVED;
                float float_data = info_data->float_var[index];
                // �ж�Ϊ������Ϊ����
                if (!info_data->bool1_var[index]) float_data *= -1.f;
                rect.right = rect.left + x_plus;
                rect.top = rect.bottom - float_data * rect_unit * 0.5f;
                // ��֡��ɫ
                m_pRenderTarget->FillRectangle(rect, info_data->bool2_var[index] ? m_pBrushHandOpen : m_pBrushHandOpen);
                rect.left += x_plus*2.f;
            }
        }
        // ����
        else{
            // ��ʾ���ο�
            m_pRenderTarget->DrawRectangle(rect, m_pBrushHandLasso);
            const auto x_plus = rect_unit / static_cast<float>(GESTURES_SAVED * 2);
            rect.bottom -= rect_unit * 0.5f;
            // ��Ⱦ���� ---- ���ȴ���߶�
            for (UINT j = 0U; j < GESTURES_SAVED; ++j){
                float float_data = info_data->float_var[(j + info_data->index + 1)%GESTURES_SAVED];
                rect.right = rect.left + x_plus;
                rect.top = rect.bottom - float_data * rect_unit * 0.5f;
                // ������ֵ
                m_pRenderTarget->FillRectangle(rect, float_data > c_GestureThreshold ? m_pBrushHandLasso : m_pBrushHandClosed);
                rect.left += x_plus*2.f;
            }
        }
        // ��ʾ��������
        rect_base.bottom += 20.f;
        ppGestures[i]->get_Name(lengthof(buffer), buffer);
        auto length = wcslen(buffer);
        m_pRenderTarget->DrawTextW(
            buffer,
            length,
            m_pTextFormatMain,
            rect_base,
            m_pBrushJointInferred
            );
    }
    // ------ ��ʾ�����ļ���
    // ��Ϊ��Ⱦλ��
    rect_base.left = 0.f;
    rect_base.top += rect_unit;
    rect_base.right = rect_unit * 3.f;
    rect_base.bottom = rect_base.top + rect_unit;
    // ��ȡ����
    auto file_name = m_pBoss->GetFileName();
    auto length = wcslen(file_name);
    // ��ʽ��Ⱦ
    m_pRenderTarget->DrawTextW(
        file_name,
        length,
        m_pTextFormatMain,
        rect_base,
        m_pBrushBoneTracked
        );

}


// �̻�����
void ImageRenderer::DrawBody(){
    m_fFPS = 1000.f / m_timer.DeltaF_ms();
    m_timer.MovStartEnd();
    for (int i = 0; i < BODY_COUNT; ++i){
        DrawBody(m_bodyInfo[i].joints, m_bodyInfo[i].jointPoints);
        DrawHand(m_bodyInfo[i].leftHandState, m_bodyInfo[i].jointPoints[JointType_HandLeft]);
        DrawHand(m_bodyInfo[i].rightHandState, m_bodyInfo[i].jointPoints[JointType_HandRight]);
    }
}



// 
void ImageRenderer::DrawBody(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints){

    DrawBone(pJoints, pJointPoints, JointType_Head, JointType_Neck);
    DrawBone(pJoints, pJointPoints, JointType_Neck, JointType_SpineShoulder);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_SpineMid);
    DrawBone(pJoints, pJointPoints, JointType_SpineMid, JointType_SpineBase);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderRight);
    DrawBone(pJoints, pJointPoints, JointType_SpineShoulder, JointType_ShoulderLeft);
    DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipRight);
    DrawBone(pJoints, pJointPoints, JointType_SpineBase, JointType_HipLeft);
 
    DrawBone(pJoints, pJointPoints, JointType_ShoulderRight, JointType_ElbowRight);
    DrawBone(pJoints, pJointPoints, JointType_ElbowRight, JointType_WristRight);
    DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_HandRight);
    DrawBone(pJoints, pJointPoints, JointType_HandRight, JointType_HandTipRight);
    DrawBone(pJoints, pJointPoints, JointType_WristRight, JointType_ThumbRight);

    DrawBone(pJoints, pJointPoints, JointType_ShoulderLeft, JointType_ElbowLeft);
    DrawBone(pJoints, pJointPoints, JointType_ElbowLeft, JointType_WristLeft);
    DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_HandLeft);
    DrawBone(pJoints, pJointPoints, JointType_HandLeft, JointType_HandTipLeft);
    DrawBone(pJoints, pJointPoints, JointType_WristLeft, JointType_ThumbLeft);

    DrawBone(pJoints, pJointPoints, JointType_HipRight, JointType_KneeRight);
    DrawBone(pJoints, pJointPoints, JointType_KneeRight, JointType_AnkleRight);
    DrawBone(pJoints, pJointPoints, JointType_AnkleRight, JointType_FootRight);

    DrawBone(pJoints, pJointPoints, JointType_HipLeft, JointType_KneeLeft);
    DrawBone(pJoints, pJointPoints, JointType_KneeLeft, JointType_AnkleLeft);
    DrawBone(pJoints, pJointPoints, JointType_AnkleLeft, JointType_FootLeft);

    for (int i = 0; i < JointType_Count; ++i)
    {
        D2D1_ELLIPSE ellipse = D2D1::Ellipse(pJointPoints[i], c_JointThickness, c_JointThickness);

        if (pJoints[i].TrackingState == TrackingState_Inferred)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointInferred);
        }
        else if (pJoints[i].TrackingState == TrackingState_Tracked)
        {
            m_pRenderTarget->FillEllipse(ellipse, m_pBrushJointTracked);
        }
    }
}

//
void ImageRenderer::DrawBone(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1)
{
    TrackingState joint0State = pJoints[joint0].TrackingState;
    TrackingState joint1State = pJoints[joint1].TrackingState;

    if ((joint0State == TrackingState_NotTracked) || (joint1State == TrackingState_NotTracked))
    {
        return;
    }

    if ((joint0State == TrackingState_Inferred) && (joint1State == TrackingState_Inferred))
    {
        return;
    }

    if ((joint0State == TrackingState_Tracked) && (joint1State == TrackingState_Tracked))
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneTracked, c_TrackedBoneThickness);
    }
    else
    {
        m_pRenderTarget->DrawLine(pJointPoints[joint0], pJointPoints[joint1], m_pBrushBoneInferred, c_InferredBoneThickness);
    }
}

//
void ImageRenderer::DrawHand(HandState handState, const D2D1_POINT_2F& handPosition)
{
    D2D1_ELLIPSE ellipse = D2D1::Ellipse(handPosition, c_HandSize, c_HandSize);

    switch (handState)
    {
    case HandState_Closed:
        m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandClosed);
        break;

    case HandState_Open:
        m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandOpen);
        break;

    case HandState_Lasso:
        m_pRenderTarget->FillEllipse(ellipse, m_pBrushHandLasso);
        break;
    }
}
