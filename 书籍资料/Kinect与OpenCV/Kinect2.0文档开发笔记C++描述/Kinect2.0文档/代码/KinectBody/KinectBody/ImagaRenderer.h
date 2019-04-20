// ImageRender�� ����ͼ��ͼ����Ⱦ

#pragma once

typedef std::map<std::wstring, ID2D1Bitmap*> BitmapCacheMap;
#define DEPTH_WIDTH 512
#define DEPTH_HEIGHT 424


struct BodyInfo{
    BOOL ok = FALSE;
    Joint joints[JointType_Count];
    D2D1_POINT_2F jointPoints[JointType_Count];
    HandState leftHandState = HandState_Unknown;
    HandState rightHandState = HandState_Unknown;
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
    // ���ù�������
    void SetBodyInfo(int, BodyInfo*);
private:
	// ��ȡͼƬ
	// bitmapName	[in] : �ļ���
	// ����: NULL��ʾʧ�� �������Ϊλͼ��ָ��
	ID2D1Bitmap* GetBitmap(std::wstring& bitmapName);
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
    void DrawBody(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints);
    // �̻���
    void DrawHand(HandState handState, const D2D1_POINT_2F& handPosition);
    // �̻�����
    void DrawBone(const Joint* pJoints, const D2D1_POINT_2F* pJointPoints, JointType joint0, JointType joint1);
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
	// D2D��ȾĿ��
    ID2D1HwndRenderTarget*				m_pRenderTarget = nullptr;
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
    // ��ʱ��
    PrecisionTimer                      m_timer;
    // FPS
    FLOAT                               m_fFPS = 0.f;
    // ��������
    BodyInfo                            m_bodyInfo[BODY_COUNT];
	// ͼ�񻺴�
	BitmapCacheMap						m_mapBitmapCache;
};