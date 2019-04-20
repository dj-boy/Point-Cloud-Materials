#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"

#define lengthof(a) (sizeof(a)/sizeof(*a))


// ThisApp���캯��
ThisApp::ThisApp(){
    m_pColorFrameBuffer = new RGBQUAD[IMAGE_WIDTH * IMAGE_HEIGHT];
}

// ThisApp��������
ThisApp::~ThisApp(){
    // �ͷ�MultiSourceFrameReader
    SafeRelease(m_pColorFrameReader);
    SafeRelease(m_pCoordinateMapper);
    SafeRelease(m_pBodyFrameReader);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    SafeRelease(m_pKinect);
    if (m_pColorFrameBuffer){
        delete[] m_pColorFrameBuffer;
        m_pColorFrameBuffer = nullptr;
    }
}

// ��Ⱦ����
void ThisApp::Render(ThisApp* pThis){
    // ��Ⱦ
    while (true){
        pThis->Update();
        pThis->m_ImagaRenderer.OnRender(1);
        if (pThis->m_bExit) break;
    }
}

// ��ʼ��
HRESULT ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow){
    HRESULT hr = E_FAIL;
    //register window class
    WNDCLASSEX wcex = { sizeof(WNDCLASSEX) };
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = ThisApp::WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = sizeof(LONG_PTR);
    wcex.hInstance = hInstance;
    wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
    wcex.hbrBackground = nullptr;
    wcex.lpszMenuName = nullptr;
    wcex.lpszClassName = L"Direct2DTemplate";
    wcex.hIcon = nullptr;
    // ע�ᴰ��
    RegisterClassEx(&wcex);
    // ��������
    RECT window_rect = { 0, 0, WNDWIDTH, WNDHEIGHT };
    DWORD window_style = WS_CAPTION | WS_VISIBLE | WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX;
    AdjustWindowRect(&window_rect, window_style, FALSE);
    window_rect.right -= window_rect.left;
    window_rect.bottom -= window_rect.top;
    window_rect.left = (GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
    window_rect.top = (GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;

    m_hwnd = CreateWindowExW(0, wcex.lpszClassName, TITLE, window_style,
        window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, 0, 0, hInstance, this);
    hr = m_hwnd ? S_OK : E_FAIL;
    // ���ô��ھ��
    if (SUCCEEDED(hr)) {
        hr = m_ImagaRenderer.SetHwnd(m_hwnd);
    }
    // ��ʾ����
    if (SUCCEEDED(hr)) {
        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);
        m_threadMSG.std::thread::~thread();
        m_threadMSG.std::thread::thread(Render, this);
    }
	return hr;
}



// ��Ϣѭ��
void ThisApp::RunMessageLoop(){
    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


// ���ڹ��̺���
LRESULT CALLBACK ThisApp::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
		ThisApp *pOurApp = (ThisApp *)pcs->lpCreateParams;

		::SetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA,
			PtrToUlong(pOurApp)
			);

        // ����ʼ��Kinect
        if (FAILED(pOurApp->init_kinect())){
            ::MessageBoxW(hwnd, L"��ʼ��Kinect v2ʧ��", L"��ĺ��ź�", MB_ICONERROR);
        }
		result = 1;
	}
	else
	{
		ThisApp *pOurApp = reinterpret_cast<ThisApp *>(static_cast<LONG_PTR>(
			::GetWindowLongPtrW(
			hwnd,
			GWLP_USERDATA
			)));

		bool wasHandled = false;
		if (pOurApp){
            switch (message)
			{
            case WM_CLOSE:
                // ����β����(�����ȫ�����߳�)��������
                pOurApp->m_bExit = TRUE;
                // join
                pOurApp->m_threadMSG.join();
				DestroyWindow(hwnd);
				result = 1;
				wasHandled = true;
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				result = 1;
				wasHandled = true;
				break;
			}
		}

		if (!wasHandled)
		{
			result = DefWindowProc(hwnd, message, wParam, lParam);
		}
	}

	return result;
}


// ��ʼ��Kinect
HRESULT ThisApp::init_kinect(){
    IColorFrameSource* pColorFrameSource = nullptr;
    IBodyFrameSource* pBodyFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // �򿪲�ɫ֡Դ
    if (SUCCEEDED(hr)) {
        hr = m_pKinect->get_ColorFrameSource(&pColorFrameSource);
    }
    // �򿪲�ɫ֡��ȡ��
    if (SUCCEEDED(hr)) {
        hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
    }
    // �򿪹���֡Դ
    if (SUCCEEDED(hr)) {
        hr = m_pKinect->get_BodyFrameSource(&pBodyFrameSource);
    }
    // �򿪹���֡��ȡ��
    if (SUCCEEDED(hr)) {
        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    }
    // ��ȡ����ӳ����
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_CoordinateMapper(&m_pCoordinateMapper);
    }
    SafeRelease(pBodyFrameSource);
    SafeRelease(pColorFrameSource);
    return hr;
}

// ����ɫ֡
void ThisApp::check_color_frame(){
    IColorFrame* pColorFrame = nullptr;
    IFrameDescription* pColorFrameDescription = nullptr;
    int width = 0, height = 0;
    ColorImageFormat imageFormat = ColorImageFormat_None;
    UINT nBufferSize = 0;
    RGBQUAD *pBuffer = nullptr;

    // ����Ƿ������֡
    HRESULT hr = m_pColorFrameReader->AcquireLatestFrame(&pColorFrame);

    // ��ȡ��ɫ��Ϣ
    if (SUCCEEDED(hr)){
        hr = pColorFrame->get_FrameDescription(&pColorFrameDescription);
    }
    // ��ȡ��ɫ֡���
    if (SUCCEEDED(hr)){
        hr = pColorFrameDescription->get_Width(&width);
    }
    // ��ȡ��ɫ֡�߶�
    if (SUCCEEDED(hr)){
        hr = pColorFrameDescription->get_Height(&height);
    }
    // ��ȡ֡��ʽ
    if (SUCCEEDED(hr)) {
        hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
    }
    // ��ȡ��ɫ֡����
    if (SUCCEEDED(hr)){
        // ���Ѿ���BGRA������� ֱ�ӻ�ȡԴ����
        if (imageFormat == ColorImageFormat_Bgra) {
            hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
        }
        // �������Դ��ķ���ת��
        else{
            pBuffer = m_pColorFrameBuffer;
            nBufferSize = width * height * sizeof(RGBQUAD);
            hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
        }
    }
    // ��������
    if (SUCCEEDED(hr)){
        hr = m_ImagaRenderer.LoadData(pBuffer, width, height);
    }

    SafeRelease(pColorFrame);
    SafeRelease(pColorFrameDescription);
}



// ������֡
void ThisApp::check_body_frame(){
    IBodyFrame* pBodyFrame = nullptr;
    IBody*  ppBody[BODY_COUNT] = { 0 };
    // �����֡
    HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);

    // ��ȡ��������
    if (SUCCEEDED(hr)) {
        hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, ppBody);
    }
    // ��������
    if (SUCCEEDED(hr)){
        for (int i = 0; i < BODY_COUNT; ++i){
            // ��֤����
            BOOLEAN bTracked = false;
            hr = ppBody[i]->get_IsTracked(&bTracked);
            if (!ppBody[i] || !bTracked){
                m_ImagaRenderer.m_men[i].man = LaughingMan::Offline;
                continue;
            }
            IBody* pNowBody = ppBody[i];
            Joint joints[JointType_Count];
            hr = pNowBody->GetJoints(JointType_Count, joints); 
            if (SUCCEEDED(hr)) {
                m_ImagaRenderer.m_men[i].man = LaughingMan::Online;
                m_ImagaRenderer.m_men[i].update();
                // ����ӳ��
                ColorSpacePoint depthPoint = { 0.f, 0.f };
                m_pCoordinateMapper->MapCameraPointToColorSpace(joints[JointType_Head].Position, &depthPoint);
                // ģ�ⶶ��
                m_ImagaRenderer.m_men[i].pos.width = depthPoint.X + 2.f * static_cast<float>((rand() % 4) - 2);
                m_ImagaRenderer.m_men[i].pos.height = depthPoint.Y + 2.f * static_cast<float>((rand() % 4) - 2);
                // ����Զ����С
                m_ImagaRenderer.m_men[i].zoom = 1.f / joints[JointType_Head].Position.Z;
            }
        }

    }
    for (int i = 0; i < BODY_COUNT; ++i)SafeRelease(ppBody[i]);
    SafeRelease(pBodyFrame);
}

// ˢ�¶���
void ThisApp::Update(){
    // ����ɫ֡
    check_color_frame();
    // ������֡
    check_body_frame();
}

