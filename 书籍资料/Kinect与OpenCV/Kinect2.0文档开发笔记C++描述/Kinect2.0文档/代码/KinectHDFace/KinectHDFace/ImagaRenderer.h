// ImageRender�� ����ͼ��ͼ����Ⱦ

#pragma once

#define IMAGE_WIDTH 1920
#define IMAGE_HEIGHT 1080

struct FaceData{
    // ������?
    UINT32              tracked;
    // ������Ԫ
    float               au[FaceShapeAnimations_Count];
    // �沿���
    RectI               face_box;
    // �沿��ת��Ԫ��
    Vector4             face_rotation;
    // ������ת������
    union{
        D2D1_POINT_2F       head_pivot_1;
        ColorSpacePoint     head_pivot_2;
    };
};

class ImageRenderer{
public:
	// ���캯��
	ImageRenderer();
	// ��������
	~ImageRenderer();
	// ��Ⱦ
	HRESULT OnRender();
	// ���ô��ھ��
	void SetHwnd(HWND hwnd){ m_hwnd = hwnd; }
	// ���س�ʼ�����
	operator HRESULT() const{ return m_hrInit; }
    // ��ȡ����
    __forceinline RGBQUAD* GetBuffer(){ return m_pColorRGBX; }
    // д������
    void WriteBitmapData(RGBQUAD*, int, int);
private:
    // �̻��沿����
    void draw_facedata();
public:
    // ��ת��Ԫ�� ת���� ��ת�Ƕ�
    static void ExtractFaceRotationInDegrees(const Vector4* pQuaternion, float* pPitch, float* pYaw, float* pRoll);
	// �����豸�޹���Դ
	HRESULT CreateDeviceIndependentResources();
	// �����豸�й���Դ
	HRESULT CreateDeviceResources();
	// �����豸�й���Դ
	void DiscardDeviceResources();
	// ���ļ���ȡλͼ
	HRESULT LoadBitmapFromFile(ID2D1RenderTarget*, IWICImagingFactory *, PCWSTR uri, UINT, UINT, ID2D1Bitmap **);
public:
    // D2Dת������
    D2D1_MATRIX_3X2_F                   matrix = D2D1::Matrix3x2F::Identity();
private:
	// ��ʼ�����
    HRESULT								m_hrInit = E_FAIL;
	// ���ھ��
    HWND								m_hwnd = nullptr;
	// D2D ����
    ID2D1Factory*						m_pD2DFactory = nullptr;
	// WIC ����
    IWICImagingFactory*					m_pWICFactory = nullptr;
	// DWrite����
    IDWriteFactory*						m_pDWriteFactory = nullptr;
	// �����ı���Ⱦ��ʽ
    IDWriteTextFormat*					m_pTextFormatMain = nullptr;
    // ��ʾ�ı���Ⱦ��ʽ
    IDWriteTextFormat*					m_pTextFormatShow = nullptr;
	// D2D��ȾĿ��
    ID2D1HwndRenderTarget*				m_pRenderTarget = nullptr;
    // ֡��������
    RGBQUAD*                            m_pColorRGBX = nullptr;
    // ����λͼ֡
    ID2D1Bitmap*                        m_pDrawBitmap = nullptr;
    // ��ɫ��ˢ
    ID2D1SolidColorBrush*               m_pWhiteBrush = nullptr;
    // ��ʱ��
    PrecisionTimer                      m_timer;
    // FPS
    FLOAT                               m_fFPS = 0.f;
public:
    // ����
    FaceData                            data;
};