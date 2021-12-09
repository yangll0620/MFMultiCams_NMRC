#include <windows.h>
#include <windowsx.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <assert.h>
#include <strsafe.h>
#include <shlwapi.h>
#include <Dbt.h>
#include <ks.h>
#include <ksmedia.h>


template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


#include "resource.h"


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void    OnInitDialog(HWND hDlg);
void    OnCloseDialog();

HRESULT EnumerateDevices(IMFActivate** m_ppDevices, UINT32 m_cDevices)
{
    HRESULT hr = S_OK;
    IMFAttributes* pAttributes = NULL;


    // Initialize an attribute store. We will use this to 
    // specify the enumeration parameters.

    hr = MFCreateAttributes(&pAttributes, 1);

    // Ask for source type = video capture devices
    if (SUCCEEDED(hr))
    {
        hr = pAttributes->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID
        );
    }

    // Enumerate devices.
    if (SUCCEEDED(hr))
    {
        hr = MFEnumDeviceSources(pAttributes, &m_ppDevices, &m_cDevices);
    }

    SafeRelease(&pAttributes);

    return hr;
}



int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
    INT_PTR ret = DialogBox(
        hInstance,
        MAKEINTRESOURCE(IDD_DIALOG1),
        NULL,
        DialogProc
    );

    if (ret == 0 || ret == -1)
    {
        MessageBox(NULL, L"Could not create dialog", L"Error", MB_OK | MB_ICONERROR);
    }

    return 0;
}


//-----------------------------------------------------------------------------
// Dialog procedure
//-----------------------------------------------------------------------------

INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_INITDIALOG:
        OnInitDialog(hDlg);
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
            case IDCANCEL:
                //OnCloseDialog();
                ::EndDialog(hDlg, IDCANCEL);
                return TRUE;

        }
        break;
    }

    return FALSE;
}


//-----------------------------------------------------------------------------
// OnInitDialog
// Handler for WM_INITDIALOG message.
//-----------------------------------------------------------------------------

void OnInitDialog(HWND hDlg)
{
    HRESULT hr = S_OK;

    HWND hEdit = GetDlgItem(hDlg, IDC_OUTPUT_FILE);
    SetWindowText(hEdit, TEXT("capture.mp4"));

    IMFActivate** m_ppDevices = NULL;
    UINT32 m_cDevices = 0;
    EnumerateDevices(m_ppDevices, m_cDevices);
}


//-----------------------------------------------------------------------------
// OnCloseDialog
// 
// Frees resources before closing the dialog.
//-----------------------------------------------------------------------------

void OnCloseDialog()
{
    CoUninitialize();
}



//-----------------------------------------------------------------------------
// UpdateDeviceList
// 
// Enumerates the video capture devices and populates the list of device
// names in the dialog UI.
//-----------------------------------------------------------------------------



