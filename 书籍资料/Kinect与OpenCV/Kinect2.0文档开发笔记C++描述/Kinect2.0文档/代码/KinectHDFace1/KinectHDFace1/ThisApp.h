// ThisApp�� ������ĳ���

#pragma once
#define TITLE L"Title"
#define WNDWIDTH 1024
#define WNDHEIGHT 768

// ThisApp��
class ThisApp
{
public:
	// ���캯��
	ThisApp();
	// ��������
	~ThisApp();
	// ��ʼ��
	HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
	// ��Ϣѭ��
	void RunMessageLoop();
private:
	// ���ڹ��̺���
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    // ��ʼ��Kinect
    HRESULT init_kinect();
    // ����ɫ֡
    void check_color_frame();
    // ������֡
    void check_body_frame();
    // �������沿
    void check_hd_face_frame();
private:
	// ���ھ��
	HWND			                    m_hwnd = nullptr;
    // Kinect v2 ������
    IKinectSensor*                      m_pKinect = nullptr;
    // ��ɫ֡��ȡ��
    IColorFrameReader*                  m_pColorFrameReader = nullptr;
    // �����沿֡Դ
    IHighDefinitionFaceFrameSource*     m_pHDFaceFrameSource = nullptr;
    // �����沿֡��ȡ��
    IHighDefinitionFaceFrameReader*     m_pHDFaceFrameReader = nullptr;
    // �沿��������
    IFaceAlignment*                     m_pFaceAlignment = nullptr;
    // ����֡��ȡ��
    IBodyFrameReader*                   m_pBodyFrameReader = nullptr;
    // �沿ģ��
    IFaceModel*                         m_pFaceModel = nullptr;
    // ����ӳ����
    ICoordinateMapper*                  m_pMapper = nullptr;
    // �沿ģ�Ͷ���
    CameraSpacePoint*                   m_pFaceVertices = nullptr;
    // �沿ģ�Ͷ�������
    UINT                                m_cFaceVerticeCount = 0;
    // ���� ��֤��64λ�������� 64λ ����
    UINT                                unused = 0;
    // ��ɫ��֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE                     m_hColorFrameArrived = 0;
    // ������֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE                     m_hBodyFrameArrived = 0;
    // �����沿��֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE                     m_hHDFFrameArrived = 0;
	// ��Ⱦ��
	ImageRenderer	                    m_ImagaRenderer;
};