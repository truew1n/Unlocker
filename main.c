#include <stdio.h>
#include <stdint.h>
#include <windows.h>


typedef struct ParamProc {
    HANDLE ComboBox;
    uint8_t *Mainloop;
    uint16_t *Filepath;
} ParamProc;

void ResolveAction(uint16_t *Action, uint16_t *Filepath);
LRESULT CALLBACK WinProcedure(HWND HWnd, UINT UMsg, WPARAM WParam, LPARAM LParam);

int main(int argc, uint8_t *argv[]) {
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

    uint8_t running = 1;
    ParamProc ProcData;
    ProcData.ComboBox = HComboBox;
    ProcData.Mainloop = &running;
    
    if (argc >= 2) {
        uint32_t StrLength = MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, NULL, 0);

        ProcData.Filepath = (uint16_t *) malloc(sizeof(uint16_t)*StrLength + 1);

        MultiByteToWideChar(CP_UTF8, 0, argv[1], -1, ProcData.Filepath, StrLength);
        ProcData.Filepath[StrLength] = L'\0';
    } else {
        ProcData.Filepath = NULL;
    }

    MessageBoxExW(NULL, ProcData.Filepath, L"Info!", MB_ICONWARNING | MB_OK, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
    
    SetWindowLongPtr(Window, GWLP_USERDATA, (LONG_PTR) &ProcData);

    MSG Msg = { 0 };
    while (running) {
        while (PeekMessageW(&Msg, NULL, 0, 0, PM_REMOVE)) {
            switch (Msg.message) {
                case WM_QUIT: {
                    running = 0;
                    break;
                }
            }
            TranslateMessage(&Msg);
            DispatchMessageW(&Msg);
        }
    }
    
    free(ProcData.Filepath);
    return 0;
}

uint8_t IsDirectory(uint16_t* Filepath)
{
    DWORD Attributes = GetFileAttributesW(Filepath);
    return (Attributes != INVALID_FILE_ATTRIBUTES && (Attributes & FILE_ATTRIBUTE_DIRECTORY));
}


void ResolveAction(uint16_t *Action, uint16_t *Filepath)
{
    if(IsDirectory(Filepath)) {
        SHFILEOPSTRUCTW FileOp;
        ZeroMemory(&FileOp, sizeof(FileOp));
        FileOp.hwnd = NULL;
        FileOp.wFunc = FO_DELETE;
        FileOp.pFrom = Filepath;
        FileOp.pTo = Filepath;
        FileOp.fFlags = FOF_NOERRORUI | FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOCONFIRMMKDIR;

        int32_t OperationResult = SHFileOperationW(&FileOp);
        if (OperationResult != 0) {
            wprintf(L"%ls\n", Filepath);
            printf("SHFileOperationW failed with error code: %d\n", OperationResult);
            MessageBoxExW(NULL, L"Couldn't delete directory!", L"Deletion Failed!", MB_ICONWARNING | MB_OK, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
        }
    } else {
        if(!wcscmp(Action, L"Delete")) {
            uint8_t DeleteResult = DeleteFileW(Filepath);
            if(!DeleteResult) {
                printf("%i", GetLastError());
                MessageBoxExW(NULL, L"Couldn't delete file!", L"Deletion Failed!", MB_ICONWARNING | MB_OK, MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US));
            }
        }
    }
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
                    ParamProc *Parameter = (ParamProc *) GetWindowLongPtr(HWnd, GWLP_USERDATA);
                    HANDLE HComboBox = Parameter->ComboBox;

                    int SelectedIndex = SendMessage(HComboBox, CB_GETCURSEL, 0, 0);

                    if (SelectedIndex != CB_ERR)
                    {
                        int TextLength = SendMessageW(HComboBox, CB_GETLBTEXTLEN, SelectedIndex, 0);
                        uint16_t* Buffer = (uint16_t *) malloc(sizeof(uint16_t) * (TextLength + 1));

                        SendMessageW(HComboBox, CB_GETLBTEXT, SelectedIndex, (LPARAM) Buffer);
                        ResolveAction(
                            Buffer,
                            Parameter->Filepath
                        );
                        
                        free(Buffer);
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