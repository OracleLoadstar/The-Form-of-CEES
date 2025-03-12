#include <windows.h>
#include <wchar.h>
#include <time.h>
#include <dwmapi.h>
#include <iostream>
#include <cstdlib>

#pragma comment(lib, "dwmapi.lib")

#define WINDOW_SIZE 250
#define TIMER_ID 1

int calculate_days_left() {
    SYSTEMTIME st;
    GetLocalTime(&st);

    struct tm current_date = { 0 };
    current_date.tm_year = st.wYear - 1900;
    current_date.tm_mon = st.wMonth - 1;
    current_date.tm_mday = st.wDay;

    struct tm next_june_seventh = { 0 };
    next_june_seventh.tm_year = st.wYear - 1900;
    next_june_seventh.tm_mon = 6 - 1;
    next_june_seventh.tm_mday = 7;

    time_t current_time = mktime(&current_date);
    time_t next_june_seventh_time = mktime(&next_june_seventh);

    if (difftime(next_june_seventh_time, current_time) < 0) {
        next_june_seventh.tm_year += 1;
        next_june_seventh_time = mktime(&next_june_seventh);
    }

    double difference = difftime(next_june_seventh_time, current_time);
    return (int)(difference / (60 * 60 * 24));
}

void get_current_time(wchar_t* buffer, size_t bufferSize) {
    SYSTEMTIME st;
    GetLocalTime(&st);

    wchar_t monthStr[10];
    switch (st.wMonth) {
    case 1: wcscpy_s(monthStr, L"1"); break;
    case 2: wcscpy_s(monthStr, L"2"); break;
    case 3: wcscpy_s(monthStr, L"3"); break;
    case 4: wcscpy_s(monthStr, L"4"); break;
    case 5: wcscpy_s(monthStr, L"5"); break;
    case 6: wcscpy_s(monthStr, L"6"); break;
    case 7: wcscpy_s(monthStr, L"7"); break;
    case 8: wcscpy_s(monthStr, L"8"); break;
    case 9: wcscpy_s(monthStr, L"9"); break;
    case 10: wcscpy_s(monthStr, L"10"); break;
    case 11: wcscpy_s(monthStr, L"11"); break;
    case 12: wcscpy_s(monthStr, L"12"); break;
    default: wcscpy_s(monthStr, L""); break;
    }

    swprintf(buffer, bufferSize, L"%s/%d  %02d:%02d", monthStr, st.wDay, st.wHour, st.wMinute);
}

void EnableBlur(HWND hwnd) {
    DWM_BLURBEHIND bb = { 0 };
    bb.dwFlags = DWM_BB_ENABLE;
    bb.fEnable = TRUE;
    bb.hRgnBlur = NULL;
    DwmEnableBlurBehindWindow(hwnd, &bb);

    const HINSTANCE hModule = LoadLibrary(TEXT("user32.dll"));
    if (hModule) {
        typedef struct _ACCENT_POLICY {
            int nAccentState;
            int nFlags;
            int nColor;
            int nAnimationId;
        } ACCENT_POLICY;

        typedef struct _WINDOWCOMPOSITIONATTRIBDATA {
            int nAttribute;
            PVOID pData;
            SIZE_T ulDataSize;
        } WINDOWCOMPOSITIONATTRIBDATA;

        typedef BOOL(WINAPI* pSetWindowCompositionAttribute)(HWND, WINDOWCOMPOSITIONATTRIBDATA*);

        const pSetWindowCompositionAttribute SetWindowCompositionAttribute = reinterpret_cast<pSetWindowCompositionAttribute>(GetProcAddress(hModule, "SetWindowCompositionAttribute"));

        if (SetWindowCompositionAttribute) {
            ACCENT_POLICY accent = { 3, 2, 0, 0 };
            WINDOWCOMPOSITIONATTRIBDATA data = { 19, &accent, sizeof(accent) };
            SetWindowCompositionAttribute(hwnd, &data);
        }

        FreeLibrary(hModule);
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static wchar_t prevTimeStr[50] = L"";
    static int prevDaysLeft = -1;

    switch (uMsg) {
    case WM_CREATE:
        EnableBlur(hwnd);
        AddFontResourceEx(L".\\HYQH85W.otf", FR_PRIVATE, NULL); // 加载外部字体
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SYSCOMMAND:
        if (wParam == SC_CLOSE) {
            return 0;
        }
        break;
    case WM_TIMER: {
        wchar_t timeStr[50];
        get_current_time(timeStr, 50);

        if (wcscmp(timeStr, prevTimeStr) != 0) {
            wcscpy_s(prevTimeStr, 50, timeStr);
            RECT timeRect;
            GetClientRect(hwnd, &timeRect);
            timeRect.top += 120;
            InvalidateRect(hwnd, &timeRect, TRUE);
        }

        int days_left = calculate_days_left();
        if (days_left != prevDaysLeft) {
            prevDaysLeft = days_left;
            RECT daysRect;
            GetClientRect(hwnd, &daysRect);
            daysRect.top += 20;
            InvalidateRect(hwnd, &daysRect, TRUE);

            RECT weeksDaysRect = daysRect;
            weeksDaysRect.top += 60;
            InvalidateRect(hwnd, &weeksDaysRect, TRUE);
        }
        break;
    }
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        RECT rect;
        GetClientRect(hwnd, &rect);
        HBRUSH backgroundBrush = CreateSolidBrush(RGB(250, 250, 250));
        FillRect(hdc, &rect, backgroundBrush);
        DeleteObject(backgroundBrush);

        int days_left = calculate_days_left();
        int weeks_left = days_left / 7;
        int remaining_days = days_left % 7;

        wchar_t text[50];
        swprintf(text, 50, L"%d天", days_left);

        SetTextColor(hdc, RGB(240, 123, 124));
        SetBkMode(hdc, TRANSPARENT);

        HFONT hFont = CreateFont(60, 0, 0, 0, FW_HEAVY, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"HYQiHeiY3-HEW");
        SelectObject(hdc, hFont);

        RECT daysRect = rect;
        daysRect.top += 20;

        DrawText(hdc, text, -1, &daysRect, DT_CENTER | DT_SINGLELINE);

        DeleteObject(hFont);

        wchar_t weeksDaysText[50];
        swprintf(weeksDaysText, 50, L"%d周%d天", weeks_left, remaining_days);

        SetTextColor(hdc, RGB(111, 147, 231));
        HFONT hSmallFont = CreateFont(30, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"HYQH85W");
        SelectObject(hdc, hSmallFont);

        RECT weeksDaysRect = rect;
        weeksDaysRect.top += 80;

        DrawText(hdc, weeksDaysText, -1, &weeksDaysRect, DT_CENTER | DT_SINGLELINE);

        DeleteObject(hSmallFont);

        wchar_t timeStr[50];
        get_current_time(timeStr, 50);

        hSmallFont = CreateFont(38, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
            DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"HYQH85W");
        SelectObject(hdc, hSmallFont);

        RECT timeRect = rect;
        timeRect.top += 120;

        SetTextColor(hdc, RGB(0, 0, 0));

        DrawText(hdc, timeStr, -1, &timeRect, DT_CENTER | DT_SINGLELINE);

        DeleteObject(hSmallFont);

        EndPaint(hwnd, &ps);
        break;
    }
    case WM_NCHITTEST:
        return HTCAPTION;
    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"MyWindowClass";

    WNDCLASS wc = {};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOOLWINDOW,
        CLASS_NAME,
        L"高考倒计时：",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        screenWidth - WINDOW_SIZE,
        0,
        WINDOW_SIZE, WINDOW_SIZE,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    SetWindowLong(hwnd, GWL_STYLE, GetWindowLong(hwnd, GWL_STYLE) & ~(WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX));
    SetLayeredWindowAttributes(hwnd, 0, (BYTE)(255 * 0.75), LWA_ALPHA);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    SetTimer(hwnd, TIMER_ID, 1000, NULL);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}