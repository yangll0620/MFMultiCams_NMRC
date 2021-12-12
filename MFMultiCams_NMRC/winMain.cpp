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
#include "DeviceList.h"
#include "CVideo.h"
#include "RenderingWindow.h"


DeviceList  g_devices;
CVideo* g_pVideo = NULL;
HDEVNOTIFY  g_hdevnotify = NULL;


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);

void    OnInitDialog(HWND hDlg);
void    OnCloseDialog();

void    StartCapture(HWND hDlg);

HRESULT UpdateDeviceList(HWND hDlg);


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

            case IDC_CAPTURE:
                StartCapture(hDlg);
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

    // Initialize the COM library
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

    // Initialize Media Foundation
    if (SUCCEEDED(hr))
    {
        hr = MFStartup(MF_VERSION);
    }

    // Register for device notifications
    if (SUCCEEDED(hr))
    {
        DEV_BROADCAST_DEVICEINTERFACE di = { 0 };

        di.dbcc_size = sizeof(di);
        di.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        di.dbcc_classguid = KSCATEGORY_CAPTURE;

        g_hdevnotify = RegisterDeviceNotification(
            hDlg,
            &di,
            DEVICE_NOTIFY_WINDOW_HANDLE
        );

        if (g_hdevnotify == NULL)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
    }

    // Enumerate the video capture devices.
    if (SUCCEEDED(hr))
    {
        hr = UpdateDeviceList(hDlg);
    }
}


//-----------------------------------------------------------------------------
// OnCloseDialog
// 
// Frees resources before closing the dialog.
//-----------------------------------------------------------------------------

void OnCloseDialog()
{
    /*if (g_pCapture)
    {
        g_pCapture->EndCaptureSession();
    }

    SafeRelease(&g_pCapture);

    g_devices.Clear();

    if (g_hdevnotify)
    {
        UnregisterDeviceNotification(g_hdevnotify);
    }

    MFShutdown();
    CoUninitialize();*/
}



//-----------------------------------------------------------------------------
// UpdateDeviceList
// 
// Enumerates the video capture devices and populates the list of device
// names in the dialog UI.
//-----------------------------------------------------------------------------
HRESULT UpdateDeviceList(HWND hDlg)
{
    HRESULT hr = S_OK;

    WCHAR* szFriendlyName = NULL;

    HWND hListbox = GetDlgItem(hDlg, IDC_DEVICE_LIST);

    g_devices.Clear();

    hr = g_devices.EnumerateDevices();

    if (FAILED(hr)) { goto done; }

    for (UINT32 iDevice = 0; iDevice < g_devices.Count(); iDevice++)
    {
        // Get the friendly name of the device.

        hr = g_devices.GetDeviceName(iDevice, &szFriendlyName);
        if (FAILED(hr)) { goto done; }

        // Add the string to the list box. 
        int pos = (int)SendMessage(hListbox, LB_ADDSTRING, 0, (LPARAM)szFriendlyName);
        // Set the array index of the player as item data.
        // This enables us to retrieve the item from the array
        // even after the items are sorted by the list box.
        SendMessage(hListbox, LB_SETITEMDATA, pos, (LPARAM)iDevice);

        CoTaskMemFree(szFriendlyName);
        szFriendlyName = NULL;
    }

done:
    return hr;
}



void StartCapture(HWND hDlg)
{
    IMFActivate* pActivate = NULL;

    UINT32 iDeviceIndex = 0;
    g_devices.GetDevice(0, &pActivate);

    // Create the media source for the capture device.
    g_pVideo = new CVideo(hDlg);

    // Set Source Reader
    g_pVideo->SetSourceReader(pActivate);


    MSG msg{ 0 };
    RenderingWindow window((LPWSTR)"Microsoft Media Foundation Example", g_pVideo->width, g_pVideo->height, 10);
    while (msg.message != WM_QUIT)
    {
        //window.Draw(g_pVideo->rawData, g_pVideo->width, g_pVideo->height);
        while (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
        {
            if (window.windowHandle && IsDialogMessage(window.windowHandle, &msg))
            {
                continue;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

    }
}

