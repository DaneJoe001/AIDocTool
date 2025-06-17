#include "mainwindow.h"
#include "stylesheetmanager.h"
#include "stylesettingsdialog.h"
#include "filterrulesdialog.h"
#include "batchrenamedialog.h"
#include "codestatsdialog.h"
#include "docgeneratordialog.h"

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
#include <QDesktopServices>
#include <QUrl>

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

    // 连接样式管理器信号
    connect(StyleSheetManager::instance(), &StyleSheetManager::themeChanged, this, &MainWindow::onThemeChanged);

    // 连接过滤规则列表部件的信号
    connect(filterRuleListWidget, &FilterRuleListWidget::rulesChanged, this, &MainWindow::handleFilterRulesChanged);

    // 初始状态
    cancelButton->setEnabled(false);
    toggleFilterOptions(false);
    filterCheckBox->setChecked(false);
    
    // 设置目录树读取器
    directoryReader->setTreeWidget(directoryTreeWidget);

    // 加载保存的样式设置
    StyleSheetManager::instance()->loadSettings();
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
    
    // 创建过滤规则列表部件
    filterRuleListWidget = new FilterRuleListWidget(optionsGroupBox);
    filterRuleListWidget->setEnabled(false);
    
    readFilesCheckBox = new QCheckBox("读取文件名", optionsGroupBox);
    readFilesCheckBox->setChecked(true);
    
    optionsLayout->addWidget(depthLabel, 0, 0);
    optionsLayout->addWidget(depthSpinBox, 0, 1);
    optionsLayout->addWidget(filterCheckBox, 1, 0, 1, 2);
    optionsLayout->addWidget(filterRuleListWidget, 2, 0, 1, 2);
    optionsLayout->addWidget(readFilesCheckBox, 3, 0, 1, 2);
    
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

    // 工具菜单
    toolsMenu = menuBar->addMenu("工具");
    batchRenameAction = toolsMenu->addAction("批量文件重命名", this, &MainWindow::openBatchRenameDialog);
    codeStatsAction = toolsMenu->addAction("代码统计工具", this, &MainWindow::openCodeStatsDialog);
    docGeneratorAction = toolsMenu->addAction("文档生成工具", this, &MainWindow::openDocGeneratorDialog);

    // 设置菜单
    settingsMenu = menuBar->addMenu("设置");
    styleSettingsAction = settingsMenu->addAction("样式设置", this, &MainWindow::openStyleSettings);
    filterRulesAction = settingsMenu->addAction("过滤规则管理", this, &MainWindow::openFilterRulesDialog);

    // 帮助菜单
    helpMenu = menuBar->addMenu("帮助");
    helpAction = helpMenu->addAction("使用帮助", this, &MainWindow::showHelpDocument);
    helpMenu->addSeparator();
    aboutAction = helpMenu->addAction("关于", this, &MainWindow::showAboutDialog);
}

void MainWindow::openStyleSettings()
{
    StyleSettingsDialog dialog(this);
    dialog.exec();
}

void MainWindow::onThemeChanged(const QString &themeName)
{
    // 可以在这里添加主题变更后的特定处理
    // 例如更新状态栏显示当前主题等
    statusLabel->setText(QString("当前主题: %1").arg(themeName));
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
    QString rootPath = directoryLineEdit->text();
    if (rootPath.isEmpty()) {
        QMessageBox::warning(this, "警告", "请选择一个目录");
        return;
    }

    // 清空树控件
    directoryTreeWidget->clear();
    directoryTextDisplay->clear();
    
    // 设置选项
    directoryReader->setMaxDepth(depthSpinBox->value());
    directoryReader->setReadFiles(readFilesCheckBox->isChecked());
    
    // 设置过滤规则
    if (filterCheckBox->isChecked()) {
        directoryReader->setFilterRules(filterRuleListWidget->getFilterRules());
    } else {
        directoryReader->setFilterRules(QList<FileFilterUtil::FilterRule>());
    }
    
    // 更新UI状态
    startButton->setEnabled(false);
    cancelButton->setEnabled(true);
    progressBar->setVisible(true);
    progressBar->setValue(0);
    statusLabel->setText("正在读取目录...");
    
    // 开始读取（现在是异步的）
    directoryReader->read(rootPath);
}

void MainWindow::cancelReading()
{
    directoryReader->cancel();
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
    filterRuleListWidget->setEnabled(enabled);
    
    // 更新DirectoryTreeReader的过滤规则状态
    if (directoryReader) {
        if (enabled) {
            directoryReader->setFilterRules(filterRules);
        } else {
            directoryReader->setFilterRules(QList<FileFilterUtil::FilterRule>());
        }
        
        // 提示用户重新读取，但不自动触发
        if (directoryTreeWidget->topLevelItemCount() > 0) {
            statusLabel->setText("过滤选项已" + QString(enabled ? "启用" : "禁用") + "，请点击\"开始读取\"按钮重新应用");
        }
    }
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
        QString text;
        // 使用选中的项生成文本表示
        if (rootItem == directoryTreeWidget->topLevelItem(0)) {
            // 如果是根项，使用 DirectoryTreeReader 的方法
            text = directoryReader->generateTextRepresentation();
        } else {
            // 否则手动生成文本表示
            text = generateTextRepresentation(rootItem);
        }
        directoryTextDisplay->setPlainText(text);
    }
}

QString MainWindow::generateTextRepresentation(QTreeWidgetItem *item, int level)
{
    if (!item) {
        return QString();
    }
    
    QString result;
    
    // 根节点特殊处理
    if (level == 0) {
        result = item->text(0) + "/\n";
    } else {
        // 构建前缀
        QString prefix;
        for (int i = 0; i < level - 1; i++) {
            prefix += "│   ";
        }
        
        // 添加当前项的连接线
        if (level > 0) {
            prefix += "├── ";
        }
        
        // 添加当前项
        if (item->text(1) == "目录") {
            result = prefix + item->text(0) + "/\n";
        } else {
            result = prefix + item->text(0) + "\n";
        }
    }
    
    // 处理子项
    int childCount = item->childCount();
    for (int i = 0; i < childCount; ++i) {
        QTreeWidgetItem *child = item->child(i);
        
        // 最后一个子项使用不同的连接线
        if (i == childCount - 1) {
            // 保存原始结果
            QString originalResult = result;
            
            // 将下一级的"├──"替换为"└──"
            QString childText = generateTextRepresentation(child, level + 1);
            if (level >= 0) {
                childText.replace(level * 4, 4, "└── ");
                
                // 将后续行的"│   "替换为"    "
                int pos = childText.indexOf('\n');
                while (pos >= 0 && pos < childText.length() - 1) {
                    int prefixPos = pos + 1 + level * 4;
                    if (prefixPos + 4 <= childText.length() && 
                        childText.mid(prefixPos, 4) == "│   ") {
                        childText.replace(prefixPos, 4, "    ");
                    }
                    pos = childText.indexOf('\n', pos + 1);
                }
            }
            result = originalResult + childText;
        } else {
            result += generateTextRepresentation(child, level + 1);
        }
    }
    
    return result;
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
    // 使用新的过滤规则对话框替代原有的导入逻辑
    openFilterRulesDialog();
}

void MainWindow::openFilterRulesDialog()
{
    FilterRulesDialog dialog(this);
    dialog.setFilterRules(filterRules);
    
    if (dialog.exec() == QDialog::Accepted) {
        filterRules = dialog.getFilterRules();
        filterRuleListWidget->setFilterRules(filterRules);
    }
}

void MainWindow::openBatchRenameDialog()
{
    BatchRenameDialog dialog(this);
    dialog.exec();
}

void MainWindow::openCodeStatsDialog()
{
    CodeStatsDialog dialog(this);
    dialog.exec();
}

void MainWindow::openDocGeneratorDialog()
{
    DocGeneratorDialog dialog(this);
    dialog.exec();
}

void MainWindow::handleFilterRulesChanged(const QList<FileFilterUtil::FilterRule> &rules)
{
    filterRules = rules;
    
    // 立即更新DirectoryTreeReader的过滤规则
    if (directoryReader) {
        directoryReader->setFilterRules(filterRules);
    }
    
    // 如果启用了过滤并且已经读取了目录，则重新应用过滤规则
    // 但不立即触发重新读取，而是让用户手动点击"开始读取"按钮
    if (filterCheckBox->isChecked() && directoryTreeWidget->topLevelItemCount() > 0) {
        statusLabel->setText("过滤规则已更新，请点击\"开始读取\"按钮重新应用");
    }
}

void MainWindow::showAboutDialog()
{
    QMessageBox about(this);
    about.setWindowTitle("关于 AI文档工具集");
    about.setTextFormat(Qt::RichText);
    
    QString aboutText = "<h2>AI文档工具集 v0.7</h2>";
    aboutText += "<p>一款专为开发者设计的代码文档工具集，提供目录树读取、代码统计、文件合并等功能。</p>";
    aboutText += "<p><b>开发者信息：</b></p>";
    aboutText += "<ul>";
    aboutText += "<li>作者：DaneJoe001</li>";
    aboutText += "<li>邮箱：<a href='mailto:2845547447@qq.com'>2845547447@qq.com</a></li>";
    aboutText += "<li>博客：<a href='https://danejoe001.github.io/'>https://danejoe001.github.io/</a></li>";
    aboutText += "<li>版权所有 © 2023-2024 DaneJoe001</li>";
    aboutText += "</ul>";
    aboutText += "<p><b>开发工具与技术：</b></p>";
    aboutText += "<ul>";
    aboutText += "<li>使用 Cursor 编辑器开发</li>";
    aboutText += "<li>基于 Qt 6.5.3 框架</li>";
    aboutText += "<li>使用 C++17 标准</li>";
    aboutText += "<li>AI 辅助：Claude 3.7 Sonnet</li>";
    aboutText += "</ul>";
    
    about.setText(aboutText);
    about.setIconPixmap(QPixmap(":/icons/main_icon.ico").scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    about.setStandardButtons(QMessageBox::Ok);
    
    about.exec();
}

void MainWindow::showHelpDocument()
{
    QMessageBox helpMsg(this);
    helpMsg.setWindowTitle("使用帮助");
    helpMsg.setTextFormat(Qt::RichText);
    
    QString helpText = "<h2>AI文档工具集使用指南</h2>";
    helpText += "<h3>主要功能：</h3>";
    helpText += "<ol>";
    helpText += "<li><b>目录树读取器</b>：读取并显示指定目录的文件结构，支持过滤规则和深度控制。</li>";
    helpText += "<li><b>文件合并工具</b>：根据条件搜索并合并多个文本文件。</li>";
    helpText += "<li><b>批量文件重命名</b>：批量修改文件名，支持正则表达式和自定义规则。</li>";
    helpText += "<li><b>代码统计工具</b>：统计代码行数、注释比例等指标。</li>";
    helpText += "<li><b>文档生成工具</b>：基于代码自动生成文档结构。</li>";
    helpText += "</ol>";
    
    helpText += "<h3>基本操作指南：</h3>";
    helpText += "<ul>";
    helpText += "<li><b>目录浏览</b>：点击\"浏览\"按钮选择要读取的目录。</li>";
    helpText += "<li><b>搜索深度</b>：设置目录递归读取的最大深度。</li>";
    helpText += "<li><b>过滤规则</b>：启用过滤可根据规则排除或包含特定文件。</li>";
    helpText += "<li><b>导出结果</b>：读取完成后可将结果导出为文本文件。</li>";
    helpText += "</ul>";
    
    helpText += "<h3>使用技巧：</h3>";
    helpText += "<ul>";
    helpText += "<li>使用过滤规则可以排除不必要的文件（如 .git 目录）。</li>";
    helpText += "<li>设置合理的搜索深度可以提高读取性能。</li>";
    helpText += "<li>可以通过\"样式设置\"更改界面主题风格。</li>";
    helpText += "</ul>";
    
    helpMsg.setText(helpText);
    helpMsg.setStandardButtons(QMessageBox::Ok);
    
    helpMsg.exec();
} 