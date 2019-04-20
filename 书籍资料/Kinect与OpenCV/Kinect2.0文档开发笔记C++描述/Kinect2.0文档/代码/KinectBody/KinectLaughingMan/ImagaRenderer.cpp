#include "stdafx.h"
#include "included.h"

#undef 	PixelFormat
// ImageRenderer�๹�캯��
ImageRenderer::ImageRenderer()
{
    m_parameters.DirtyRectsCount = 0;
    m_parameters.pDirtyRects = nullptr;
    m_parameters.pScrollRect = nullptr;
    m_parameters.pScrollOffset = nullptr;


}


// ������Դ
HRESULT ImageRenderer::CreateResources(){
    // ������ʹ��ָ��

    // D3D �豸
    ID3D11Device*						pD3DDevice = nullptr;
    // D3D �豸������
    ID3D11DeviceContext*				pD3DDeviceContext = nullptr;
    // DXGI ������
    IDXGIAdapter*						pDxgiAdapter = nullptr;
    // DXGI ����
    IDXGIFactory2*						pDxgiFactory = nullptr;
    // 2D ���� ��̨����
    ID3D11Texture2D*					pBackBuffer = nullptr;
    // DXGI Surface ��̨����
    IDXGISurface*						pDxgiBackBuffer = nullptr;
    // DXGI �豸
    IDXGIDevice1*						pDxgiDevice = nullptr;


    HRESULT hr = S_OK;



    // ����D3D��ض���

    UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef _DEBUG
    // Debug״̬ ��D3D DebugLayer��ȡ��ע��
    //creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    // This array defines the set of DirectX hardware feature levels this app  supports.
    // The ordering is important and you should  preserve it.
    // Don't forget to declare your app's minimum required feature level in its
    // description.  All apps are assumed to support 9.1 unless otherwise stated.
    D3D_FEATURE_LEVEL featureLevels[] =
    {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0,
        D3D_FEATURE_LEVEL_9_3,
        D3D_FEATURE_LEVEL_9_2,
        D3D_FEATURE_LEVEL_9_1
    };
    /*if (SUCCEEDED(hr)){
    IDXGIFactory * pFactory = nullptr;

    hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)&pFactory);
    if (SUCCEEDED(hr)){
    for (UINT i = 0; pFactory->EnumAdapters(i, &pDxgiAdapter) != DXGI_ERROR_NOT_FOUND;	++i){
    DXGI_ADAPTER_DESC desc;
    pDxgiAdapter->GetDesc(&desc);
    //SafeRelease(pDxgiAdapter);
    }
    }
    SafeRelease(pFactory);
    }*/
    // ���� m_pD3DDevice & Context 
    if (SUCCEEDED(hr)){
        hr = D3D11CreateDevice(
            pDxgiAdapter,                    // specify null to use the default adapter
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            creationFlags,              // optionally set debug and Direct2D compatibility flags
            featureLevels,              // list of feature levels this app can support
            ARRAYSIZE(featureLevels),   // number of possible feature levels
            D3D11_SDK_VERSION,
            &pD3DDevice,              // returns the Direct3D device created
            &m_featureLevel,            // returns feature level of device created
            &pD3DDeviceContext        // returns the device immediate context
            );
        SafeRelease(pDxgiAdapter);
    }
    if (SUCCEEDED(hr))
    {
        // ���� IDXGIDevice
        hr = pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (void **)&pDxgiDevice);
    }


    if (SUCCEEDED(hr))
    {
        // ���� D2D ����
        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            __uuidof(ID2D1Factory1),
            reinterpret_cast<void**>(&m_pD2DFactory)
            );

    }
    if (SUCCEEDED(hr))
    {
        // ˳���ȡdpi
        m_pD2DFactory->GetDesktopDpi(&m_dpi, &m_dpi);
        // ���� WIC ����.
        hr = CoCreateInstance(
            CLSID_WICImagingFactory2,
            nullptr,
            CLSCTX_INPROC_SERVER,
            IID_IWICImagingFactory2,
            reinterpret_cast<void **>(&m_pWICFactory)
            );
    }

    if (SUCCEEDED(hr))
    {
        // ���� DirectWrite ����.
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }


    if (SUCCEEDED(hr))
    {
        // ��ʱ����D2D�豸
        hr = m_pD2DFactory->CreateDevice(pDxgiDevice, &m_pD2DDevice);
    }


    if (SUCCEEDED(hr))
    {
        // ��ʱ����D2D�豸
        hr = m_pD2DDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pD2DDeviceContext
            );
    }

    // Allocate a descriptor.
    RECT rect;
    ::GetClientRect(m_hwnd, &rect);
    rect.right -= rect.left;
    rect.bottom -= rect.top;

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
    swapChainDesc.Width = rect.right;
    swapChainDesc.Height = rect.bottom;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.Stereo = false;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;

    swapChainDesc.BufferCount = 2;
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
    // WIN8
    //swapChainDesc.Scaling = DXGI_SCALING_NONE;
    //swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL; // all apps must use this SwapEffect
    // WIN7
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    swapChainDesc.Flags = 0;

    // Identify the physical adapter (GPU or card) this device is runs on.
    if (SUCCEEDED(hr))
    {
        hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
    }
    // ��ȡ�豸��Ϣ
    if (SUCCEEDED(hr))
    {
        // hr = pDxgiAdapter->GetDesc(&Game::info().desc);
    }

    // Get the factory object that created the DXGI device.
    if (SUCCEEDED(hr))
    {
        // pDxgiAdapter->GetDesc(&Game::info().desc);
        hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
    }
    // Get the final swap chain for this window from the DXGI factory

    if (SUCCEEDED(hr))  {
        // ����ȫ������

        DXGI_SWAP_CHAIN_FULLSCREEN_DESC fullscreenDesc;
        // ����ˢ����60FPS
        fullscreenDesc.RefreshRate.Numerator = 60;
        fullscreenDesc.RefreshRate.Denominator = 1;
        // ɨ�跽��
        fullscreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
        // ���ŷ���
        fullscreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
        // ȫ����ʾ
        fullscreenDesc.Windowed = FALSE;

        //IDXGIOutput* pRestrictToOutput = nullptr;
        hr = pDxgiFactory->CreateSwapChainForHwnd(
            pD3DDevice,
            m_hwnd,
            &swapChainDesc,
            false ? &fullscreenDesc : nullptr,
            nullptr,
            &m_pSwapChain
            );
    }
    // ȷ��DXGI������߲��ᳬ��һ֡T
    if (SUCCEEDED(hr))
    {
        hr = pDxgiDevice->SetMaximumFrameLatency(1);
    }

    // Get the backbuffer for this window which is be the final 3D render target.
    if (SUCCEEDED(hr))
    {
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    }
    // Now we set up the Direct2D render target bitmap linked to the swapchain. 
    // Whenever we render to this bitmap, it is directly rendered to the 
    // swap chain associated with the window.

    D2D1_BITMAP_PROPERTIES1 bitmapProperties =
        D2D1::BitmapProperties1(
        D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
        D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
        m_dpi,
        m_dpi
        );
    // Direct2D needs the dxgi version of the backbuffer surface pointer.
    if (SUCCEEDED(hr))
    {
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDxgiBackBuffer));
    }
    // Get a D2D surface from the DXGI back buffer to use as the D2D render target.
    if (SUCCEEDED(hr))
    {
        hr = m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pD2DTargetBimtap
            );
        // ���� �������� Direct2D ��ȾĿ�� ��.
        m_pD2DDeviceContext->SetTarget(m_pD2DTargetBimtap);
    }

    // �����̻�������
    if (SUCCEEDED(hr)){
        hr = m_pD2DDeviceContext->CreateBitmap(
            D2D1::SizeU(rect.right, rect.bottom),
            nullptr,
            0,
            &bitmapProperties,
            &m_pBufferBitmap
            );
    }

    // ������ɫ֡
    if (SUCCEEDED(hr)){
        D2D1_BITMAP_PROPERTIES1 bitmapProperties =
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            m_dpi,
            m_dpi
            );
        hr = m_pD2DDeviceContext->CreateBitmap(
            D2D1::SizeU(IMAGE_WIDTH, IMAGE_HEIGHT),
            nullptr,
            0,
            &bitmapProperties,
            &m_pColorFrame
            );
    }
    // 1.8 1.c 0.5
    if (SUCCEEDED(hr)){
        hr = m_pDWriteFactory->CreateCustomRenderingParams(1.8F, 0.5F, 1.F, DWRITE_PIXEL_GEOMETRY_RGB,
            DWRITE_RENDERING_MODE_OUTLINE, &m_pDWParams);
    }


    if (SUCCEEDED(hr)){
        m_pD2DDeviceContext->SetTextRenderingParams(m_pDWParams);
    }


    // �����ı���Ⱦ��
    PathTextRenderer::CreatePathTextRenderer(m_dpi / 96.f, &m_pTextRenderer);

    // ����Ц�������
    CreateLaughingMan();

    SafeRelease(pD3DDevice);
    SafeRelease(pD3DDeviceContext);
    SafeRelease(pDxgiDevice);
    SafeRelease(pDxgiAdapter);
    SafeRelease(pDxgiFactory);
    SafeRelease(pBackBuffer);
    SafeRelease(pDxgiBackBuffer);

    return hr;
}

// ImageRenderer��������
ImageRenderer::~ImageRenderer(){
    DiscardDeviceResources();

    SafeRelease(m_pSwapChain);
    SafeRelease(m_pTextAnimationPath);
    SafeRelease(m_pLaughingManGeometryBlue);
    SafeRelease(m_pLaughingManGeometryWhite);
    
    SafeRelease(m_pD2DFactory);
    SafeRelease(m_pWICFactory);
    SafeRelease(m_pDWParams);
    SafeRelease(m_pTextRenderer);
    SafeRelease(m_pTextLayoutLaughingMan);
    SafeRelease(m_pDWriteFactory);

}

// �����豸�����Դ
void ImageRenderer::DiscardDeviceResources(){
    SafeRelease(m_pLaughingBlueBrush);
    SafeRelease(m_pD2DDevice);
    SafeRelease(m_pD2DDeviceContext);
    SafeRelease(m_pD2DTargetBimtap);
    SafeRelease(m_pBufferBitmap);
    SafeRelease(m_pColorFrame);
}

// ��Ⱦͼ��ͼ��
HRESULT ImageRenderer::OnRender(UINT syn){
    HRESULT hr = E_POINTER;
    // ��̬���� ������ʾFPS
    static struct STaticData{
        STaticData(HWND hwnd){
            timer.Start();
            ::GetWindowTextW(hwnd, window_title_now, MAX_PATH);
        }
        PrecisionTimer timer;
        FLOAT fps;
        const UINT refresh_count = 30;
        UINT frame_counter = 0;
        WCHAR window_title_now[MAX_PATH];
    } data(this->m_hwnd);



    WCHAR window_title_buffer[MAX_PATH];

    if (this->m_pD2DDeviceContext){
        // ��ʼ
        this->m_pD2DDeviceContext->BeginDraw();
        // ����ת��
        const FLOAT zoom_base = static_cast<FLOAT>(WNDWIDTH) / static_cast<FLOAT>(IMAGE_WIDTH);
        this->m_pD2DDeviceContext->SetTransform(D2D1::Matrix3x2F::Scale(zoom_base, zoom_base));
        // �̻� 66CCFF
        this->m_pD2DDeviceContext->Clear(D2D1::ColorF::ColorF(0x66CCFF));
        // �̻���ɫͼ��
        this->m_pD2DDeviceContext->DrawBitmap(this->m_pColorFrame);
        // �̻�Ц����
        this->DrawLaughingMan();
        // �����̻�
        hr = this->m_pD2DDeviceContext->EndDraw();
        // �յ��ؽ���Ϣʱ(ps:D2DDeviceContext �������Ӧ�����ղ����� �Կ���������ʱ���յ�)
        if (hr == D2DERR_RECREATE_TARGET)
        {
#ifdef _DEBUG
            _cwprintf(L"�յ�D2D �ؽ���Ϣ : D2DERR_RECREATE_TARGET\n");
#else
            MessageBox(nullptr, L"�յ�Direct2D �ؽ���Ϣ,��������Ϊ��ʾ�����ĸı�����Կ�������������",
                L"�r�����������q ���� �r�����������q", MB_OK);
#endif

            hr = S_FALSE;
        }
        // �ֶ�����ǰ�󻺴���
        if (SUCCEEDED(hr)){
            hr = this->m_pSwapChain->Present1(syn, 0, &this->m_parameters);
            // ����FPS
            if (data.frame_counter > data.refresh_count){
                data.fps = (FLOAT)(data.frame_counter * 1000) / data.timer.DeltaF_ms();
                data.timer.MovStartEnd();
                data.timer.RefreshFrequency();
                swprintf_s(window_title_buffer, L"%s - %d.%02d FPS", data.window_title_now, (UINT)data.fps,
                    ((UINT)(data.fps*100.F)) % 100);
                SetWindowTextW(this->m_hwnd, window_title_buffer);
                data.frame_counter = 0;
            }
            ++data.frame_counter;
        }
    }
    return hr;
}


// ���ļ���ȡλͼ
HRESULT ImageRenderer::LoadBitmapFromFile(
    ID2D1DeviceContext *pRenderTarget,
    IWICImagingFactory2 *pIWICFactory,
    PCWSTR uri,
    UINT destinationWidth,
    UINT destinationHeight,
    ID2D1Bitmap1 **ppBitmap
    )
{
    IWICBitmapDecoder *pDecoder = nullptr;
    IWICBitmapFrameDecode *pSource = nullptr;
    IWICStream *pStream = nullptr;
    IWICFormatConverter *pConverter = nullptr;
    IWICBitmapScaler *pScaler = nullptr;

    HRESULT hr = pIWICFactory->CreateDecoderFromFilename(
        uri,
        nullptr,
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
                        nullptr,
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
                nullptr,
                0.f,
                WICBitmapPaletteTypeMedianCut
                );
        }
    }
    if (SUCCEEDED(hr))
    {
        hr = pRenderTarget->CreateBitmapFromWicBitmap(
            pConverter,
            nullptr,
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



// ����Ц�������
HRESULT ImageRenderer::CreateLaughingMan(){
    // �����뾶
    const FLOAT BASE_RADIUS = 135.f;

    HRESULT hr = S_OK;
    IDWriteTextFormat* pImpactFormat = nullptr;
    // ����Impact�ı���ʽ
    hr = m_pDWriteFactory->CreateTextFormat(
        L"Impact",
        nullptr,
        DWRITE_FONT_WEIGHT_NORMAL,
        DWRITE_FONT_STYLE_NORMAL,
        DWRITE_FONT_STRETCH_CONDENSED,
        41.f/96.f*72.f,
        L"",
        &pImpactFormat
        );

    // �����ı�����
    if (SUCCEEDED(hr)){
        pImpactFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);
        WCHAR* text = L"I thought what I'd do was, I'd pretend I was one of those deaf-mutes";
        auto length = wcslen(text);
        hr = m_pDWriteFactory->CreateTextLayout(text, length, pImpactFormat, BASE_RADIUS, BASE_RADIUS, &m_pTextLayoutLaughingMan);
    }
    // �����ı�����·��: һ��Բ
    if (SUCCEEDED(hr)){
        D2D1_ELLIPSE ellipse;
        ellipse.point.x = 0.f;
        ellipse.point.y = 0.f;
        ellipse.radiusX = BASE_RADIUS;
        ellipse.radiusY = BASE_RADIUS;
        hr = m_pD2DFactory->CreateEllipseGeometry(&ellipse, &m_pTextAnimationPath);
    }
    // Ц����·��
    if (SUCCEEDED(hr)){
        hr = m_pD2DFactory->CreatePathGeometry(&m_pLaughingManGeometryBlue);
        // ����
        ID2D1GeometrySink* pSink = nullptr;
        if (SUCCEEDED(hr)){
            hr = m_pLaughingManGeometryBlue->Open(&pSink);
        }
        if (SUCCEEDED(hr)){
            auto nowPoint = D2D1::Point2F();
            pSink->SetFillMode(D2D1_FILL_MODE_WINDING);
            D2D1_ARC_SEGMENT arc;
            D2D1_BEZIER_SEGMENT bezier;
            arc.rotationAngle = 0.f;
            // <path d="m-8-119h16 l2,5h-20z"/>
            nowPoint.x = -8.f; nowPoint.y = -124.f;
            pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            nowPoint.x += 16.f;
            pSink->AddLine(nowPoint);
            nowPoint.x += 2.f; nowPoint.y += 5.f;
            pSink->AddLine(nowPoint);
            nowPoint.x -= 20.f;
            pSink->AddLine(nowPoint);
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            // <path d = "m-95-20v-20h255a40,40 0,0 1 0,80h-55v-20z" / >
            nowPoint.x = -105.f; nowPoint.y = -20.f;
            pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            nowPoint.y -= 20.f;
            pSink->AddLine(nowPoint);
            nowPoint.x += 270.f;
            pSink->AddLine(nowPoint);
            nowPoint.y += 80.f;
            arc.size.height = 40.f;
            arc.size.width = 40.f;
            arc.sweepDirection = D2D1_SWEEP_DIRECTION_CLOCKWISE;
            arc.point = nowPoint;
            arc.arcSize = D2D1_ARC_SIZE_SMALL;
            pSink->AddArc(&arc);
            nowPoint.x -= 55.f;
            pSink->AddLine(nowPoint);
            nowPoint.y -= 20.f;
            pSink->AddLine(nowPoint);
            nowPoint.x += 55.f;
            pSink->AddLine(nowPoint);
            nowPoint.y -= 40.f;
            arc.size.height = 20.f;
            arc.size.width = 20.f;
            arc.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
            arc.point = nowPoint;
            pSink->AddArc(&arc);
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            // <path d="m-85 0a85,85 0,0 0 170,0h-20a65,65 0,0 1-130,0z"/>
            nowPoint.x = -85.f; nowPoint.y= 20.f;
            pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            nowPoint.x += 170.f;
            arc.size.height = 90.f;
            arc.size.width = 90.f;
            arc.sweepDirection = D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE;
            arc.arcSize = D2D1_ARC_SIZE_SMALL;
            arc.point = nowPoint;
            pSink->AddArc(&arc);
            nowPoint.x -= 20.f;
            pSink->AddLine(nowPoint);
            nowPoint.x -= 130.f;
            arc.size.height = 70.f;
            arc.size.width = 70.f;
            arc.sweepDirection = D2D1_SWEEP_DIRECTION_CLOCKWISE;
            arc.point = nowPoint;
            pSink->AddArc(&arc);
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            // <path d="m-65 20v20h130v-20z"/>  
            nowPoint.x = -65.f; nowPoint.y = 20.f;
            pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            nowPoint.y += 20.f;
            pSink->AddLine(nowPoint);
            nowPoint.x += 130.f;
            pSink->AddLine(nowPoint);
            nowPoint.y -= 20.f;
            pSink->AddLine(nowPoint);
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            //pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            // <path d = "m-20 10c-17-14-27-14-44 0 6-25 37-25 44 0z" / >
            nowPoint.x = -20.f; nowPoint.y = 10.f;
            pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            bezier.point1.x = nowPoint.x - 17.f;
            bezier.point1.y = nowPoint.y - 14.f;
            bezier.point2.x = nowPoint.x - 27.f;
            bezier.point2.y = nowPoint.y - 14.f;
            nowPoint.x -= 44.f;
            bezier.point3 = nowPoint;
            pSink->AddBezier(&bezier);
            bezier.point1.x = nowPoint.x + 6.f;
            bezier.point1.y = nowPoint.y - 25.f;
            bezier.point2.x = nowPoint.x + 37.f;
            bezier.point2.y = nowPoint.y - 25.f;
            nowPoint.x += 44.f;
            bezier.point3 = nowPoint;
            pSink->AddBezier(&bezier);
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            // <path d = "m60 10c-17-14-27-14-44 0 6-25 37-25 44 0z" / >
            nowPoint.x = 60.f; nowPoint.y = 10.f;
            pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            bezier.point1.x = nowPoint.x - 17.f;
            bezier.point1.y = nowPoint.y - 14.f;
            bezier.point2.x = nowPoint.x - 27.f;
            bezier.point2.y = nowPoint.y - 14.f;
            nowPoint.x -= 44.f;
            bezier.point3 = nowPoint;
            pSink->AddBezier(&bezier);
            bezier.point1.x = nowPoint.x + 6.f;
            bezier.point1.y = nowPoint.y - 25.f;
            bezier.point2.x = nowPoint.x + 37.f;
            bezier.point2.y = nowPoint.y - 25.f;
            nowPoint.x += 44.f;
            bezier.point3 = nowPoint;
            pSink->AddBezier(&bezier);
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            hr = pSink->Close();
        }
        SafeRelease(pSink);
    }
    // Ц���а�ɫ����
    if (SUCCEEDED(hr)){
        hr = m_pD2DFactory->CreatePathGeometry(&m_pLaughingManGeometryWhite);
        // ����
        ID2D1GeometrySink* pSink = nullptr;
        if (SUCCEEDED(hr)){
            hr = m_pLaughingManGeometryWhite->Open(&pSink);
        }
        if (SUCCEEDED(hr)){
            auto nowPoint = D2D1::Point2F();
            // <path d = "m-115-20v10h25v30h250a20,20 0,0 0 0,-40z" fill = "#fff" / >
            nowPoint.x = -125.f; nowPoint.y = -20.f;
            pSink->BeginFigure(nowPoint, D2D1_FIGURE_BEGIN_FILLED);
            nowPoint.y += 10.f;
            pSink->AddLine(nowPoint);
            nowPoint.x += 35.f;
            pSink->AddLine(nowPoint);
            nowPoint.y += 30.f;
            pSink->AddLine(nowPoint);
            nowPoint.x += 260.f;
            pSink->AddLine(nowPoint);
            nowPoint.y -= 40.f;
            D2D1_ARC_SEGMENT arc = { nowPoint, D2D1::SizeF(20.f, 20.f), 0.f, D2D1_SWEEP_DIRECTION_COUNTER_CLOCKWISE, D2D1_ARC_SIZE_SMALL };
            pSink->AddArc(&arc);
            pSink->EndFigure(D2D1_FIGURE_END_CLOSED);
            hr = pSink->Close();
        }
    }
    // Ц����
    if (SUCCEEDED(hr)){
        hr = m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0x005577), &m_pLaughingBlueBrush);
    }
    // Ц���
    if (SUCCEEDED(hr)){
        hr = m_pD2DDeviceContext->CreateSolidColorBrush(D2D1::ColorF(0xFFFFFF), &m_pLaughingWhiteBrush);
    }
    SafeRelease(pImpactFormat);
    return hr;
}


// �̻�Ц����
void ImageRenderer::DrawLaughingMan(){
    D2D1_MATRIX_3X2_F org_matrix, tra_matrix, tex_matrix;
    m_pD2DDeviceContext->GetTransform(&org_matrix);
    for (int i = 0; i < BODY_COUNT; ++i) {
        // ����ת��
        tra_matrix = D2D1::Matrix3x2F::Scale(D2D1::SizeF(m_men[i].zoom, m_men[i].zoom))*
            D2D1::Matrix3x2F::Translation(m_men[i].pos) * org_matrix;
        m_pD2DDeviceContext->SetTransform(&tra_matrix);
        if (m_men[i].man == LaughingMan::Offline) continue;
#ifdef _DEBUG
        _cwprintf(L"LaughingMan@%d@%4d, %4d\n", i, (int)m_men[i].pos.width, (int)m_men[i].pos.height);
#endif
        D2D1_ELLIPSE ellopse;
        ellopse.point.x = 0.f;
        ellopse.point.y = 0.f;
        ellopse.radiusX = 170.f;
        ellopse.radiusY = 170.f;
        // ���Ц������Բ
        m_pD2DDeviceContext->FillEllipse(&ellopse, m_pLaughingBlueBrush);
        // ���Ц��״δ�Բ
        ellopse.radiusX = 162.f;
        ellopse.radiusY = 162.f;
        m_pD2DDeviceContext->FillEllipse(&ellopse, m_pLaughingWhiteBrush);
        // �̻�����
        PathTextDrawingContext context;
        context.pBrush = m_pLaughingBlueBrush;
        context.pGeometry = m_pTextAnimationPath;
        context.pRenderTarget = m_pD2DDeviceContext;
        ellopse.radiusX = 110.f;
        ellopse.radiusY = 110.f;
        tex_matrix = tra_matrix * D2D1::Matrix3x2F::Rotation(m_men[i].angle, D2D1::Point2F(tra_matrix._31, tra_matrix._32));
        m_pD2DDeviceContext->SetTransform(&tex_matrix);
        m_pTextLayoutLaughingMan->Draw(&context, m_pTextRenderer, 0.f, 0.f);
        m_pD2DDeviceContext->SetTransform(&tra_matrix);
        // �̻�СԲ
        m_pD2DDeviceContext->DrawEllipse(&ellopse, m_pLaughingBlueBrush, 20.f);
        // ��串�ǰ�
        m_pD2DDeviceContext->FillGeometry(m_pLaughingManGeometryWhite, m_pLaughingWhiteBrush);
        // ��串����
        m_pD2DDeviceContext->FillGeometry(m_pLaughingManGeometryBlue, m_pLaughingBlueBrush);
    }
    m_pD2DDeviceContext->SetTransform(&org_matrix);
}