// ImageRender�� ����ͼ��ͼ����Ⱦ

#pragma once

#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424


#define IMAGE_WIDTH 1920
#define IMAGE_HEIGHT 1080

//
struct BodyInfo{
    Joint joints[JointType_Count];
    D2D1_POINT_2F jointPoints[JointType_Count];
    HandState leftHandState = HandState_Unknown;
    HandState rightHandState = HandState_Unknown;
};



class ThisApp;

// ImageRenderer
class ImageRenderer{
public:
    // ���캯��
    ImageRenderer(ThisApp* boss);
    // ��������
    ~ImageRenderer();
    // ��Ⱦ
    HRESULT OnRender();
    // ���ô��ھ��
    void SetHwnd(HWND hwnd){ m_hwnd = hwnd; }
    // �ı��С
    void OnSize(UINT width, UINT height){
        if (!m_pRenderTarget)return;
        D2D1_SIZE_U size = {
            width, height
        };
        m_pRenderTarget->Resize(size);
    }
    // ���س�ʼ�����
    operator HRESULT() const{ return m_hrInit; }
    // ���ù�������
    __forceinline void SetBodyInfo(int i, BodyInfo* info){ memcpy(m_bodyInfo + i, info, sizeof(BodyInfo)); }
    // ��ȡ��ɫ֡����
    __forceinline RGBQUAD* GetColorBuffer(){ return m_pColorRGBX; }
    // ��ȡ���֡����
    __forceinline RGBQUAD* GetDepthBuffer(){ return m_pDepthRGBX; }
    // д������
    void WriteBitmapData(RGBQUAD*,int, int, int);
private:
    // �����豸�޹���Դ
    HRESULT CreateDeviceIndependentResources();
    // �����豸�й���Դ
    HRESULT CreateDeviceResources();
    // �����豸�й���Դ
    void DiscardDeviceResources();
    // ���ļ���ȡλͼ
    HRESULT LoadBitmapFromFile(ID2D1RenderTarget*, IWICImagingFactory *, PCWSTR uri, UINT, UINT, ID2D1Bitmap **);
    // �̻�����
    void DrawBody();
    // �̻�����
    void DrawGestureInfo();
    // �̻�����
    void DrawBody(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints);
    // �̻���
    void DrawHand(HandState handState, const D2D1_POINT_2F& handPosition);
    // �̻�����
    void DrawBone(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1);
public:
    // ƫ����
    float                               show_offset = 0.f;
private:
    // ��ʼ�����
    HRESULT                             m_hrInit = E_FAIL;
    // ��˾
    ThisApp*                            m_pBoss = nullptr;
    // ���ھ��
    HWND                                m_hwnd = nullptr;
    // D2D ����
    ID2D1Factory*                       m_pD2DFactory = nullptr;
    // WIC ����
    IWICImagingFactory*                 m_pWICFactory = nullptr;
    // DWrite����
    IDWriteFactory*                     m_pDWriteFactory = nullptr;
    // �����ı���Ⱦ��ʽ
    IDWriteTextFormat*                  m_pTextFormatMain = nullptr;
    // D2D��ȾĿ��
    ID2D1HwndRenderTarget*              m_pRenderTarget = nullptr;
    // ��ɫ��ˢ
    ID2D1Brush*                         m_pWhiteBrush = nullptr;
    // �ؽڱ�׷�ٵ���ɫ
    ID2D1SolidColorBrush*               m_pBrushJointTracked = nullptr;
    // �ؽڱ��ƶϵ���ɫ
    ID2D1SolidColorBrush*               m_pBrushJointInferred = nullptr;
    // ������׷�ٵ���ɫ
    ID2D1SolidColorBrush*               m_pBrushBoneTracked = nullptr;
    // �������ƶϵ���ɫ
    ID2D1SolidColorBrush*               m_pBrushBoneInferred = nullptr;
    // �ִ򿪵���ɫ
    ID2D1SolidColorBrush*               m_pBrushHandClosed = nullptr;
    // ��ȭ����ɫ
    ID2D1SolidColorBrush*               m_pBrushHandOpen = nullptr;
    // ������ָ�쿪����ɫ
    ID2D1SolidColorBrush*               m_pBrushHandLasso = nullptr;
    // ��ɫ֡��������
    RGBQUAD*                            m_pColorRGBX = nullptr;
    // ��ɫ֡λͼ
    ID2D1Bitmap*                        m_pColorBitmap = nullptr;
    // ���֡��������
    RGBQUAD*                            m_pDepthRGBX = nullptr;
    // ���֡λͼ
    ID2D1Bitmap*                        m_pDepthBitmap = nullptr;
    // ��ʱ��
    PrecisionTimer                      m_timer;
    // FPS
    FLOAT                               m_fFPS = 0.f;
    // ��������
    BodyInfo                            m_bodyInfo[BODY_COUNT];
};