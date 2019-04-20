#include "stdafx.h"
#include "included.h"

#define TITLE L"Title"
#define WNDWIDTH 1024
#define WNDHEIGHT 768

#define lengthof(a) (sizeof(a)/sizeof(*a))

#define SRLANGUAGE L""


// ThisApp���캯��
ThisApp::ThisApp(){
    if (SUCCEEDED(init_kinect()) && SUCCEEDED(init_speech_recognizer())){
        m_threadAudio.std::thread::~thread();
        m_threadAudio.std::thread::thread(AudioThread, this);
    }
    else{
        m_threadAudio.std::thread::~thread();
        m_threadAudio.std::thread::thread(AudioThread, this);
    }
}

// ThisApp��������
ThisApp::~ThisApp(){
    // ���ŵعر�Kinect
    if (m_pKinect){
        m_pKinect->Close();
    }
    // �ͷ�����ʶ�����
    SafeRelease(m_pSpeechGrammar);
    SafeRelease(m_pSpeechContext);
    SafeRelease(m_pSpeechRecognizer);
    SafeRelease(m_pSpeechStream);
    // �رհ�װ��
    SafeRelease(m_p16BitPCMAudioStream);
    SafeRelease(m_pAudioBeam);
    SafeRelease(m_pKinect);
    //�ر��¼�
    if (m_hExit){
        ::CloseHandle(m_hExit);
        m_hExit = nullptr;
    }
    if (m_hSpeechEvent){
        // SR��رյ�
        //::CloseHandle(m_hSpeechEvent);
        m_hSpeechEvent = nullptr;
    }
}


// ��Ƶ�߳�
void ThisApp::AudioThread(ThisApp* pointer){
    // ������
    HANDLE events[] = { pointer->m_hExit, pointer->m_hSpeechEvent };
    bool exit = false;
    while (!exit) {
        switch (::WaitForMultipleObjects(lengthof(events), events, FALSE, INFINITE)){
        case WAIT_OBJECT_0 + 0:
            // �˳�����
            exit = true;
            pointer->m_p16BitPCMAudioStream->SetSpeechState(FALSE);
            break;
        case WAIT_OBJECT_0 + 1:
            // ����ʶ��
            pointer->speech_process();
            break;
        }
    }
}

// ��ʼ��Kinect
HRESULT ThisApp::init_kinect(){
    IAudioSource* pAudioSource = nullptr;
    IAudioBeamList* pAudioBeamList = nullptr;
    // ���ҵ�ǰĬ��Kinect
    HRESULT hr = ::GetDefaultKinectSensor(&m_pKinect);
    // ��ʿ�ش�Kinect
    if (SUCCEEDED(hr)){
        hr = m_pKinect->Open();
    }
    // ��ȡ��ƵԴ
    if (SUCCEEDED(hr)){
        hr = m_pKinect->get_AudioSource(&pAudioSource);
    }
    // ��ȡ��Ƶ����
    if (SUCCEEDED(hr)){
        hr = pAudioSource->get_AudioBeams(&pAudioBeamList);
    }
    // ��ȡ��Ƶ
    if (SUCCEEDED(hr)){
        hr = pAudioBeamList->OpenAudioBeam(0, &m_pAudioBeam);
    }
    // ��ȡ������Ƶ��
    if (SUCCEEDED(hr)){
        IStream* pStream = nullptr;
        hr = m_pAudioBeam->OpenInputStream(&pStream);
        // ���ÿ������ɰ�װ����
        m_p16BitPCMAudioStream = new KinectAudioStreamWrapper(pStream);
        SafeRelease(pStream);
    }
    SafeRelease(pAudioBeamList);
    SafeRelease(pAudioSource);
    return hr;
}


// ��ʼ������ʶ��
HRESULT ThisApp::init_speech_recognizer(){
    HRESULT hr = S_OK;
    // ��������������
    if (SUCCEEDED(hr)){
        hr = CoCreateInstance(CLSID_SpStream, nullptr, CLSCTX_INPROC_SERVER, __uuidof(ISpStream), (void**)&m_pSpeechStream);;
    }
    // �����ǵ�Kinect��������������
    if (SUCCEEDED(hr)){
        WAVEFORMATEX wft = {
            WAVE_FORMAT_PCM, // PCM����
            1, // ������
            16000,  // ������Ϊ16KHz
            32000, // ÿ���������� = ������ * ����
            2, // ���� : ������ * ������� = 2byte
            16, // ������� 16BIT
            0 // ��������
        };
        // ����״̬
        hr = m_pSpeechStream->SetBaseStream(m_p16BitPCMAudioStream, SPDFID_WaveFormatEx, &wft);
    }
    // ��������ʶ�����
    if (SUCCEEDED(hr)){
        ISpObjectToken *pEngineToken = nullptr;
        // ��������ʶ����
        hr = CoCreateInstance(CLSID_SpInprocRecognizer, nullptr, CLSCTX_INPROC_SERVER, __uuidof(ISpRecognizer), (void**)&m_pSpeechRecognizer);
        if (SUCCEEDED(hr)) {
            // �������Ǵ�������������������
            m_pSpeechRecognizer->SetInput(m_pSpeechStream, TRUE);
            // ������ʶ������ ����ѡ���½����(zh-cn) 
            // Ŀǰû��Kinect�ĺ�������ʶ��� �еĻ���������"language=804;Kinect=Ture"
            hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"Language=804", nullptr, &pEngineToken);
            if (SUCCEEDED(hr)) {
                // ���ô�ʶ������
                m_pSpeechRecognizer->SetRecognizer(pEngineToken);
                // ��������ʶ��������
                hr = m_pSpeechRecognizer->CreateRecoContext(&m_pSpeechContext);
                // ��Ӧ�� ON! ��ֹ��ʱ��Ĵ��������ʶ���������˻�
                if (SUCCEEDED(hr))  {
                    hr = m_pSpeechRecognizer->SetPropertyNum(L"AdaptationOn", 0);
                }
            }
        }
        SafeRelease(pEngineToken);
    }
    // �����﷨
    if (SUCCEEDED(hr)){
        hr = m_pSpeechContext->CreateGrammar(1, &m_pSpeechGrammar);
    }
    // ���ؾ�̬SRGS�﷨�ļ�
    if (SUCCEEDED(hr)){
        hr = m_pSpeechGrammar->LoadCmdFromFile(s_GrammarFileName, SPLO_STATIC);
    }
    // �����﷨����
    if (SUCCEEDED(hr)){
        hr = m_pSpeechGrammar->SetRuleState(nullptr, nullptr, SPRS_ACTIVE);
    }
    // ����ʶ����һֱ��ȡ����
    if (SUCCEEDED(hr)){
        hr = m_pSpeechRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
    }
    // ���ö�ʶ���¼�����Ȥ
    if (SUCCEEDED(hr)){
        hr = m_pSpeechContext->SetInterest(SPFEI(SPEI_RECOGNITION), SPFEI(SPEI_RECOGNITION));
    }
    // ��֤����ʶ���ڼ���״̬
    if (SUCCEEDED(hr)){
        hr = m_pSpeechContext->Resume(0);
    }
    // ��ȡʶ���¼�
    if (SUCCEEDED(hr)){
        m_p16BitPCMAudioStream->SetSpeechState(TRUE);
        m_hSpeechEvent = m_pSpeechContext->GetNotifyEventHandle();
    }
#ifdef _DEBUG
    else
        printf_s("init_speech_recognizer failed\n");
#endif
    return hr;
}


// ��Ƶ����
void ThisApp::speech_process() {
    // ������ֵ
    const float ConfidenceThreshold = 0.3f;

    SPEVENT curEvent = { SPEI_UNDEFINED, SPET_LPARAM_IS_UNDEFINED, 0, 0, 0, 0 };
    ULONG fetched = 0;
    HRESULT hr = S_OK;
    // ��ȡ�¼�
    m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
    while (fetched > 0)
    {
        // ȷ����ʶ���¼�
        switch (curEvent.eEventId)
        {
        case SPEI_RECOGNITION:
            // ��֤λ����
            if (SPET_LPARAM_IS_OBJECT == curEvent.elParamType) {
                ISpRecoResult* result = reinterpret_cast<ISpRecoResult*>(curEvent.lParam);
                SPPHRASE* pPhrase = nullptr;
                // ��ȡʶ�����
                hr = result->GetPhrase(&pPhrase);
                if (SUCCEEDED(hr)) {
#ifdef _DEBUG
                    // DEBUGʱ��ʾʶ���ַ���
                    WCHAR* pwszFirstWord;
                    result->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &pwszFirstWord, nullptr);
                    _cwprintf(pwszFirstWord);
                    ::CoTaskMemFree(pwszFirstWord);
#endif
                    pPhrase->pProperties;
                    const SPPHRASEELEMENT* pointer = pPhrase->pElements + 1;
                    if ((pPhrase->pProperties != nullptr) && (pPhrase->pProperties->pFirstChild != nullptr)) {
                        const SPPHRASEPROPERTY* pSemanticTag = pPhrase->pProperties->pFirstChild;
#ifdef _DEBUG
                        _cwprintf(L"   ���Ŷ�:%d%%\n", (int)(pSemanticTag->SREngineConfidence*100.f));
#endif
                        if (pSemanticTag->SREngineConfidence > ConfidenceThreshold) {
                            speech_behavior(pSemanticTag);
                        }
                    }
                    ::CoTaskMemFree(pPhrase);
                }
            }
            break;
        }

        m_pSpeechContext->GetEvents(1, &curEvent, &fetched);
    }

    return;
}


// ������Ϊ
void ThisApp::speech_behavior(const SPPHRASEPROPERTY* tag){
    if (!tag) return;
    if (!wcscmp(tag->pszName, L"ս��")){
        enum class Subject{
            US = 0,
            Enemy
        } ;
        enum class Predicate{
            Destroy = 0,
            Defeat,
            Breakdown
        };
        // ����ս��
        union  Situation{
            struct{
                // ����
                Subject subject;
                // ν��
                Predicate predicate;
                // ����
                int object2;
                // ����
                int object;

            };
            UINT32 data[4];
        };
        Situation situation;
        auto obj = tag->pFirstChild;
        auto pointer = situation.data;
        // ��д����
        while (obj) {
            *pointer = obj->vValue.lVal;
            ++pointer;
            obj = obj->pNextSibling;
        }
        // XXX
    }
    else if (!wcscmp(tag->pszName, L"���ֶ���")){
        // ���ֶ���
    }
}