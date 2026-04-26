#include "pixeldrawer.h"
#include <QMutexLocker>
#include <QApplication>
#include <windows.h>
#include <winuser.h>
#include <windowsx.h>
#include <tpcshrd.h>
#include <stdio.h>
#include <tpcshrd.h>

static HWND targetHWnd = NULL;

BOOL CALLBACK EnumWindowsCallback(HWND hwnd, LPARAM lParam) {
    wchar_t title[256];
    GetWindowTextW(hwnd, title, 256);
    if (wcsstr(title, L"Seebo") || wcsstr(title, L"希沃") || wcsstr(title, L"SeeBoard")) {
        targetHWnd = hwnd;
        return FALSE;
    }
    if (IsWindowVisible(hwnd)) {
        HWND childHwnd = FindWindowExW(hwnd, NULL, NULL, NULL);
        while (childHwnd) {
            wchar_t childTitle[256];
            GetWindowTextW(childHwnd, childTitle, 256);
            if (wcsstr(childTitle, L"Seebo") || wcsstr(childTitle, L"希沃") || wcsstr(childTitle, L"SeeBoard")) {
                targetHWnd = childHwnd;
                return FALSE;
            }
            childHwnd = FindWindowExW(hwnd, childHwnd, NULL, NULL);
        }
    }
    return TRUE;
}

PixelDrawer::PixelDrawer()
    : threshold(128), offsetX(0), offsetY(0), stopFlag(nullptr)
    , m_parent(nullptr), mouseMethod(MouseMethod::SendInput_Normalized)
    , moveDelay(1), downDelay(2), userMouseDown(false)
{
    moveMouseFunc = &PixelDrawer::moveMethod3;
    targetHWnd = NULL;
    EnumWindows(EnumWindowsCallback, 0);
}

PixelDrawer::~PixelDrawer() {}

QList<PixelDrawer::ScreenInfo> PixelDrawer::getScreenInfo() {
    QList<ScreenInfo> screens;

    DISPLAY_DEVICE dd = {};
    dd.cb = sizeof(DISPLAY_DEVICE);
    DISPLAY_DEVICE ddMon = {};
    ddMon.cb = sizeof(DISPLAY_DEVICE);

    int deviceIdx = 0;
    while (EnumDisplayDevices(nullptr, deviceIdx, &dd, 0)) {
        DWORD monIdx = 0;
        while (EnumDisplayDevices(dd.DeviceName, monIdx, &ddMon, 0)) {
            if (ddMon.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP) {
                DEVMODE dmTemp = {};
                dmTemp.dmSize = sizeof(DEVMODE);
                if (EnumDisplaySettings(ddMon.DeviceName, ENUM_CURRENT_SETTINGS, &dmTemp)) {
                    ScreenInfo info;
                    info.left = dmTemp.dmPosition.x;
                    info.top = dmTemp.dmPosition.y;
                    info.width = dmTemp.dmPelsWidth;
                    info.height = dmTemp.dmPelsHeight;
                    info.isPrimary = (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE);

                    HDC hdc = GetDC(NULL);
                    int dpiX = GetDeviceCaps(hdc, LOGPIXELSX);
                    info.dpiScale = dpiX / 96.0;
                    ReleaseDC(NULL, hdc);

                    screens.append(info);
                }
            }
            monIdx++;
        }
        deviceIdx++;
    }
    return screens;
}

int PixelDrawer::getScreenCount() {
    return GetSystemMetrics(SM_CMONITORS);
}

QPoint PixelDrawer::virtualScreenSize() {
    return QPoint(GetSystemMetrics(SM_CXVIRTUALSCREEN),
                  GetSystemMetrics(SM_CYVIRTUALSCREEN));
}

QPoint PixelDrawer::getCursorPos() {
    POINT pt;
    GetCursorPos(&pt);
    return QPoint(pt.x, pt.y);
}

QImage PixelDrawer::captureScreen(int x, int y, int w, int h) {
    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, w, h);
    SelectObject(hdcMem, hBitmap);

    BitBlt(hdcMem, 0, 0, w, h, hdcScreen, x, y, SRCCOPY);

    QImage image(w, h, QImage::Format_ARGB32);
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = w;
    bmi.bmiHeader.biHeight = -h;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    GetDIBits(hdcMem, hBitmap, 0, h, image.bits(), &bmi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    return image;
}

QColor PixelDrawer::getPixelColor(int x, int y) {
    HDC hdc = GetDC(nullptr);
    COLORREF color = GetPixel(hdc, x, y);
    ReleaseDC(nullptr, hdc);

    int r = GetRValue(color);
    int g = GetGValue(color);
    int b = GetBValue(color);

    return QColor(r, g, b);
}

bool PixelDrawer::findSeeBoardWindow() {
    targetHWnd = NULL;
    EnumWindows(EnumWindowsCallback, 0);
    return targetHWnd != NULL;
}

bool PixelDrawer::clickPenIcon() {
    if (m_penZone.isEmpty()) {
        qDebug() << "Pen zone not set";
        return false;
    }

    int penX = m_penZone.center().x();
    int penY = m_penZone.center().y();

    SetCursorPos(penX, penY);
    QThread::msleep(50);

    INPUT down = {};
    down.type = INPUT_MOUSE;
    down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    SendInput(1, &down, sizeof(INPUT));
    QThread::msleep(50);

    INPUT up = {};
    up.type = INPUT_MOUSE;
    up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    SendInput(1, &up, sizeof(INPUT));

    qDebug() << "Clicked pen icon at" << penX << penY;
    return true;
}

bool PixelDrawer::detectAndSelectColor(QColor& selected) {
    QThread::msleep(500);

    if (m_colorZone.isEmpty()) {
        qDebug() << "Color zone not set";
        selected = Qt::red;
        return false;
    }

    int zoneW = m_colorZone.width();
    int zoneH = m_colorZone.height();
    int zoneX = m_colorZone.x();
    int zoneY = m_colorZone.y();

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, zoneW, zoneH);
    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, zoneW, zoneH, hdcScreen, zoneX, zoneY, SRCCOPY);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = zoneW;
    bmi.bmiHeader.biHeight = -zoneH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    QImage zoneImg(zoneW, zoneH, QImage::Format_ARGB32);
    GetDIBits(hdcMem, hBitmap, 0, zoneH, zoneImg.bits(), &bmi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    if (m_paletteColors.isEmpty()) {
        m_paletteColors = { Qt::red, Qt::green, Qt::blue, Qt::yellow, Qt::white, Qt::black };
    }

    int colorCount = m_paletteColors.count();
    int sampleW = zoneW / colorCount;

    for (int i = 0; i < colorCount; i++) {
        int sampleX = zoneX + sampleW * i + sampleW / 2;
        int sampleY = zoneY + zoneH / 2;

        QColor detected = zoneImg.pixel(sampleX - zoneX, sampleY - zoneY);

        qDebug() << "Sample" << i << "at" << sampleX << sampleY << "color:" << detected.name();

        SetCursorPos(sampleX, sampleY);
        QThread::msleep(100);

        INPUT down = {};
        down.type = INPUT_MOUSE;
        down.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
        SendInput(1, &down, sizeof(INPUT));
        QThread::msleep(50);

        INPUT up = {};
        up.type = INPUT_MOUSE;
        up.mi.dwFlags = MOUSEEVENTF_LEFTUP;
        SendInput(1, &up, sizeof(INPUT));

        selected = detected;
        m_currentColor = detected;

        qDebug() << "Clicked color:" << detected.name() << "at" << sampleX << sampleY;
        return true;
    }

    selected = m_paletteColors.first();
    return false;
}

bool PixelDrawer::detectRightEdge(int startX, int startY, int& outX) {
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int checkWidth = 100;
    int threshold = 50;

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, checkWidth, 1);
    SelectObject(hdcMem, hBitmap);
    BitBlt(hdcMem, 0, 0, checkWidth, 1, hdcScreen, startX, startY, SRCCOPY);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = checkWidth;
    bmi.bmiHeader.biHeight = -1;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    QImage lineImg(checkWidth, 1, QImage::Format_ARGB32);
    GetDIBits(hdcMem, hBitmap, 0, 1, lineImg.bits(), &bmi, DIB_RGB_COLORS);

    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    QColor startColor = lineImg.pixel(0, 0);
    for (int i = 1; i < checkWidth; i++) {
        QColor c = lineImg.pixel(i, 0);
        if (qAbs(c.red() - startColor.red()) > threshold ||
            qAbs(c.green() - startColor.green()) > threshold ||
            qAbs(c.blue() - startColor.blue()) > threshold) {
            outX = startX + i;
            return true;
        }
    }

    outX = startX + checkWidth;
    return false;
}

void PixelDrawer::mouseDown() {
    QMutexLocker locker(&mouseMutex);
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
    lastMouseDownTime = GetTickCount();
}

void PixelDrawer::mouseUp() {
    QMutexLocker locker(&mouseMutex);
    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
    QThread::msleep(10);
}

void PixelDrawer::moveMethod0(int x, int y) {
    int virtW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int virtH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;
    input.mi.dx = (DWORD)((x * 65535.0) / virtW);
    input.mi.dy = (DWORD)((y * 65535.0) / virtH);
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod1(int x, int y) {
    int dx = x - lastPos.x();
    int dy = y - lastPos.y();

    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE;
    input.mi.dx = dx;
    input.mi.dy = dy;
    input.mi.time = 0;
    SendInput(1, &input, sizeof(INPUT));
    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod2(int x, int y) {
    SetCursorPos(x, y);
    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod3(int x, int y) {
    double virtW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    double virtH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (virtW <= 0) virtW = 1920;
    if (virtH <= 0) virtH = 1080;

    INPUT input[2] = {};
    input[0].type = INPUT_MOUSE;
    input[0].mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE_NOCOALESCE;
    input[0].mi.dx = (LONG)((x * 65535.0) / virtW);
    input[0].mi.dy = (LONG)((y * 65535.0) / virtH);
    input[0].mi.time = 0;

    SendInput(2, input, sizeof(INPUT));
    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod4(int x, int y) {
    double virtW = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    double virtH = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    if (virtW <= 0) virtW = 1920;
    if (virtH <= 0) virtH = 1080;

    INPUT input = {};
    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK | MOUSEEVENTF_MOVE_NOCOALESCE;
    input.mi.dx = (LONG)(x * 65535.0 / virtW);
    input.mi.dy = (LONG)(y * 65535.0 / virtH);
    input.mi.time = 0;

    for (int i = 0; i < 3; ++i) {
        SendInput(1, &input, sizeof(INPUT));
    }
    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod5_TouchInput(int x, int y) {
    if (!targetHWnd) {
        EnumWindows(EnumWindowsCallback, 0);
    }

    HWND hwnd = targetHWnd ? targetHWnd : GetForegroundWindow();
    if (!hwnd) hwnd = GetDesktopWindow();

    LPARAM lParam = MAKELPARAM(x, y);
    WPARAM wParam = 0;
    SendMessageTimeoutW(hwnd, WM_MOUSEMOVE, wParam, lParam, SMTO_ABORTIFHUNG, 100, NULL);
}

void PixelDrawer::moveMethod6_TouchInputMulti(int x, int y) {
    if (!targetHWnd) {
        EnumWindows(EnumWindowsCallback, 0);
    }

    HWND hwnd = targetHWnd ? targetHWnd : GetForegroundWindow();
    if (!hwnd) hwnd = GetDesktopWindow();

    LPARAM lParam = MAKELPARAM(x, y);
    WPARAM wParam = 0;

    for (int i = 0; i < 3; ++i) {
        SendMessageTimeoutW(hwnd, WM_MOUSEMOVE, wParam, lParam, SMTO_ABORTIFHUNG, 50, NULL);
    }
}

void PixelDrawer::moveMethod7_PointerTouch(int x, int y) {
    SetCursorPos(x, y);
    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod8_PointerTouchMulti(int x, int y) {
    SetCursorPos(x, y);

    mouseDown();

    for (int i = 0; i < 5; ++i) {
        SetCursorPos(x + i, y);
        QThread::msleep(1);
    }

    mouseUp();
    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod9_PointerInjection(int x, int y) {
    static bool initialized = false;
    static int touchCount = 0;

    if (!initialized) {
        InitializeTouchInjection(10, TOUCH_FEEDBACK_DEFAULT);
        initialized = true;
        touchCount = 0;
    }

    POINT pt = {x, y};

    TOUCHINPUT ti[10] = {};

    ti[touchCount].dwID = touchCount % 10;
    ti[touchCount].dwFlags = TOUCHEVENTF_DOWN | TOUCHEVENTF_INRANGE;
    ti[touchCount].x = pt.x * 100;
    ti[touchCount].y = pt.y * 100;
    ti[touchCount].dwTime = 0;
    ti[touchCount].dwExtraInfo = 0;

    HWND hwnd = GetForegroundWindow();
    if (hwnd) {
        SendMessage(hwnd, WM_TOUCH, 1, (LPARAM)ti);
    }

    QThread::msleep(10);

    ti[touchCount].dwFlags = TOUCHEVENTF_UP;
    if (hwnd) {
        SendMessage(hwnd, WM_TOUCH, 1, (LPARAM)ti);
    }

    touchCount = (touchCount + 1) % 10;

    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

void PixelDrawer::moveMethod10_RealTouchInjection(int x, int y) {
    static bool initialized = false;
    static int touchId = 0;

    if (!initialized) {
        BOOL result = InitializeTouchInjection(10, TOUCH_FEEDBACK_DEFAULT);
        if (!result) {
            qDebug() << "InitializeTouchInjection failed";
            moveMethod3(x, y);
            return;
        }
        initialized = true;
        touchId = 0;
    }

    POINTER_TOUCH_INFO contact = {};
    contact.pointerInfo.pointerType = PT_TOUCH;
    contact.pointerInfo.ptPixelLocation.x = x;
    contact.pointerInfo.ptPixelLocation.y = y;
    contact.pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
    contact.pointerInfo.pointerId = touchId % 10;

    BOOL result = InjectTouchInput(1, &contact);
    if (!result) {
        qDebug() << "InjectTouchInput DOWN failed:" << GetLastError();
    }

    QThread::msleep(20);

    contact.pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
    contact.pointerInfo.ptPixelLocation.x = x;
    contact.pointerInfo.ptPixelLocation.y = y;
    result = InjectTouchInput(1, &contact);
    if (!result) {
        qDebug() << "InjectTouchInput MOVE failed:" << GetLastError();
    }

    QThread::msleep(20);

    contact.pointerInfo.pointerFlags = POINTER_FLAG_UP;
    result = InjectTouchInput(1, &contact);
    if (!result) {
        qDebug() << "InjectTouchInput UP failed:" << GetLastError();
    }

    touchId = (touchId + 1) % 10;

    lastPos = QPoint(x, y);
    lastMoveTime = GetTickCount64();
}

static HHOOK g_mouseHook = nullptr;
static HINSTANCE g_hInstance = nullptr;
static bool g_hookInitialized = false;
static POINTER_TOUCH_INFO g_currentTouch[2] = {};
static bool g_touchActive[2] = {false, false};

LRESULT CALLBACK LowLevelMouseProc(int code, WPARAM wparam, LPARAM lparam) {
    if (code < 0) {
        return CallNextHookEx(nullptr, code, wparam, lparam);
    }

    bool ctrlPressed = GetAsyncKeyState(VK_CONTROL) & 0x8000;
    bool shiftPressed = GetAsyncKeyState(VK_RSHIFT) & 0x8000;

    if (wparam == WM_LBUTTONDOWN && shiftPressed) {
        MSLLHOOKSTRUCT* info = (MSLLHOOKSTRUCT*)lparam;

        g_currentTouch[0] = {};
        g_currentTouch[0].pointerInfo.pointerType = PT_TOUCH;
        g_currentTouch[0].pointerInfo.ptPixelLocation.x = info->pt.x;
        g_currentTouch[0].pointerInfo.ptPixelLocation.y = info->pt.y;
        g_currentTouch[0].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
        g_currentTouch[0].pointerInfo.pointerId = 0;

        BOOL result = InjectTouchInput(1, &g_currentTouch[0]);
        if (result) {
            g_currentTouch[0].pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
            g_touchActive[0] = true;
        }

        if (ctrlPressed || GetAsyncKeyState(VK_RCONTROL) & 0x8000) {
            g_currentTouch[1] = {};
            g_currentTouch[1].pointerInfo.pointerType = PT_TOUCH;
            g_currentTouch[1].pointerInfo.ptPixelLocation.x = info->pt.x + 80;
            g_currentTouch[1].pointerInfo.ptPixelLocation.y = info->pt.y + 80;
            g_currentTouch[1].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
            g_currentTouch[1].pointerInfo.pointerId = 1;

            result = InjectTouchInput(1, &g_currentTouch[1]);
            if (result) {
                g_currentTouch[1].pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
                g_touchActive[1] = true;
            }
        }

        return 1;
    }
    else if (wparam == WM_RBUTTONDOWN && shiftPressed) {
        MSLLHOOKSTRUCT* info = (MSLLHOOKSTRUCT*)lparam;

        g_currentTouch[1] = {};
        g_currentTouch[1].pointerInfo.pointerType = PT_TOUCH;
        g_currentTouch[1].pointerInfo.ptPixelLocation.x = info->pt.x;
        g_currentTouch[1].pointerInfo.ptPixelLocation.y = info->pt.y;
        g_currentTouch[1].pointerInfo.pointerFlags = POINTER_FLAG_DOWN | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
        g_currentTouch[1].pointerInfo.pointerId = 1;

        BOOL result = InjectTouchInput(1, &g_currentTouch[1]);
        if (result) {
            g_currentTouch[1].pointerInfo.pointerFlags = POINTER_FLAG_UPDATE | POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT;
            g_touchActive[1] = true;
        }

        return 1;
    }
    else if (wparam == WM_MOUSEMOVE) {
        MSLLHOOKSTRUCT* info = (MSLLHOOKSTRUCT*)lparam;

        if (g_touchActive[0]) {
            g_currentTouch[0].pointerInfo.ptPixelLocation.x = info->pt.x;
            g_currentTouch[0].pointerInfo.ptPixelLocation.y = info->pt.y;
        }

        if (g_touchActive[1]) {
            g_currentTouch[1].pointerInfo.ptPixelLocation.x = info->pt.x + 80;
            g_currentTouch[1].pointerInfo.ptPixelLocation.y = info->pt.y + 80;
        }

        if (g_touchActive[0] || g_touchActive[1]) {
            POINTER_TOUCH_INFO contacts[2];
            int count = 0;
            if (g_touchActive[0]) contacts[count++] = g_currentTouch[0];
            if (g_touchActive[1]) contacts[count++] = g_currentTouch[1];
            InjectTouchInput(count, contacts);
        }
    }
    else if (wparam == WM_LBUTTONUP && g_touchActive[0]) {
        g_currentTouch[0].pointerInfo.pointerFlags = POINTER_FLAG_UP;
        InjectTouchInput(1, &g_currentTouch[0]);
        g_touchActive[0] = false;
        return 1;
    }
    else if (wparam == WM_RBUTTONUP && g_touchActive[1]) {
        g_currentTouch[1].pointerInfo.pointerFlags = POINTER_FLAG_UP;
        InjectTouchInput(1, &g_currentTouch[1]);
        g_touchActive[1] = false;
        return 1;
    }

    return CallNextHookEx(nullptr, code, wparam, lparam);
}

void PixelDrawer::moveMethod11_HookInjected(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
}

void PixelDrawer::initGlobalHook() {
    if (g_hookInitialized) return;

    g_hInstance = GetModuleHandleW(nullptr);
    if (!g_hInstance) {
        qDebug() << "GetModuleHandleW failed";
        return;
    }

    g_mouseHook = SetWindowsHookExW(WH_MOUSE_LL, LowLevelMouseProc, g_hInstance, 0);
    if (!g_mouseHook) {
        qDebug() << "SetWindowsHookExW failed:" << GetLastError();
        return;
    }

    BOOL initResult = InitializeTouchInjection(10, TOUCH_FEEDBACK_DEFAULT);
    if (!initResult) {
        qDebug() << "InitializeTouchInjection failed:" << GetLastError();
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = nullptr;
        return;
    }

    g_hookInitialized = true;
    qDebug() << "Global mouse hook initialized";
}

void PixelDrawer::cleanupGlobalHook() {
    if (g_mouseHook) {
        UnhookWindowsHookEx(g_mouseHook);
        g_mouseHook = nullptr;
    }
    g_hookInitialized = false;
    g_touchActive[0] = false;
    g_touchActive[1] = false;
}

bool PixelDrawer::checkUserInterference() {
    DWORD now = GetTickCount();

    if (now - lastMoveTime > 500) {
        QPoint currentPos = getCursorPos();
        int dx = currentPos.x() - lastPos.x();
        int dy = currentPos.y() - lastPos.y();
        if (dx * dx + dy * dy > 100) {
            qDebug() << "User interference detected, moved" << dx << dy;
            return true;
        }
    }

    return false;
}

void PixelDrawer::drawPixmapOnScreen(const QPixmap& pixmap, QPoint offset) {
    QImage image = pixmap.toImage();
    if (image.format() != QImage::Format_Grayscale8) {
        image = image.convertToFormat(QImage::Format_Grayscale8);
    }

    int w = image.width();
    int h = image.height();

    lastPos = getCursorPos();
    lastMoveTime = GetTickCount64();
    QThread::sleep(1);

    const uchar* bits = image.bits();
    int bytesPerLine = image.bytesPerLine();

    bool currentlyDown = false;
    int lastDetectedRow = -1;

    for (int y = 0; y < h && (!stopFlag || !*stopFlag); ++y) {
        if (stopFlag && *stopFlag) break;

        if (checkUserInterference()) {
            break;
        }

        if (m_autoColor && (y == 0 || y > lastDetectedRow)) {
            QColor detectedColor = getPixelColor(lastPos.x(), lastPos.y());
            if (detectedColor.isValid()) {
                m_currentColor = detectedColor;
                lastDetectedRow = y;
                qDebug() << "Row" << y << "detected color:" << detectedColor.name();

                for (int x = 0; x < w; ++x) {
                    if (bits[y * bytesPerLine + x] < threshold) {
                        QColor c = m_currentColor;
                        if (c.isValid()) {
                            image.setPixel(x, y, qRgba(c.red(), c.green(), c.blue(), 255));
                        }
                    }
                }
            }
        }

        bool rowHasDark = false;
        for (int x = 0; x < w; ++x) {
            if (bits[y * bytesPerLine + x] < threshold) {
                rowHasDark = true;
                break;
            }
        }

        if (currentlyDown && !rowHasDark) {
            mouseUp();
            currentlyDown = false;
            QThread::msleep(3);
        }

        bool inStroke = false;

        for (int x = 0; x < w && (!stopFlag || !*stopFlag); ++x) {
            if (stopFlag && *stopFlag) break;

            if (checkUserInterference()) {
                break;
            }

            bool isDark = bits[y * bytesPerLine + x] < threshold;
            int screenX = x + offset.x();
            int screenY = y + offset.y();

            int screenWidth = GetSystemMetrics(SM_CXSCREEN);
            if (m_autoWrap && screenX > screenWidth - m_wrapMargin) {
                int edgeX = 0;
                if (detectRightEdge(offset.x() + screenX - 100, screenY, edgeX)) {
                    if (currentlyDown) {
                        mouseUp();
                        currentlyDown = false;
                    }
                    screenX = edgeX + 20;
                    screenY = screenY + 50;
                    (this->*moveMouseFunc)(screenX, screenY);
                    QThread::msleep(10);
                    mouseDown();
                    currentlyDown = true;
                }
            }

            if (isDark && !inStroke) {
                inStroke = true;
                (this->*moveMouseFunc)(screenX, screenY);
                QThread::msleep(downDelay);
                mouseDown();
                currentlyDown = true;
            } else if (isDark && inStroke) {
                int dx = screenX - lastPos.x();
                int dy = screenY - lastPos.y();
                if (dx * dx + dy * dy > 2) {
                    (this->*moveMouseFunc)(screenX, screenY);
                    QThread::msleep(moveDelay);
                }
            } else if (!isDark && inStroke) {
                mouseUp();
                currentlyDown = false;
                inStroke = false;
                QThread::msleep(2);
            }
        }

        if (inStroke) {
            mouseUp();
            currentlyDown = false;
            QThread::msleep(3);
        }
    }

    if (currentlyDown) {
        mouseUp();
    }

    SetCursorPos(lastPos.x(), lastPos.y());
}