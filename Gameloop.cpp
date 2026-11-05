#include <windows.h>
//#include <algorithm>
//#include <wingdi.h> 
//#include <vector>
//#include <cstdlib>
//#include <ctime>
//#include <cmath>
#pragma comment(lib, "Msimg32.lib")
#pragma comment(lib, "Winmm.lib")
#include "Config.h"
#include "Components.h"
#include "Textures.h"
#include "System.h"

using namespace std;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM); 

int APIENTRY wWinMain( 
	_In_ HINSTANCE hInstance, 
	_In_opt_ HINSTANCE hPrevInstance, 
	_In_ PWSTR pCmdLine,
	_In_ int nCmdShow  
) 
{ 
    const wchar_t* CLASS_NAME = L"Main";  
    WNDCLASSEX wc = {}; 
    
    wc.cbSize = sizeof(WNDCLASSEX); 
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance; 
    wc.hCursor = NULL; 
    wc.lpszClassName = CLASS_NAME;  

    if (!RegisterClassEx(&wc)) { 
        MessageBox(NULL, L"Window class registration error!", L"Error:Window", MB_ICONERROR);
        return 0;
    }

    window.width = GetSystemMetrics(SM_CXSCREEN); 
    window.height = GetSystemMetrics(SM_CYSCREEN);

    window.hwnd = CreateWindowEx
    (
        0, 
        CLASS_NAME, 
        L"Roguelike prototype", 
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 
        window.width, window.height, 
        NULL, NULL, hInstance, NULL
    );

    if (!window.hwnd) {
        MessageBox(
            NULL, 
            L"Error creating window!", 
            L"Error:Window", 
            MB_ICONERROR 
        );
        return 0;  
    }

    InitWindow();
    ShowWindow(window.hwnd, SW_SHOW);
    UpdateWindow(window.hwnd);

    MSG msg = {}; 

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg); 
        DispatchMessage(&msg); 

    }
}

LRESULT CALLBACK WndProc(
    HWND hwnd, 
    UINT msg, 
    WPARAM wParam, 
    LPARAM lParam  

) {

    switch (msg) {

    case WM_CREATE:
        InitGame();
        ShowCursor(FALSE); 
        SetTimer(hwnd, 1, 16, NULL);

        break;

    case WM_PAINT: 
    {   
        PAINTSTRUCT ps;
        window.hdc = BeginPaint(hwnd, &ps);
        ShowGame();
        EndPaint(hwnd, &ps);

    }

        break;

    case WM_TIMER: 
    {
        ProcessGame();
        InvalidateRect(hwnd, NULL, FALSE);
    }

        break;

    case WM_KEYDOWN: 
        keys[wParam] = true;
        InvalidateRect(hwnd, NULL, TRUE);

        break;
    case WM_KEYUP:
        keys[wParam] = false;
        break;

    case WM_MOUSEWHEEL:
    {
        short delta = GET_WHEEL_DELTA_WPARAM(wParam);

        ZOOM += delta / 120.0f * 0.2f;

        if (ZOOM < 1.0f) ZOOM = 1.0f;
        if (ZOOM > 10.0f) ZOOM = 10.0f;

        return 0;
    }
       break;

    case WM_LBUTTONDOWN:
        mouseLeftDown = true;
        break;
        
    case WM_LBUTTONUP:
        mouseLeftDown = false;
        break;

    case WM_DESTROY: 
        ClearGame();
        KillTimer(hwnd, 1);
        PostQuitMessage(0);
        break;

    default: 
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}