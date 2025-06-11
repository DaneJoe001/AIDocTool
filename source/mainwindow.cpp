#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QRegularExpression>
#include <QDebug>
#include <QSplitter>
#include <QTabWidget>
#include <QTextStream>
#include <QStandardPaths>
#include <QApplication>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , directoryReader(new DirectoryTreeReader(this))
{
    setupUI();
    setupMenus();

    // 设置应用程序图标
    setWindowIcon(QIcon(":/icons/main_icon.ico"));

    // 连接信号和槽
    connect(browseButton, &QPushButton::clicked, this, &MainWindow::browseDirectory);
    connect(startButton, &QPushButton::clicked, this, &MainWindow::startReading);
    connect(cancelButton, &QPushButton::clicked, this, &MainWindow::cancelReading);
    connect(filterCheckBox, &QCheckBox::toggled, this, &MainWindow::toggleFilterOptions);
    connect(directoryTreeWidget, &QTreeWidget::itemSelectionChanged, this, &MainWindow::updateTextDisplay);
    
    // 连接目录读取器的信号
    connect(directoryReader, &DirectoryTreeReader::progressUpdated, this, &MainWindow::updateProgress);
    connect(directoryReader, &DirectoryTreeReader::readingFinished, this, &MainWindow::readingFinished);

    // 初始状态
    cancelButton->setEnabled(false);
    toggleFilterOptions(false);
    filterCheckBox->setChecked(false);
    wildcardRadioButton->setChecked(true);
    
    // 设置目录树读取器
    directoryReader->setTreeWidget(directoryTreeWidget);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    // 设置窗口标题和大小
    setWindowTitle("AI文档工具集");
    resize(1000, 700);

    // 创建堆叠部件作为中央部件
    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    // 创建目录读取页面
    directoryReaderPage = new QWidget(stackedWidget);
    stackedWidget->addWidget(directoryReaderPage);
    QVBoxLayout *mainLayout = new QVBoxLayout(directoryReaderPage);

    // 创建主分割器
    mainSplitter = new QSplitter(Qt::Horizontal, directoryReaderPage);
    mainSplitter->setChildrenCollapsible(false);

    // 左侧控件 - 目录树和选项
    QWidget *leftWidget = new QWidget(mainSplitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);
    
    // 目录选择区域
    QGroupBox *directoryGroupBox = new QGroupBox("目录选择", leftWidget);
    QHBoxLayout *directoryLayout = new QHBoxLayout(directoryGroupBox);
    
    directoryLineEdit = new QLineEdit(directoryGroupBox);
    browseButton = new QPushButton("浏览...", directoryGroupBox);
    
    directoryLayout->addWidget(directoryLineEdit);
    directoryLayout->addWidget(browseButton);
    
    // 选项区域
    QGroupBox *optionsGroupBox = new QGroupBox("读取选项", leftWidget);
    QGridLayout *optionsLayout = new QGridLayout(optionsGroupBox);
    
    QLabel *depthLabel = new QLabel("搜索深度:", optionsGroupBox);
    depthSpinBox = new QSpinBox(optionsGroupBox);
    depthSpinBox->setMinimum(1);
    depthSpinBox->setMaximum(999);
    depthSpinBox->setValue(3);
    
    filterCheckBox = new QCheckBox("启用文件过滤", optionsGroupBox);
    
    QWidget *filterOptionsWidget = new QWidget(optionsGroupBox);
    QHBoxLayout *filterOptionsLayout = new QHBoxLayout(filterOptionsWidget);
    filterOptionsLayout->setContentsMargins(0, 0, 0, 0);
    
    wildcardRadioButton = new QRadioButton("通配符", filterOptionsWidget);
    regexRadioButton = new QRadioButton("正则表达式", filterOptionsWidget);
    QButtonGroup *filterTypeGroup = new QButtonGroup(this);
    filterTypeGroup->addButton(wildcardRadioButton);
    filterTypeGroup->addButton(regexRadioButton);
    
    filterPatternLineEdit = new QLineEdit(filterOptionsWidget);
    filterPatternLineEdit->setPlaceholderText("输入过滤模式 (例如: *.txt 或 .*\\.txt)");
    
    filterOptionsLayout->addWidget(wildcardRadioButton);
    filterOptionsLayout->addWidget(regexRadioButton);
    filterOptionsLayout->addWidget(filterPatternLineEdit);
    
    readFilesCheckBox = new QCheckBox("读取文件名", optionsGroupBox);
    readFilesCheckBox->setChecked(true);
    
    // 添加过滤规则文本框
    QLabel *filterRulesLabel = new QLabel("过滤规则 (类似.gitignore):", optionsGroupBox);
    filterRulesTextEdit = new QPlainTextEdit(optionsGroupBox);
    filterRulesTextEdit->setPlaceholderText("每行一个规则\n例如:\n*.tmp\n*.log\nnode_modules/\n.git/");
    filterRulesTextEdit->setMaximumHeight(100);
    
    optionsLayout->addWidget(depthLabel, 0, 0);
    optionsLayout->addWidget(depthSpinBox, 0, 1);
    optionsLayout->addWidget(filterCheckBox, 1, 0);
    optionsLayout->addWidget(filterOptionsWidget, 1, 1);
    optionsLayout->addWidget(readFilesCheckBox, 2, 0);
    optionsLayout->addWidget(filterRulesLabel, 3, 0, 1, 2);
    optionsLayout->addWidget(filterRulesTextEdit, 4, 0, 1, 2);
    
    // 操作按钮区域
    QHBoxLayout *actionLayout = new QHBoxLayout();
    startButton = new QPushButton("开始读取", leftWidget);
    cancelButton = new QPushButton("取消", leftWidget);
    actionLayout->addWidget(startButton);
    actionLayout->addWidget(cancelButton);
    
    // 进度条和状态标签
    progressBar = new QProgressBar(leftWidget);
    progressBar->setVisible(false);
    statusLabel = new QLabel(leftWidget);
    statusLabel->setText("就绪");
    
    // 目录树显示区域
    directoryTreeWidget = new QTreeWidget(leftWidget);
    directoryTreeWidget->setHeaderLabels(QStringList() << "名称" << "类型" << "路径");
    directoryTreeWidget->setColumnWidth(0, 200);
    directoryTreeWidget->setColumnWidth(1, 80);
    directoryTreeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);
    directoryTreeWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    
    // 将所有组件添加到左侧布局
    leftLayout->addWidget(directoryGroupBox);
    leftLayout->addWidget(optionsGroupBox);
    leftLayout->addLayout(actionLayout);
    leftLayout->addWidget(progressBar);
    leftLayout->addWidget(statusLabel);
    leftLayout->addWidget(directoryTreeWidget);
    
    // 右侧控件 - 文本显示
    directoryTextDisplay = new QTextEdit(mainSplitter);
    directoryTextDisplay->setReadOnly(true);
    directoryTextDisplay->setFont(QFont("Courier New", 10));
    directoryTextDisplay->setPlaceholderText("目录结构将在这里以文本形式显示");
    
    // 添加左右两侧部件到分割器
    mainSplitter->addWidget(leftWidget);
    mainSplitter->addWidget(directoryTextDisplay);
    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);
    
    // 将分割器添加到主布局
    mainLayout->addWidget(mainSplitter);
    
    // 创建文件合并页面
    fileMergerPage = new FileMergerWidget(stackedWidget);
    stackedWidget->addWidget(fileMergerPage);
    
    // 默认显示第一页
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::setupMenus()
{
    // 创建菜单栏
    menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // 文件菜单
    fileMenu = menuBar->addMenu("文件");
    exportAction = fileMenu->addAction("导出为TXT文件", this, &MainWindow::exportToTxtFile);
    importFilterAction = fileMenu->addAction("导入过滤规则", this, &MainWindow::importFilterRules);
    fileMenu->addSeparator();
    fileMenu->addAction("退出", this, &QMainWindow::close);
    
    // 视图菜单
    viewMenu = menuBar->addMenu("视图");
    viewMenu->addAction("目录树读取器", [this](){ switchToPage(0); });
    viewMenu->addAction("文件合并工具", [this](){ switchToPage(1); });
}

void MainWindow::switchToPage(int index)
{
    stackedWidget->setCurrentIndex(index);
}

void MainWindow::browseDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择目录",
                                                   directoryLineEdit->text(),
                                                   QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!dir.isEmpty()) {
        directoryLineEdit->setText(dir);
    }
}

void MainWindow::startReading()
{
    QString dirPath = directoryLineEdit->text();
    if (dirPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择一个目录");
        return;
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        QMessageBox::warning(this, "警告", "所选目录不存在");
        return;
    }

    // 清空文本显示
    directoryTextDisplay->clear();
    
    // 设置目录读取器参数
    directoryReader->setMaxDepth(depthSpinBox->value());
    directoryReader->setReadFiles(readFilesCheckBox->isChecked());
    
    // 设置过滤选项
    if (filterCheckBox->isChecked()) {
        directoryReader->setFilterPattern(filterPatternLineEdit->text(), regexRadioButton->isChecked());
    } else {
        directoryReader->setFilterPattern("", false);
    }
    
    // 设置过滤规则
    QStringList rules;
    QString rulesText = filterRulesTextEdit->toPlainText();
    if (!rulesText.isEmpty()) {
        rules = rulesText.split("\n", Qt::SkipEmptyParts);
    }
    directoryReader->setFilterRules(rules);
    
    // 禁用开始按钮，启用取消按钮
    startButton->setEnabled(false);
    cancelButton->setEnabled(true);
    progressBar->setVisible(true);
    progressBar->setValue(0);
    
    statusLabel->setText("正在读取目录...");
    
    // 开始读取
    directoryReader->startReading(dirPath);
}

void MainWindow::cancelReading()
{
    directoryReader->cancelReading();
    statusLabel->setText("正在取消...");
}

void MainWindow::updateProgress(int value)
{
    progressBar->setValue(value);
}

void MainWindow::readingFinished()
{
    startButton->setEnabled(true);
    cancelButton->setEnabled(false);
    progressBar->setVisible(false);
    
    if (directoryTreeWidget->topLevelItemCount() > 0) {
        statusLabel->setText("读取完成");
        directoryTreeWidget->expandItem(directoryTreeWidget->topLevelItem(0));
        updateTextDisplay();
    } else {
        statusLabel->setText("操作已取消");
    }
}

void MainWindow::toggleFilterOptions(bool enabled)
{
    wildcardRadioButton->setEnabled(enabled);
    regexRadioButton->setEnabled(enabled);
    filterPatternLineEdit->setEnabled(enabled);
}

void MainWindow::updateTextDisplay()
{
    directoryTextDisplay->clear();
    
    QTreeWidgetItem *rootItem = nullptr;
    if (directoryTreeWidget->selectedItems().isEmpty()) {
        if (directoryTreeWidget->topLevelItemCount() > 0) {
            rootItem = directoryTreeWidget->topLevelItem(0);
        }
    } else {
        rootItem = directoryTreeWidget->selectedItems().first();
    }
    
    if (rootItem) {
        QString text = directoryReader->generateTextRepresentation(rootItem);
        directoryTextDisplay->setPlainText(text);
    }
}

void MainWindow::exportToTxtFile()
{
    if (directoryTextDisplay->toPlainText().isEmpty()) {
        QMessageBox::information(this, "提示", "没有可导出的内容");
        return;
    }
    
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getSaveFileName(this, "导出为TXT文件",
                                                  defaultPath + "/目录结构.txt",
                                                  "文本文件 (*.txt)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件进行写入");
        return;
    }
    
    QTextStream out(&file);
    out << directoryTextDisplay->toPlainText();
    file.close();
    
    QMessageBox::information(this, "成功", "文件已成功导出");
}

void MainWindow::importFilterRules()
{
    QString fileName = QFileDialog::getOpenFileName(this, "导入过滤规则文件",
                                                  QString(),
                                                  "文本文件 (*.txt *.gitignore);;所有文件 (*)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "错误", "无法打开文件");
        return;
    }
    
    QTextStream in(&file);
    filterRulesTextEdit->setPlainText(in.readAll());
    file.close();
} 