#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <windows.h>

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch(msg) {
        //NOTE: Handle when the user quits the window
        case WM_DESTROY:
        case WM_CLOSE: {
            PostQuitMessage(0);
        } break;
        default:
            result = DefWindowProcW(hwnd, msg, wparam, lparam);
    }
    return result;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    // Open a window
    HWND hwnd;
    {	
    	//First register the type of window we are going to create
        WNDCLASSEXW winClass = {};
        winClass.cbSize = sizeof(WNDCLASSEXW);
        winClass.style = 0;
        winClass.lpfnWndProc = &WndProc;
        winClass.hInstance = hInstance;
        winClass.hIcon = LoadIconW(0, IDI_APPLICATION);
        winClass.hCursor = LoadCursorW(0, IDC_ARROW);
        winClass.lpszClassName = L"MyWindowClass";
        winClass.hIconSm = LoadIconW(0, IDI_APPLICATION);

        if(!RegisterClassExW(&winClass)) {
            MessageBoxA(0, "RegisterClassEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }

        //Now create the actual window
        RECT initialRect = { 0, 0, 960, 540 };
        AdjustWindowRectEx(&initialRect, WS_OVERLAPPEDWINDOW, FALSE, WS_EX_OVERLAPPEDWINDOW);
        LONG initialWidth = initialRect.right - initialRect.left;
        LONG initialHeight = initialRect.bottom - initialRect.top;

        hwnd = CreateWindowExW(WS_EX_OVERLAPPEDWINDOW,
                                winClass.lpszClassName,
                                L"Direct X Window - not yet",
                                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                initialWidth, 
                                initialHeight,
                                0, 0, hInstance, 0);

        if(!hwnd) {
            MessageBoxA(0, "CreateWindowEx failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
    }

    bool running = true;
    while(running) {
    	MSG msg = {};
        while(PeekMessageW(&msg, 0, 0, 0, PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                running = false;
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    

    return 0;

}