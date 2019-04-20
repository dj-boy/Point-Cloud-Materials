// ThisApp�� ������ĳ���

#pragma once
#define TITLE L"Title"
// ��֤16:9
#define WNDWIDTH 1024
#define WNDHEIGHT 576

// ThisApp��
class ThisApp {
public:
    // ���캯��
    ThisApp();
    // ��������
    ~ThisApp();
    // ��Ⱦ����
    static void Render(ThisApp* pThis);
    // ��ʼ��
    HRESULT Initialize(HINSTANCE hInstance, int nCmdShow);
    // ��Ϣѭ��
    void RunMessageLoop();
    // ˢ��
    void Update();
private:
    // ��ʼ��Kinect
    HRESULT init_kinect();
    // ����ɫ֡
    void check_color_frame();
    // ������֡
    void check_body_frame();
    // �������沿
    void check_hd_face_frame();
    // ��ʼ����Ϸ����
    void initialize_game_objects();
    // ����ʼ����Ϸ����
    void finalize_game_objects();
    // ���ڹ��̺���
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
private:
    // ��Ⱦ��
    SceneRenderer                       m_SceneRenderer;
    // ��Ⱦ�߳�
    std::thread                         m_threadRender;
    // ���ھ��
    HWND                                m_hwnd = nullptr;
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
    // �沿ģ�͹�����
    IFaceModelBuilder*                  m_pFaceModelBuilder = nullptr;
    // �沿ģ�Ͷ�������
    UINT                                m_cFaceVerticeCount = 0;
    // �沿ģ���Ƿ񱻹������
    UINT                                m_bProduced = FALSE;
    // ���㻺�滥����
    std::mutex                          m_muxFaceVertice;
    // �˳�
    std::atomic<BOOL>                   m_bExit = FALSE;
    // ����֡
    IBody*                              m_apBody[BODY_COUNT];
private:
    // ����
    float                               m_afFSD[FaceShapeDeformations_Count];
};
