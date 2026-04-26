#include "keyboardhook.h"
#include <QCoreApplication>

static KeyboardHook* g_keyboardHook = nullptr;

LRESULT CALLBACK keyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0 && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
        KBDLLHOOKSTRUCT* kb = (KBDLLHOOKSTRUCT*)lParam;
        if (kb->vkCode == VK_ESCAPE && g_keyboardHook) {
            emit g_keyboardHook->escapePressed();
        }
        if (kb->vkCode == 0x50 && g_keyboardHook) {  // 'P' key
            emit g_keyboardHook->pausePressed();
        }
    }
    return CallNextHookEx(nullptr, nCode, wParam, lParam);
}

KeyboardHook::KeyboardHook(QObject* parent)
    : QObject(parent), m_hook(nullptr) {
    g_keyboardHook = this;
}

KeyboardHook::~KeyboardHook() {
    stop();
    g_keyboardHook = nullptr;
}

void KeyboardHook::start() {
    if (m_hook) return;
    HMODULE hModule = GetModuleHandle(nullptr);
    m_hook = SetWindowsHookEx(WH_KEYBOARD_LL, keyboardHookProc, hModule, 0);
}

void KeyboardHook::stop() {
    if (m_hook) {
        UnhookWindowsHookEx(m_hook);
        m_hook = nullptr;
    }
}