#pragma once
// Author  : DustPG
// License : MIT: see more in "License.txt"

// ��;: ��ͼ��ͼ��ĳ��� �����ͼ����Ⱦ


#define IMAGE_WIDTH 1920
#define IMAGE_HEIGHT 1080

#define WNDWIDTH 1280
#define WNDHEIGHT 720

#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424

// Ц����״̬
enum class LaughingMan{
    Offline = 0,
    Online,
};

struct LaughingManState
{
    // ״̬
    LaughingMan man = LaughingMan::Offline;
    // ��ǰ���ֽǶ�
    FLOAT angle = 0.f;
    // Ԥ��������
    FLOAT zoom = 1.f;
    // ��ǰ��Ļ����
    D2D1_SIZE_F pos = D2D1::SizeF();
    // ����
    __forceinline void update(){ angle -= 2.f; if (angle < 0.f) angle += 360.f; }
};

// ͼ����Ⱦ��
class ImageRenderer{
public:
    // ���캯��
    ImageRenderer();
    // ��������
    ~ImageRenderer();
    // ���ô��ھ��
    HRESULT SetHwnd(HWND hwnd){ m_hwnd = hwnd; return CreateResources(); }
public:
    // ��Ⱦ֡
    HRESULT OnRender(UINT syn);
    // ���ǲ�ɫ֡λͼ
    HRESULT LoadData(void* data, int width, int height){
        if (m_pColorFrame){
            D2D1_RECT_U rect = { 0, 0, width, height };
            return m_pColorFrame->CopyFromMemory(
                &rect,
                data,
                width* sizeof(RGBQUAD)
                );
        }
        return S_FALSE;
    }
    // ��ȡ�ļ�
    static HRESULT ImageRenderer::LoadBitmapFromFile(
        ID2D1DeviceContext *pRenderTarget,
        IWICImagingFactory2 *pIWICFactory,
        PCWSTR uri,
        UINT destinationWidth,
        UINT destinationHeight,
        ID2D1Bitmap1 **ppBitmap
        );
private:
    // ������Դ
    HRESULT CreateResources();
    // �����豸�й���Դ
    void DiscardDeviceResources();
    // ����Ц����
    HRESULT CreateLaughingMan();
    // �̻�Ц����
    void DrawLaughingMan();
private:
    // D2D ����
    ID2D1Factory1*						m_pD2DFactory = nullptr;
    // D2D �豸
    ID2D1Device*						m_pD2DDevice = nullptr;
    // D2D �豸������
    ID2D1DeviceContext*					m_pD2DDeviceContext = nullptr;
    // WIC ����
    IWICImagingFactory2*				m_pWICFactory = nullptr;
    // DWrite����
    IDWriteFactory1*					m_pDWriteFactory = nullptr;
    // ������Ⱦ����
    IDWriteRenderingParams*				m_pDWParams = nullptr;
    // DXGI ������
    IDXGISwapChain1*					m_pSwapChain = nullptr;
    // D2D λͼ ���浱ǰ��ʾ��λͼ
    ID2D1Bitmap1*						m_pD2DTargetBimtap = nullptr;
    // D2D �̻�������
    ID2D1Bitmap1*						m_pBufferBitmap = nullptr;
    // ��ɫλͼ
    ID2D1Bitmap1*                       m_pColorFrame = nullptr;
    //  �����豸���Եȼ�
    D3D_FEATURE_LEVEL					m_featureLevel;
    // �ֶ�������
    DXGI_PRESENT_PARAMETERS				m_parameters;
    // ���ھ��
    HWND								m_hwnd = nullptr;
    // DWrite�ı���Ⱦ��
    PathTextRenderer*                   m_pTextRenderer = nullptr;
    // Ц�����ı�����
    IDWriteTextLayout*                  m_pTextLayoutLaughingMan = nullptr;
    // ����·��
    ID2D1EllipseGeometry*               m_pTextAnimationPath = nullptr;
    // Ц����·����ɫ����
    ID2D1PathGeometry*                  m_pLaughingManGeometryBlue = nullptr;
    // Ц����·����ɫ����
    ID2D1PathGeometry*                  m_pLaughingManGeometryWhite = nullptr;
    // Ц����
    ID2D1SolidColorBrush*               m_pLaughingBlueBrush = nullptr;
    // Ц���
    ID2D1SolidColorBrush*               m_pLaughingWhiteBrush = nullptr;
    // ��ĻDPI
    FLOAT								m_dpi = 0.F;
public:
    LaughingManState                    m_men[BODY_COUNT];
};