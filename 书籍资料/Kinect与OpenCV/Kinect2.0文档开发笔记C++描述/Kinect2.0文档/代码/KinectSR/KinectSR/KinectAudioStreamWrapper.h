#pragma once


// Kinect ��Ƶ���򵥷�װ
class KinectAudioStreamWrapper : public IStream{
public:
    // ���캯��
    KinectAudioStreamWrapper(IStream *p32BitAudioStream);
    // ��������
    ~KinectAudioStreamWrapper();
    // ɾ��Ĭ�Ϲ���
    KinectAudioStreamWrapper() = delete;
    // ��������״̬
    void SetSpeechState(BOOL state){ m_SpeechActive = state; }
    // IUnknown ���� ʵ��
    STDMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&m_cRef); }
    STDMETHODIMP_(ULONG) Release() {
        UINT ref = InterlockedDecrement(&m_cRef);
        if (ref == 0){
            delete this;
        }
        return ref;
    }
    STDMETHODIMP QueryInterface(REFIID riid, void **ppv) {
        if (riid == IID_IUnknown) {
            AddRef();
            *ppv = (IUnknown*)this;
            return S_OK;
        }
        else if (riid == IID_IStream) {
            AddRef();
            *ppv = (IStream*)this;
            return S_OK;
        }
        else {
            return E_NOINTERFACE;
        }
    }
    // IStream ����
    STDMETHODIMP Read(void *, ULONG, ULONG *);
    STDMETHODIMP Write(const void *, ULONG, ULONG *);
    STDMETHODIMP Seek(LARGE_INTEGER, DWORD, ULARGE_INTEGER *);
    STDMETHODIMP SetSize(ULARGE_INTEGER);
    STDMETHODIMP CopyTo(IStream *, ULARGE_INTEGER, ULARGE_INTEGER *, ULARGE_INTEGER *);
    STDMETHODIMP Commit(DWORD);
    STDMETHODIMP Revert();
    STDMETHODIMP LockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    STDMETHODIMP UnlockRegion(ULARGE_INTEGER, ULARGE_INTEGER, DWORD);
    STDMETHODIMP Stat(STATSTG *, DWORD);
    STDMETHODIMP Clone(IStream **);
private:
    // ���ü���
    UINT                    m_cRef = 1;
    // ���㻺����
    float*                  m_pFloatBuffer = nullptr;
    // ��������С
    UINT                    m_uFloatBuferSize = 0;
    // ��װ����
    IStream*                m_p32BitAudio;
    // ����״̬ ʹ��BOOL��֤���ݶ���
    BOOL                    m_SpeechActive = FALSE;
};