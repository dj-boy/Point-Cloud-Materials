// SceneRenderer�� ������Ⱦ��

#pragma once

// ���ü�����
#define SCREEN_NEAR_Z (0.01f)
// Զ�ü�����
#define SCREEN_FAR_Z (100.f)

// Kinect
//#define IMAGE_WIDTH (1920)
//#define IMAGE_HEIGHT (1080)
// Depth
#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424




// ������Ⱦ
class SceneRenderer{
public:
    // ���㳣������
    struct MatrixBufferType {
        // ����ת��
        DirectX::XMFLOAT4X4   world;
        // �ӽ�ת��
        DirectX::XMFLOAT4X4   view;
        // ͸��ת��
        DirectX::XMFLOAT4X4   projection;
    };
    // ����ģ��
    class SceneModel{
    public:
        // ����
        SceneModel();
        // ����
        HRESULT Create(ID3D11Device* device);
        // ˢ�³�������
        void RefreshCB(ID3D11DeviceContext* context);
        // ˢ�¶��㻺��
        void RefreshVB(ID3D11DeviceContext* context);
        // ��ȡ���㻺��
        __forceinline CameraSpacePoint* GetVB(){ return m_pScenePoints; }
        // ��ȡ��������
        __forceinline const UINT GetVertexCount(){ return c_cVertexCount; }
        // ����
        ~SceneModel(){
            SafeRelease(m_pVertexBuffer);
            SafeRelease(m_pInputLayout);
            SafeRelease(m_pVSCBuffer);
            SafeRelease(m_pVertexShader);
            SafeRelease(m_pPixelShader);
            if (m_pScenePoints){
                delete[] m_pScenePoints;
                m_pScenePoints = nullptr;
            }
        }
        // ��Ⱦ
        void Render(ID3D11DeviceContext* context);
    private:
        // ģ�Ͷ��㻺��
        ID3D11Buffer*           m_pVertexBuffer = nullptr;
        // �������벼��
        ID3D11InputLayout*      m_pInputLayout = nullptr;
        // ������ɫ�� ��������
        ID3D11Buffer*           m_pVSCBuffer = nullptr;
        // ������ɫ��    
        ID3D11VertexShader*     m_pVertexShader = nullptr;
        // ������ɫ��
        ID3D11PixelShader*      m_pPixelShader = nullptr;
        // ��������
        const UINT              c_cVertexCount = DEPTH_WIDTH * DEPTH_HEIGHT;
        // �沿����
        CameraSpacePoint*       m_pScenePoints = new CameraSpacePoint[c_cVertexCount];
    public:
        // Vertex Shader CBuffer
        MatrixBufferType        vscb;
    };
public:
    // ���캯��
    SceneRenderer();
    // ��������
    ~SceneRenderer();
    // ��Ⱦ
    HRESULT OnRender();
    // ���ô��ھ��
    HRESULT SetHwnd(HWND hwnd){ m_hwnd = hwnd; return this->CreateDeviceResources(); }
    // �����豸�޹���Դ
    HRESULT CreateDeviceIndependentResources();
    // �����豸�����Դ
    HRESULT CreateDeviceResources();
    // �����豸��Դ
    void DiscardDeviceResources();
    // ˢ������
    void RefreshData(){
        scene_model.RefreshVB(m_pD3DDeviceContext);
    }
    // ��ȡ D3D �豸
    ID3D11Device* Get3DDevice(){
        if (m_pD3DDevice){
            m_pD3DDevice->AddRef();
        }
        return m_pD3DDevice;
    }
private:
    // �����豸��ʧ
    HRESULT handle_device_lost();
    // �̻���Ϣ
    void draw_info();
private:
    // �������
    HRESULT                             m_hrDIR = S_OK;
    // ���ھ��
    HWND                                m_hwnd = nullptr;
    // D3D11 �豸
    ID3D11Device*                       m_pD3DDevice = nullptr;
    // D2D11 �豸������
    ID3D11DeviceContext*                m_pD3DDeviceContext = nullptr;
    // DXGI ������
    IDXGISwapChain1*                    m_pSwapChain = nullptr;
    // D3D ��ȾĿ��
    ID3D11RenderTargetView*             m_pD3DRTView = nullptr;
    // D3D ��Ȼ���
    ID3D11Texture2D*                    m_pT2DDepthBuffer = nullptr;
    // D3D ���ģ����ͼ
    ID3D11DepthStencilView*             m_pDepthStencilView = nullptr;
    // ��դ��״̬
    ID3D11RasterizerState*              m_pRasterizerState = nullptr;
#ifdef USING_DIRECT2D
    // DWrite ����
    IDWriteFactory1*                    m_pDWriteFactory = nullptr;
    // D2D ����
    ID2D1Factory1*                      m_pD2DFactory = nullptr;
    // D2D �豸
    ID2D1Device*                        m_pD2DDevice = nullptr;
    // D2D �豸������
    ID2D1DeviceContext*                 m_pD2DDeviceContext = nullptr;
    // D2D ��ȾĿ��λͼ
    ID2D1Bitmap1*                       m_pD2DTargetBimtap = nullptr;
    // ��׼�ı���ʽ
    IDWriteTextFormat*                  m_pStandardTF = nullptr;
    // ��׼��ˢ
    ID2D1SolidColorBrush*               m_pStandardBrush = nullptr;
#endif
    // �����豸���Եȼ�
    D3D_FEATURE_LEVEL                   m_featureLevel;
    // �ֶ�����������
    DXGI_PRESENT_PARAMETERS             m_parameters;
public:
    // ����ģ��
    SceneModel                          scene_model;
    // X
    std::atomic<float>                  x = 0.f;
    // Y
    std::atomic<float>                  y = 0.f;
    // Z
    std::atomic<float>                  z = 1.f;
#ifdef _DEBUG
public:
    // ������Ϣ
    WCHAR*                              debug_info = new WCHAR[2048];
    // ��Ϣ����
    size_t                              info_length = 0;
#endif
};