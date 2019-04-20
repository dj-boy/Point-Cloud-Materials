// ThisApp�� ������ĳ���

#pragma once

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
    // ��鸴Դ֡
    void check_color_frame();
private:
	// ���ھ��
	HWND			            m_hwnd = nullptr;
    // Kinect v2 ������
    IKinectSensor*              m_pKinect = nullptr;
    // ����ӳ����
    ICoordinateMapper*          m_pCoordinateMapper = nullptr;
    // ��Դ֡��ȡ��
    IMultiSourceFrameReader*    m_pMultiSourceFrameReader = nullptr;
    // ��Դ��֡�¼� ������nullptr��ʼ�� ����
    WAITABLE_HANDLE             m_hMultiSourceFrameArrived = 0;
    // ӳ�����ı��¼�
    WAITABLE_HANDLE             m_hCoordinateMapperChanged = 0;
    // ���֡����
    ColorSpacePoint*            m_pColorCoordinates = nullptr;
    // ��ɫ֡����
    RGBQUAD*                    m_pColorFrameBuffer = nullptr;
	// ��Ⱦ��
	ImageRenderer	            m_ImagaRenderer;
};