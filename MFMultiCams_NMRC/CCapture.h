#pragma once

const UINT WM_APP_PREVIEW_ERROR = WM_APP + 1;    // wparam = HRESULT

struct EncodingParameters
{
    GUID    subtype;
    UINT32  bitrate;
};

class CCapture : public IMFSourceReaderCallback
{
public:
	HRESULT     EndCaptureSession();

protected:

    enum State
    {
        State_NotReady = 0,
        State_Ready,
        State_Capturing,
    };

    // Constructor is private. Use static CreateInstance method to instantiate.
    CCapture(HWND hwnd);

    // Destructor is private. Caller should call Release.
    virtual ~CCapture();

    void    NotifyError(HRESULT hr) { PostMessage(m_hwndEvent, WM_APP_PREVIEW_ERROR, (WPARAM)hr, 0L); }

    HRESULT OpenMediaSource(IMFMediaSource* pSource);
    HRESULT ConfigureCapture(const EncodingParameters& param);
    HRESULT EndCaptureInternal();

    long                    m_nRefCount;        // Reference count.
    CRITICAL_SECTION        m_critsec;

    HWND                    m_hwndEvent;        // Application window to receive events. 

    IMFSourceReader* m_pReader;
    IMFSinkWriter* m_pWriter;

    BOOL                    m_bFirstSample;
    LONGLONG                m_llBaseTime;

    WCHAR* m_pwszSymbolicLink;
};

