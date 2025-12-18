// PipeServer.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include <windows.h>
#include "PipeServer.h"
#include <iostream>
#include <string>


#define MAX_LOADSTRING 100

static HWND hwndEdit;

// структура для асинхронних подій
OVERLAPPED ov = {};

// Ідентифікатор каналу Pipe
HANDLE hNamedPipe;

// Ідентифікатор потоку читання даних
HANDLE hReaderThread;

// Подія для завершення очікування сервера на підключення клієнта
HANDLE hStopListenForConnection;

// Ім'я створюваного каналу Pipe
LPCSTR pipeName = "\\\\.\\pipe\\lab7pipe";

// Буфер для передачі даних
char czBuf[512];

// Кількість байт даних, прийнятих через канал
DWORD cbRead;

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
    LoadStringW(hInstance, IDC_PIPESERVER, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PIPESERVER));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PIPESERVER));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_PIPESERVER);
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
       320, 150,
       430, 375,
       nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

DWORD WINAPI ReaderThread(LPVOID lpParam)
{
    HWND hWnd = (HWND)lpParam;
    HANDLE hEvents[2] = { hStopListenForConnection, ov.hEvent };

    while(true) {
        
        ResetEvent(ov.hEvent);
        BOOL fConnected = ConnectNamedPipe(hNamedPipe, &ov);

        if (!fConnected) {
            DWORD err = GetLastError();
            if (err == ERROR_PIPE_CONNECTED) {
                // клієта під'єднано
            }
            else if (err == ERROR_IO_PENDING) {
                DWORD dw = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
                if (dw == WAIT_OBJECT_0) {
                    break;
                }
                else if (dw == WAIT_OBJECT_0 + 1) {
                    DWORD transfered = 0;
                    sprintf_s(mess, "%sВстановлено з'єдання з клієнтом\r\n", m_mess);
                    SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
                    if (!GetOverlappedResult(hNamedPipe, &ov, &transfered, FALSE)) {
                        if (GetLastError() == ERROR_OPERATION_ABORTED) break;
                        DisconnectNamedPipe(hNamedPipe);
                        continue;
                    }
                }
                else {
                    continue;
                }
            }
            else {
                DisconnectNamedPipe(hNamedPipe);
                continue;
            }
        }

        ResetEvent(ov.hEvent);
        cbRead = 0;
        BOOL fRead = ReadFile(hNamedPipe, czBuf, sizeof(czBuf)-1, &cbRead, &ov);
        if (!fRead) {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING) {
                DWORD dw = WaitForMultipleObjects(2, hEvents, FALSE, INFINITE);
                if (dw == WAIT_OBJECT_0) {
                    break;
                }
                else if (dw == WAIT_OBJECT_0 + 1){
                    if (!GetOverlappedResult(hNamedPipe, &ov, &cbRead, FALSE)) {
                        if (GetLastError() == ERROR_OPERATION_ABORTED) break;
                        DisconnectNamedPipe(hNamedPipe);
                        continue;
                    }
                }
                else {
                    DisconnectNamedPipe(hNamedPipe);
                    continue;
                }
            }
            else if (err == ERROR_MORE_DATA) {
                if (!GetOverlappedResult(hNamedPipe, &ov, &cbRead, FALSE)) {
                    if (GetLastError() == ERROR_OPERATION_ABORTED) break;
                    DisconnectNamedPipe(hNamedPipe);
                    continue;
                }
            }
            else {
                if (err == ERROR_OPERATION_ABORTED) break;
                DisconnectNamedPipe(hNamedPipe);
                continue;
            }
        }
        czBuf[cbRead] = '\0';
        PostMessage(hWnd, WM_COMMAND, ID_PIPE_READ, 0);
        DisconnectNamedPipe(hNamedPipe);
    }
    return 0;
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
            10, 10, 400, 300,
            hWnd, nullptr, hInst, nullptr);
        hStopListenForConnection = CreateEvent(NULL, TRUE, FALSE, NULL);
        break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case ID_PIPE_CREATE:
                // Створюємо канал Pipe з ім'ям pipeName
                ResetEvent(hStopListenForConnection);
                hNamedPipe = CreateNamedPipeA(
                    pipeName,
                    PIPE_ACCESS_INBOUND | FILE_FLAG_OVERLAPPED,
                    PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                    PIPE_UNLIMITED_INSTANCES,
                    512, 512, 5000, NULL);
                
                if (hNamedPipe == INVALID_HANDLE_VALUE) {
                    MessageBox(hWnd, L"Неможливо створити канал", L"NamedPipeError", MB_OK);
                    break;
                }

                // подія для асинхронних операцій
                ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

                sprintf_s(mess, "Канал створено\r\nОчікуємо на підключення клієнтів...\r\n");
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
                hReaderThread = CreateThread(NULL, 0, ReaderThread, hWnd, 0, NULL);

                if (!hReaderThread) {
                    CloseHandle(hNamedPipe);
                    MessageBox(hWnd, L"Не вдалося створити потік для прийому клієнтських з'єднань. Відміна...", L"NamedPipeError", MB_OK);
                    break;
                }
                break;
            case ID_PIPE_READ:
                // Відображаємо вміст буфера отриманих повідомлень
                sprintf_s(mess, "%s\r\n%s\r\n", m_mess, (LPCSTR)czBuf);
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
                break;
            case ID_PIPE_CLOSE:
                // Закриваємо потік для прийому даних від клієнтів
                SetEvent(hStopListenForConnection);
                CancelIoEx(hNamedPipe, NULL);
                
                // чекаємо завершення потоку
                if (hReaderThread) {
                    WaitForSingleObject(hReaderThread, INFINITE);
                    CloseHandle(hReaderThread);
                    hReaderThread = NULL;
                }
                // закриваємо канал
                if (hNamedPipe) {
                    DisconnectNamedPipe(hNamedPipe);
                    CloseHandle(hNamedPipe);
                    hNamedPipe = NULL;
                }

                // закриття події
                if (ov.hEvent) {
                    CloseHandle(ov.hEvent);
                    ov.hEvent = NULL;
                }
                
                sprintf_s(mess, "Канал закрито\r\n");
                SendMessageA(hwndEdit, WM_SETTEXT, 0, (LPARAM)m_mess);
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
        PostMessage(hWnd, WM_COMMAND, ID_PIPE_CLOSE, 0);
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
