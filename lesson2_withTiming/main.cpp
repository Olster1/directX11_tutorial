#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define UNICODE
#include <assert.h>
#include <d3d11_1.h>
#include <windows.h>
#include <stdio.h>
#include "easy_time.h"


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = 0;
    switch(msg) {
        case WM_KEYDOWN: {
            if(wparam == VK_ESCAPE)
                // PostQuitMessage(0);
            break;
        }
        case WM_DESTROY: {
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
        winClass.style = CS_HREDRAW | CS_VREDRAW;
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
                                L"Direct X Window",
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

    // Create D3D11 Device and Context
    ID3D11Device1* d3d11Device;
    ID3D11DeviceContext1* d3d11DeviceContext;
    {
        ID3D11Device* baseDevice;
        ID3D11DeviceContext* baseDeviceContext;
        D3D_FEATURE_LEVEL featureLevels[] = { D3D_FEATURE_LEVEL_11_0 }; //we just want d3d 11 features, not below
        UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT; 
        #if defined(DEBUG_BUILD)
        creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
        #endif

        HRESULT hResult = D3D11CreateDevice(0, D3D_DRIVER_TYPE_HARDWARE, //hardware rendering instead of software rendering
                                            0, creationFlags, 
                                            featureLevels, ARRAYSIZE(featureLevels),  //feature levels: we want direct11 features - don't want any below
                                            D3D11_SDK_VERSION, &baseDevice, 
                                            0, &baseDeviceContext);
        if(FAILED(hResult)){
            MessageBoxA(0, "D3D11CreateDevice() failed", "Fatal Error", MB_OK);
            return GetLastError();
        }
        
        // Get 1.1 interface of D3D11 Device and Context
        hResult = baseDevice->QueryInterface(__uuidof(ID3D11Device1), (void**)&d3d11Device);
        assert(SUCCEEDED(hResult));
        baseDevice->Release();

        hResult = baseDeviceContext->QueryInterface(__uuidof(ID3D11DeviceContext1), (void**)&d3d11DeviceContext);
        assert(SUCCEEDED(hResult));
        baseDeviceContext->Release();
    }

    #ifdef DEBUG_BUILD
        // Set up debug layer to break on D3D11 errors
        ID3D11Debug *d3dDebug = nullptr;
        d3d11Device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug);
        if (d3dDebug)
        {
            ID3D11InfoQueue *d3dInfoQueue = nullptr;
            if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
            {
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
                d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
                d3dInfoQueue->Release();
            }
            d3dDebug->Release();
        }
    #endif

        // Create Swap Chain
        IDXGISwapChain1* d3d11SwapChain;
        { 
            // Get DXGI Factory (needed to create Swap Chain)
            IDXGIFactory2* dxgiFactory;
            {
                IDXGIDevice1* dxgiDevice;
                HRESULT hResult = d3d11Device->QueryInterface(__uuidof(IDXGIDevice1), (void**)&dxgiDevice);
                assert(SUCCEEDED(hResult));

                IDXGIAdapter* dxgiAdapter;
                hResult = dxgiDevice->GetAdapter(&dxgiAdapter);
                assert(SUCCEEDED(hResult));
                dxgiDevice->Release();

                DXGI_ADAPTER_DESC adapterDesc;
                dxgiAdapter->GetDesc(&adapterDesc);

                OutputDebugStringA("Graphics Device: \n");
                OutputDebugStringW(adapterDesc.Description);

                hResult = dxgiAdapter->GetParent(__uuidof(IDXGIFactory2), (void**)&dxgiFactory);
                assert(SUCCEEDED(hResult));
                dxgiAdapter->Release();
            }

            DXGI_SWAP_CHAIN_DESC1 d3d11SwapChainDesc = {};
            d3d11SwapChainDesc.Width = 0; // use window width
            d3d11SwapChainDesc.Height = 0; // use window height
            d3d11SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
            d3d11SwapChainDesc.SampleDesc.Count = 1;
            d3d11SwapChainDesc.SampleDesc.Quality = 0;
            d3d11SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            d3d11SwapChainDesc.BufferCount = 2;
            d3d11SwapChainDesc.Scaling = DXGI_SCALING_STRETCH;
            d3d11SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
            d3d11SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
            d3d11SwapChainDesc.Flags = 0;

            HRESULT hResult = dxgiFactory->CreateSwapChainForHwnd(d3d11Device, hwnd, &d3d11SwapChainDesc, 0, 0, &d3d11SwapChain);
            assert(SUCCEEDED(hResult));

            dxgiFactory->Release();
        }

        // Create Framebuffer Render Target
        ID3D11RenderTargetView* d3d11FrameBufferView;
        {
            ID3D11Texture2D* d3d11FrameBuffer;
            HRESULT hResult = d3d11SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&d3d11FrameBuffer);
            assert(SUCCEEDED(hResult));

            hResult = d3d11Device->CreateRenderTargetView(d3d11FrameBuffer, 0, &d3d11FrameBufferView);
            assert(SUCCEEDED(hResult));
            d3d11FrameBuffer->Release();
        }

        EasyTime_setupTimeDatums();

        s64 startCount = EasyTime_GetTimeCount();

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

        FLOAT backgroundColor[4] = { 0.1f, 0.2f, 0.6f, 1.0f };
        d3d11DeviceContext->ClearRenderTargetView(d3d11FrameBufferView, backgroundColor);

        d3d11SwapChain->Present(1, 0);

        //Timing the frame to see if vync is working. If you have a 60fps monitor you should see a time around 16.66 (not exact since your the OS can decide when to call your program's thread)
        //If you change the first d3d11SwapChain->Present argument to 0 it will run as fast as it can, if you change it to more than 1, it will be in intervals of your monitor rate. 
        //So for a monitor with 60hz and a swap interval of 2 it will run at ~33.33ms per frame and so on. 
        s64 currentCount = EasyTime_GetTimeCount();
        float seconds = EasyTime_GetMillisecondsElapsed(currentCount, startCount);
        char buffer[256];
        sprintf(buffer, "%f\n", seconds);
        startCount = currentCount;
         OutputDebugStringA(buffer);
         //////////////////////////////////
        
    }
    

    return 0;

}