/*
 *  Focus Fader
 *
 *  Antoine calando - 2025 - Public domain
 */

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#if 0
// print in file
#define P(...) do { FILE* fdbg = fopen("c:\\Temp\\dbg.txt", "a" ); \
        fprintf(fdbg, __VA_ARGS__); \
        fclose(fdbg); } while(0)

#elif 0
// print in dbgview64.exe
#define P(...) do { \
        char buf[256]; \
        snprintf(buf, 256, __VA_ARGS__); \
        OutputDebugString(buf);  \
} while(0)
#else
// print in stderr
#define P(...) do { \
        fprintf(stderr, __VA_ARGS__); \
} while(0)
#endif

#define BORDER_SLIDER 3

HWND hwnd_main;     // this app invisible window
HMENU hmenu_popup;  // save popup id to retrieve rect
HWND hwnd_focus;    // current window with focus

ATOM hotkey_atom0, hotkey_atom1;    // for global shortcut
bool enabled = true;                // app enabled ?
int alpha = 180;                    // alpha 0->255 for dimmed win
RECT slider;                        // for config popup

// filter real app windows from dummy ones
static int win_valid(HWND hwnd, LONG exstyle, int unfade_all) {
    return ((exstyle & (WS_EX_WINDOWEDGE | WS_EX_TOOLWINDOW)) == WS_EX_WINDOWEDGE)
        && (IsWindowVisible(hwnd) || unfade_all)
        && GetWindowTextLength(hwnd);
}

// each window is unfaded, or faded, or ignored
static BOOL CALLBACK enum_window_cb(HWND hwnd, LPARAM unfade_all)
{
    LONG exstyle = GetWindowLongA(hwnd, GWL_EXSTYLE);

    //char buffer[32] = { 0 };
    //GetWindowText(hwnd, buffer, 32);
    //if (GetWindowTextLength(hwnd) 
    //    && strncmp(buffer, "MSCTFIME", 7)
    //    && strncmp(buffer, "Default IME", 10) )
    //P("win:%32s = ", buffer);

    if (win_valid(hwnd, exstyle, unfade_all)) {
        if ((exstyle & WS_EX_LAYERED) == 0)
            SetWindowLong(hwnd, GWL_EXSTYLE, exstyle | WS_EX_LAYERED);

        if (hwnd == hwnd_focus || unfade_all) {
            SetLayeredWindowAttributes(hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
            //P("unfade\n");
        } else {
            //SetLayeredWindowAttributes(hwnd, RGB(255,255,255), alpha, LWA_COLORKEY | LWA_ALPHA);
            
            SetLayeredWindowAttributes(hwnd, RGB(0,255,255), alpha, LWA_COLORKEY | LWA_ALPHA);
            
            //SetLayeredWindowAttributes(hwnd, RGB(alpha,alpha,alpha), alpha, LWA_COLORKEY);
            //SetLayeredWindowAttributes(hwnd, RGB(0,0,0), alpha, LWA_COLORKEY | LWA_ALPHA);
            //SetLayeredWindowAttributes(hwnd, RGB(0,255,0), alpha, LWA_COLORKEY | LWA_ALPHA);
            //SetLayeredWindowAttributes(hwnd, RGB(255,0,0), alpha, LWA_COLORKEY | LWA_ALPHA);
            //P("fade\n");
            #if 0
            //HDC hdc = GetDC(hwnd);
            HDC hdc = GetWindowDC(hwnd);
            //HDC hdc = GetDCEx(hwnd, NULL, DCX_WINDOW | DCX_PARENTCLIP);
            RECT rect, rect2;
            BOOL a = GetUpdateRect(hwnd, &rect2, FALSE);
            BOOL b = GetClientRect(hwnd, &rect);
            
            HDC hdc2 = CreateCompatibleDC(hdc);
            BLENDFUNCTION bfunc = { AC_SRC_OVER, 0, 200,  AC_SRC_ALPHA};            
            //BOOL c = UpdateLayeredWindow(hwnd, hdc, NULL, NULL, hdc2, NULL, RGB(180, 180, 180), &bfunc, ULW_ALPHA | ULW_COLORKEY);
            //BOOL c = UpdateLayeredWindow(hwnd, hdc, NULL, NULL, hdc2, NULL, RGB(180, 180, 180), NULL, 
            BOOL c = UpdateLayeredWindow(hwnd, hdc, NULL, NULL, hdc2, NULL, RGB(0, 0, 0), &bfunc, ULW_ALPHA);
            
            P("RECT %p %p, %d %d %d, %ld-%ld-%ld-%ld\n", hdc, hdc2, a, b, c, rect.top, rect.bottom, rect.left, rect.right);
            
            //HBRUSH brush = CreateHatchBrush(HS_CROSS, COLOR_WINDOW+2);
            //FillRect(hdc, &rect, brush);
            //InvertRect(hdc, &rect);
            ReleaseDC(hwnd, hdc);
            #endif
        }
    } else {
        //if (GetWindowTextLength(hwnd) 
        //    && strncmp(buffer, "MSCTFIME", 7)
        //    && strncmp(buffer, "Default IME", 10) )
        //P("invalid\n");
    }

    if (unfade_all && (exstyle & WS_EX_LAYERED) == WS_EX_LAYERED)
        SetWindowLong(hwnd, GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED);

    return TRUE;
}

void refresh_all(int unfade_all) {
    //P("--------------------------------------- refr %d\n", unfade_all);
    EnumWindows(enum_window_cb, unfade_all);
}

// dump debug log for one window
static BOOL CALLBACK enum_window_dbg_cb(HWND hwnd, LPARAM all)
{
    char buffer[256] = { 0 };
    GetWindowText(hwnd, buffer, 256);
    LONG exstyle = GetWindowLongA(hwnd, GWL_EXSTYLE);
    LONG style = GetWindowLongA(hwnd, GWL_STYLE);

    DWORD proc = 0;
    DWORD thread = GetWindowThreadProcessId(hwnd, &proc);

    //GetLayeredWindowAttributes(hwnd, *pcrKey, *pbAlpha, *pdwFlags);
    //BOOL GetTitleBarInfo(hwnd, pti);

    WINDOWINFO wininfo = { sizeof(wininfo) };
    GetWindowInfo(hwnd, &wininfo);

    if (all || IsWindowVisible(hwnd)) {
        P("   %p: v%d  s:%8lx s:x%8lx  p:%8ld  t:%8ld  <[%3d]%s>\n",
            hwnd, IsWindowVisible(hwnd),
            style, exstyle, proc, thread,
            GetWindowTextLength(hwnd), buffer);
    }
    return TRUE;
}

// dump debug log for visible windows then all
static void dbg_log() {
    P("============ VISIBLE WINs ==== (F:%p) ===\n", hwnd_focus);
    EnumWindows(enum_window_dbg_cb, 0);
    P("------------------ ALL WINs --------------------\n");
    EnumWindows(enum_window_dbg_cb, 1);
    P("-------------------- end -----------------------\n");
}


void update_alpha(int posx, int left, int width) {

    left += BORDER_SLIDER;
    width -= 2*BORDER_SLIDER;

    alpha = (posx - left)*256 / width;
    alpha = min(max(alpha, 0), 255);

    if (enabled)
        refresh_all(FALSE);
}

// display a popup when systray icon is clicked
void popup_display() {

    // Load menu resource
    HINSTANCE hInst;
    HMENU hMenu = LoadMenu(hInst, MAKEINTRESOURCE(10002));
    if(!hMenu)
        return;

    // Select the first submenu
    hmenu_popup = GetSubMenu(hMenu, 0);
    if(!hmenu_popup)
        goto end;

    CheckMenuItem(hMenu, 10101, MF_BYCOMMAND | (enabled? MF_CHECKED:MF_UNCHECKED));
    ModifyMenu(hMenu, 10102, MF_DISABLED | MF_BYCOMMAND | MF_OWNERDRAW, 10102, NULL);

    POINT lpos;
    GetCursorPos(&lpos);     // for menu pos
    SetForegroundWindow(hwnd_main); // allow popup to close when clicked outdide

    // display menu and process ret code (the item ID)
    switch(LOWORD(TrackPopupMenuEx(hmenu_popup,
        TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN | TPM_RETURNCMD /*| TPM_NONOTIFY*/,
        lpos.x, lpos.y, hwnd_main, NULL)
    )) {
        case 10101:
            enabled = !enabled;
            refresh_all(!enabled);
            break;
        case 10102:
            GetCursorPos(&lpos);
            update_alpha(lpos.x, lpos.x + slider.left, slider.right - slider.left);
            break;
        case 10103:
            dbg_log();
            break;
        case 10198:
            MessageBox(hwnd_main,_T("Focus Fader v0.0.0.0.0.1"),_T("About"), MB_OK | MB_ICONINFORMATION);
            break;
        case 10199:
            DestroyWindow(hwnd_main);		// Destroy Window
            break;
    }

end:
    DestroyMenu(hMenu);
}



LRESULT CALLBACK
MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    //if (msg == WM_ENTERIDLE)
    //    return 0;

    #if 0
    if (WM_USER <= msg) {
        P("  rx USER 0x%x %dx\n", msg - WM_USER, msg - WM_USER);
    } else {
        P("  rx WIN  0x%x %d == %llx %llx\n", msg, msg, wParam, lParam);
    }
    #endif

    switch (msg) {
    case WM_HOTKEY: // handle global hotkeys
        if (wParam == hotkey_atom0) {
            DestroyWindow(hwnd);
        } else if (wParam == hotkey_atom1) {
            enabled = !enabled;
            refresh_all(!enabled);
        }
        break;

    case WM_USER: // systray icon Messages
        switch(LOWORD(lParam))
        {
            case WM_LBUTTONDOWN:
            case WM_RBUTTONDOWN:
                popup_display();
                break;
        }
        break;
    case WM_DRAWITEM: {     // used for systray popup
            LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
            CopyRect(&slider, &lpdis->rcItem); // we will need it when user clicks the slider

            SelectObject(lpdis->hDC, CreatePen(PS_SOLID, 1, RGB(0,0,0)));
            SelectObject(lpdis->hDC, CreateSolidBrush(RGB(100,100,100)));

            LPRECT rc = &lpdis->rcItem;
            Rectangle(lpdis->hDC,
                rc->left + BORDER_SLIDER,
                rc->top + BORDER_SLIDER,
                rc->right - BORDER_SLIDER,
                rc->bottom - BORDER_SLIDER);

            SelectObject(lpdis->hDC, CreatePen(PS_NULL, 0, RGB(0,0,0)));
            SelectObject(lpdis->hDC, CreateSolidBrush(RGB(200,200,200)));

            int width = (rc->right - rc->left - 2*(BORDER_SLIDER+1)) * alpha / 256;
            Rectangle(lpdis->hDC,
                rc->left + BORDER_SLIDER+1,
                rc->top + BORDER_SLIDER+1,
                rc->left + BORDER_SLIDER+1 + width,
                rc->bottom - (BORDER_SLIDER+1));
            break;
    }
    case WM_ENTERIDLE: // received when overing over popup
        // need to check GetSystemMetrics(SM_SWAPBUTTON) if buttons swapped
        if (GetAsyncKeyState(VK_LBUTTON) & (SHORT) 0x8000) {
            POINT lpos;
            GetCursorPos(&lpos);
            RECT lrect;
            GetMenuItemRect(hwnd_main, hmenu_popup, 1, &lrect);
            if (lrect.top < lpos.y && lpos.y < lrect.bottom) {
                update_alpha(lpos.x, lrect.left, lrect.right - lrect.left);

                // only way I found to force popup redraw
                HWND sub = (HWND) lParam;
                InvalidateRect(sub, NULL, TRUE);
            }
        }
        break;

    case WM_DESTROY:        // quit app
        refresh_all(TRUE);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    return 0;
}

// called fo every focus changed
void CALLBACK HandleWinEvent(HWINEVENTHOOK hook, DWORD event, HWND hwnd,
    LONG idObject, LONG idChild,
    DWORD dwEventThread, DWORD dwmsEventTime)
{
    LONG exstyle = GetWindowLongA(hwnd, GWL_EXSTYLE);
    if (hwnd_focus == hwnd || !win_valid(hwnd, exstyle, 0))
        return;

    hwnd_focus = hwnd;

    //P("-------------------- start -----------------------\n");
    //P("--------------------------------------- evt %d\n", 0);
    EnumWindows(enum_window_cb, 0);
    //P("-------------------- end -----------------------\n");
}


// main
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, INT nCmdShow)
{
    WNDCLASS wc = { 0 };

    wc.style = 0;
    wc.lpfnWndProc = (WNDPROC) MainWndProc;
    wc.hInstance = hInstance ;

    wc.lpszClassName = _T("FocusFader.exe");

    if (!RegisterClass(&wc))
        return 0;

    hwnd_main = CreateWindow(_T("FocusFader.exe"),
                     _T("FocusFader"),
                     WS_POPUP,
                     CW_USEDEFAULT,
                     0,
                     CW_USEDEFAULT,
                     0,
                     NULL,
                     NULL,
                     hInstance,
                     NULL);

    if ( hwnd_main == (HWND) 0) {
        MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    NOTIFYICONDATA structNID;

    // tray icon settings
    structNID.cbSize = sizeof(NOTIFYICONDATA);
    structNID.hWnd = (HWND)hwnd_main;
    structNID.uID = 10001;
    structNID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    strcpy(structNID.szTip, "Focus Fader");
    structNID.hIcon = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(10001));
    structNID.uCallbackMessage = WM_USER;

    // display tray icon
    if(!Shell_NotifyIcon(NIM_ADD, &structNID)) {
        MessageBox(NULL, "Systray Icon Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // get IDs then register global shortcut
    hotkey_atom0 = GlobalAddAtom("hotkey1");
    hotkey_atom1 = GlobalAddAtom("hotkey2");
    if (   RegisterHotKey(hwnd_main, hotkey_atom0, MOD_SHIFT | MOD_CONTROL, VK_SUBTRACT) == 0
        || RegisterHotKey(hwnd_main, hotkey_atom1, MOD_SHIFT | MOD_CONTROL, VK_ADD) == 0) {
        MessageBox(NULL, "Canno register hotkeys!", "Error!", MB_ICONEXCLAMATION | MB_OK);
        return 0;
    }

    // add hook for focus change event
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, HandleWinEvent, 0, 0, WINEVENT_OUTOFCONTEXT);

    // main messge loop
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return msg.wParam;
}
