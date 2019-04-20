// SceneRenderer�� ������Ⱦ��

#pragma once

// ���ü�����
#define SCREEN_NEAR_Z (0.01f)
// Զ�ü�����
#define SCREEN_FAR_Z (10.f)

// Kinect
#define IMAGE_WIDTH (1920)
#define IMAGE_HEIGHT (1080)




// ������Ⱦ
class SceneRenderer{
public:
    // ����-���� ģ��
    struct VertexNormal{
        // ����
        DirectX::XMFLOAT3   pos;
    };
    // ���㳣������
    struct MatrixBufferType {
        // ����ת��
        DirectX::XMFLOAT4X4   world;
        // �ӽ�ת��
        DirectX::XMFLOAT4X4   view;
        // ͸��ת��
        DirectX::XMFLOAT4X4   projection;
    };
    // ���ղ���
    struct  LightMaterialBufferType  {
        //ƽ�йⷽ��
        DirectX::XMFLOAT3 lightDirection;
        //�߹�ָ��
        float shininess;
        //��Դλ��
        DirectX::XMFLOAT4 lightPosition;
        //��Դ��ɫ
        DirectX::XMFLOAT4 lightColor;
        //��Դ�Ļ����ⷴ��ϵ��
        DirectX::XMFLOAT4 globalAmbient;
        //�������λ��
        DirectX::XMFLOAT4 cameraPosition;
        //���ʵ��Է���
        DirectX::XMFLOAT4 Ke;
        //���ʵĻ�����ϵ��
        DirectX::XMFLOAT4 Ka;
        //���ʵ�������ϵ��
        DirectX::XMFLOAT4 Kd;
        //���ʵĸ߹�ϵ��
        DirectX::XMFLOAT4 Ks;
    };
    // �沿ģ��
    class FaceModel{
    public:
        // ����
        FaceModel();
        // ����
        HRESULT Create(ID3D11Device* device);
        // ˢ�³�������
        void RefreshCB(ID3D11DeviceContext* context);
        // ˢ�¶��㻺��
        void RefreshVB(ID3D11DeviceContext* context);
        // ����
        ~FaceModel(){ 
            SafeRelease(m_pVertexBuffer);
            SafeRelease(m_pIndexBuffer);
            SafeRelease(m_pInputLayout);
            SafeRelease(m_pVSCBuffer);
            SafeRelease(m_pPSCBuffer);
            SafeRelease(m_pVertexShader);
            SafeRelease(m_pGeometryShader);
            SafeRelease(m_pPixelShader);
        }
        // ��Ⱦ
        void Render(ID3D11DeviceContext* context);
    private:
        // ģ�Ͷ��㻺��
        ID3D11Buffer*           m_pVertexBuffer = nullptr;
        // ģ����������
        ID3D11Buffer*           m_pIndexBuffer = nullptr;
        // �������벼��
        ID3D11InputLayout*      m_pInputLayout = nullptr;
        // ������ɫ�� ��������
        ID3D11Buffer*           m_pVSCBuffer = nullptr;
        // ������ɫ�� ��������
        ID3D11Buffer*           m_pPSCBuffer = nullptr;
        // ������ɫ��    
        ID3D11VertexShader*     m_pVertexShader = nullptr;
        // ������ɫ��
        ID3D11GeometryShader*   m_pGeometryShader = nullptr;
        // ������ɫ��
        ID3D11PixelShader*      m_pPixelShader = nullptr;
    public:
        // ������ɫ����������
        MatrixBufferType                    vscb;
        // ������ɫ����������
        LightMaterialBufferType             pscb;
        // ��������
        UINT                                vertex_count = 0;
        // �沿����
        VertexNormal*                       face_mash_vertexs = nullptr;
        // ��������
        UINT32                              index_count = 0;
        // ����
        UINT32*                             index = nullptr;
        // �ռ�״̬
        FaceModelBuilderCollectionStatus    co_status = FaceModelBuilderCollectionStatus_MoreFramesNeeded;
        // �ռ�״̬
        FaceModelBuilderCaptureStatus       ca_status = FaceModelBuilderCaptureStatus_GoodFrameCapture;
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
    // д������
    void WriteBitmapData(RGBQUAD*, int, int);
    // ��ȡ����
    __forceinline RGBQUAD* GetBuffer(){ return m_pColorRGBX; }
    // ˢ��ģ������
    void RefreshFaceData(){
        face_model.RefreshVB(m_pD3DDeviceContext);
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
    // ֡��������
    RGBQUAD*                            m_pColorRGBX = nullptr;
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
    // ����λͼ֡
    ID2D1Bitmap1*                       m_pDrawBitmap = nullptr;
    // ��׼�ı���ʽ
    IDWriteTextFormat*                  m_pStandardTF = nullptr;
    // ��׼��ˢ
    ID2D1SolidColorBrush*               m_pStandardBrush = nullptr;
    // �����豸���Եȼ�
    D3D_FEATURE_LEVEL                   m_featureLevel;
    // �ֶ�����������
    DXGI_PRESENT_PARAMETERS             m_parameters;
public:
    // �沿ģ��
    FaceModel                           face_model;
#ifdef _DEBUG
public:
    // ������Ϣ
    WCHAR*                              debug_info = new WCHAR[2048];
    // ��Ϣ����
    size_t                              info_length = 0;
#endif
};