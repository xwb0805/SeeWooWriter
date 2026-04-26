#include "mousehook.h"
#include <QCursor>

MouseHookOverlay::MouseHookOverlay(QObject* parent)
    : QObject(parent), m_active(false) {
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MouseHookOverlay::checkMouse);
}

MouseHookOverlay::~MouseHookOverlay() {
    stop();
}

void MouseHookOverlay::start() {
    if (m_active) return;
    m_active = true;
    m_lastPos = QCursor::pos();
    m_timer->start(50);
}

void MouseHookOverlay::stop() {
    m_active = false;
    m_timer->stop();
}

void MouseHookOverlay::checkMouse() {
    if (!m_active) return;
    QPoint pos = QCursor::pos();
    if (pos != m_lastPos) {
        m_lastPos = pos;
        emit mouseHookTriggered(pos);
    }
}