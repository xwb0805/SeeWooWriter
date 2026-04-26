#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QWidget>
#include <QGroupBox>
#include <QProgressBar>
#include <QThread>
#include <QLabel>
#include <QTimer>
#include <QComboBox>
#include <QCheckBox>
#include <QSystemTrayIcon>
#include <QSlider>
#include <QFont>
#include <QColor>
#include <QPixmap>
#include <QFontDatabase>
#include "ui_mainwindow.h"
#include "pixeldrawer.h"
#include "mousehook.h"
#include "keyboardhook.h"

class DrawingThread : public QThread {
    Q_OBJECT
public:
    DrawingThread(PixelDrawer* drawer, const QPixmap& pixmap, const QPoint& offset, QObject* parent = nullptr)
        : QThread(parent), m_drawer(drawer), m_pixmap(pixmap), m_offset(offset) {
        m_drawer->setParent(parent);
    }

protected:
    void run() override {
        m_drawer->drawPixmapOnScreen(m_pixmap, m_offset);
    }

private:
    PixelDrawer* m_drawer;
    QPixmap m_pixmap;
    QPoint m_offset;
};

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent* event) override;

private:
    void setupUi();
    void connectSignals();

public slots:
    void onInputTextChanged();
    void onSelectFont();
    void onPickColor();
    void onSelectImage();
    void onStartDrawing();
    void onStopDrawing();
    void onDrawingFinished();
    void onCursorUpdate();
    void onFontSizeChanged(int val);
    void onScaleChanged(int val);
    void onCountdownTick();
    void onMouseMethodChanged(int idx);
    void onAutoColorToggled(bool checked);
    void onColorDrawToggled(bool checked);
    void startDrawingNow();
    void onSelectPenZone();
    void onSelectColorZone();
    void onSelectDrawZone();
    void setPenZone(const QRect& zone);
    void setColorZone(const QRect& zone);
    void setDrawZone(const QRect& zone);

    void loadSettings();
    void saveSettings();
    void applyStyle();

private:
    void updatePreview();
    void log(const QString& msg);
    void updateScreenInfo();
    void setColorFromPalette(int index);
    void applyMultiColorFilter(const QColor& baseColor);

    QLineEdit* m_textInput;
    QFont m_font;
    QColor m_color;
    QLabel* m_colorLabel;
    QLabel* m_preview;
    QTextEdit* m_log;
    QLineEdit* m_widthInput;
    QLineEdit* m_heightInput;
    QSpinBox* m_thresholdBox;
    QSpinBox* m_fontSizeBox;
    QSpinBox* m_offsetXBox;
    QSpinBox* m_offsetYBox;
    QPushButton* m_drawBtn;
    QPushButton* m_stopBtn;
    QProgressBar* m_progress;
    PixelDrawer* m_drawer;
    DrawingThread* m_drawThread;
    QPixmap m_currentPixmap;
    QPixmap m_originalPixmap;
    bool m_isDrawing;
    std::atomic<bool> m_stopFlag;
    QTimer* m_cursorTimer;
    QLabel* m_cursorPosLabel;
    QLabel* m_screenInfoLabel;
    QList<PixelDrawer::ScreenInfo> m_screens;
    int m_fontSize;
    int m_fontWeight;
    QComboBox* m_mouseMethodCombo;
    int m_scalePercent;
    QSlider* m_scaleSlider;
    QLabel* m_scaleLabel;
    QSpinBox* m_moveDelayBox;
    QSpinBox* m_startDelayBox;
    QTimer* m_countdownTimer;
    int m_countdownRemaining;
    QLabel* m_countdownLabel;
    MouseHookOverlay* m_mouseHook;
    KeyboardHook* m_keyboardHook;
    QCheckBox* m_mouseDetectionCheck;
    QLabel* m_detectionStatusLabel;
QSystemTrayIcon* m_trayIcon;
    QWidget* m_overlayPanel;
    QLabel* m_overlayPreview;
    QProgressBar* m_overlayProgress;
    QPushButton* m_overlayStopBtn;
    QPoint m_fixedPoint;
};

#endif