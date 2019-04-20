#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"
#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) sizeof(a)/sizeof(*a)

static const DWORD THIS_APP_FACE_FRAME_FEATURES =
FaceFrameFeatures::FaceFrameFeatures_BoundingBoxInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_PointsInColorSpace
| FaceFrameFeatures::FaceFrameFeatures_RotationOrientation
| FaceFrameFeatures::FaceFrameFeatures_Happy
| FaceFrameFeatures::FaceFrameFeatures_RightEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_LeftEyeClosed
| FaceFrameFeatures::FaceFrameFeatures_MouthOpen
| FaceFrameFeatures::FaceFrameFeatures_MouthMoved
| FaceFrameFeatures::FaceFrameFeatures_LookingAway
| FaceFrameFeatures::FaceFrameFeatures_Glasses
| FaceFrameFeatures::FaceFrameFeatures_FaceEngagement;

// ThisApp���캯��
ThisApp::ThisApp(){
    // �������
    ZeroMemory(m_pFaceFrameSources, sizeof(m_pFaceFrameSources));
    ZeroMemory(m_pFaceFrameReaders, sizeof(m_pFaceFrameReaders));
    ZeroMemory(m_pBodies, sizeof(m_pBodies));
}

// ThisApp��������
ThisApp::~ThisApp(){
    // �����¼�
    if (m_hColorFrameArrived && m_pColorFrameReader){
        m_pColorFrameReader->UnsubscribeFrameArrived(m_hColorFrameArrived);
        m_hColorFrameArrived = 0;
    }
    // �ͷ��沿֡��ȡ��
    for (int i = 0; i < BODY_COUNT; ++i){
        SafeRelease(m_pFaceFrameReaders[i]);
    }
    // �ͷ��沿֡Դ
    for (int i = 0; i < BODY_COUNT; ++i){
        SafeRelease(m_pFaceFrameSources[i]);
    }
    // �ͷŹ�������
    for (int i = 0; i < BODY_COUNT; ++i){
        SafeRelease(m_pBodies[i]);
    }
    // �ͷŹ���֡��ȡ��
    SafeRelease(m_pBodyFrameReader);
    // �ͷ�ColorFrameReader
    SafeRelease(m_pColorFrameReader);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    SafeRelease(m_pKinect);
}

// ��ʼ��
HRESULT ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow){
	HRESULT hr = E_FAIL;
	if (SUCCEEDED(static_cast<HRESULT>(m_ImagaRenderer)))
	{
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
		DWORD window_style = WS_CAPTION | WS_VISIBLE | WS_POPUP | WS_SYSMENU |	WS_MINIMIZEBOX;
		AdjustWindowRect(&window_rect, window_style, FALSE);
		AdjustWindowRect(&window_rect, window_style, FALSE);
		window_rect.right -= window_rect.left;
		window_rect.bottom -= window_rect.top;
		window_rect.left = (GetSystemMetrics(SM_CXFULLSCREEN) - window_rect.right) / 2;
		window_rect.top = (GetSystemMetrics(SM_CYFULLSCREEN) - window_rect.bottom) / 2;

		m_hwnd = CreateWindowExW(0, wcex.lpszClassName, TITLE, window_style,
			window_rect.left, window_rect.top, window_rect.right, window_rect.bottom, 0, 0, hInstance, this);
		hr = m_hwnd ? S_OK : E_FAIL;
		if (SUCCEEDED(hr))
		{	
			// ���ô��ھ��
			m_ImagaRenderer.SetHwnd(m_hwnd);
			// ��ʾ����
			ShowWindow(m_hwnd, nCmdShow);
			UpdateWindow(m_hwnd);
		}
	}

	return hr;
}



// ��Ϣѭ��
void ThisApp::RunMessageLoop()
{
	MSG msg;
    HANDLE events[] = { 
        reinterpret_cast<HANDLE>(m_hColorFrameArrived),
        reinterpret_cast<HANDLE>(m_hBodyFrameArrived),

    };
    while (true){
        // ��Ϣ����
        if (PeekMessageW(&msg, nullptr, 0, 0, PM_REMOVE)){
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        // �����¼�
        // �¼�0: ��ɫ��֡�¼�
        events[0] = reinterpret_cast<HANDLE>(m_hColorFrameArrived);
        // �¼�1: ������֡�¼�
        events[1] = reinterpret_cast<HANDLE>(m_hBodyFrameArrived);
        // ����¼�
        switch (MsgWaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE, QS_ALLINPUT))
        {
            // events[0]
        case WAIT_OBJECT_0 + 0:
            this->check_color_frame();
            break;
            // events[1]
        case WAIT_OBJECT_0 + 1:
            this->check_body_frame();
            break;
        default:
            break;
        }
        // �˳�
        if (msg.message == WM_QUIT){
            break;
        }
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
		if (pOurApp)
		{
			switch (message)
			{
			case WM_DISPLAYCHANGE:
				InvalidateRect(hwnd, NULL, FALSE);
				result = 0;
				wasHandled = true;
				break;
            case WM_MOUSEWHEEL:
            
                pOurApp->m_ImagaRenderer.matrix._11 += 0.05f * static_cast<float>(static_cast<short>(HIWORD(wParam))) 
                    / static_cast<float>(WHEEL_DELTA);
                    pOurApp->m_ImagaRenderer.matrix._22 = pOurApp->m_ImagaRenderer.matrix._11;
                pOurApp->m_ImagaRenderer.OnRender();
            
                break;
			case WM_PAINT:
				pOurApp->m_ImagaRenderer.OnRender();
				break;
			case WM_CLOSE:
				// ����β����(�����ȫ�����߳�)��������
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
    // ��ȡ��ɫ֡Դ(ColorFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_ColorFrameSource(&pColorFrameSource);
    }
    // �ٻ�ȡ��ɫ֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
        m_pColorFrameReader->SubscribeFrameArrived(&m_hColorFrameArrived);
    }
    // ��ȡ����֡Դ
    if (SUCCEEDED(hr)) {
        hr = m_pKinect->get_BodyFrameSource(&pBodyFrameSource);
    }
    // ��ȡ����֡��ȡ��
    if (SUCCEEDED(hr)) {
        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    }
    // ע����֡�¼�
    if (SUCCEEDED(hr)){
        m_pBodyFrameReader->SubscribeFrameArrived(&m_hBodyFrameArrived);
    }
    // Ϊÿ�˴���һ���沿֡�������
    if (SUCCEEDED(hr)) {
        // �沿֡Դ ������ ��Ҫ�ͷŵ�
        for (int i = 0; i < BODY_COUNT; i++) {
            // �����Լ���Ҫ�����Դ����沿֡Դ
            if (SUCCEEDED(hr)) {
                hr = CreateFaceFrameSource(m_pKinect, 0, THIS_APP_FACE_FRAME_FEATURES, m_pFaceFrameSources + i);
            }
            // ��ȡ��Ӧ�Ķ�ȡ��
            if (SUCCEEDED(hr)) {
                hr = m_pFaceFrameSources[i]->OpenReader(&m_pFaceFrameReaders[i]);
            }

        }
    }
    SafeRelease(pBodyFrameSource);
    SafeRelease(pColorFrameSource);
    return hr;
}


// ����ɫ֡
void ThisApp::check_color_frame(){
    if (!m_pColorFrameReader) return;
#ifdef _DEBUG
    static int frame_num = 0;
    ++frame_num; 
    _cwprintf(L"<ThisApp::check_color_frame>Frame@%8d ", frame_num);
#endif 
    // ��ɫ��֡�¼�����
    IColorFrameArrivedEventArgs* pArgs = nullptr;
    // ��ɫ֡����
    IColorFrameReference* pCFrameRef = nullptr;
    // ��ɫ֡
    IColorFrame* pColorFrame = nullptr;
    // ֡����
    IFrameDescription* pFrameDescription = nullptr;
    // ��ɫ֡�������
    int width = 0;
    // ��ɫ֡�߶�����
    int height = 0;
    // ֡��ʽ
    ColorImageFormat imageFormat = ColorImageFormat_None;
    // ֡�����С
    UINT nBufferSize = 0;
    // ֡����
    RGBQUAD *pBuffer = nullptr;

    // ��ȡ����
    HRESULT hr = m_pColorFrameReader->GetFrameArrivedEventData(m_hColorFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pCFrameRef);
    }
    // ��ȡ��ɫ֡
    if (SUCCEEDED(hr)) {
        hr = pCFrameRef->AcquireFrame(&pColorFrame);
    }
    // ��ȡ֡����
    if (SUCCEEDED(hr)) {
        hr = pColorFrame->get_FrameDescription(&pFrameDescription);
    }
    // ��ȡ֡���
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Width(&width);
    }
    // ��ȡ֡�߶�
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Height(&height);
    }
    // ��ȡ֡��ʽ
    if (SUCCEEDED(hr)) {
        hr = pColorFrame->get_RawColorImageFormat(&imageFormat);
    }
    // ��ȡ֡������
    if (SUCCEEDED(hr)) {
        // ���Ѿ���BGRA������� ֱ�ӻ�ȡԴ����
        if (imageFormat == ColorImageFormat_Bgra) {
            hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
        }
        // �������Դ��ķ���ת��
        else{
            pBuffer = m_ImagaRenderer.GetBuffer();
            nBufferSize = IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(RGBQUAD);
            hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
        }
    }
    // ��ѯ�沿��Ϣ
    if (SUCCEEDED(hr)){
        this->check_faces();
    }
    // ��������
    if (SUCCEEDED(hr)){
        m_ImagaRenderer.WriteBitmapData(pBuffer, width, height);
    }
    // ��ȫ�ͷ�
    SafeRelease(pFrameDescription);
    SafeRelease(pColorFrame);
    SafeRelease(pCFrameRef);
    SafeRelease(pArgs);
#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _cwprintf(L" �ɹ�\n");
    else
        _cwprintf(L" ʧ��\n");
#endif
}


// ����沿
void ThisApp::check_faces(){
    HRESULT hr = S_OK;
    // ѭ�����
    for (int i = 0; i < BODY_COUNT; ++i){
        m_ImagaRenderer.face_data[i].tracked = FALSE;
        if (!m_pBodies[i]) continue;
        IFaceFrame* pFaceFrame = nullptr;
        // ��ȡ�����һ֡
        hr = m_pFaceFrameReaders[i]->AcquireLatestFrame(&pFaceFrame);
        // ����Ƿ����
        BOOLEAN tracked = false;
        if (SUCCEEDED(hr)){
            hr = pFaceFrame->get_IsTrackingIdValid(&tracked);
        }
        // ���ݴ���
        if (SUCCEEDED(hr)){
            // ������״̬
            if (tracked){
                IFaceFrameResult* pFaceFrameResult = nullptr;
                // ��ȡ�沿֡���
                hr = pFaceFrame->get_FaceFrameResult(&pFaceFrameResult);
                // ��Ҫ������֤
                if (SUCCEEDED(hr) && pFaceFrameResult) {
                    // ��ȡ��ɫ�ռ���沿�߿�λ��
                    hr = pFaceFrameResult->get_FaceBoundingBoxInColorSpace(
                        &m_ImagaRenderer.face_data[i].faceBox
                        );
                    // ��ȡ��ɫ�ռ���沿������λ��
                    if (SUCCEEDED(hr)) {
                        hr = pFaceFrameResult->GetFacePointsInColorSpace(
                            FacePointType::FacePointType_Count,
                            m_ImagaRenderer.face_data[i].facePoints
                            );
                    }
                    // ��ȡ�沿��ת��Ԫ��
                    if (SUCCEEDED(hr)) {
                        hr = pFaceFrameResult->get_FaceRotationQuaternion(
                            &m_ImagaRenderer.face_data[i].faceRotation
                            );
                    }
                    // ��ȡ�沿�������
                    if (SUCCEEDED(hr)) {
                        hr = pFaceFrameResult->GetFaceProperties(
                            FaceProperty::FaceProperty_Count, 
                            m_ImagaRenderer.face_data[i].faceProperties
                            );
                    }
                    // ����Ϊ�������١�
                    if (SUCCEEDED(hr)){
                        m_ImagaRenderer.face_data[i].tracked = TRUE;
                    }
                }
                // �ͷŵ�
                SafeRelease(pFaceFrameResult);
            }
            // û�б��������Ը�������id
            else if (m_pBodies[i]){
                BOOLEAN bTracked = false;
                // ��鱻��������û��
                hr = m_pBodies[i]->get_IsTracked(&bTracked);
                UINT64 bodyTId;
                if (SUCCEEDED(hr) && bTracked) {
#ifdef _DEBUG
                    _cwprintf(L"���¸���ID\n");
#endif
                    // ��ȡ����id
                    hr = m_pBodies[i]->get_TrackingId(&bodyTId);
                    if (SUCCEEDED(hr)) {
                        // ���¸���id
                        m_pFaceFrameSources[i]->put_TrackingId(bodyTId);
                    }
                }
            }
        }
        SafeRelease(pFaceFrame);
    }
}

// ������֡
void ThisApp::check_body_frame(){
    // �ͷŹ�������
    for (int i = 0; i < BODY_COUNT; ++i){
        SafeRelease(m_pBodies[i]);
    }
    // ������֡�¼�����
    IBodyFrameArrivedEventArgs* pArgs = nullptr;
    // ����֡����
    IBodyFrameReference* pBFrameRef = nullptr;
    // ����֡
    IBodyFrame* pBodyFrame = nullptr;

    // ��ȡ����
    HRESULT hr = m_pBodyFrameReader->GetFrameArrivedEventData(m_hBodyFrameArrived, &pArgs);
    // ��ȡ����
    if (SUCCEEDED(hr)) {
        hr = pArgs->get_FrameReference(&pBFrameRef);
    }
    // ��ȡ����֡
    if (SUCCEEDED(hr)) {
        hr = pBFrameRef->AcquireFrame(&pBodyFrame);
    }
    // ��ȡ��������
    if (SUCCEEDED(hr)) {
        hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, m_pBodies);
    }
    SafeRelease(pBodyFrame);
    SafeRelease(pBFrameRef);
    SafeRelease(pArgs);
}