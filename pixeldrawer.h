#ifndef PIXELDRAWER_H
#define PIXELDRAWER_H

#include <QPoint>
#include <QPixmap>
#include <QMutex>
#include <QThread>
#include <atomic>

#ifdef _WIN32
#include <windows.h>
#endif

enum class MouseMethod {
    SendInput_Absolute,
    SendInput_Relative,
    SetCursorPos_WinAPI,
    SendInput_Normalized,
    SendInput_Multiplied,
    Touch_Single_Touch,
    Touch_Multi_Touch,
    InjectTouch,
    InjectTouchMulti,
    PointerInjection,
    RealTouchInjection,
    Hook_Injected
};

class PixelDrawer {
public:
    PixelDrawer();
    ~PixelDrawer();

    void setThreshold(int t) { threshold = t; }
    void setOffset(int x, int y) { offsetX = x; offsetY = y; }
    void setStopFlag(std::atomic<bool>* flag) { stopFlag = flag; }
    void setMouseMethod(MouseMethod m) {
        mouseMethod = m;
        switch (m) {
            case MouseMethod::SendInput_Absolute: moveMouseFunc = &PixelDrawer::moveMethod0; break;
            case MouseMethod::SendInput_Relative: moveMouseFunc = &PixelDrawer::moveMethod1; break;
            case MouseMethod::SetCursorPos_WinAPI: moveMouseFunc = &PixelDrawer::moveMethod2; break;
            case MouseMethod::SendInput_Normalized: moveMouseFunc = &PixelDrawer::moveMethod3; break;
            case MouseMethod::SendInput_Multiplied: moveMouseFunc = &PixelDrawer::moveMethod4; break;
            case MouseMethod::Touch_Single_Touch: moveMouseFunc = &PixelDrawer::moveMethod5_TouchInput; break;
            case MouseMethod::Touch_Multi_Touch: moveMouseFunc = &PixelDrawer::moveMethod6_TouchInputMulti; break;
            case MouseMethod::InjectTouch: moveMouseFunc = &PixelDrawer::moveMethod7_PointerTouch; break;
            case MouseMethod::InjectTouchMulti: moveMouseFunc = &PixelDrawer::moveMethod8_PointerTouchMulti; break;
            case MouseMethod::PointerInjection: moveMouseFunc = &PixelDrawer::moveMethod9_PointerInjection; break;
            case MouseMethod::RealTouchInjection: moveMouseFunc = &PixelDrawer::moveMethod10_RealTouchInjection; break;
            case MouseMethod::Hook_Injected:
                moveMouseFunc = &PixelDrawer::moveMethod11_HookInjected;
                initGlobalHook();
                break;
        }
    }
    void setMoveDelay(int ms) { moveDelay = ms; }
    void setDownDelay(int ms) { downDelay = ms; }
    void setDelays(int moveMs, int downMs) { moveDelay = moveMs; downDelay = downMs; }
    QObject* parent() const { return m_parent; }
    void setParent(QObject* p) { m_parent = p; }

    void drawPixmapOnScreen(const QPixmap& pixmap, QPoint offset);

    void enableAutoWrap(bool enable) { m_autoWrap = enable; }
    void enableAutoColor(bool enable) { m_autoColor = enable; }
    void enableColorDraw(bool enable) { m_colorDraw = enable; }
    void enableAutoSelectColor(bool enable) { m_autoSelectColor = enable; }
    void setWrapMargin(int margin) { m_wrapMargin = margin; }
    void setCurrentColor(QColor color) { m_currentColor = color; }
    QColor getCurrentColor() const { return m_currentColor; }

    void setPenZone(QRect zone) { m_penZone = zone; }
    void setColorZone(QRect zone) { m_colorZone = zone; }
    void setDrawZone(QRect zone) { m_drawZone = zone; }
    QRect getPenZone() const { return m_penZone; }
    QRect getColorZone() const { return m_colorZone; }
    QRect getDrawZone() const { return m_drawZone; }

    static bool findSeeBoardWindow();
    bool clickPenIcon();
    bool detectAndSelectColor(QColor& selected);

    struct ScreenInfo {
        int left, top, width, height;
        double dpiScale;
        bool isPrimary;
    };
    static QList<ScreenInfo> getScreenInfo();
    static int getScreenCount();
    static QPoint virtualScreenSize();
    static QPoint getCursorPos();

    static QImage captureScreen(int x, int y, int w, int h);
    static QColor getPixelColor(int x, int y);
    static bool detectRightEdge(int startX, int startY, int& outX);

private:
    void mouseDown();
    void mouseUp();
    bool checkUserInterference();

    void moveMethod0(int x, int y);
    void moveMethod1(int x, int y);
    void moveMethod2(int x, int y);
    void moveMethod3(int x, int y);
    void moveMethod4(int x, int y);
    void moveMethod5_TouchInput(int x, int y);
    void moveMethod6_TouchInputMulti(int x, int y);
    void moveMethod7_PointerTouch(int x, int y);
    void moveMethod8_PointerTouchMulti(int x, int y);
    void moveMethod9_PointerInjection(int x, int y);
    void moveMethod10_RealTouchInjection(int x, int y);
    void moveMethod11_HookInjected(int x, int y);

    static void initGlobalHook();
    static void cleanupGlobalHook();

    void (PixelDrawer::*moveMouseFunc)(int, int);

    int threshold;
    int offsetX, offsetY;
    std::atomic<bool>* stopFlag;
    QMutex mouseMutex;
    QPoint lastPos;
    QObject* m_parent;
    MouseMethod mouseMethod;
    int moveDelay;
    int downDelay;
    bool userMouseDown;
    quint64 lastMoveTime;
    DWORD lastMouseDownTime;

    bool m_autoWrap = false;
    bool m_autoColor = false;
    bool m_colorDraw = false;
    bool m_autoSelectColor = false;
    int m_wrapMargin = 50;
    QColor m_currentColor = Qt::red;
    QRect m_penZone;
    QRect m_colorZone;
    QRect m_drawZone;
    QList<QColor> m_paletteColors;
};

#endif