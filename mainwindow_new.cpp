#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFontDialog>
#include <QColorDialog>
#include <QFileDialog>
#include <QPainter>
#include <QMessageBox>
#include <QApplication>
#include <QCoreApplication>
#include <QFileInfo>
#include <QTime>
#include <QScreen>
#include <QFontDatabase>
#include <QKeyEvent>
#include <QInputDialog>
#include <QDebug>
#include <QLabel>
#include <QSpinBox>

class ZoneSelector : public QWidget {
    Q_OBJECT
public:
    explicit ZoneSelector(const QString& title, QWidget* parent = nullptr)
        : QWidget(parent), m_title(title), m_selecting(false) {
        setWindowTitle(title);
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_DeleteOnClose);

        QScreen* primaryScreen = QGuiApplication::primaryScreen();
        setGeometry(primaryScreen->geometry());

        setCursor(Qt::CrossCursor);
    }

    QRect getSelectedZone() const { return m_selectedZone; }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        m_startPoint = event->pos();
        m_selecting = true;
        m_selectedZone = QRect();
        update();
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (m_selecting) {
            m_endPoint = event->pos();
            m_selectedZone = QRect(m_startPoint, m_endPoint).normalized();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        m_selecting = false;
        m_endPoint = event->pos();
        m_selectedZone = QRect(m_startPoint, m_endPoint).normalized();
        update();
        if (m_selectedZone.width() > 10 && m_selectedZone.height() > 10) {
            accept();
        }
    }

    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);

        painter.setPen(QPen(Qt::blue, 2));
        painter.setBrush(QBrush(QColor(0, 0, 255, 30)));
        if (!m_selectedZone.isEmpty()) {
            painter.drawRect(m_selectedZone);
        }

        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(QBrush(Qt::black));
        painter.drawRect(rect().adjusted(10, 10, -10, -10));

        QString text = m_title + "\n拖动框选区域";
        painter.setPen(Qt::white);
        painter.drawText(20, 30, text);
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            reject();
        } else if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            if (!m_selectedZone.isEmpty() && m_selectedZone.width() > 10 && m_selectedZone.height() > 10) {
                accept();
            }
        }
    }

private slots:
    void accept() {
        hide();
        QWidget* p = parentWidget();
        if (p) {
            p->show();
            MainWindow* mw = qobject_cast<MainWindow*>(p);
            if (mw && !m_selectedZone.isEmpty()) {
                if (m_title.contains("笔")) {
                    mw->setPenZone(m_selectedZone);
                } else {
                    mw->setColorZone(m_selectedZone);
                }
            }
        }
        close();
    }

    void reject() {
        hide();
        QWidget* p = parentWidget();
        if (p) p->show();
        close();
    }

private:
    QString m_title;
    QPoint m_startPoint, m_endPoint;
    QRect m_selectedZone;
    bool m_selecting;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_textInput(nullptr)
    , m_color(Qt::red), m_colorLabel(nullptr), m_preview(nullptr)
    , m_log(nullptr), m_widthInput(nullptr), m_heightInput(nullptr)
    , m_thresholdBox(nullptr), m_fontSizeBox(nullptr)
    , m_offsetXBox(nullptr), m_offsetYBox(nullptr)
    , m_drawBtn(nullptr), m_stopBtn(nullptr), m_progress(nullptr)
    , m_drawer(new PixelDrawer(this)), m_drawThread(nullptr)
    , m_isDrawing(false), m_cursorTimer(nullptr)
    , m_cursorPosLabel(nullptr), m_screenInfoLabel(nullptr)
    , m_fontSize(48), m_fontWeight(QFont::Bold)
    , m_mouseMethodCombo(nullptr), m_scalePercent(100)
    , m_scaleSlider(nullptr), m_scaleLabel(nullptr)
    , m_moveDelayBox(nullptr), m_startDelayBox(nullptr)
    , m_countdownTimer(new QTimer(this)), m_countdownRemaining(0)
    , m_countdownLabel(nullptr), m_mouseHook(nullptr)
    , m_keyboardHook(nullptr), m_mouseDetectionCheck(nullptr)
    , m_detectionStatusLabel(nullptr), m_trayIcon(nullptr)
    , m_overlayPanel(nullptr)
{
    ui->setupUi(this);
    setupUi();
    connectSignals();
}

void MainWindow::setupUi() {
    setWindowTitle("MouseWriter - 鼠标写字");
    resize(600, 750);

    QGroupBox* groupInput = new QGroupBox("输入文本/图片", this);
    QVBoxLayout* inputLayout = new QVBoxLayout(groupInput);

    m_textInput = new QLineEdit(this);
    m_textInput->setPlaceholderText("输入要书写的文字...");
    inputLayout->addWidget(m_textInput);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* selectImageBtn = new QPushButton("选择图片", this);
    QPushButton* selectFontBtn = new QPushButton("选择字体", this);
    QPushButton* pickColorBtn = new QPushButton("选择颜色", this);

    btnLayout->addWidget(selectImageBtn);
    btnLayout->addWidget(selectFontBtn);
    btnLayout->addWidget(pickColorBtn);
    inputLayout->addLayout(btnLayout);

    QHBoxLayout* sizeLayout = new QHBoxLayout();
    sizeLayout->addWidget(new QLabel("宽度:", this));
    m_widthInput = new QLineEdit("800", this);
    sizeLayout->addWidget(m_widthInput);
    sizeLayout->addWidget(new QLabel("高度:", this));
    m_heightInput = new QLineEdit("300", this);
    sizeLayout->addWidget(m_heightInput);
    sizeLayout->addWidget(new QLabel("字号:", this));
    m_fontSizeBox = new QSpinBox(this);
    m_fontSizeBox->setRange(12, 200);
    m_fontSizeBox->setValue(48);
    sizeLayout->addWidget(m_fontSizeBox);
    inputLayout->addLayout(sizeLayout);

    QHBoxLayout* colorPaletteLayout = new QHBoxLayout();
    QStringList colors = {"#FF0000", "#00FF00", "#0000FF", "#FFFF00", "#000000"};
    for (int i = 0; i < colors.count(); i++) {
        QPushButton* btn = new QPushButton(this);
        btn->setFixedSize(30, 25);
        btn->setStyleSheet(QString("background-color: %1; border: 2px solid black;").arg(colors[i]));
        connect(btn, &QPushButton::clicked, [this, i]() { setColorFromPalette(i); });
        colorPaletteLayout->addWidget(btn);
    }
    inputLayout->addLayout(colorPaletteLayout);

    m_colorLabel = new QLabel(this);
    m_colorLabel->setFixedSize(100, 30);
    m_colorLabel->setStyleSheet("background-color: red;");
    inputLayout->addWidget(m_colorLabel);

    QGroupBox* groupPreview = new QGroupBox("预览", this);
    m_preview = new QLabel(this);
    m_preview->setAlignment(Qt::AlignCenter);
    m_preview->setMinimumHeight(150);
    m_preview->setStyleSheet("border: 1px solid gray; background: white;");

    QVBoxLayout* previewLayout = new QVBoxLayout(groupPreview);
    previewLayout->addWidget(m_preview);

    QGroupBox* groupSettings = new QGroupBox("设置", this);
    QGridLayout* settingsLayout = new QGridLayout(groupSettings);

    int row = 0;
    settingsLayout->addWidget(new QLabel("阈值:"), row, 0);
    m_thresholdBox = new QSpinBox(this);
    m_thresholdBox->setRange(0, 255);
    m_thresholdBox->setValue(128);
    settingsLayout->addWidget(m_thresholdBox, row, 1);

    settingsLayout->addWidget(new QLabel("偏移X:"), row, 2);
    m_offsetXBox = new QSpinBox(this);
    m_offsetXBox->setRange(-5000, 5000);
    settingsLayout->addWidget(m_offsetXBox, row, 3);

    row++;
    settingsLayout->addWidget(new QLabel("缩放(%):"), row, 0);
    m_scaleSlider = new QSlider(Qt::Horizontal, this);
    m_scaleSlider->setRange(50, 200);
    m_scaleSlider->setValue(100);
    m_scaleLabel = new QLabel("100", this);
    settingsLayout->addWidget(m_scaleSlider, row, 1, 1, 2);
    settingsLayout->addWidget(m_scaleLabel, row, 3);

    row++;
    settingsLayout->addWidget(new QLabel("移动方式:"), row, 0);
    m_mouseMethodCombo = new QComboBox(this);
    QStringList methods = {"SendInput (默认)", "mouse_event", "SetCursorPos", "SendMessage", "PostMessage",
                     "触控-单点", "触控-多点", "InjectTouch", "InjectTouchMulti",
                     "Pointer-注入", "RealTouch注入", "Hook注入(左键+RSHIFT, 双指:RCTRL)"};
    m_mouseMethodCombo->addItems(methods);
    m_mouseMethodCombo->setCurrentIndex(10);
    settingsLayout->addWidget(m_mouseMethodCombo, row, 1, 1, 3);

    row++;
    settingsLayout->addWidget(new QLabel("移动延迟(ms):"), row, 0);
    m_moveDelayBox = new QSpinBox(this);
    m_moveDelayBox->setRange(0, 1000);
    m_moveDelayBox->setValue(5);
    settingsLayout->addWidget(m_moveDelayBox, row, 1);

    settingsLayout->addWidget(new QLabel("开始延迟(s):"), row, 2);
    m_startDelayBox = new QSpinBox(this);
    m_startDelayBox->setRange(0, 60);
    m_startDelayBox->setValue(3);
    settingsLayout->addWidget(m_startDelayBox, row, 3);

    row++;
    m_mouseDetectionCheck = new QCheckBox("检测移动自动停止", this);
    m_mouseDetectionCheck->setChecked(true);
    settingsLayout->addWidget(m_mouseDetectionCheck, row, 1);

    QCheckBox* autoColorSelectCheck = new QCheckBox("自动点击希沃选色", this);
    settingsLayout->addWidget(autoColorSelectCheck, row, 2);

    row++;
    QPushButton* selectPenZoneBtn = new QPushButton("选择笔图标", this);
    selectPenZoneBtn->setFixedWidth(100);
    settingsLayout->addWidget(selectPenZoneBtn, row, 1);

    QPushButton* selectColorZoneBtn = new QPushButton("选择调色板", this);
    selectColorZoneBtn->setFixedWidth(100);
    settingsLayout->addWidget(selectColorZoneBtn, row, 2);

    connect(selectPenZoneBtn, &QPushButton::clicked, this, &MainWindow::onSelectPenZone);
    connect(selectColorZoneBtn, &QPushButton::clicked, this, &MainWindow::onSelectColorZone);
    connect(autoColorSelectCheck, &QCheckBox::toggled, this, &MainWindow::onAutoColorToggled);

    QGroupBox* groupControl = new QGroupBox("控制", this);
    QHBoxLayout* controlLayout = new QHBoxLayout(groupControl);

    m_drawBtn = new QPushButton("开始书写", this);
    m_drawBtn->setMinimumHeight(40);
    m_stopBtn = new QPushButton("停止", this);
    m_stopBtn->setMinimumHeight(40);
    m_stopBtn->setEnabled(false);

    controlLayout->addWidget(m_drawBtn);
    controlLayout->addWidget(m_stopBtn);

    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    m_progress->setValue(0);

    m_log = new QTextEdit(this);
    m_log->setMaximumHeight(80);
    m_log->setReadOnly(true);

    m_cursorPosLabel = new QLabel("鼠标: 0, 0", this);
    m_screenInfoLabel = new QLabel("屏幕: ", this);
    m_detectionStatusLabel = new QLabel("", this);

    centralWidget()->setLayout(newQVBoxLayout = new QVBoxLayout());
    centralWidget()->layout()->addWidget(groupInput);
    centralWidget()->layout()->addWidget(groupPreview);
    centralWidget()->layout()->addWidget(groupSettings);
    centralWidget()->layout()->addWidget(m_progress);
    centralWidget()->layout()->addWidget(groupControl);
    centralWidget()->layout()->addWidget(m_log);
    centralWidget()->layout()->addWidget(m_cursorPosLabel);
    centralWidget()->layout()->addWidget(m_screenInfoLabel);
    centralWidget()->layout()->addWidget(m_detectionStatusLabel);

    connect(m_textInput, &QLineEdit::textChanged, this, &MainWindow::onInputTextChanged);
    connect(selectImageBtn, &QPushButton::clicked, this, &MainWindow::onSelectImage);
    connect(selectFontBtn, &QPushButton::clicked, this, &MainWindow::onSelectFont);
    connect(pickColorBtn, &QPushButton::clicked, this, &MainWindow::onPickColor);
    connect(m_drawBtn, &QPushButton::clicked, this, &MainWindow::onStartDrawing);
    connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopDrawing);
    connect(m_mouseMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMouseMethodChanged);
    connect(m_fontSizeBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onFontSizeChanged);
    connect(m_scaleSlider, &QSlider::valueChanged, this, &MainWindow::onScaleChanged);

    m_cursorTimer = new QTimer(this);
    connect(m_cursorTimer, &QTimer::timeout, this, &MainWindow::onCursorUpdate);
    m_cursorTimer->start(100);
    connect(m_countdownTimer, &QTimer::timeout, this, &MainWindow::onCountdownTick);

    m_keyboardHook = new KeyboardHook(this);
    connect(m_keyboardHook, &KeyboardHook::escapePressed, this, [this]() {
        if (m_isDrawing) {
            m_stopFlag = true;
            log("按下Escape，已停止!");
        }
    });
    m_keyboardHook->start();

    m_mouseHook = new MouseHookOverlay(this);
    m_screens = PixelDrawer::getScreenInfo();
    for (const auto& screen : m_screens) {
        log(QString("屏幕: %1x%2").arg(screen.width).arg(screen.height));
    }
}

void MainWindow::onSelectPenZone() {
    ZoneSelector* selector = new ZoneSelector("选择笔图标区域", this);
    selector->show();
}

void MainWindow::onSelectColorZone() {
    ZoneSelector* selector = new ZoneSelector("选择调色板区域", this);
    selector->show();
}

void MainWindow::setPenZone(const QRect& zone) {
    m_drawer->setPenZone(zone);
    log(QString("笔图标区域: %1,%2 %3x%4").arg(zone.x()).arg(zone.y()).arg(zone.width()).arg(zone.height()));
}

void MainWindow::setColorZone(const QRect& zone) {
    m_drawer->setColorZone(zone);
    log(QString("调色板区域: %1,%2 %3x%4").arg(zone.x()).arg(zone.y()).arg(zone.width()).arg(zone.height()));
}

void MainWindow::onAutoColorToggled(bool checked) {
    if (m_drawer) {
        m_drawer->enableAutoColor(checked);
    }
    if (checked) {
        if (m_drawer->getPenZone().isEmpty() || m_drawer->getColorZone().isEmpty()) {
            QMessageBox::information(this, "提示", "请先框选笔图标区域和调色板区域");
        }
    }
}