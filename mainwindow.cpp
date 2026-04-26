#include "mainwindow.h"
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
#include <functional>
#include <QSettings>

class ZoneSelector : public QWidget {
public:
    ZoneSelector(const QString& title, QWidget* parent, std::function<void(const QRect&)> callback)
        : QWidget(parent), m_title(title), m_callback(callback), m_selecting(false) {
        setWindowTitle(title);
        setWindowFlags(Qt::Window | Qt::FramelessWindowHint | Qt::Tool);
        setAttribute(Qt::WA_TranslucentBackground);
        setAttribute(Qt::WA_DeleteOnClose);

        QScreen* primaryScreen = QGuiApplication::primaryScreen();
        setGeometry(primaryScreen->geometry());
        setCursor(Qt::CrossCursor);
    }

protected:
    void mousePressEvent(QMouseEvent* event) override {
        m_startPoint = event->globalPosition().toPoint();
        m_selecting = true;
        m_selectedZone = QRect();
        update();
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        if (m_selecting) {
            m_endPoint = event->globalPosition().toPoint();
            m_selectedZone = QRect(m_startPoint, m_endPoint).normalized();
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        m_selecting = false;
        m_endPoint = event->globalPosition().toPoint();
        m_selectedZone = QRect(m_startPoint, m_endPoint).normalized();
        update();
        if (m_selectedZone.width() > 10 && m_selectedZone.height() > 10) {
            if (m_callback) m_callback(m_selectedZone);
            hide();
            close();
        }
    }

    void paintEvent(QPaintEvent* event) override {
        Q_UNUSED(event);
        QPainter painter(this);
        painter.setPen(QPen(QColor(0, 120, 215), 3));
        painter.setBrush(QBrush(QColor(0, 120, 215, 50)));
        if (!m_selectedZone.isEmpty()) painter.drawRect(m_selectedZone);
        painter.setPen(QPen(Qt::white, 1));
        painter.setBrush(QBrush(QColor(0, 0, 0, 150)));
        painter.drawRect(rect());
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 16));
        painter.drawText(20, 40, m_title);
        painter.setFont(QFont("Microsoft YaHei", 14));
        painter.drawText(20, 70, "拖动框选区域，按 ESC 取消");
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            hide();
            close();
        }
    }

private:
    QString m_title;
    QPoint m_startPoint, m_endPoint;
    QRect m_selectedZone;
    bool m_selecting;
    std::function<void(const QRect&)> m_callback;
};

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_drawThread(nullptr)
    , m_drawer(new PixelDrawer())
    , m_isDrawing(false)
    , m_fontSize(48)
    , m_fontWeight(50)
    , m_scalePercent(100)
    , m_mouseHook(nullptr)
    , m_countdownTimer(nullptr)
    , m_countdownRemaining(0)
    , m_progress(nullptr)
    , m_scaleSlider(nullptr)
    , m_scaleLabel(nullptr)
    , m_keyboardHook(nullptr)
    , m_trayIcon(nullptr)
    , m_overlayPanel(nullptr)
    , m_overlayPreview(nullptr)
    , m_overlayProgress(nullptr)
    , m_overlayStopBtn(nullptr)
{
    setupUi();

    m_font.setFamily(QFontDatabase::families().contains("SimHei") ? "SimHei" : QFontDatabase::families().first());
    m_font.setPointSize(48);
    m_font.setWeight(QFont::Weight(50));
    m_color = Qt::red;

    connectSignals();
    loadSettings();
    updateScreenInfo();
}

MainWindow::~MainWindow() {
    saveSettings();
    if (m_drawThread && m_drawThread->isRunning()) {
        m_stopFlag = true;
        m_drawThread->wait(3000);
    }
    delete m_drawer;
    delete m_keyboardHook;
    delete m_mouseHook;
}

void MainWindow::loadSettings() {
    QSettings settings("MouseWriter", "MouseWriter");
    m_widthInput->setText(settings.value("width", "800").toString());
    m_heightInput->setText(settings.value("height", "300").toString());
    m_offsetXBox->setValue(settings.value("offsetX", 0).toInt());
    m_offsetYBox->setValue(settings.value("offsetY", 0).toInt());
    m_thresholdBox->setValue(settings.value("threshold", 128).toInt());
    m_fontSizeBox->setValue(settings.value("fontSize", 48).toInt());
    m_moveDelayBox->setValue(settings.value("moveDelay", 5).toInt());
    m_startDelayBox->setValue(settings.value("startDelay", 3).toInt());

    QRect penZone = settings.value("penZone").toRect();
    if (!penZone.isEmpty()) m_drawer->setPenZone(penZone);
    QRect colorZone = settings.value("colorZone").toRect();
    if (!colorZone.isEmpty()) m_drawer->setColorZone(colorZone);
    QRect drawZone = settings.value("drawZone").toRect();
    if (!drawZone.isEmpty()) m_drawer->setDrawZone(drawZone);

    QCheckBox* autoColorSelectCheck = findChild<QCheckBox*>("autoColorSelectCheck");
    if (autoColorSelectCheck) {
        autoColorSelectCheck->setChecked(settings.value("autoColorSelect", false).toBool());
    }

    QCheckBox* colorDrawCheck = findChild<QCheckBox*>("colorDrawCheck");
    if (colorDrawCheck) {
        colorDrawCheck->setChecked(settings.value("colorDraw", true).toBool());
        if (!colorDrawCheck->isChecked()) {
            m_currentPixmap = m_originalPixmap;
        }
    }

    log("已加载上次设置");
}

void MainWindow::saveSettings() {
    QSettings settings("MouseWriter", "MouseWriter");
    settings.setValue("width", m_widthInput->text());
    settings.setValue("height", m_heightInput->text());
    settings.setValue("offsetX", m_offsetXBox->value());
    settings.setValue("offsetY", m_offsetYBox->value());
    settings.setValue("threshold", m_thresholdBox->value());

    QCheckBox* colorDrawCheck = findChild<QCheckBox*>("colorDrawCheck");
    if (colorDrawCheck) {
        settings.setValue("colorDraw", colorDrawCheck->isChecked());
    }

    QCheckBox* autoColorSelectCheck = findChild<QCheckBox*>("autoColorSelectCheck");
    if (autoColorSelectCheck) {
        settings.setValue("autoColorSelect", autoColorSelectCheck->isChecked());
    }
    settings.setValue("fontSize", m_fontSizeBox->value());
    settings.setValue("moveDelay", m_moveDelayBox->value());
    settings.setValue("startDelay", m_startDelayBox->value());

    if (!m_drawer->getPenZone().isEmpty())
        settings.setValue("penZone", m_drawer->getPenZone());
    if (!m_drawer->getColorZone().isEmpty())
        settings.setValue("colorZone", m_drawer->getColorZone());
    if (!m_drawer->getDrawZone().isEmpty())
        settings.setValue("drawZone", m_drawer->getDrawZone());
}

void MainWindow::setupUi() {
    Ui_MainWindow ui;
    ui.setupUi(this);

    setWindowTitle("SeeWoo Writer");
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
    setMinimumSize(600, 720);

    applyStyle();

    m_textInput = ui.textInput;
    m_preview = ui.preview;
    m_log = ui.log;
    m_widthInput = ui.widthInput;
    m_heightInput = ui.heightInput;
    m_thresholdBox = ui.thresholdBox;
    m_fontSizeBox = ui.fontSizeBox;
    m_offsetXBox = ui.offsetXBox;
    m_offsetYBox = ui.offsetYBox;
    m_drawBtn = ui.drawBtn;
    m_stopBtn = ui.stopBtn;
    m_mouseMethodCombo = ui.mouseMethodCombo;
    m_scaleSlider = ui.scaleSlider;
    m_moveDelayBox = ui.moveDelayBox;
    m_startDelayBox = ui.startDelayBox;
    m_mouseDetectionCheck = ui.mouseDetectionCheck;
    m_cursorPosLabel = ui.cursorPosLabel;
    m_screenInfoLabel = ui.screenInfoLabel;

    setWindowTitle("MouseWriter - 鼠标写字");
    setMinimumSize(600, 720);
}

void MainWindow::connectSignals() {
    if (m_textInput) connect(m_textInput, &QLineEdit::textChanged, this, &MainWindow::onInputTextChanged);

    QPushButton* selectFontBtn = findChild<QPushButton*>("selectFontBtn");
    QPushButton* pickColorBtn = findChild<QPushButton*>("pickColorBtn");
    QPushButton* selectImageBtn = findChild<QPushButton*>("selectImageBtn");

    if (selectFontBtn) connect(selectFontBtn, &QPushButton::clicked, this, &MainWindow::onSelectFont);
    if (pickColorBtn) connect(pickColorBtn, &QPushButton::clicked, this, &MainWindow::onPickColor);
    if (selectImageBtn) connect(selectImageBtn, &QPushButton::clicked, this, &MainWindow::onSelectImage);

    QPushButton* colorBtn1 = findChild<QPushButton*>("colorBtn1");
    QPushButton* colorBtn2 = findChild<QPushButton*>("colorBtn2");
    QPushButton* colorBtn3 = findChild<QPushButton*>("colorBtn3");
    QPushButton* colorBtn4 = findChild<QPushButton*>("colorBtn4");
    QPushButton* colorBtn5 = findChild<QPushButton*>("colorBtn5");
    if (colorBtn1) connect(colorBtn1, &QPushButton::clicked, [this]() { setColorFromPalette(0); });
    if (colorBtn2) connect(colorBtn2, &QPushButton::clicked, [this]() { setColorFromPalette(1); });
    if (colorBtn3) connect(colorBtn3, &QPushButton::clicked, [this]() { setColorFromPalette(2); });
    if (colorBtn4) connect(colorBtn4, &QPushButton::clicked, [this]() { setColorFromPalette(3); });
    if (colorBtn5) connect(colorBtn5, &QPushButton::clicked, [this]() { setColorFromPalette(4); });

    QPushButton* selectPenZoneBtn = findChild<QPushButton*>("selectPenZoneBtn");
    QPushButton* selectColorZoneBtn = findChild<QPushButton*>("selectColorZoneBtn");
    QPushButton* selectDrawZoneBtn = findChild<QPushButton*>("selectDrawZoneBtn");
    if (selectPenZoneBtn) connect(selectPenZoneBtn, &QPushButton::clicked, this, &MainWindow::onSelectPenZone);
    if (selectColorZoneBtn) connect(selectColorZoneBtn, &QPushButton::clicked, this, &MainWindow::onSelectColorZone);
    if (selectDrawZoneBtn) connect(selectDrawZoneBtn, &QPushButton::clicked, this, &MainWindow::onSelectDrawZone);

    if (m_drawBtn) connect(m_drawBtn, &QPushButton::clicked, this, &MainWindow::onStartDrawing);
    if (m_stopBtn) connect(m_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopDrawing);

    if (m_fontSizeBox) connect(m_fontSizeBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &MainWindow::onFontSizeChanged);
    if (m_scaleSlider) connect(m_scaleSlider, &QSlider::valueChanged, this, &MainWindow::onScaleChanged);
    if (m_mouseMethodCombo) connect(m_mouseMethodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onMouseMethodChanged);

    QCheckBox* autoColorSelectCheck = findChild<QCheckBox*>("autoColorSelectCheck");
    if (autoColorSelectCheck) {
        connect(autoColorSelectCheck, &QCheckBox::toggled, this, &MainWindow::onAutoColorToggled);
    }

    QCheckBox* colorDrawCheck = findChild<QCheckBox*>("colorDrawCheck");
    if (colorDrawCheck) {
        connect(colorDrawCheck, &QCheckBox::toggled, this, &MainWindow::onColorDrawToggled);
    }

    m_cursorTimer = new QTimer(this);
    connect(m_cursorTimer, &QTimer::timeout, this, &MainWindow::onCursorUpdate);
    m_cursorTimer->start(100);

    m_countdownTimer = new QTimer(this);
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

    QScreen* primaryScreen = QGuiApplication::primaryScreen();
    QRect screen = primaryScreen->geometry();

    m_overlayPanel = new QWidget;
    m_overlayPanel->setWindowFlags(Qt::WindowStaysOnTopHint | Qt::Tool | Qt::FramelessWindowHint);

    int panelWidth = 220;
    int panelHeight = 260;
    int x = screen.right() - panelWidth - 30;
    int y = screen.top() + 30;
    m_overlayPanel->setGeometry(x, y, panelWidth, panelHeight);
    m_overlayPanel->hide();

    QVBoxLayout* overlayLayout = new QVBoxLayout(m_overlayPanel);
    overlayLayout->setSpacing(10);
    overlayLayout->setContentsMargins(12, 12, 12, 12);
    m_overlayPanel->setStyleSheet("background-color: white; border-radius: 12px;");

    m_overlayPreview = new QLabel;
    m_overlayPreview->setMinimumSize(196, 140);
    m_overlayPreview->setStyleSheet("background-color: #ECEFF1; border-radius: 8px; color: #607D8B;");
    m_overlayPreview->setAlignment(Qt::AlignCenter);
    overlayLayout->addWidget(m_overlayPreview);

    m_overlayProgress = new QProgressBar;
    m_overlayProgress->setStyleSheet("QProgressBar { height: 12px; border-radius: 6px; background-color: #E0E0E0; } QProgressBar::chunk { background-color: #4CAF50; border-radius: 6px; }");
    overlayLayout->addWidget(m_overlayProgress);

    m_overlayStopBtn = new QPushButton("紧急停止");
    m_overlayStopBtn->setMinimumHeight(40);
    m_overlayStopBtn->setStyleSheet("background-color: #F44336; color: white; border: none; border-radius: 6px;");
    connect(m_overlayStopBtn, &QPushButton::clicked, this, &MainWindow::onStopDrawing);
    connect(m_overlayStopBtn, &QPushButton::clicked, this, [] { QCoreApplication::quit(); });
    overlayLayout->addWidget(m_overlayStopBtn);
}

void MainWindow::updatePreview() {
    if (m_originalPixmap.isNull()) {
        if (m_textInput->text().isEmpty()) return;

        QPixmap pixmap(m_widthInput->text().toInt(), m_heightInput->text().toInt());
        pixmap.fill(Qt::white);

        QPainter painter(&pixmap);
        painter.setFont(m_font);
        painter.setPen(m_color);

        QString text = m_textInput->text();
        QRectF rect(0, 0, pixmap.width(), pixmap.height());
        painter.drawText(rect, Qt::AlignCenter, text);
        painter.end();

        m_currentPixmap = pixmap;
        m_originalPixmap = pixmap;
    }

    if (m_preview) {
        QPixmap scaled = m_currentPixmap.scaled(560, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_preview->setPixmap(scaled);
    }
    if (m_overlayPreview) {
        QPixmap scaled = m_currentPixmap.scaled(196, 140, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        m_overlayPreview->setPixmap(scaled);
    }
}

void MainWindow::log(const QString& msg) {
    if (!m_log) return;
    QString time = QTime::currentTime().toString("HH:mm:ss");
    m_log->append(QString("[%1] %2").arg(time, msg));
}

void MainWindow::updateScreenInfo() {
    QList<QScreen*> screens = QGuiApplication::screens();
    if (!screens.isEmpty()) {
        QScreen* primary = screens.first();
        QRect geom = primary->geometry();
        if (m_widthInput) m_widthInput->setText(QString::number(geom.width()));
        if (m_heightInput) m_heightInput->setText(QString::number(geom.height()));
        if (m_screenInfoLabel) m_screenInfoLabel->setText(QString("检测到 %1 个屏幕，主屏幕: %2x%3")
            .arg(screens.size()).arg(geom.width()).arg(geom.height()));
    }
}

void MainWindow::onInputTextChanged() {
    m_originalPixmap = QPixmap();
    if (!m_textInput->text().isEmpty() && !m_color.isValid()) {
        m_color = Qt::red;
    }
    updatePreview();
}

void MainWindow::onSelectFont() {
    bool ok;
    QFont font = QFontDialog::getFont(&ok, m_font, this);
    if (ok) {
        m_font = font;
        updatePreview();
    }
}

void MainWindow::onPickColor() {
    QColor color = QColorDialog::getColor(m_color, this);
    if (color.isValid()) {
        m_color = color;
    }
}

void MainWindow::setColorFromPalette(int index) {
    QList<QColor> paletteColors = {
        QColor("#FF0000"),
        QColor("#00FF00"),
        QColor("#0000FF"),
        QColor("#FFFF00"),
        QColor("#000000")
    };
    if (index >= 0 && index < paletteColors.count()) {
        m_color = paletteColors[index];
        if (m_colorLabel) {
            QString style = QString("background-color: %1;").arg(m_color.name());
            m_colorLabel->setStyleSheet(style);
        }
        log(QString("选择颜色: %1").arg(m_color.name()));
        if (!m_currentPixmap.isNull()) {
            applyMultiColorFilter(m_color);
        }
    }
}

void MainWindow::applyMultiColorFilter(const QColor& baseColor) {
    if (m_currentPixmap.isNull()) return;

    QImage img = m_currentPixmap.toImage();
    if (img.format() != QImage::Format_ARGB32) {
        img = img.convertToFormat(QImage::Format_ARGB32);
    }

    QList<QColor> palette;
    palette.append(baseColor);
    palette.append(QColor(qMin(255, baseColor.red() + 80), qMin(255, baseColor.green() + 80), qMin(255, baseColor.blue() + 80)));
    palette.append(QColor(qMax(0, baseColor.red() - 60), qMax(0, baseColor.green() - 60), qMax(0, baseColor.blue() - 60)));
    palette.append(QColor(baseColor.red(), baseColor.green(), baseColor.blue()));

    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            QRgb pixel = img.pixel(x, y);
            int gray = qGray(pixel);
            if (gray < 128) {
                int colorIndex = (gray * palette.count()) / 128;
                colorIndex = qMin(colorIndex, palette.count() - 1);
                QColor c = palette[colorIndex];
                img.setPixel(x, y, qRgba(c.red(), c.green(), c.blue(), qAlpha(pixel)));
            }
        }
    }

    m_currentPixmap = QPixmap::fromImage(img);
    updatePreview();
}

QPixmap MainWindow::extractColorChannel(const QPixmap& source, const QString& colorType, int threshold) {
    if (source.isNull()) return source;

    QImage img = source.toImage();
    QImage result(img.width(), img.height(), QImage::Format_Grayscale8);

    int r, g, b;
    for (int y = 0; y < img.height(); y++) {
        for (int x = 0; x < img.width(); x++) {
            QRgb pixel = img.pixel(x, y);
            r = qRed(pixel);
            g = qGreen(pixel);
            b = qBlue(pixel);
            int gray = 0;

            if (colorType == "red") {
                gray = r;
            } else if (colorType == "green") {
                gray = g;
            } else if (colorType == "blue") {
                gray = b;
            } else if (colorType == "yellow") {
                gray = qMin(255, r + g);
            } else if (colorType == "cyan") {
                gray = qMin(255, g + b);
            } else if (colorType == "magenta") {
                gray = qMin(255, r + b);
            } else if (colorType == "white") {
                gray = qMin(255, r + g + b);
            } else if (colorType == "dark") {
                gray = 255 - qMin(255, r + g + b);
            } else {
                gray = qGray(pixel);
            }

            result.setPixel(x, y, gray > threshold ? 255 : 0);
        }
    }

    return QPixmap::fromImage(result);
}

void MainWindow::setDrawColor(const QString& colorType) {
    if (m_originalPixmap.isNull()) return;

    int threshold = m_thresholdBox ? m_thresholdBox->value() : 128;
    m_currentPixmap = extractColorChannel(m_originalPixmap, colorType, threshold);

    QString colorName = colorType;
    colorName[0] = colorName[0].toUpper();
    log(QString("提取颜色: %1, 阈值: %2").arg(colorName).arg(threshold));

    updatePreview();
}

void MainWindow::onSelectImage() {
    QString fileName = QFileDialog::getOpenFileName(this, "选择图片", "", "Images (*.png *.jpg *.jpeg *.bmp)");
    if (!fileName.isEmpty()) {
        QPixmap pixmap(fileName);
        if (!pixmap.isNull()) {
            m_originalPixmap = pixmap;
            m_currentPixmap = pixmap;
            updatePreview();
            log(QString("已加载图片: %1").arg(QFileInfo(fileName).fileName()));
        }
    }
}

void MainWindow::onMouseMethodChanged(int idx) {
    m_drawer->setMouseMethod((MouseMethod)idx);
}

void MainWindow::onSelectPenZone() {
    hide();
    ZoneSelector* selector = new ZoneSelector("选择笔图标区域", this, [this](const QRect& zone) {
        setPenZone(zone);
        show();
    });
    selector->show();
}

void MainWindow::onSelectColorZone() {
    hide();
    ZoneSelector* selector = new ZoneSelector("选择调色板区域", this, [this](const QRect& zone) {
        setColorZone(zone);
        show();
    });
    selector->show();
}

void MainWindow::onSelectDrawZone() {
    hide();
    ZoneSelector* selector = new ZoneSelector("选择书写区域", this, [this](const QRect& zone) {
        setDrawZone(zone);
        show();
    });
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

void MainWindow::setDrawZone(const QRect& zone) {
    m_drawer->setDrawZone(zone);
    m_offsetXBox->setValue(zone.x());
    m_offsetYBox->setValue(zone.y());
    m_widthInput->setText(QString::number(zone.width()));
    m_heightInput->setText(QString::number(zone.height()));
    log(QString("书写区域: %1,%2 %3x%4").arg(zone.x()).arg(zone.y()).arg(zone.width()).arg(zone.height()));
}

void MainWindow::onStartDrawing() {
    if (m_currentPixmap.isNull()) {
        QMessageBox::warning(this, "提示", "请先输入文字或选择图片");
        return;
    }

    int delay = m_startDelayBox ? m_startDelayBox->value() : 3;
    if (delay > 0) {
        m_countdownRemaining = delay;
        if (m_overlayPanel) m_overlayPanel->show();
        m_countdownTimer->start(1000);
        m_drawBtn->setEnabled(false);
    } else {
        startDrawingNow();
    }
}

void MainWindow::startDrawingNow() {
    m_stopFlag = false;
    m_isDrawing = true;
    m_drawBtn->setEnabled(false);
    if (m_stopBtn) m_stopBtn->setEnabled(true);
    if (m_overlayPanel) m_overlayPanel->show();

    QCheckBox* colorDrawCheck = findChild<QCheckBox*>("colorDrawCheck");

    if (colorDrawCheck && colorDrawCheck->isChecked()) {
        QStringList colors;
        colors << "red" << "green" << "blue" << "yellow" << "cyan" << "magenta" << "white" << "dark";

        bool ok;
        QString selectedColor = QInputDialog::getItem(this, "选择颜色", "请选择要绘制的颜色:", colors, 0, false, &ok);

        if (!ok || selectedColor.isEmpty()) {
            log("取消绘制");
            m_drawBtn->setEnabled(true);
            return;
        }

        m_currentDrawColor = selectedColor;
        setDrawColor(selectedColor);

        QString colorName = selectedColor;
        colorName[0] = colorName[0].toUpper();
        log(QString("开始绘制: %1").arg(colorName));

        QMessageBox::information(this, "提示", QString("请在希沃选择%1画笔\n选择好后开始绘制").arg(colorName));
    } else {
        m_currentPixmap = m_originalPixmap;
    }

    int width = m_widthInput->text().toInt();
    int height = m_heightInput->text().toInt();
    int offsetX = m_offsetXBox ? m_offsetXBox->value() : 0;
    int offsetY = m_offsetYBox ? m_offsetYBox->value() : 0;
    int threshold = m_thresholdBox ? m_thresholdBox->value() : 128;
    int moveDelay = m_moveDelayBox ? m_moveDelayBox->value() : 1;
    int mouseMethod = m_mouseMethodCombo ? m_mouseMethodCombo->currentIndex() : 0;

    m_drawer->setMouseMethod((MouseMethod)mouseMethod);
    m_drawer->setStopFlag(&m_stopFlag);

    QPoint startPos = QCursor::pos();
    m_fixedPoint = startPos;

    if (m_mouseDetectionCheck && m_mouseDetectionCheck->isChecked()) {
        m_mouseHook->start();
    }

    m_drawThread = new DrawingThread(m_drawer, m_currentPixmap, QPoint(offsetX, offsetY), this);
    connect(m_drawThread, &QThread::finished, this, &MainWindow::onDrawingFinished);
    m_drawThread->start();

    log(QString("开始书写: %1x%2, 偏移(%3,%4), 阈值%5")
        .arg(width).arg(height).arg(offsetX).arg(offsetY).arg(threshold));
}

void MainWindow::onStopDrawing() {
    m_stopFlag = true;
    m_countdownTimer->stop();
    if (m_drawBtn) m_drawBtn->setEnabled(true);
    if (m_stopBtn) m_stopBtn->setEnabled(false);
    if (m_overlayPanel) m_overlayPanel->hide();
    if (m_mouseHook) m_mouseHook->stop();
    if (m_detectionStatusLabel) m_detectionStatusLabel->setText("已开启");
    log("已停止");
    if (m_drawThread && m_drawThread->isRunning()) {
        m_drawThread->terminate();
        m_drawThread->wait();
    }
    onDrawingFinished();
}

void MainWindow::onDrawingFinished() {
    m_isDrawing = false;
    if (m_drawBtn) m_drawBtn->setEnabled(true);
    if (m_stopBtn) m_stopBtn->setEnabled(false);
    if (m_overlayPanel) m_overlayPanel->hide();
    show();
    log("绘制完成");
}

void MainWindow::onCursorUpdate() {
    QPoint pos = QCursor::pos();
    if (m_cursorPosLabel) m_cursorPosLabel->setText(QString("鼠标位置: (%1, %2)").arg(pos.x()).arg(pos.y()));
}

void MainWindow::onFontSizeChanged(int val) {
    m_fontSize = val;
    m_font.setPointSize(val);
    updatePreview();
}

void MainWindow::onScaleChanged(int val) {
    m_scalePercent = val;
    if (m_scaleLabel) m_scaleLabel->setText(QString("%1%").arg(val));
    if (!m_originalPixmap.isNull()) {
        m_currentPixmap = m_originalPixmap.scaled(m_originalPixmap.width() * val / 100.0,
                                              m_originalPixmap.height() * val / 100.0,
                                              Qt::KeepAspectRatio, Qt::SmoothTransformation);
        updatePreview();
    }
}

void MainWindow::onCountdownTick() {
    m_countdownRemaining--;
    if (m_countdownRemaining > 0) {
        if (m_overlayPreview) m_overlayPreview->setText(QString::number(m_countdownRemaining));
    } else {
        m_countdownTimer->stop();
        if (m_overlayPreview) m_overlayPreview->clear();
        startDrawingNow();
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        if (m_isDrawing) {
            m_stopFlag = true;
            log("按下Escape，已停止!");
        }
    }
    QMainWindow::keyPressEvent(event);
}

void MainWindow::onAutoColorToggled(bool checked) {
    if (m_drawer) {
        m_drawer->enableAutoColor(checked);
    }
}

void MainWindow::onColorDrawToggled(bool checked) {
    if (m_drawer) {
        m_drawer->enableColorDraw(checked);
    }
}

void MainWindow::applyStyle() {
    QString style = R"(
        QWidget {
            background-color: #1e1e2e;
            color: #cdd6f4;
            font-family: "Microsoft YaHei", "Segoe UI", sans-serif;
            font-size: 10pt;
        }
        QMainWindow {
            background-color: #1e1e2e;
        }
        QLabel {
            color: #cdd6f4;
            background-color: transparent;
        }
        QPushButton {
            background-color: #45475a;
            color: #cdd6f4;
            border: 1px solid #585b70;
            border-radius: 6px;
            padding: 8px 16px;
            min-width: 80px;
        }
        QPushButton:hover {
            background-color: #585b70;
            border-color: #89b4fa;
        }
        QPushButton:pressed {
            background-color: #313244;
        }
        QPushButton:disabled {
            background-color: #313244;
            color: #6c7086;
        }
        QLineEdit, QSpinBox, QComboBox {
            background-color: #313244;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 4px;
            padding: 6px;
            selection-background-color: #89b4fa;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border-color: #89b4fa;
        }
        QCheckBox {
            color: #cdd6f4;
            spacing: 8px;
        }
        QCheckBox::indicator {
            width: 18px;
            height: 18px;
            border-radius: 4px;
            border: 2px solid #45475a;
            background-color: #313244;
        }
        QCheckBox::indicator:checked {
            background-color: #89b4fa;
            border-color: #89b4fa;
        }
        QCheckBox::indicator:hover {
            border-color: #89b4fa;
        }
        QSlider::groove:horizontal {
            background-color: #45475a;
            height: 6px;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            background-color: #89b4fa;
            width: 16px;
            margin: -5px 0;
            border-radius: 8px;
        }
        QTabWidget::pane {
            border: 1px solid #45475a;
            border-radius: 8px;
            background-color: #181825;
        }
        QTabBar::tab {
            background-color: #313244;
            color: #a6adc8;
            padding: 10px 20px;
            border-top-left-radius: 6px;
            border-top-right-radius: 6px;
            margin-right: 2px;
        }
        QTabBar::tab:selected {
            background-color: #45475a;
            color: #cdd6f4;
        }
        QTabBar::tab:hover {
            background-color: #45475a;
            color: #cdd6f4;
        }
        QScrollArea, QTextEdit, QListWidget {
            background-color: #181825;
            color: #cdd6f4;
            border: 1px solid #45475a;
            border-radius: 6px;
        }
        QProgressBar {
            background-color: #45475a;
            border-radius: 4px;
            text-align: center;
            color: #cdd6f4;
        }
        QProgressBar::chunk {
            background-color: #89b4fa;
            border-radius: 4px;
        }
    )";

    setStyleSheet(style);

    QFont defaultFont("Microsoft YaHei", 10);
    setFont(defaultFont);

    QList<QSpinBox*> spinBoxes = findChildren<QSpinBox*>();
    for (QSpinBox* sb : spinBoxes) {
        sb->setStyleSheet(R"(
            QSpinBox {
                background-color: #313244;
                color: #cdd6f4;
                border: 1px solid #45475a;
                border-radius: 4px;
                padding: 4px;
            }
            QSpinBox:focus {
                border-color: #89b4fa;
            }
            QSpinBox::up-button, QSpinBox::down-button {
                background-color: #45475a;
                border-radius: 2px;
            }
            QSpinBox::up-button:hover, QSpinBox::down-button:hover {
                background-color: #585b70;
            }
        )");
    }

    QList<QComboBox*> combos = findChildren<QComboBox*>();
    for (QComboBox* cb : combos) {
        cb->setStyleSheet(R"(
            QComboBox {
                background-color: #313244;
                color: #cdd6f4;
                border: 1px solid #45475a;
                border-radius: 4px;
                padding: 6px;
            }
            QComboBox:focus {
                border-color: #89b4fa;
            }
            QComboBox::drop-down {
                border: none;
                width: 24px;
            }
            QComboBox QAbstractItemView {
                background-color: #313244;
                color: #cdd6f4;
                border: 1px solid #45475a;
                selection-background-color: #45475a;
            }
        )");
    }

    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    for (QPushButton* btn : buttons) {
        QString name = btn->objectName();
        if (name == "drawBtn") {
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: #313244;
                    color: #f5e0dc;
                    border: 2px solid #f5c2e7;
                    border-radius: 8px;
                    padding: 12px 24px;
                    font-size: 12pt;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #45475a;
                    border-color: #f5c2e7;
                }
            )");
        } else if (name == "stopBtn") {
            btn->setStyleSheet(R"(
                QPushButton {
                    background-color: #313244;
                    color: #f38ba8;
                    border: 2px solid #f38ba8;
                    border-radius: 8px;
                    padding: 12px 24px;
                    font-size: 12pt;
                    font-weight: bold;
                }
                QPushButton:hover {
                    background-color: #45475a;
                }
            )");
        }
    }

    log("已应用美化样式");
}

QString MainWindow::getColorName(const QColor& color) {
    int r = color.red();
    int g = color.green();
    int b = color.blue();

    if (r > 200 && g < 50 && b < 50) return "红色";
    if (g > 200 && r < 50 && b < 50) return "绿色";
    if (b > 200 && r < 50 && g < 50) return "蓝色";
    if (r > 200 && g > 200 && b < 50) return "黄色";
    if (r > 200 && g < 50 && b > 200) return "紫色";
    if (r > 200 && g > 100 && b < 50) return "橙色";
    if (r > 150 && g > 150 && b > 150) return "白色";
    if (r < 50 && g < 50 && b < 50) return "黑色";
    if (r > 100 && g > 100 && b > 200) return "浅蓝色";
    if (r > 200 && g > 100 && b > 150) return "粉色";
    if (r > 100 && g > 200 && b > 100) return "浅绿色";

    return QString("#%1%2%3")
        .arg(r, 2, 16, QChar('0'))
        .arg(g, 2, 16, QChar('0'))
        .arg(b, 2, 16, QChar('0')).toUpper();
}