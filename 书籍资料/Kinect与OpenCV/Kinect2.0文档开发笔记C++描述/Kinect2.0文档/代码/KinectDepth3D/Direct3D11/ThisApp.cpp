#include "stdafx.h"
#include "included.h"

//#define TITLE L"Title"


// ThisApp���캯��
ThisApp::ThisApp(){

}

// ThisApp��������
ThisApp::~ThisApp(){
    // �ͷ� Depth Frame Reader
    SafeRelease(m_pDepthFrameReader);
    // �ͷ� Mapper
    SafeRelease(m_pMapper);
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
        m_pKinect->Release();
        m_pKinect = nullptr;
    }
}

// ��Ⱦ����
void ThisApp::Render(ThisApp* pThis){
    // ��Ⱦ
    while (true){
        pThis->Update();
        pThis->m_SceneRenderer.OnRender();
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
    wcex.lpszClassName = L"D3D11 Template";
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
        hr = m_SceneRenderer.SetHwnd(m_hwnd);
    }
#ifdef USING_DIRECTINPUT
    // DInput...
    if (SUCCEEDED(hr)){
        hr = KMInput.Init(hInstance, m_hwnd);
    }
#endif
    // ��ʼ����Ϸ����
    if (SUCCEEDED(hr)){
        this->initialize_game_objects();
    }
    // ��ʾ����
    if (SUCCEEDED(hr)) {
        ShowWindow(m_hwnd, nCmdShow);
        UpdateWindow(m_hwnd);
        m_threadRender.std::thread::~thread();
        m_threadRender.std::thread::thread(Render, this);
    }
#ifdef USING_GAMEIME
    // IME...
    if (SUCCEEDED(hr)){
        m_gameIME.SetHwnd(m_hwnd);
    }
#endif
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
#ifdef USING_GAMEIME
    uint32_t temp_input;
#endif

    if (message == WM_CREATE)
    {
        LPCREATESTRUCT pcs = (LPCREATESTRUCT)lParam;
        ThisApp *pOurApp = (ThisApp *)pcs->lpCreateParams;

        ::SetWindowLongPtrW(
            hwnd,
            GWLP_USERDATA,
            PtrToUlong(pOurApp)
            );

        if (SUCCEEDED(pOurApp->init_kinect())){
            result = 1;
        }
        else{
            result = 0;
        }
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
#ifdef USING_GAMEIME
            uint32_t flags = pOurApp->m_imeFlag;
#endif
            switch (message)
            {
#ifdef USING_GAMEIME
            case WM_INPUTLANGCHANGE:
                // ���뷨�ı�
                flags |= GameIME_LangChange;
                break;
            case WM_IME_STARTCOMPOSITION:
                pOurApp->m_cInputCount = 0;
                // �̾����뿪ʼ
                flags |= GameIME_StartComposition;
                break;
            case WM_IME_ENDCOMPOSITION:
                // �̾��������
                flags |= GameIME_EndComposition;
                break;
            case WM_IME_NOTIFY:
                // �������ϲ㴦��
                result = 1;
                wasHandled = true;
                flags |= 1 << wParam;
                break;
            case WM_CHAR:
                break;
            case WM_IME_CHAR:
                // ����
                temp_input = pOurApp->m_cInputCount;
                pOurApp->m_mux4Input.lock();
                pOurApp->m_scBuffer4Input[temp_input] = static_cast<WCHAR>(wParam);
                ++temp_input;
                pOurApp->m_scBuffer4Input[temp_input] = 0;
                // ����
                pOurApp->m_mux4Input.unlock();
                pOurApp->m_cInputCount = temp_input;
                break;
#endif
            case WM_LBUTTONDOWN:
                // ��¼XYλ��
                pOurApp->m_lastFrameX = LOWORD(lParam);
                pOurApp->m_lastFrameY = HIWORD(lParam);
                break;
            case WM_MOUSEWHEEL:
                if (GET_WHEEL_DELTA_WPARAM(wParam) > 0){
                    pOurApp->m_SceneRenderer.z = pOurApp->m_SceneRenderer.z * 0.9f;
                }
                else{
                    pOurApp->m_SceneRenderer.z = pOurApp->m_SceneRenderer.z * 1.1f;
                }
                break;
            case WM_MOUSEMOVE:
                // �������
                if (wParam & MK_LBUTTON){
                    int tx = LOWORD(lParam);
                    int ty = HIWORD(lParam);
                    // �޸�
                    pOurApp->m_SceneRenderer.x = pOurApp->m_SceneRenderer.x + static_cast<float>(tx - pOurApp->m_lastFrameX) / 256.f;
                    pOurApp->m_SceneRenderer.y = pOurApp->m_SceneRenderer.y + static_cast<float>(ty - pOurApp->m_lastFrameY) / 256.f;
                    //
                    pOurApp->m_lastFrameX = tx;
                    pOurApp->m_lastFrameY = ty;
                }
                break;
            case WM_CLOSE:
                // ����β����(�����ȫ�����߳�)��������
                pOurApp->m_bExit = TRUE;
                // join
                pOurApp->m_threadRender.join();
                // ����ʼ��
                pOurApp->finalize_game_objects();
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
#ifdef USING_GAMEIME
            pOurApp->m_imeFlag = flags;
#endif
        }
        if (!wasHandled)
        {
            result = DefWindowProc(hwnd, message, wParam, lParam);
        }
    }

    return result;
}


// ˢ�¶���
void ThisApp::Update(){
    check_depth_frame();
}


// ��ʼ����Ϸ����
void ThisApp::initialize_game_objects(){
    /*ID3D11Device* device = m_SceneRenderer.Get3DDevice();
    
    SafeRelease(device);*/
}

// ����ʼ����Ϸ����
void ThisApp::finalize_game_objects(){


}




// ��ʼ��Kinect
HRESULT ThisApp::init_kinect(){
    IDepthFrameSource* pDepthFrameSource = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��ȡ���֡Դ(DepthFrameSource)
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_DepthFrameSource(&pDepthFrameSource);
    }
    // �ٻ�ȡ���֡��ȡ��
    if (SUCCEEDED(hr)){
        hr = pDepthFrameSource->OpenReader(&m_pDepthFrameReader);
    }
    // ��ȡ��ɫ֡֡����
    if(SUCCEEDED(hr)){
            float angle_y=45.f;
            // ����͸��ת��
            auto projection = DirectX::XMMatrixPerspectiveFovLH(
                DirectX::XM_PI * angle_y / 180.f,
                static_cast<float>(WNDWIDTH) / (static_cast<float>(WNDHEIGHT)),
                SCREEN_NEAR_Z,
                SCREEN_FAR_Z
                );
            // ͸��
            DirectX::XMStoreFloat4x4(
                &m_SceneRenderer.scene_model.vscb.projection,
                projection
                );
        
    }
    // ��ȡӳ����
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_CoordinateMapper(&m_pMapper);
    }
    SafeRelease(pDepthFrameSource);
    return hr;
}

// ������֡
void ThisApp::check_depth_frame(){
    if (!m_pDepthFrameReader) return;
#ifdef _DEBUG
    static int frame_num = 0;
    ++frame_num;
    _cwprintf(L"<ThisApp::check_depth_frame>Frame@%8d ", frame_num);
#endif
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
    // ��ȡ����һ֡
    HRESULT hr = m_pDepthFrameReader->AcquireLatestFrame(&pDepthFrame);
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
    // ӳ��Ϊ����ռ�����
    if (SUCCEEDED(hr)) {
        hr = m_pMapper->MapDepthFrameToCameraSpace(
            DEPTH_WIDTH*DEPTH_HEIGHT,
            pBuffer,
            m_SceneRenderer.scene_model.GetVertexCount(),
            m_SceneRenderer.scene_model.GetVB()
            );
    }
    // ��������
    if (SUCCEEDED(hr)){
        m_SceneRenderer.RefreshData();
    }
    // ��ȫ�ͷ�
    SafeRelease(pFrameDescription);
    SafeRelease(pDepthFrame);
#ifdef _DEBUG
    if (SUCCEEDED(hr))
        _cwprintf(L"      �ɹ�\n");
    else
        _cwprintf(L" ʧ��\n");
#endif
}