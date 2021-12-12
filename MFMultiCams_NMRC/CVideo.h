#pragma once
const UINT WM_APP_PREVIEW_ERROR = WM_APP + 1;    // wparam = HRESULT

class CVideo : public IMFSourceReaderCallback
{
public:
    UINT height;
    UINT width;
    GUID videoFormat;
    BYTE* rawData;
    LONG stride;
    int bytesPerPixel;

    CVideo(HWND hwnd);
    ~CVideo();


    HRESULT SetSourceReader(IMFActivate* device);
    HRESULT IsMediaTypeSupported(IMFMediaType* pType);
    HRESULT GetDefaultStride(IMFMediaType* type, LONG* stride);


    // the class must implement the methods from IUnknown
    STDMETHODIMP QueryInterface(REFIID iid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();


    //  the class must implement the methods from IMFSourceReaderCallback
    STDMETHODIMP OnReadSample(HRESULT status, DWORD streamIndex, DWORD streamFlags, LONGLONG timeStamp, IMFSample* sample);
    STDMETHODIMP OnEvent(DWORD, IMFMediaEvent*)
    {
        return S_OK;
    }
    STDMETHODIMP OnFlush(DWORD)
    {
        return S_OK;
    }

protected:
    void    NotifyError(HRESULT hr) { PostMessage(m_hwndEvent, WM_APP_PREVIEW_ERROR, (WPARAM)hr, 0L); }

    long m_nRefCount; // Reference count.
    CRITICAL_SECTION        m_critsec;

    HWND                    m_hwndEvent;        // Application window to receive events.

    WCHAR* m_pwszSymbolicLink;

    IMFSourceReader* m_pReader;

    BOOL                    m_bFirstSample;
    LONGLONG                m_llBaseTime;
};

