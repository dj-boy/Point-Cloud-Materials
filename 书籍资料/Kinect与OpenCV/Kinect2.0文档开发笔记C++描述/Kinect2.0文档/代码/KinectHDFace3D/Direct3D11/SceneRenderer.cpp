#include "stdafx.h"
#include "included.h"

// D3D11 ��Ⱦ����
// * 1. IA:  Input Assembler Stage ����װ��׶�
// *    



// SceneRenderer �� ���캯��
SceneRenderer::SceneRenderer(){
    // ��ʼ�����ֲ��� �����....�ø߼�
    m_parameters.DirtyRectsCount = 0U;
    m_parameters.pDirtyRects = nullptr;
    m_parameters.pScrollOffset = nullptr;
    m_parameters.pScrollRect = nullptr;

    // �豸�޹���Դ
    m_hrDIR = this->CreateDeviceIndependentResources();

    // ��������
    if (SUCCEEDED(m_hrDIR)){
        // ����������
        m_pColorRGBX = new RGBQUAD[IMAGE_WIDTH*IMAGE_HEIGHT];
        if (!m_pColorRGBX) m_hrDIR = E_OUTOFMEMORY;
    }
}

// SceneRenderer �� ��������
SceneRenderer::~SceneRenderer(){
    this->DiscardDeviceResources();
    SafeRelease(m_pStandardTF);
    SafeRelease(m_pDWriteFactory);
    SafeRelease(m_pD2DFactory);
#ifdef _DEBUG
    if (this->debug_info){
        delete[] this->debug_info;
        this->debug_info = nullptr;
    }
#endif
    if (m_pColorRGBX){
        delete[] m_pColorRGBX;
        m_pColorRGBX = nullptr;
    }
}

// �����豸�޹���Դ
HRESULT SceneRenderer::CreateDeviceIndependentResources(){
    HRESULT hr = S_OK;
    // ����DirectWrite����
    if (SUCCEEDED(hr)){
        hr = DWriteCreateFactory(
            DWRITE_FACTORY_TYPE_SHARED,
            __uuidof(m_pDWriteFactory),
            reinterpret_cast<IUnknown **>(&m_pDWriteFactory)
            );
    }
    // ���� D2D ����
    if (SUCCEEDED(hr)) {
        hr = D2D1CreateFactory(
            D2D1_FACTORY_TYPE_SINGLE_THREADED,
            IID_PPV_ARGS(&m_pD2DFactory)
            );

    }
    // ������׼�ı���ʽ
    if (SUCCEEDED(hr)){
        hr = m_pDWriteFactory->CreateTextFormat(
            L"Consolas",
            nullptr,
            DWRITE_FONT_WEIGHT_NORMAL,
            DWRITE_FONT_STYLE_NORMAL,
            DWRITE_FONT_STRETCH_NORMAL,
            23.3333f * 2.33333f,
            L"",
            &m_pStandardTF
            );
    }
    // ���ж���
    if (SUCCEEDED(hr)){
        m_pStandardTF->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
        m_pStandardTF->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    }
    return hr;
}

// SceneRenderer �� �����豸�����Դ
HRESULT SceneRenderer::CreateDeviceResources(){
    HRESULT hr = S_OK;
    // DXGI �豸
    IDXGIDevice1*                        pDxgiDevice = nullptr;
    // DXGI ������
    IDXGIAdapter*                        pDxgiAdapter = nullptr;
    // DXGI ����
    IDXGIFactory2*                        pDxgiFactory = nullptr;
    // �󱸻���֡
    ID3D11Texture2D*                    pBackBuffer = nullptr;
    // �󱸻���֡
    IDXGISurface*                        pDxgiBackBuffer = nullptr;
#pragma region ��ʼ�� Direct3D
    // ����D3D�豸 ����ֱ��ʹ��D3D11CreateDeviceAndSwapChain
    if (SUCCEEDED(hr)){
#ifdef USING_DIRECT2D
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#else
        UINT creationFlags = 0;
#endif
#ifdef _DEBUG
        // Debug״̬ ��D3D DebugLayer��ȡ��ע��
        //creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
        D3D_FEATURE_LEVEL featureLevels[] =  {
            D3D_FEATURE_LEVEL_11_1,
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_1,
            D3D_FEATURE_LEVEL_10_0,
            D3D_FEATURE_LEVEL_9_3,
            D3D_FEATURE_LEVEL_9_2,
            D3D_FEATURE_LEVEL_9_1
        };
        hr = D3D11CreateDevice(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            0,
            creationFlags,
            featureLevels,
            lengthof(featureLevels),
            D3D11_SDK_VERSION,
            &m_pD3DDevice,
            &m_featureLevel,
            &m_pD3DDeviceContext
            );
        // ��ʾ�Կ�����
        if (FAILED(hr)){
            ::MessageBoxW(m_hwnd, L"����ʾ��������֧��DX11", L"DX11", MB_OK);
        }
    }
    // ��ȡ DXGI �豸
    if (SUCCEEDED(hr)) {
        hr = m_pD3DDevice->QueryInterface(IID_PPV_ARGS(&pDxgiDevice));
    }
    // ��ȡDxgi ������
    if (SUCCEEDED(hr)) {
        hr = pDxgiDevice->GetAdapter(&pDxgiAdapter);
    }
    // ��ȡDxgi ����
    if (SUCCEEDED(hr)) {
        hr = pDxgiAdapter->GetParent(IID_PPV_ARGS(&pDxgiFactory));
    }
    // ��ֹ Alt + Enter ȫ��
    if (SUCCEEDED(hr)){
        hr = pDxgiFactory->MakeWindowAssociation(m_hwnd, DXGI_MWA_NO_WINDOW_CHANGES | DXGI_MWA_NO_ALT_ENTER);
    }
    // ��ȡ���ڴ�С
    RECT rect;
    ::GetClientRect(m_hwnd, &rect);
    rect.right -= rect.left;
    rect.bottom -= rect.top;
    // �������ڽ�����
    if (SUCCEEDED(hr))  {
        // ������������
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
        swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
        swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
        swapChainDesc.Flags = 0;

        hr = pDxgiFactory->CreateSwapChainForHwnd(
            m_pD3DDevice,
            m_hwnd,
            &swapChainDesc,
            nullptr,
            nullptr,
            &m_pSwapChain
            );
    }
    // ��ȡ����֡ Texeture2D
    if (SUCCEEDED(hr)){
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    }
    // ������ȾĿ��
    if (SUCCEEDED(hr)){
        hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer, nullptr, &m_pD3DRTView);
    }
    // ������Ȼ���
    if (SUCCEEDED(hr)){
        // ��ʼ����Ȼ�������.
        D3D11_TEXTURE2D_DESC depthBufferDesc = { 0 };
        // ������Ȼ�������
        depthBufferDesc.Width = rect.right;
        depthBufferDesc.Height = rect.bottom;
        depthBufferDesc.MipLevels = 1;
        depthBufferDesc.ArraySize = 1;
        depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthBufferDesc.SampleDesc.Count = 1;
        depthBufferDesc.SampleDesc.Quality = 0;
        depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;
        depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthBufferDesc.CPUAccessFlags = 0;
        depthBufferDesc.MiscFlags = 0;
        // ������Ȼ���
        hr = m_pD3DDevice->CreateTexture2D(
            &depthBufferDesc, 
            nullptr, 
            &m_pT2DDepthBuffer
            );
    }
    // �������ģ����ͼ
    if (SUCCEEDED(hr)){
        hr = m_pD3DDevice->CreateDepthStencilView(
            m_pT2DDepthBuffer, 
            nullptr, 
            &m_pDepthStencilView);
    }
    // ���øû���֡(�����������)Ϊ���Ŀ�� OM =  Output-Merger Stage �ϲ�����׶�
    if (SUCCEEDED(hr)){
        m_pD3DDeviceContext->OMSetRenderTargets(1, &m_pD3DRTView, m_pDepthStencilView);
    }
    // ����
    if (SUCCEEDED(hr)){
        D3D11_RASTERIZER_DESC rasterDesc;
        // ���ù�դ��������ָ���������α���Ⱦ.
        rasterDesc.AntialiasedLineEnable = false;
        rasterDesc.CullMode = D3D11_CULL_BACK;
        rasterDesc.DepthBias = 0;
        rasterDesc.DepthBiasClamp = 0.0f;
        rasterDesc.DepthClipEnable = true;
        rasterDesc.FillMode = D3D11_FILL_SOLID;
        rasterDesc.FrontCounterClockwise = false;
        rasterDesc.MultisampleEnable = false;
        rasterDesc.ScissorEnable = false;
        rasterDesc.SlopeScaledDepthBias = 0.0f;
        // ������դ��״̬
        hr = m_pD3DDevice->CreateRasterizerState(&rasterDesc, &m_pRasterizerState);
    }
    // ����Ϊ��ǰ�Ĺ�դ��״̬ RS =  Rasterizer Stage  ��դ���׶�
    if (SUCCEEDED(hr)){
        m_pD3DDeviceContext->RSSetState(m_pRasterizerState);
    }
    // �����ӿ�
    if (SUCCEEDED(hr)){
        D3D11_VIEWPORT viewPort = { 0 };
        viewPort.Width = static_cast<FLOAT>(rect.right);
        viewPort.Height = static_cast<FLOAT>(rect.bottom);
        viewPort.MinDepth = 0.f;
        viewPort.MaxDepth = 1.f;
        viewPort.TopLeftX = 0.f;
        viewPort.TopLeftY = 0.f;
        m_pD3DDeviceContext->RSSetViewports(1, &viewPort);
    }
    // ����ģ��
    if (SUCCEEDED(hr)){
        hr = this->face_model.Create(m_pD3DDevice);
    }
#pragma endregion

#pragma region ��ʼ�� Direct2D
    // ��ȡ����֡ DXGI Surface
    if (SUCCEEDED(hr)){
        hr = m_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pDxgiBackBuffer));
    }
    // ����D2D�豸
    if (SUCCEEDED(hr)) {
        hr = m_pD2DFactory->CreateDevice(pDxgiDevice, &m_pD2DDevice);
    }
    // ����D2D�豸������
    if (SUCCEEDED(hr)) {
        // ��ʱ����D2D�豸
        hr = m_pD2DDevice->CreateDeviceContext(
            D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
            &m_pD2DDeviceContext
            );
    }
    // ������׼��ˢ
    if (SUCCEEDED(hr)){
        hr = m_pD2DDeviceContext->CreateSolidColorBrush(
            D2D1::ColorF(D2D1::ColorF::DarkOrange),
            &m_pStandardBrush
            );
    }
    // ������ɫ֡
    if (SUCCEEDED(hr)){
        D2D1_BITMAP_PROPERTIES1 bitmapProperties =
            D2D1::BitmapProperties1(D2D1_BITMAP_OPTIONS_NONE,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.f,
            96.f
            );
        hr = m_pD2DDeviceContext->CreateBitmap(
            D2D1::SizeU(IMAGE_WIDTH, IMAGE_HEIGHT),
            nullptr,
            0,
            &bitmapProperties,
            &m_pDrawBitmap
            );
    }
    // ����D2D λͼ
    if (SUCCEEDED(hr)){
        D2D1_BITMAP_PROPERTIES1 bitmapProperties =
            D2D1::BitmapProperties1(
            D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
            D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED),
            96.f,
            96.f
            );
        // ����
        hr = m_pD2DDeviceContext->CreateBitmapFromDxgiSurface(
            pDxgiBackBuffer,
            &bitmapProperties,
            &m_pD2DTargetBimtap
            );
        // ����D2D��ȾĿ��
        m_pD2DDeviceContext->SetTarget(m_pD2DTargetBimtap);
        // ��������Ϊ��λ
        m_pD2DDeviceContext->SetUnitMode(D2D1_UNIT_MODE_PIXELS);
    }
#pragma endregion

    SafeRelease(pDxgiDevice);
    SafeRelease(pDxgiAdapter);
    SafeRelease(pDxgiFactory);
    SafeRelease(pBackBuffer);
    SafeRelease(pDxgiBackBuffer);
    return hr;
}


// �����豸��Դ
void SceneRenderer::DiscardDeviceResources(){

    SafeRelease(m_pDrawBitmap);
    SafeRelease(m_pD2DTargetBimtap);
    SafeRelease(m_pStandardBrush);
    SafeRelease(m_pD2DDeviceContext);
    SafeRelease(m_pD2DDevice);

    SafeRelease(m_pRasterizerState);
    SafeRelease(m_pDepthStencilView);
    SafeRelease(m_pD3DRTView);
    SafeRelease(m_pT2DDepthBuffer);
    SafeRelease(m_pSwapChain);
    SafeRelease(m_pD3DDeviceContext);
    SafeRelease(m_pD3DDevice);
}

// ��Ⱦ
HRESULT SceneRenderer::OnRender(){
    // ������ɫ
    FLOAT color[] = { 
        static_cast<float>(0x66) / 255.f,
        static_cast<float>(0xCC) / 255.f,
        static_cast<float>(0xFF) / 255.f,
        1.f
    };
    // ����
    m_pD3DDeviceContext->ClearRenderTargetView(m_pD3DRTView, color);
    m_pD3DDeviceContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
    // ��Ⱦ�ײ���ɫ֡
    m_pD2DDeviceContext->BeginDraw();
    D2D1_RECT_F rect = { 0.f };
    reinterpret_cast<D2D1_SIZE_F&>(rect.right) = m_pD2DDeviceContext->GetSize();
    m_pD2DDeviceContext->DrawBitmap(
        m_pDrawBitmap, &rect
        );
    m_pD2DDeviceContext->EndDraw();

    // ��Ⱦ�沿
    face_model.Render(m_pD3DDeviceContext);
    // �̻���Ϣ
    draw_info();

    // ����
    //m_pSwapChain->Present1(1, 0, &this->m_parameters);
    HRESULT hr = m_pSwapChain->Present(1, 0);
    // �豸��ʧ?
    if (hr == DXGI_ERROR_DEVICE_REMOVED){
#ifdef _DEBUG
        _cwprintf(L"DXGI_ERROR_DEVICE_REMOVED: D3D11 �豸��ʧ\n");
#endif
        // �����豸��ʧ 
        hr = handle_device_lost();
    }
    return hr;
}



// �̻���Ϣ
void SceneRenderer::draw_info(){
    m_pD2DDeviceContext->BeginDraw();
    D2D1_RECT_F rect = { 0 };
    reinterpret_cast<D2D1_SIZE_F&>(rect.right) = m_pD2DDeviceContext->GetSize();
    // ��ʾ��Ϣ
    if (this->face_model.co_status == FaceModelBuilderCollectionStatus_Complete){
        m_pD2DDeviceContext->DrawText(
            L"�沿��Ϣ��Ϣ�Ѽ����, ��л�������\n����S���ɱ�����˲���沿ģ��",
            lengthof(L"�沿��Ϣ��Ϣ�Ѽ����, ��л�������\n����S���ɱ�����˲���沿ģ��") - 1,
            m_pStandardTF,
            rect,
            m_pStandardBrush
            );
    }
    // �������ҿ��ֲܷ������ ����׷��LR��ĸ
    else if (this->face_model.co_status & FaceModelBuilderCollectionStatus_LeftViewsNeeded){
        m_pD2DDeviceContext->DrawText(
            L"������(L)�߿���, �����Ѽ��沿��Ϣ",
            lengthof(L"������(L)�߿���, �������Ѽ��沿��Ϣ") - 1,
            m_pStandardTF,
            rect,
            m_pStandardBrush
            );
    }
    else if (this->face_model.co_status & FaceModelBuilderCollectionStatus_RightViewsNeeded){
        m_pD2DDeviceContext->DrawText(
            L"������(R)�߿���, �������Ѽ��沿��Ϣ",
            lengthof(L"������(R)�߿���, �������Ѽ��沿��Ϣ") - 1,
            m_pStandardTF,
            rect,
            m_pStandardBrush
            );
    }
    else if (this->face_model.co_status & FaceModelBuilderCollectionStatus_FrontViewFramesNeeded){
        m_pD2DDeviceContext->DrawText(
            L"������Kinect, �������Ѽ��沿��Ϣ",
            lengthof(L"������Kinect, �������Ѽ��沿��Ϣ") - 1,
            m_pStandardTF,
            rect,
            m_pStandardBrush
            );
    }
    else if (this->face_model.co_status & FaceModelBuilderCollectionStatus_TiltedUpViewsNeeded){
        m_pD2DDeviceContext->DrawText(
            L"��̧ͷ���Ͽ�, �����Ѽ��沿��Ϣ",
            lengthof(L"��̧ͷ���Ͽ�, �����Ѽ��沿��Ϣ") - 1,
            m_pStandardTF,
            rect,
            m_pStandardBrush
            );
    }
    m_pD2DDeviceContext->EndDraw();
}



// D3D11 �豸��ʧ
HRESULT SceneRenderer::handle_device_lost(){
    // ����Ͳ�������
    return S_OK;
}




// д������
void SceneRenderer::WriteBitmapData(RGBQUAD* data, int width, int height){
    D2D1_RECT_U rect = { 0, 0, width, height };
    m_pDrawBitmap->CopyFromMemory(
        &rect,
        data,
        width* sizeof(RGBQUAD)
        );
}




// ceneRenderer::FaceModel ����
SceneRenderer::FaceModel::FaceModel(){
    //  ---- ����ת������
    DirectX::XMMATRIX mat = DirectX::XMMatrixIdentity();
    // ����ת��
    DirectX::XMStoreFloat4x4(
        &this->vscb.world,
        mat
        );

    // �ӽ��ӽ�ת��
    DirectX::XMVECTOR position = { 0.f, 0.f, 0.f, 1.f };
    DirectX::XMVECTOR look_at = { 0.f, 0.f, 1.f, 1.f };
    DirectX::XMVECTOR up = { 0.f, 1.f, 0.f, 1.f };
    mat = DirectX::XMMatrixLookAtLH(position, look_at, up);
    DirectX::XMStoreFloat4x4(
        &this->vscb.view,
        mat
        );

    // ---- ���ò���
    pscb.Ke = DirectX::XMFLOAT4(0.8f, 0.0f, 0.2f, 1.0f);
    pscb.Ka = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    pscb.Kd = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    pscb.Ks = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    pscb.lightDirection = DirectX::XMFLOAT3(-1.0f, -1.0f, 1.0f);
    pscb.lightPosition = DirectX::XMFLOAT4(5.0f, 5.0f, -3.0f, 1.0f);
    pscb.lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    pscb.globalAmbient = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    pscb.cameraPosition = DirectX::XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f);
    pscb.shininess = 10.f;
}

// SceneRenderer::FaceModel ����
HRESULT SceneRenderer::FaceModel::Create(ID3D11Device* device){
    //if (!device) return ;
    D3D11_BUFFER_DESC buffer_desc = { 0 };
    buffer_desc.ByteWidth = this->vertex_count * sizeof(SceneRenderer::VertexNormal);
    buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    // ÿ֡����ͬ ������ҪGPU�ɶ� CPU��д
    buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
    buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    // �������㻺��
    HRESULT hr = device->CreateBuffer(
        &buffer_desc,
        nullptr,
        &m_pVertexBuffer
        );
    // ������������
    if (SUCCEEDED(hr)){
        buffer_desc = {
            index_count * sizeof(UINT32),
            D3D11_USAGE_IMMUTABLE,
            D3D11_BIND_INDEX_BUFFER,
            0,
            0,
            0
        };
        // ����Դ����
        D3D11_SUBRESOURCE_DATA ibData = { 0 };
        ibData.pSysMem = index;
        //�������������ݴ�����������
        hr = device->CreateBuffer(&buffer_desc, &ibData, &m_pIndexBuffer);
    }
    // ��ȡ������ɫ��CSO�ļ�
    if (SUCCEEDED(hr)){
        hr = FileLoader.ReadFile(L"SimpleLightingVS.cso") ? S_OK : STG_E_FILENOTFOUND;
    }
    // ����������ɫ��
    if (SUCCEEDED(hr)){
        hr = device->CreateVertexShader(
            FileLoader.GetData(),
            FileLoader.GetLength(),
            nullptr,
            &m_pVertexShader
            );
    }
    // ����λ��
    D3D11_INPUT_ELEMENT_DESC inputele_desc[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
    // �������벼��
    if (SUCCEEDED(hr)){
        hr = device->CreateInputLayout(
            inputele_desc,
            lengthof(inputele_desc),
            FileLoader.GetData(),
            FileLoader.GetLength(),
            &m_pInputLayout
            );
    }
    // ��ȡ������ɫ��CSO�ļ�
    if (SUCCEEDED(hr)){
        hr = FileLoader.ReadFile(L"SimpleLightingGS.cso") ? S_OK : STG_E_FILENOTFOUND;
    }
    // ����������ɫ��
    if (SUCCEEDED(hr)){
        hr = device->CreateGeometryShader(
            FileLoader.GetData(),
            FileLoader.GetLength(),
            nullptr,
            &m_pGeometryShader
            );
    }
    // ��ȡ������ɫ��CSO�ļ�
    if (SUCCEEDED(hr)){
        hr = FileLoader.ReadFile(L"SimpleLightingPS.cso") ? S_OK : STG_E_FILENOTFOUND;
    }
    // ����������ɫ��
    if (SUCCEEDED(hr)){
        hr = device->CreatePixelShader(
            FileLoader.GetData(),
            FileLoader.GetLength(),
            nullptr,
            &m_pPixelShader
            );
    }
    // ����VS��������
    if (SUCCEEDED(hr)){
        // ��������: �������ĳ���(CPUд GPU��)����
        buffer_desc = {
            sizeof(MatrixBufferType),
            D3D11_USAGE_DYNAMIC,
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_CPU_ACCESS_WRITE,
            0,
            0
        };
        // ����
        hr = device->CreateBuffer(&buffer_desc, nullptr, &m_pVSCBuffer);
    }
    // ����PS��������
    if (SUCCEEDED(hr)){
        // ��������: ������յĳ���(CPUд GPU��)����
        buffer_desc = {
            sizeof(LightMaterialBufferType),
            D3D11_USAGE_DYNAMIC,
            D3D11_BIND_CONSTANT_BUFFER,
            D3D11_CPU_ACCESS_WRITE,
            0,
            0
        };
        // ����
        hr = device->CreateBuffer(&buffer_desc, nullptr, &m_pPSCBuffer);
    }
    // ˳�������״�����
    if (SUCCEEDED(hr)){
        ID3D11DeviceContext* context = nullptr;
        device->GetImmediateContext(&context);
        if (device){
            this->RefreshCB(context);
            this->RefreshVB(context);
        }
        SafeRelease(context);
    }
    return hr;
}


// SceneRenderer::FaceModel ��Ⱦ
void SceneRenderer::FaceModel::Render(ID3D11DeviceContext* context){
    //if (!context) return;
    // ---- װ�䶥��
    // ���ö��㻺���Ⱥ�ƫ��.
    UINT offset = 0;
    UINT stride = sizeof(SceneRenderer::VertexNormal);
    // IA: ���뱾ʵ���Ķ��㻺��
    context->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
    // IA : ������������
    context->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
    // ���˽ṹ: ������
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    // ---- װ����ɫ������
    // ���ö�����ɫ���������� д�볣������Ĵ���0 (b0)
    context->VSSetConstantBuffers(0, 1, &m_pVSCBuffer);
    // ����������ɫ���������� д�볣������Ĵ���0 (b0)
    context->PSSetConstantBuffers(0, 1, &m_pPSCBuffer);
    // ���ò���
    context->IASetInputLayout(m_pInputLayout);
    // ����VS
    context->VSSetShader(m_pVertexShader, nullptr, 0);
    // ����PS
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    // ---- װ����ɫ��
    // ���ò���
    context->IASetInputLayout(m_pInputLayout);
    // ����VS
    context->VSSetShader(m_pVertexShader, nullptr, 0);
    // ����GS
    context->GSSetShader(m_pGeometryShader, nullptr, 0);
    // ����PS
    context->PSSetShader(m_pPixelShader, nullptr, 0);
    // ---- ��Ⱦ
    context->DrawIndexed(index_count,0, 0);
}


// ��������
void SceneRenderer::FaceModel::RefreshCB(ID3D11DeviceContext* context){
    D3D11_MAPPED_SUBRESOURCE mapped_subsource;
    // ���¶�����ɫ����������
    HRESULT hr = context->Map(m_pVSCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subsource);
    
    // д������
    if (SUCCEEDED(hr)){
        auto* data = reinterpret_cast<MatrixBufferType*>(mapped_subsource.pData);
        // D3D11 Ҫ��: ����ת��
        DirectX::XMMATRIX mat_temp;
        mat_temp = DirectX::XMLoadFloat4x4(&vscb.projection);
        mat_temp = DirectX::XMMatrixTranspose(mat_temp);
        DirectX::XMStoreFloat4x4(&data->projection, mat_temp);

        mat_temp = DirectX::XMLoadFloat4x4(&vscb.view);
        mat_temp = DirectX::XMMatrixTranspose(mat_temp);
        DirectX::XMStoreFloat4x4(&data->view, mat_temp);

        mat_temp = DirectX::XMLoadFloat4x4(&vscb.world);
        mat_temp = DirectX::XMMatrixTranspose(mat_temp);
        DirectX::XMStoreFloat4x4(&data->world, mat_temp);
        // ����ӳ��
        context->Unmap(m_pVSCBuffer, 0);
    }


    // ����������ɫ������
    hr = context->Map(m_pPSCBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subsource);

    // д������
    if (SUCCEEDED(hr)){
        // д������
        memcpy(mapped_subsource.pData, &pscb, sizeof(pscb));
        // ����ӳ��
        context->Unmap(m_pPSCBuffer, 0);
    }

}


// ˢ�¶��㻺��
void SceneRenderer::FaceModel::RefreshVB(ID3D11DeviceContext* context){

    D3D11_MAPPED_SUBRESOURCE mapped_subsource;
    // ���¶��㻺��
    HRESULT hr = context->Map(m_pVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subsource);

    // д������
    if (SUCCEEDED(hr)){
        // д������
        memcpy(mapped_subsource.pData, face_mash_vertexs, sizeof(VertexNormal)*this->vertex_count);
        // ����ӳ��
        context->Unmap(m_pVertexBuffer, 0);
    }
}