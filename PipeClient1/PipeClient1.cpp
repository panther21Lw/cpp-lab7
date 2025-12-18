// PipeClient1.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "PipeClient1.h"
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_LOADSTRING 100

static HWND hwndEdit;

// Ідентифікатор каналу Pipe
HANDLE hNamedPipe;

// Дескриптор каналу
HANDLE hPipe;

// Ім'я конвеєра, що створює сервер
LPCSTR pipeName = "\\\\.\\pipe\\lab7pipe";

// Буфер для передачі даних
char czBuf[512];

// Кількість записаних байтів даних
DWORD cbWritten;

// Буфер для відправки повідомлень у клієнтську область вікна
char mess[2048];
char* m_mess = mess;


// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PIPECLIENT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIPECLIENT1));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIPECLIENT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PIPECLIENT1);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       770, 150,
       430, 275,
       nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        hwndEdit = CreateWindowExW(
            0, L"EDIT", nullptr,
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
            10, 10, 400, 200,
            hWnd, nullptr, hInst, nullptr);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);

            int batteryPercent = 0;
            const char* chargerStatus = "невідомо";
            SYSTEM_POWER_STATUS sps = {};
            int freq = 0;
            int colorDepth = 0;
            HDC hdc = nullptr;
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_PIPE_SENDMESSAGE:
                hPipe = CreateFileA(
                    pipeName,
                    GENERIC_WRITE,
                    0,
                    NULL,
                    OPEN_EXISTING,
                    0,
                    NULL
                );
                if (hPipe == INVALID_HANDLE_VALUE) {
                    MessageBox(hWnd, L"Не вдалося під'єднатися до каналу", L"Помилка", MB_OK);
                    break;
                }

                // формування повідомлення для відправлення

            // стан батареї
                if (GetSystemPowerStatus(&sps))
                {
                    // рівень заряду у відсотках
                    batteryPercent = sps.BatteryLifePercent;
                    // чи підключено адаптер живлення
                    chargerStatus = (sps.ACLineStatus == 1) ? "під'єднано" : "не під'єднано";
                }
                hdc = GetDC(hWnd);
                if (hdc) {
                    // частота оновлення екрану
                    freq = GetDeviceCaps(hdc, VREFRESH);

                    // глибина кольору
                    colorDepth = GetDeviceCaps(hdc, BITSPIXEL);

                    ReleaseDC(hWnd, hdc);
                }


                sprintf_s(mess, sizeof(mess), "Дані Клієнта #1:\r\n"
                    " - Рівент заряду акумулятора: %d%%\r\n"
                    " - Зарядний пристрій %s\r\n"
                    " - Частота оновлення екрана: %d Гц\r\n"
                    " - Глибина кольору: %d біт\r\n",
                    batteryPercent, chargerStatus,
                    freq,
                    colorDepth
                );
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);

                cbWritten = SendMessageA(hwndEdit, WM_GETTEXTLENGTH, 0, 0);
                SendMessageA(hwndEdit, WM_GETTEXT, (WPARAM)cbWritten + 1, (LPARAM)czBuf);


                WriteFile(hPipe, czBuf, sizeof(czBuf), &cbWritten, NULL);

                CloseHandle(hNamedPipe);
                CloseHandle(hPipe);
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        CloseHandle(hNamedPipe);
        CloseHandle(hPipe);
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}
