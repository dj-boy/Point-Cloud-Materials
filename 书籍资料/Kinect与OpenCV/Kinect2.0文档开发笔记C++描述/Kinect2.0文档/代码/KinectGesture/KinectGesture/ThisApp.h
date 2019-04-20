// ThisApp�� ������ĳ���

#pragma once
#define WNDWIDTH 640
#define WNDHEIGHT 480


#define GESTURES_SAVED 32
// ������ʾ��Ϣ
struct GesturesInfo {
    // ��������
    float       float_var[GESTURES_SAVED];
    // ��������1
    BOOLEAN     bool1_var[GESTURES_SAVED];
    // ��������2
    BOOLEAN     bool2_var[GESTURES_SAVED];
    // ����λ��
    UINT32      index;
    // ����
    BOOL        type;
};

// ThisApp��
class ThisApp{
public:
    // ���캯��
    ThisApp(WCHAR* file_name);
    // ��������
    ~ThisApp();
    // ��ʼ��
    HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
    // ��Ϣѭ��
    void RunMessageLoop();
    // ��ȡ�ļ���
    __forceinline const WCHAR* GetFileName(){ return m_szFileNameBuffer; }
    // ��ȡ��������
    __forceinline UINT GetGestureSize(){ return m_cGestureSize; }
    // ��ȡ����
    __forceinline IGesture** const GetGestures(){ return m_ppGestures; }
    // ��ȡ������Ϣ
    __forceinline const GesturesInfo* GetGestureInfo(UINT index){ return &(m_vGesturesInfo[index]); }
private:
    // ���ڹ��̺���
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    // ��ʼ��Kinect
    HRESULT init_kinect();
    // ����ɫ֡
    void check_color_frame();
    // ������֡
    void check_depth_frame();
    // ������֡
    void check_body_frame();
    // �������֡
    void check_gesture_frame();
    // ���������ļ�
    HRESULT load_gesture_database_file(const WCHAR*);
    // �ͷ�����
    void release_gesture_data();
private:
    // ���ھ��
    HWND                                m_hwnd = nullptr;
    // ��Ⱦ��
    ImageRenderer                       m_ImagaRenderer;
    // Kinect v2 ������
    IKinectSensor*                      m_pKinect = nullptr;
    // ��ɫ֡��ȡ��
    IColorFrameReader*                  m_pColorFrameReader = nullptr;
    // ���֡��ȡ��
    IDepthFrameReader*                  m_pDepthFrameReader = nullptr;
    // ����֡��ȡ��
    IBodyFrameReader*                   m_pBodyFrameReader = nullptr;
    // ����ӳ����
    ICoordinateMapper*                  m_pCoordinateMapper = nullptr;
    // �������ݿ�
    IVisualGestureBuilderDatabase*      m_pGestureDatabase = nullptr;
    // ����֡��ȡ��
    IVisualGestureBuilderFrameReader*   m_pGestureFrameReader = nullptr;
    // ����֡Դ
    IVisualGestureBuilderFrameSource*   m_pGestureFrameSource = nullptr;
    // ��������
    IBody*                              m_apBodies[BODY_COUNT];
    // ����ָ������
    IGesture**                          m_ppGestures = m_apGestures;
    // ����ָ�����黺���С
    UINT                                m_cGesturesBufferSize = lengthof(m_apGestures);
    // ����ָ�������С
    UINT                                m_cGestureSize = 0;
    // ������ʾ��Ϣ
    std::vector<GesturesInfo>           m_vGesturesInfo = std::vector<GesturesInfo>(lengthof(m_apGestures));
    // ����ָ������-����
    IGesture*                           m_apGestures[64];
    // �ļ����ֻ���
    WCHAR                               m_szFileNameBuffer[MAX_PATH];
};