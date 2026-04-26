void MainWindow::applyStyle() {
    setStyleSheet(R"(
        QWidget { background-color: #2b2b2b; color: #ffffff; }
        QPushButton { background-color: #4a4a4a; color: #ffffff; border: 1px solid #666; }
        QLineEdit { background-color: #3a3a3a; color: #ffffff; border: 1px solid #555; }
        QSpinBox { background-color: #3a3a3a; color: #ffffff; border: 1px solid #555; }
        QComboBox { background-color: #3a3a3a; color: #ffffff; border: 1px solid #555; }
        QCheckBox { color: #ffffff; }
        QLabel { color: #ffffff; background-color: transparent; }
        QTabWidget::pane { border: 1px solid #555; background-color: #2b2b2b; }
        QTabBar::tab { background-color: #3a3a3a; color: #fff; padding: 5px; }
        QScrollArea, QTextEdit { background-color: #222; color: #fff; border: 1px solid #555; }
    )");
    setFont(QFont("Microsoft YaHei", 10));
}