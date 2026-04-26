#ifndef MOUSEHOOK_H
#define MOUSEHOOK_H

#include <QObject>
#include <QTimer>
#include <QPoint>
#include <windows.h>

class MouseHookOverlay : public QObject {
    Q_OBJECT
public:
    explicit MouseHookOverlay(QObject* parent = nullptr);
    ~MouseHookOverlay();

    void start();
    void stop();
    bool isActive() const { return m_active; }

signals:
    void mouseHookTriggered(const QPoint& pos);

private slots:
    void checkMouse();

private:
    bool m_active;
    QTimer* m_timer;
    QPoint m_lastPos;
};

#endif