#include <stdio.h>
#include <stdint.h>
#include <windows.h>

typedef struct ParamProc {
    HANDLE ComboBox;
    uint8_t *Mainloop;
} ParamProc;

LRESULT CALLBACK WinProcedure(HWND HWnd, UINT UMsg, WPARAM WParam, LPARAM LParam);

int main(int argc, char *argv[]) {
    HINSTANCE WinInstance = GetModuleHandleW(NULL);
    
    WNDCLASSW WinClass = {0};
    WinClass.lpszClassName = L"UnlockerWindow";
    WinClass.hbrBackground = (HBRUSH) COLOR_WINDOW;
    WinClass.hCursor = LoadCursor(NULL, IDC_ARROW);
    WinClass.hInstance = WinInstance;
    WinClass.lpfnWndProc = WinProcedure;

    if(!RegisterClassW(&WinClass)) return -1;

    uint32_t Padding = 5;

    uint32_t Width = 300;
    uint32_t Height = 200;

    RECT WindowRect = { 0 };
    WindowRect.right = Width;
    WindowRect.bottom = Height;
    WindowRect.left = 0;
    WindowRect.top = 0;

    AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME | WS_VISIBLE, 0);
    HWND Window = CreateWindowW(
        WinClass.lpszClassName,
        L"Unlocker",
        WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        WindowRect.right - WindowRect.left,
        WindowRect.bottom - WindowRect.top,
        NULL, NULL,
        NULL, NULL
    );
    
    GetWindowRect(Window, &WindowRect);


    HWND HComboBox = CreateWindowW(
        L"COMBOBOX", NULL,
        WS_VISIBLE | WS_CHILD | CBS_DROPDOWNLIST | WS_VSCROLL,
        Padding, 0, 200 - Padding * 2, 200,
        Window,NULL, NULL, NULL
    );

    SendMessageW(HComboBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM)L"No Action");
    SendMessageW(HComboBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM)L"Unlock");    
    SendMessageW(HComboBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM)L"Delete");
    SendMessageW(HComboBox, CB_ADDSTRING, (WPARAM) 0, (LPARAM)L"Move");
    SendMessageW(HComboBox, CB_SETCURSEL, (WPARAM) 0, (LPARAM) 0);


    HWND hButton1 = CreateWindowW(
        L"BUTTON",
        L"OK",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        200, 0, 50 - Padding, 23,
        Window,
        (HMENU)1,
        WinInstance,
        NULL
    );

    HWND hButton2 = CreateWindowW(
        L"BUTTON",
        L"Quit",
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
        250, 0, 50 - Padding, 23,
        Window,
        (HMENU)2,
        WinInstance,
        NULL
    );

    MSG msg = { 0 };
    uint8_t running = 1;
    if (argc >= 2) {
        printf("Selected file: %s\n", argv[1]);
    } else {
        printf("No file selected.\n");
    }
    
    ParamProc ProcData;
    ProcData.ComboBox = HComboBox;
    ProcData.Mainloop = &running;
    SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR) &ProcData);

    while (running) {
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE)) {
            switch (msg.message) {
                case WM_QUIT: {
                    running = 0;
                    break;
                }
            }
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }
    
    return 0;
}

LRESULT CALLBACK WinProcedure(HWND HWnd, UINT UMsg, WPARAM WParam, LPARAM LParam)
{
    switch (UMsg) {
        case WM_DESTROY: {
            PostQuitMessage(0);
            break;
        }
        case WM_COMMAND: {
            switch(LOWORD(WParam)) {
                case 1: {
                    HANDLE HComboBox = (((ParamProc *) GetWindowLongPtr(HWnd, GWLP_USERDATA))->ComboBox);

                    int selectedIndex = SendMessage(HComboBox, CB_GETCURSEL, 0, 0);

                    if (selectedIndex != CB_ERR)
                    {
                        int textLength = SendMessageW(HComboBox, CB_GETLBTEXTLEN, selectedIndex, 0);
                        wchar_t* buffer = (wchar_t *) malloc(sizeof(wchar_t) * (textLength + 1));

                        SendMessageW(HComboBox, CB_GETLBTEXT, selectedIndex, (LPARAM) buffer);
                        MessageBoxW(HWnd, buffer, L"Selected Item", MB_OK | MB_ICONINFORMATION);
                        
                        free(buffer);
                    }
                    break;
                }
                case 2: {
                    *((ParamProc *) GetWindowLongPtr(HWnd, GWLP_USERDATA))->Mainloop = 0;
                    break;
                }
            }
            break;
        }
        default: {
            return DefWindowProcW(HWnd, UMsg, WParam, LParam);
            break;
        }
    }
    return 0;
}