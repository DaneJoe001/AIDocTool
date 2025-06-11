#include "filemergerwidget.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QGroupBox>
#include <QLabel>
#include <QSplitter>
#include <QDateTime>

FileMergerWidget::FileMergerWidget(QWidget *parent)
    : QWidget(parent)
    , fileMerger(new FileMerger(this))
{
    setupUI();
    createConnections();
    updateUIState(false);
}

FileMergerWidget::~FileMergerWidget()
{
    if (fileMerger->isRunning()) {
        fileMerger->cancelOperation();
    }
}

void FileMergerWidget::setupUI()
{
    // 主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建分割器，上面是配置区域，下面是结果区域
    QSplitter *splitter = new QSplitter(Qt::Vertical);
    mainLayout->addWidget(splitter);
    
    // 配置区域容器
    QWidget *configWidget = new QWidget;
    QVBoxLayout *configLayout = new QVBoxLayout(configWidget);
    
    // 目录选择区域
    QGroupBox *dirGroupBox = new QGroupBox(tr("目录设置"));
    QGridLayout *dirLayout = new QGridLayout(dirGroupBox);
    
    dirLayout->addWidget(new QLabel(tr("根目录:")), 0, 0);
    rootPathEdit = new QLineEdit;
    dirLayout->addWidget(rootPathEdit, 0, 1);
    browseButton = new QPushButton(tr("浏览..."));
    dirLayout->addWidget(browseButton, 0, 2);
    
    dirLayout->addWidget(new QLabel(tr("最大深度:")), 1, 0);
    depthSpinBox = new QSpinBox;
    depthSpinBox->setRange(1, 100);
    depthSpinBox->setValue(3);
    dirLayout->addWidget(depthSpinBox, 1, 1);
    
    configLayout->addWidget(dirGroupBox);
    
    // 文件过滤区域
    QGroupBox *filterGroupBox = new QGroupBox(tr("文件选择"));
    QVBoxLayout *filterLayout = new QVBoxLayout(filterGroupBox);
    
    QHBoxLayout *filterTypeLayout = new QHBoxLayout;
    filterTypeLayout->addWidget(new QLabel(tr("过滤类型:")));
    filterTypeCombo = new QComboBox;
    filterTypeCombo->addItem(tr("通配符"), false);
    filterTypeCombo->addItem(tr("正则表达式"), true);
    filterTypeLayout->addWidget(filterTypeCombo);
    filterLayout->addLayout(filterTypeLayout);
    
    QHBoxLayout *filterPatternLayout = new QHBoxLayout;
    filterPatternLayout->addWidget(new QLabel(tr("文件模式:")));
    filterPatternEdit = new QLineEdit;
    filterPatternEdit->setPlaceholderText(tr("例如: *.txt, *.md"));
    filterPatternLayout->addWidget(filterPatternEdit);
    filterLayout->addLayout(filterPatternLayout);
    
    QLabel *filterRulesLabel = new QLabel(tr("包含规则 (匹配以下规则的文件将被包含):"));
    filterLayout->addWidget(filterRulesLabel);
    
    filterRulesList = new QListWidget;
    filterLayout->addWidget(filterRulesList);
    
    QHBoxLayout *filterRuleEditLayout = new QHBoxLayout;
    filterRuleEdit = new QLineEdit;
    filterRuleEdit->setPlaceholderText(tr("输入包含规则，例如: *.cpp, src/*.h"));
    filterRuleEditLayout->addWidget(filterRuleEdit);
    addRuleButton = new QPushButton(tr("添加"));
    filterRuleEditLayout->addWidget(addRuleButton);
    removeRuleButton = new QPushButton(tr("删除"));
    filterRuleEditLayout->addWidget(removeRuleButton);
    filterLayout->addLayout(filterRuleEditLayout);
    
    configLayout->addWidget(filterGroupBox);
    
    // 合并选项区域
    QGroupBox *mergeOptionsGroupBox = new QGroupBox(tr("合并选项"));
    QVBoxLayout *mergeOptionsLayout = new QVBoxLayout(mergeOptionsGroupBox);
    
    QHBoxLayout *separatorLayout = new QHBoxLayout;
    separatorCheckBox = new QCheckBox(tr("使用分隔符"));
    separatorCheckBox->setChecked(true);
    separatorLayout->addWidget(separatorCheckBox);
    separatorEdit = new QLineEdit("----------");
    separatorLayout->addWidget(separatorEdit);
    mergeOptionsLayout->addLayout(separatorLayout);
    
    QHBoxLayout *extractionLayout = new QHBoxLayout;
    extractionCheckBox = new QCheckBox(tr("使用正则提取内容"));
    extractionLayout->addWidget(extractionCheckBox);
    extractionRegexEdit = new QLineEdit;
    extractionRegexEdit->setPlaceholderText(tr("输入正则表达式"));
    extractionRegexEdit->setEnabled(false);
    extractionLayout->addWidget(extractionRegexEdit);
    mergeOptionsLayout->addLayout(extractionLayout);
    
    QHBoxLayout *headerLayout = new QHBoxLayout;
    headerLayout->addWidget(new QLabel(tr("文件头模板:")));
    headerTemplateEdit = new QLineEdit;
    headerTemplateEdit->setPlaceholderText(tr("例如: '文件: {filename} (#{index})'"));
    headerLayout->addWidget(headerTemplateEdit);
    mergeOptionsLayout->addLayout(headerLayout);
    
    QLabel *placeholdersLabel = new QLabel(tr("可用占位符: {filename}, {index}, {path}, {basename}, {suffix}, {size}, {date}, {time}"));
    placeholdersLabel->setStyleSheet("color: gray; font-size: 9pt;");
    mergeOptionsLayout->addWidget(placeholdersLabel);
    
    configLayout->addWidget(mergeOptionsGroupBox);
    
    // 操作按钮区域
    QHBoxLayout *actionButtonsLayout = new QHBoxLayout;
    startButton = new QPushButton(tr("开始合并"));
    actionButtonsLayout->addWidget(startButton);
    cancelButton = new QPushButton(tr("取消"));
    cancelButton->setEnabled(false);
    actionButtonsLayout->addWidget(cancelButton);
    actionButtonsLayout->addStretch();
    exportButton = new QPushButton(tr("导出..."));
    exportButton->setEnabled(false);
    actionButtonsLayout->addWidget(exportButton);
    copyButton = new QPushButton(tr("复制"));
    copyButton->setEnabled(false);
    actionButtonsLayout->addWidget(copyButton);
    
    configLayout->addLayout(actionButtonsLayout);
    
    // 进度区域
    QHBoxLayout *progressLayout = new QHBoxLayout;
    progressBar = new QProgressBar;
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressLayout->addWidget(progressBar);
    statusLabel = new QLabel(tr("就绪"));
    progressLayout->addWidget(statusLabel);
    
    configLayout->addLayout(progressLayout);
    
    // 将配置区域添加到分割器
    splitter->addWidget(configWidget);
    
    // 结果区域
    QGroupBox *resultGroupBox = new QGroupBox(tr("合并结果"));
    QVBoxLayout *resultLayout = new QVBoxLayout(resultGroupBox);
    resultTextEdit = new QTextEdit;
    resultTextEdit->setReadOnly(true);
    resultTextEdit->setLineWrapMode(QTextEdit::NoWrap);
    resultTextEdit->setFont(QFont("Consolas, Courier New, monospace", 10));
    resultLayout->addWidget(resultTextEdit);
    
    // 将结果区域添加到分割器
    splitter->addWidget(resultGroupBox);
    
    // 设置初始分割器大小
    splitter->setSizes(QList<int>() << 400 << 300);
    
    // 设置窗口标题和大小
    setWindowTitle(tr("文件合并工具"));
    resize(800, 700);
}

void FileMergerWidget::createConnections()
{
    // 按钮连接
    connect(browseButton, &QPushButton::clicked, this, &FileMergerWidget::selectRootDirectory);
    connect(startButton, &QPushButton::clicked, this, &FileMergerWidget::startMerging);
    connect(cancelButton, &QPushButton::clicked, this, &FileMergerWidget::cancelMerging);
    connect(exportButton, &QPushButton::clicked, this, &FileMergerWidget::exportToFile);
    connect(copyButton, &QPushButton::clicked, this, &FileMergerWidget::copyToClipboard);
    connect(addRuleButton, &QPushButton::clicked, this, &FileMergerWidget::addFilterRule);
    connect(removeRuleButton, &QPushButton::clicked, this, &FileMergerWidget::removeFilterRule);
    
    // 过滤器类型变更
    connect(filterTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &FileMergerWidget::onFilterTypeChanged);
    
    // 提取规则状态变更
    connect(extractionCheckBox, &QCheckBox::toggled, this, &FileMergerWidget::onExtractionToggled);
    
    // 文件合并器信号连接
    connect(fileMerger, &FileMerger::progressUpdated, this, &FileMergerWidget::onProgressUpdated);
    connect(fileMerger, &FileMerger::mergingFinished, this, &FileMergerWidget::onMergingFinished);
    connect(fileMerger, &FileMerger::processingFile, this, &FileMergerWidget::onProcessingFile);
}

void FileMergerWidget::selectRootDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择根目录"), rootPathEdit->text());
    if (!dir.isEmpty()) {
        rootPathEdit->setText(dir);
    }
}

void FileMergerWidget::startMerging()
{
    QString rootPath = rootPathEdit->text();
    if (rootPath.isEmpty()) {
        QMessageBox::warning(this, tr("错误"), tr("请选择根目录"));
        return;
    }
    
    // 更新UI状态
    updateUIState(true);
    statusLabel->setText(tr("正在处理..."));
    progressBar->setValue(0);
    resultTextEdit->clear();
    
    // 配置文件合并器
    fileMerger->setRootPath(rootPath);
    fileMerger->setMaxDepth(depthSpinBox->value());
    
    // 设置文件过滤
    bool useRegex = filterTypeCombo->currentData().toBool();
    fileMerger->setFileFilter(filterPatternEdit->text(), useRegex);
    
    // 设置过滤规则
    fileMerger->setFilterRules(getFilterRules());
    
    // 设置分隔符
    fileMerger->setSeparator(separatorCheckBox->isChecked(), separatorEdit->text());
    
    // 设置提取规则
    fileMerger->setExtractionRule(extractionRegexEdit->text(), extractionCheckBox->isChecked());
    
    // 设置文件头模板
    fileMerger->setHeaderTemplate(headerTemplateEdit->text());
    
    // 开始合并
    fileMerger->startMerging();
}

void FileMergerWidget::cancelMerging()
{
    fileMerger->cancelOperation();
    statusLabel->setText(tr("已取消"));
    updateUIState(false);
}

void FileMergerWidget::exportToFile()
{
    QString filePath = QFileDialog::getSaveFileName(this, tr("导出合并结果"), 
                                                   QDir::homePath() + "/merged_" + QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss") + ".txt",
                                                   tr("文本文件 (*.txt);;所有文件 (*.*)"));
    if (!filePath.isEmpty()) {
        if (fileMerger->exportToFile(filePath)) {
            QMessageBox::information(this, tr("导出成功"), tr("文件已成功导出到: %1").arg(filePath));
        } else {
            QMessageBox::warning(this, tr("导出失败"), tr("无法写入文件: %1").arg(filePath));
        }
    }
}

void FileMergerWidget::copyToClipboard()
{
    QApplication::clipboard()->setText(resultTextEdit->toPlainText());
    statusLabel->setText(tr("已复制到剪贴板"));
}

void FileMergerWidget::addFilterRule()
{
    QString rule = filterRuleEdit->text().trimmed();
    if (!rule.isEmpty()) {
        filterRulesList->addItem(rule);
        filterRuleEdit->clear();
        filterRuleEdit->setFocus();
    }
}

void FileMergerWidget::removeFilterRule()
{
    QListWidgetItem *currentItem = filterRulesList->currentItem();
    if (currentItem) {
        delete filterRulesList->takeItem(filterRulesList->row(currentItem));
    }
}

void FileMergerWidget::onMergingFinished(int fileCount)
{
    updateUIState(false);
    
    if (fileCount > 0) {
        statusLabel->setText(tr("完成，共处理 %1 个文件").arg(fileCount));
        resultTextEdit->setText(fileMerger->getMergedText());
        exportButton->setEnabled(true);
        copyButton->setEnabled(true);
    } else {
        statusLabel->setText(tr("未找到匹配的文件"));
    }
}

void FileMergerWidget::onProgressUpdated(int value)
{
    progressBar->setValue(value);
}

void FileMergerWidget::onProcessingFile(const QString &filePath)
{
    statusLabel->setText(tr("正在处理: %1").arg(QFileInfo(filePath).fileName()));
}

void FileMergerWidget::onFilterTypeChanged(int index)
{
    bool isRegex = filterTypeCombo->itemData(index).toBool();
    if (isRegex) {
        filterPatternEdit->setPlaceholderText(tr("例如: \\.txt$|\\.md$"));
    } else {
        filterPatternEdit->setPlaceholderText(tr("例如: *.txt, *.md"));
    }
}

void FileMergerWidget::onExtractionToggled(bool checked)
{
    extractionRegexEdit->setEnabled(checked);
}

void FileMergerWidget::updateUIState(bool isRunning)
{
    rootPathEdit->setEnabled(!isRunning);
    browseButton->setEnabled(!isRunning);
    depthSpinBox->setEnabled(!isRunning);
    filterTypeCombo->setEnabled(!isRunning);
    filterPatternEdit->setEnabled(!isRunning);
    filterRulesList->setEnabled(!isRunning);
    filterRuleEdit->setEnabled(!isRunning);
    addRuleButton->setEnabled(!isRunning);
    removeRuleButton->setEnabled(!isRunning);
    separatorCheckBox->setEnabled(!isRunning);
    separatorEdit->setEnabled(!isRunning && separatorCheckBox->isChecked());
    extractionCheckBox->setEnabled(!isRunning);
    extractionRegexEdit->setEnabled(!isRunning && extractionCheckBox->isChecked());
    headerTemplateEdit->setEnabled(!isRunning);
    startButton->setEnabled(!isRunning);
    cancelButton->setEnabled(isRunning);
}

QStringList FileMergerWidget::getFilterRules() const
{
    QStringList rules;
    for (int i = 0; i < filterRulesList->count(); ++i) {
        rules.append(filterRulesList->item(i)->text());
    }
    return rules;
} 