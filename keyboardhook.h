#ifndef KEYBOARDHOOK_H
#define KEYBOARDHOOK_H

#include <QObject>
#include <windows.h>

class KeyboardHook : public QObject {
    Q_OBJECT
public:
    explicit KeyboardHook(QObject* parent = nullptr);
    ~KeyboardHook();

    void start();
    void stop();

signals:
    void escapePressed();

private:
    HHOOK m_hook;
};

#endif