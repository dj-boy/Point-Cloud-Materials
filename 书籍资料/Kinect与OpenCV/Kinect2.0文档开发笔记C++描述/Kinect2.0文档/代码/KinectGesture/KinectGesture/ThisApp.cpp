#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"

//#define RTRACE(a)


// ThisApp���캯��
ThisApp::ThisApp(WCHAR* file_name):m_ImagaRenderer(this){
    // �����ַ���
    wcscpy_s(m_szFileNameBuffer, file_name);
}

// ThisApp��������
ThisApp::~ThisApp(){
    // �ͷ� Coordinate Mapper
    SafeRelease(m_pCoordinateMapper);
    // �ͷ� Color Frame Reader
    SafeRelease(m_pColorFrameReader);
    // �ͷ� Depth Frame Reader
    SafeRelease(m_pDepthFrameReader);
    // �ͷŹ�������
    for (int i = 0; i < BODY_COUNT; ++i){
        SafeRelease(m_apBodies[i]);
    }
    // �ͷ���������
    this->release_gesture_data();
    // �ͷ� Gesture Frame Reader
    SafeRelease(m_pGestureFrameReader);
    // �ͷ� Gesture Frame Source
    SafeRelease(m_pGestureFrameSource);
    // ��̬�������ڴ�Ļ���Ҫ�ͷŵ�
    if (m_ppGestures != m_apGestures){
        delete[] m_ppGestures;
    }
    m_ppGestures = nullptr;
    m_cGesturesBufferSize = 0;
    // �ͷ�BodyFrameReader
    SafeRelease(m_pBodyFrameReader);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    SafeRelease(m_pKinect);
}

// ���������ļ�
HRESULT ThisApp::load_gesture_database_file(const WCHAR* file_name){
    if (*file_name == 0) return S_FALSE;
    // �����ַ���
    if (file_name != m_szFileNameBuffer)
        wcscpy_s(m_szFileNameBuffer, file_name);
    // ���ͷ�
    this->release_gesture_data();
    // ������
    HRESULT hr = S_OK;
    // ���������ļ�
    if (SUCCEEDED(hr)){
        hr = CreateVisualGestureBuilderDatabaseInstanceFromFile(file_name, &m_pGestureDatabase);
    }
    // ��ȡ��������
    if (SUCCEEDED(hr)){
        hr = m_pGestureDatabase->get_AvailableGesturesCount(&m_cGestureSize);
    }
    // ���治������������
    if (SUCCEEDED(hr)){
        if (m_cGestureSize > m_cGesturesBufferSize){
            // ��չΪ2��
            m_cGesturesBufferSize = m_cGestureSize * 2;
            m_vGesturesInfo.resize(m_cGesturesBufferSize);
            // �Ѿ�����������ͷ�
            if (m_ppGestures != m_apGestures) delete[] m_ppGestures;
            // ����
            m_ppGestures = new IGesture*[m_cGesturesBufferSize];
            // ���ָ��
            if (!m_ppGestures) hr = E_OUTOFMEMORY;
        }
    }
    // ��ȡ����
    if (SUCCEEDED(hr)){
        hr = m_pGestureDatabase->get_AvailableGestures(m_cGestureSize, m_apGestures);
    }
    // �������
    if (SUCCEEDED(hr)){
        hr = m_pGestureFrameSource->AddGestures(m_cGestureSize, m_apGestures);
    }
    return hr;
}
// �ͷ�����
void ThisApp::release_gesture_data(){
    // �ͷ���������ָ��
    for (UINT i = 0U; i < m_cGestureSize; ++i){
        // ȥ������
        m_pGestureFrameSource->RemoveGesture(m_ppGestures[i]);
        // �ͷ�����
        SafeRelease(m_ppGestures[i]);
    }
    m_cGestureSize = 0;
    // �ͷ� Gesture Database
    SafeRelease(m_pGestureDatabase);
}

// ��ʼ��
HRESULT ThisApp::Initialize(HINSTANCE hInstance, int nCmdShow){
    HRESULT hr = E_FAIL;
    if (SUCCEEDED(static_cast<HRESULT>(m_ImagaRenderer)))
    {
        //register window class
        WNDCLASSEXW wcex = { sizeof(WNDCLASSEXW) };
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = ThisApp::WndProc;
        wcex.cbClsExtra = 0;
        wcex.cbWndExtra = sizeof(LONG_PTR);
        wcex.hInstance = hInstance;
        wcex.hCursor = LoadCursorW(nullptr, IDC_ARROW);
        wcex.hbrBackground = nullptr;
        wcex.lpszMenuName = nullptr;
        wcex.lpszClassName = L"Direct2DTemplate";
        wcex.hIcon = LoadIconW(hInstance, MAKEINTRESOURCEW(1));;
        // ע�ᴰ��
        RegisterClassExW(&wcex);
        // ��������
        RECT window_rect = { 0, 0, WNDWIDTH, WNDHEIGHT };
        DWORD window_style = WS_OVERLAPPEDWINDOW;
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
            // ������ק
            DragAcceptFiles(m_hwnd, TRUE);
            // ���ö�ʱ��
            SetTimer(m_hwnd, 1, 20, nullptr);
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
        if (FAILED(pOurApp->init_kinect()) || FAILED(pOurApp->load_gesture_database_file(pOurApp->m_szFileNameBuffer))){
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
            case WM_TIMER:
                // ���
                pOurApp->check_color_frame();
                pOurApp->check_depth_frame();
                pOurApp->check_body_frame();
                pOurApp->check_gesture_frame();
                // ��Ⱦ
                pOurApp->m_ImagaRenderer.OnRender();
                break;
            case WM_MOUSEWHEEL:
                pOurApp->m_ImagaRenderer.show_offset += static_cast<float>(GET_WHEEL_DELTA_WPARAM(wParam) * 0.25f);
                //pOurApp->m_ImagaRenderer.OnRender();
                break;
            case WM_DROPFILES:
                // �Ϸ��ļ�
            {
                HDROP hDrop = reinterpret_cast<HDROP>(wParam);
                WCHAR buffer[MAX_PATH * 4];
                // ��ȡ��һ��
                DragQueryFileW(hDrop, 0, buffer, lengthof(buffer));
                pOurApp->load_gesture_database_file(buffer);
                // �ͷſռ�
                DragFinish(hDrop);
            }
                break;
            case WM_SIZE:
                // �ı䴰�ڴ�С
                pOurApp->m_ImagaRenderer.OnSize(LOWORD(lParam), HIWORD(lParam));
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
    IBodyFrameSource* pBodyFrameSource = nullptr;
    IColorFrameSource* pColorFrameSource = nullptr;
    IDepthFrameSource* pDepthFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // --------------  ����֡���� --------------
    // ��������֡Դ
    if (SUCCEEDED(hr)){
        hr = CreateVisualGestureBuilderFrameSource(m_pKinect, 0, &m_pGestureFrameSource);
    }
    // ��ȡ��ȡ��
    if (SUCCEEDED(hr)){
        hr = m_pGestureFrameSource->OpenReader(&m_pGestureFrameReader);
    }
    // --------------  ��ɫ֡���� --------------
    // ��ȡ��ɫ֡Դ(ColorFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_ColorFrameSource(&pColorFrameSource);
    }
    // �ٻ�ȡ��ɫ֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pColorFrameSource->OpenReader(&m_pColorFrameReader);
    }
    // --------------  ���֡���� --------------
    // ��ȡ���֡Դ(DepthFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_DepthFrameSource(&pDepthFrameSource);
    }
    // �ٻ�ȡ��ɫ֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
    }
    // --------------  ����֡���� --------------
    // ��ȡ����֡Դ(BodyFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_BodyFrameSource(&pBodyFrameSource);
    }
    // �ٻ�ȡ����֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pBodyFrameSource->OpenReader(&m_pBodyFrameReader);
    }
    // --------------  ����ӳ�������� --------------
    // ��ȡ����ӳ����
    if (SUCCEEDED(hr)) {
        hr = m_pKinect->get_CoordinateMapper(&m_pCoordinateMapper);
    }
    // ɨβ����
    SafeRelease(pColorFrameSource);
    SafeRelease(pDepthFrameSource);
    SafeRelease(pBodyFrameSource);
    
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
    if (!pColorFrame) return;
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
    // ��ȡ֡������
    if (SUCCEEDED(hr)) {
        // ���Ѿ���BGRA������� ֱ�ӻ�ȡԴ����
        if (imageFormat == ColorImageFormat_Bgra) {
            hr = pColorFrame->AccessRawUnderlyingBuffer(&nBufferSize, reinterpret_cast<BYTE**>(&pBuffer));
        }
        // �������Դ��ķ���ת��
        else{
            pBuffer = m_ImagaRenderer.GetColorBuffer();
            nBufferSize = IMAGE_WIDTH * IMAGE_HEIGHT * sizeof(RGBQUAD);
            hr = pColorFrame->CopyConvertedFrameDataToArray(nBufferSize, reinterpret_cast<BYTE*>(pBuffer), ColorImageFormat_Bgra);
        }
    }
    // ��������
    if (SUCCEEDED(hr)){
        m_ImagaRenderer.WriteBitmapData(pBuffer, 0, width, height);
    }

    SafeRelease(pColorFrame);
    SafeRelease(pColorFrameDescription);
}

// ������֡
void ThisApp::check_depth_frame(){
    if (!m_pDepthFrameReader) return;
    // ���֡
    IDepthFrame* pDepthFrame = nullptr;
    // ֡����
    IFrameDescription* pFrameDescription = nullptr;
    // ���֡�������
    int width = 0;
    // ���֡�߶�����
    int height = 0;
    // �����Чֵ
    USHORT depth_min_reliable_distance = 0;
    // ��Զ��Чֵ
    USHORT depth_max_reliable_distance = 0;
    // ֡�����С
    UINT nBufferSize = 0;
    // ��Ȼ���
    UINT16 *pBuffer = nullptr;

    // ��ȡ���֡
    HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
    if (!pDepthFrame) return;
    // ��ȡ֡����
    if (SUCCEEDED(hr)) {
        hr = pDepthFrame->get_FrameDescription(&pFrameDescription);
    }
    // ��ȡ֡���
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Width(&width);
    }
    // ��ȡ֡�߶�
    if (SUCCEEDED(hr)) {
        hr = pFrameDescription->get_Height(&height);
    }
    // ��ȡ�����Ч����ֵ
    if (SUCCEEDED(hr)) {
        hr = pDepthFrame->get_DepthMinReliableDistance(&depth_min_reliable_distance);
    }
    // ��ȡ��Զ��Ч����ֵ
    if (SUCCEEDED(hr))  {
        hr = pDepthFrame->get_DepthMaxReliableDistance(&depth_max_reliable_distance);
    }
    // ��ȡ�������
    if (SUCCEEDED(hr))  {
        hr = pDepthFrame->AccessUnderlyingBuffer(&nBufferSize, &pBuffer);
    }
    // ��������
    if (SUCCEEDED(hr)) {
        auto pRGBXBuffer = m_ImagaRenderer.GetDepthBuffer();
        // �����㷨
        // 0�ź�ɫ (0, min)��128~255������ɫ ����max����128~255��ɫ ֮�����0~255��ɫ
        // ��ͬ��Ƚ�������
        for (UINT i = 0; i < nBufferSize; ++i){
            if (!pBuffer[i]){
                pRGBXBuffer[i].rgbRed = 0xFF;
                pRGBXBuffer[i].rgbGreen = 0;
                pRGBXBuffer[i].rgbBlue = 0;
                pRGBXBuffer[i].rgbReserved = 0xFF;
            }
            else if (pBuffer[i] < depth_min_reliable_distance){
                pRGBXBuffer[i].rgbRed = 0;
                pRGBXBuffer[i].rgbGreen = pBuffer[i] & 0x7F + 0x80;
                pRGBXBuffer[i].rgbBlue = 0;
                pRGBXBuffer[i].rgbReserved = 0xFF;
            }
            else if (pBuffer[i] > depth_max_reliable_distance){
                pRGBXBuffer[i].rgbBlue = pBuffer[i] & 0x7F + 0x80;
                pRGBXBuffer[i].rgbGreen = 0;
                pRGBXBuffer[i].rgbRed = 0;
                pRGBXBuffer[i].rgbReserved = 0xFF;
            }
            else{
                pRGBXBuffer[i].rgbBlue = pBuffer[i] & 0xFF;
                pRGBXBuffer[i].rgbGreen = pRGBXBuffer[i].rgbBlue;
                pRGBXBuffer[i].rgbRed = pRGBXBuffer[i].rgbBlue;
                pRGBXBuffer[i].rgbReserved = 0xFF;
            }
        }
        // ��������
        m_ImagaRenderer.WriteBitmapData(pRGBXBuffer, 1, width, height);
    }
    // ��ȫ�ͷ�
    SafeRelease(pFrameDescription);
    SafeRelease(pDepthFrame);
}

// �������֡
void ThisApp::check_gesture_frame(){
    // ��ȡ���һ֡����
    IVisualGestureBuilderFrame* pGestureFrame = nullptr;
    if (SUCCEEDED(m_pGestureFrameReader->CalculateAndAcquireLatestFrame(&pGestureFrame))){
        // �������������
        IContinuousGestureResult* pCGResult = nullptr;
        // �����ɢ������
        IDiscreteGestureResult* pDGResult = nullptr;
        // ѭ�����
        for (UINT i = 0U; i < m_cGestureSize; ++i){
            // ��ȡ��Ϣ
            auto info_data = &(m_vGesturesInfo[i]);
            // �ƽ�����
            if (info_data->index >= GESTURES_SAVED){
                info_data->index = 0;
            }
            else{
                ++info_data->index;
            }
            // ��ȡ���������ƽ��
            pGestureFrame->get_ContinuousGestureResult(m_apGestures[i], &pCGResult);
            if (pCGResult){
                info_data->type = 0;
                pCGResult->get_Progress(info_data->float_var + info_data->index);
                // �ͷŶ���
                pCGResult->Release();
                pCGResult = nullptr;
                continue;
            }
            // ��ȡ��ɢ�����ƽ��
            pGestureFrame->get_DiscreteGestureResult(m_apGestures[i], &pDGResult);
            if (pDGResult){
                info_data->type = 1;
                // ��ȡ����ֵ
                pDGResult->get_Confidence(info_data->float_var + info_data->index);
                // ��ȡ���״̬
                pDGResult->get_Detected(info_data->bool1_var + info_data->index);
                // ��ȡ��֡״̬
                pDGResult->get_FirstFrameDetected(info_data->bool2_var + info_data->index);
                // �ͷŶ���
                pDGResult->Release();
                pDGResult = nullptr;
            }
        }
    }
    SafeRelease(pGestureFrame);
}

// ������֡
void ThisApp::check_body_frame(){
    IBodyFrame* pBodyFrame = nullptr;

    // ��ȡ����֡
    HRESULT hr = m_pBodyFrameReader->AcquireLatestFrame(&pBodyFrame);
    if (!pBodyFrame) return;
    // ��ȡ��������
    if (SUCCEEDED(hr)) {
        hr = pBodyFrame->GetAndRefreshBodyData(BODY_COUNT, m_apBodies);
    }
    // ��������
    if (SUCCEEDED(hr)){
        for (int i = 0; i < BODY_COUNT; ++i){
            // ��֤����
            BOOLEAN bTracked = false;
            hr = m_apBodies[i]->get_IsTracked(&bTracked);
            if (!m_apBodies[i] || FAILED(hr) || !bTracked) continue;
            IBody* pNowBody = m_apBodies[i];
            // ��������
            BodyInfo info;
            // ��ȡ������״̬
            pNowBody->get_HandLeftState(&info.leftHandState);
            pNowBody->get_HandRightState(&info.rightHandState);
            // ��ȡ����λ����Ϣ
            hr = pNowBody->GetJoints(JointType_Count, info.joints);
            if (SUCCEEDED(hr)) {
                // ����ת��
                for (int j = 0; j < JointType_Count; ++j){
                    m_pCoordinateMapper->MapCameraPointToDepthSpace(
                        info.joints[j].Position,
                        reinterpret_cast<DepthSpacePoint*>(info.jointPoints + j)
                        );
                }
                // ��������
                m_ImagaRenderer.SetBodyInfo(i, &info);
            }
        }
    }
    // ��ȫ�ͷ�
    SafeRelease(pBodyFrame);
    // ������֡û�и���ʱ���Ը���id
    BOOLEAN tracked = TRUE;
    m_pGestureFrameSource->get_IsTrackingIdValid(&tracked);
    if (tracked) return;
    // ����
    UINT64 id = 0;
    for (int i = 0; i < BODY_COUNT; ++i){
        m_apBodies[i]->get_IsTracked(&tracked);
        if (tracked){
            m_apBodies[i]->get_TrackingId(&id);
            m_pGestureFrameSource->put_TrackingId(id);
#ifdef _DEBUG
            _cwprintf(L"����: ����ID: %l64d\n", id);
#endif
            break;
        }
    }
}

