/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.10.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSlider>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QTabWidget *tabWidget;
    QWidget *tabInput;
    QVBoxLayout *verticalLayout_tab1;
    QLineEdit *textInput;
    QHBoxLayout *horizontalLayout_img;
    QPushButton *selectImageBtn;
    QPushButton *selectFontBtn;
    QPushButton *pickColorBtn;
    QHBoxLayout *horizontalLayout_size;
    QLabel *label;
    QLineEdit *widthInput;
    QLabel *label1;
    QLineEdit *heightInput;
    QLabel *label2;
    QSpinBox *fontSizeBox;
    QGroupBox *groupPreview;
    QVBoxLayout *verticalLayout_preview;
    QLabel *preview;
    QWidget *tabSettings;
    QVBoxLayout *verticalLayout_tab2;
    QGridLayout *gridLayout_settings;
    QLabel *label3;
    QSpinBox *thresholdBox;
    QLabel *label4;
    QSpinBox *offsetXBox;
    QLabel *label5;
    QSpinBox *offsetYBox;
    QLabel *label6;
    QSlider *scaleSlider;
    QHBoxLayout *horizontalLayout_method;
    QLabel *label7;
    QComboBox *mouseMethodCombo;
    QHBoxLayout *horizontalLayout_delay;
    QLabel *label8;
    QSpinBox *moveDelayBox;
    QLabel *label9;
    QSpinBox *startDelayBox;
    QHBoxLayout *horizontalLayout_check;
    QCheckBox *mouseDetectionCheck;
    QCheckBox *autoColorSelectCheck;
    QCheckBox *colorDrawCheck;
    QHBoxLayout *horizontalLayout_zones;
    QPushButton *selectPenZoneBtn;
    QPushButton *selectColorZoneBtn;
    QPushButton *selectDrawZoneBtn;
    QWidget *tabControl;
    QVBoxLayout *verticalLayout_tab3;
    QHBoxLayout *horizontalLayout_control;
    QPushButton *drawBtn;
    QPushButton *stopBtn;
    QProgressBar *progress;
    QTextEdit *log;
    QHBoxLayout *horizontalLayout_status;
    QLabel *cursorPosLabel;
    QLabel *screenInfoLabel;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(600, 750);
        tabWidget = new QTabWidget(MainWindow);
        tabWidget->setObjectName("tabWidget");
        tabInput = new QWidget();
        tabInput->setObjectName("tabInput");
        verticalLayout_tab1 = new QVBoxLayout(tabInput);
        verticalLayout_tab1->setObjectName("verticalLayout_tab1");
        textInput = new QLineEdit(tabInput);
        textInput->setObjectName("textInput");

        verticalLayout_tab1->addWidget(textInput);

        horizontalLayout_img = new QHBoxLayout();
        horizontalLayout_img->setObjectName("horizontalLayout_img");
        selectImageBtn = new QPushButton(tabInput);
        selectImageBtn->setObjectName("selectImageBtn");

        horizontalLayout_img->addWidget(selectImageBtn);

        selectFontBtn = new QPushButton(tabInput);
        selectFontBtn->setObjectName("selectFontBtn");

        horizontalLayout_img->addWidget(selectFontBtn);

        pickColorBtn = new QPushButton(tabInput);
        pickColorBtn->setObjectName("pickColorBtn");

        horizontalLayout_img->addWidget(pickColorBtn);


        verticalLayout_tab1->addLayout(horizontalLayout_img);

        horizontalLayout_size = new QHBoxLayout();
        horizontalLayout_size->setObjectName("horizontalLayout_size");
        label = new QLabel(tabInput);
        label->setObjectName("label");

        horizontalLayout_size->addWidget(label);

        widthInput = new QLineEdit(tabInput);
        widthInput->setObjectName("widthInput");

        horizontalLayout_size->addWidget(widthInput);

        label1 = new QLabel(tabInput);
        label1->setObjectName("label1");

        horizontalLayout_size->addWidget(label1);

        heightInput = new QLineEdit(tabInput);
        heightInput->setObjectName("heightInput");

        horizontalLayout_size->addWidget(heightInput);

        label2 = new QLabel(tabInput);
        label2->setObjectName("label2");

        horizontalLayout_size->addWidget(label2);

        fontSizeBox = new QSpinBox(tabInput);
        fontSizeBox->setObjectName("fontSizeBox");
        fontSizeBox->setValue(48);

        horizontalLayout_size->addWidget(fontSizeBox);


        verticalLayout_tab1->addLayout(horizontalLayout_size);

        groupPreview = new QGroupBox(tabInput);
        groupPreview->setObjectName("groupPreview");
        verticalLayout_preview = new QVBoxLayout(groupPreview);
        verticalLayout_preview->setObjectName("verticalLayout_preview");
        preview = new QLabel(groupPreview);
        preview->setObjectName("preview");
        preview->setMinimumSize(QSize(0, 150));
        preview->setStyleSheet(QString::fromUtf8("border: 1px solid gray; background: white;"));
        preview->setAlignment(Qt::AlignCenter);

        verticalLayout_preview->addWidget(preview);


        verticalLayout_tab1->addWidget(groupPreview);

        tabWidget->addTab(tabInput, QString());
        tabSettings = new QWidget();
        tabSettings->setObjectName("tabSettings");
        verticalLayout_tab2 = new QVBoxLayout(tabSettings);
        verticalLayout_tab2->setObjectName("verticalLayout_tab2");
        gridLayout_settings = new QGridLayout();
        gridLayout_settings->setObjectName("gridLayout_settings");
        label3 = new QLabel(tabSettings);
        label3->setObjectName("label3");

        gridLayout_settings->addWidget(label3, 0, 0, 1, 1);

        thresholdBox = new QSpinBox(tabSettings);
        thresholdBox->setObjectName("thresholdBox");
        thresholdBox->setMaximum(255);
        thresholdBox->setValue(128);

        gridLayout_settings->addWidget(thresholdBox, 0, 1, 1, 1);

        label4 = new QLabel(tabSettings);
        label4->setObjectName("label4");

        gridLayout_settings->addWidget(label4, 0, 2, 1, 1);

        offsetXBox = new QSpinBox(tabSettings);
        offsetXBox->setObjectName("offsetXBox");
        offsetXBox->setMinimum(-5000);
        offsetXBox->setMaximum(5000);

        gridLayout_settings->addWidget(offsetXBox, 0, 3, 1, 1);

        label5 = new QLabel(tabSettings);
        label5->setObjectName("label5");

        gridLayout_settings->addWidget(label5, 1, 0, 1, 1);

        offsetYBox = new QSpinBox(tabSettings);
        offsetYBox->setObjectName("offsetYBox");
        offsetYBox->setMinimum(-5000);
        offsetYBox->setMaximum(5000);

        gridLayout_settings->addWidget(offsetYBox, 1, 1, 1, 1);

        label6 = new QLabel(tabSettings);
        label6->setObjectName("label6");

        gridLayout_settings->addWidget(label6, 1, 2, 1, 1);

        scaleSlider = new QSlider(tabSettings);
        scaleSlider->setObjectName("scaleSlider");
        scaleSlider->setMaximum(200);
        scaleSlider->setMinimum(50);
        scaleSlider->setValue(100);
        scaleSlider->setOrientation(Qt::Horizontal);

        gridLayout_settings->addWidget(scaleSlider, 1, 3, 1, 1);


        verticalLayout_tab2->addLayout(gridLayout_settings);

        horizontalLayout_method = new QHBoxLayout();
        horizontalLayout_method->setObjectName("horizontalLayout_method");
        label7 = new QLabel(tabSettings);
        label7->setObjectName("label7");

        horizontalLayout_method->addWidget(label7);

        mouseMethodCombo = new QComboBox(tabSettings);
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->addItem(QString());
        mouseMethodCombo->setObjectName("mouseMethodCombo");

        horizontalLayout_method->addWidget(mouseMethodCombo);


        verticalLayout_tab2->addLayout(horizontalLayout_method);

        horizontalLayout_delay = new QHBoxLayout();
        horizontalLayout_delay->setObjectName("horizontalLayout_delay");
        label8 = new QLabel(tabSettings);
        label8->setObjectName("label8");

        horizontalLayout_delay->addWidget(label8);

        moveDelayBox = new QSpinBox(tabSettings);
        moveDelayBox->setObjectName("moveDelayBox");
        moveDelayBox->setMaximum(1000);
        moveDelayBox->setValue(5);

        horizontalLayout_delay->addWidget(moveDelayBox);

        label9 = new QLabel(tabSettings);
        label9->setObjectName("label9");

        horizontalLayout_delay->addWidget(label9);

        startDelayBox = new QSpinBox(tabSettings);
        startDelayBox->setObjectName("startDelayBox");
        startDelayBox->setMaximum(60);
        startDelayBox->setValue(3);

        horizontalLayout_delay->addWidget(startDelayBox);


        verticalLayout_tab2->addLayout(horizontalLayout_delay);

        horizontalLayout_check = new QHBoxLayout();
        horizontalLayout_check->setObjectName("horizontalLayout_check");
        mouseDetectionCheck = new QCheckBox(tabSettings);
        mouseDetectionCheck->setObjectName("mouseDetectionCheck");
        mouseDetectionCheck->setChecked(true);

        horizontalLayout_check->addWidget(mouseDetectionCheck);

        autoColorSelectCheck = new QCheckBox(tabSettings);
        autoColorSelectCheck->setObjectName("autoColorSelectCheck");

        horizontalLayout_check->addWidget(autoColorSelectCheck);

        colorDrawCheck = new QCheckBox(tabSettings);
        colorDrawCheck->setObjectName("colorDrawCheck");
        colorDrawCheck->setChecked(true);

        horizontalLayout_check->addWidget(colorDrawCheck);


        verticalLayout_tab2->addLayout(horizontalLayout_check);

        horizontalLayout_zones = new QHBoxLayout();
        horizontalLayout_zones->setObjectName("horizontalLayout_zones");
        selectPenZoneBtn = new QPushButton(tabSettings);
        selectPenZoneBtn->setObjectName("selectPenZoneBtn");
        selectPenZoneBtn->setMaximumSize(QSize(100, 16777215));

        horizontalLayout_zones->addWidget(selectPenZoneBtn);

        selectColorZoneBtn = new QPushButton(tabSettings);
        selectColorZoneBtn->setObjectName("selectColorZoneBtn");
        selectColorZoneBtn->setMaximumSize(QSize(100, 16777215));

        horizontalLayout_zones->addWidget(selectColorZoneBtn);

        selectDrawZoneBtn = new QPushButton(tabSettings);
        selectDrawZoneBtn->setObjectName("selectDrawZoneBtn");
        selectDrawZoneBtn->setMaximumSize(QSize(100, 16777215));

        horizontalLayout_zones->addWidget(selectDrawZoneBtn);


        verticalLayout_tab2->addLayout(horizontalLayout_zones);

        tabWidget->addTab(tabSettings, QString());
        tabControl = new QWidget();
        tabControl->setObjectName("tabControl");
        verticalLayout_tab3 = new QVBoxLayout(tabControl);
        verticalLayout_tab3->setObjectName("verticalLayout_tab3");
        horizontalLayout_control = new QHBoxLayout();
        horizontalLayout_control->setObjectName("horizontalLayout_control");
        drawBtn = new QPushButton(tabControl);
        drawBtn->setObjectName("drawBtn");
        drawBtn->setMinimumSize(QSize(0, 40));

        horizontalLayout_control->addWidget(drawBtn);

        stopBtn = new QPushButton(tabControl);
        stopBtn->setObjectName("stopBtn");
        stopBtn->setEnabled(false);
        stopBtn->setMinimumSize(QSize(0, 40));

        horizontalLayout_control->addWidget(stopBtn);


        verticalLayout_tab3->addLayout(horizontalLayout_control);

        progress = new QProgressBar(tabControl);
        progress->setObjectName("progress");
        progress->setMaximum(100);
        progress->setValue(0);

        verticalLayout_tab3->addWidget(progress);

        log = new QTextEdit(tabControl);
        log->setObjectName("log");
        log->setMaximumSize(QSize(16777215, 100));
        log->setReadOnly(true);

        verticalLayout_tab3->addWidget(log);

        horizontalLayout_status = new QHBoxLayout();
        horizontalLayout_status->setObjectName("horizontalLayout_status");
        cursorPosLabel = new QLabel(tabControl);
        cursorPosLabel->setObjectName("cursorPosLabel");

        horizontalLayout_status->addWidget(cursorPosLabel);

        screenInfoLabel = new QLabel(tabControl);
        screenInfoLabel->setObjectName("screenInfoLabel");

        horizontalLayout_status->addWidget(screenInfoLabel);


        verticalLayout_tab3->addLayout(horizontalLayout_status);

        tabWidget->addTab(tabControl, QString());
        MainWindow->setCentralWidget(tabWidget);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "MouseWriter - \351\274\240\346\240\207\345\206\231\345\255\227", nullptr));
        textInput->setPlaceholderText(QCoreApplication::translate("MainWindow", "\350\276\223\345\205\245\350\246\201\344\271\246\345\206\231\347\232\204\346\226\207\345\255\227...", nullptr));
        selectImageBtn->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\345\233\276\347\211\207", nullptr));
        selectFontBtn->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\345\255\227\344\275\223", nullptr));
        pickColorBtn->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\351\242\234\350\211\262", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "\345\256\275\345\272\246:", nullptr));
        widthInput->setText(QCoreApplication::translate("MainWindow", "800", nullptr));
        label1->setText(QCoreApplication::translate("MainWindow", "\351\253\230\345\272\246:", nullptr));
        heightInput->setText(QCoreApplication::translate("MainWindow", "300", nullptr));
        label2->setText(QCoreApplication::translate("MainWindow", "\345\255\227\345\217\267:", nullptr));
        groupPreview->setTitle(QCoreApplication::translate("MainWindow", "\351\242\204\350\247\210", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabInput), QCoreApplication::translate("MainWindow", "Page", nullptr));
        label3->setText(QCoreApplication::translate("MainWindow", "\351\230\210\345\200\274:", nullptr));
        label4->setText(QCoreApplication::translate("MainWindow", "\345\201\217\347\247\273X:", nullptr));
        label5->setText(QCoreApplication::translate("MainWindow", "\345\201\217\347\247\273Y:", nullptr));
        label6->setText(QCoreApplication::translate("MainWindow", "\347\274\251\346\224\276(%):", nullptr));
        label7->setText(QCoreApplication::translate("MainWindow", "\347\247\273\345\212\250\346\226\271\345\274\217:", nullptr));
        mouseMethodCombo->setItemText(0, QCoreApplication::translate("MainWindow", "SendInput (\351\273\230\350\256\244)", nullptr));
        mouseMethodCombo->setItemText(1, QCoreApplication::translate("MainWindow", "mouse_event", nullptr));
        mouseMethodCombo->setItemText(2, QCoreApplication::translate("MainWindow", "SetCursorPos", nullptr));
        mouseMethodCombo->setItemText(3, QCoreApplication::translate("MainWindow", "SendMessage", nullptr));
        mouseMethodCombo->setItemText(4, QCoreApplication::translate("MainWindow", "PostMessage", nullptr));
        mouseMethodCombo->setItemText(5, QCoreApplication::translate("MainWindow", "\350\247\246\346\216\247-\345\215\225\347\202\271", nullptr));
        mouseMethodCombo->setItemText(6, QCoreApplication::translate("MainWindow", "\350\247\246\346\216\247-\345\244\232\347\202\271", nullptr));
        mouseMethodCombo->setItemText(7, QCoreApplication::translate("MainWindow", "InjectTouch", nullptr));
        mouseMethodCombo->setItemText(8, QCoreApplication::translate("MainWindow", "InjectTouchMulti", nullptr));
        mouseMethodCombo->setItemText(9, QCoreApplication::translate("MainWindow", "Pointer-\346\263\250\345\205\245", nullptr));
        mouseMethodCombo->setItemText(10, QCoreApplication::translate("MainWindow", "RealTouch\346\263\250\345\205\245", nullptr));
        mouseMethodCombo->setItemText(11, QCoreApplication::translate("MainWindow", "Hook\346\263\250\345\205\245(\345\267\246\351\224\256+RSHIFT, \345\217\214\346\214\207:RCTRL)", nullptr));

        label8->setText(QCoreApplication::translate("MainWindow", "\347\247\273\345\212\250\345\273\266\350\277\237(ms):", nullptr));
        label9->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\345\273\266\350\277\237(s):", nullptr));
        mouseDetectionCheck->setText(QCoreApplication::translate("MainWindow", "\346\243\200\346\265\213\347\247\273\345\212\250\350\207\252\345\212\250\345\201\234\346\255\242", nullptr));
        autoColorSelectCheck->setText(QCoreApplication::translate("MainWindow", "\350\207\252\345\212\250\347\202\271\345\207\273\345\270\214\346\262\203\351\200\211\350\211\262", nullptr));
        colorDrawCheck->setText(QCoreApplication::translate("MainWindow", "\347\273\230\345\210\266\345\275\251\350\211\262", nullptr));
        selectPenZoneBtn->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\347\254\224\345\233\276\346\240\207", nullptr));
        selectColorZoneBtn->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\350\260\203\350\211\262\346\235\277", nullptr));
        selectDrawZoneBtn->setText(QCoreApplication::translate("MainWindow", "\351\200\211\346\213\251\344\271\246\345\206\231\345\214\272\345\237\237", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabSettings), QCoreApplication::translate("MainWindow", "Page", nullptr));
        drawBtn->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\344\271\246\345\206\231", nullptr));
        stopBtn->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        cursorPosLabel->setText(QCoreApplication::translate("MainWindow", "\351\274\240\346\240\207: 0, 0", nullptr));
        screenInfoLabel->setText(QCoreApplication::translate("MainWindow", "\345\261\217\345\271\225: ", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tabControl), QCoreApplication::translate("MainWindow", "Page", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
