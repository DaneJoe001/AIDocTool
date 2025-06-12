#include "filemergerwidget.h"
#include "filterrulesdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QClipboard>
#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QTextStream>

FileMergerWidget::FileMergerWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    createConnections();
    
    fileMerger = new FileMerger(this);
    
    // 连接文件合并器的信号和槽
    connect(fileMerger, &FileMerger::progressUpdated, this, &FileMergerWidget::updateProgress);
    connect(fileMerger, &FileMerger::mergeFinished, this, &FileMergerWidget::mergeFinished);
    connect(fileMerger, &FileMerger::processingFile, this, &FileMergerWidget::handleProcessingFile);
}

FileMergerWidget::~FileMergerWidget()
{
}

void FileMergerWidget::setFilterRules(const QList<FileFilterUtil::FilterRule> &rules)
{
    if (fileMerger) {
        fileMerger->setFilterRules(rules);
        filterRuleListWidget->setFilterRules(rules);
    }
}

QList<FileFilterUtil::FilterRule> FileMergerWidget::getFilterRules() const
{
    if (fileMerger) {
        return fileMerger->getFilterRules();
    }
    return QList<FileFilterUtil::FilterRule>();
}

void FileMergerWidget::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    
    // 创建目录选择区域
    QGroupBox *directoryGroupBox = new QGroupBox(tr("目录选择"));
    QHBoxLayout *directoryLayout = new QHBoxLayout(directoryGroupBox);
    
    directoryLineEdit = new QLineEdit(directoryGroupBox);
    directoryLayout->addWidget(directoryLineEdit);
    
    browseButton = new QPushButton(tr("浏览"), directoryGroupBox);
    directoryLayout->addWidget(browseButton);
    
    mainLayout->addWidget(directoryGroupBox);
    
    // 创建选项区域
    QGroupBox *optionsGroupBox = new QGroupBox(tr("选项"));
    QGridLayout *optionsLayout = new QGridLayout(optionsGroupBox);
    
    // 深度选择
    optionsLayout->addWidget(new QLabel(tr("最大深度:")), 0, 0);
    depthSpinBox = new QSpinBox(optionsGroupBox);
    depthSpinBox->setRange(1, 10);
    depthSpinBox->setValue(3);
    optionsLayout->addWidget(depthSpinBox, 0, 1);
    
    // 过滤选项
    filterCheckBox = new QCheckBox(tr("启用过滤"), optionsGroupBox);
    optionsLayout->addWidget(filterCheckBox, 1, 0, 1, 2);
    
    // 创建过滤规则列表部件
    filterRuleListWidget = new FilterRuleListWidget(optionsGroupBox);
    filterRuleListWidget->setEnabled(false);
    optionsLayout->addWidget(filterRuleListWidget, 2, 0, 1, 2);
    
    // 分隔符选项
    separatorCheckBox = new QCheckBox(tr("使用分隔符"), optionsGroupBox);
    separatorCheckBox->setChecked(true);
    optionsLayout->addWidget(separatorCheckBox, 3, 0);
    
    separatorLineEdit = new QLineEdit(tr("----------"), optionsGroupBox);
    optionsLayout->addWidget(separatorLineEdit, 3, 1);
    
    // 提取选项
    extractionCheckBox = new QCheckBox(tr("使用内容提取"), optionsGroupBox);
    optionsLayout->addWidget(extractionCheckBox, 4, 0);
    
    extractionLineEdit = new QLineEdit(optionsGroupBox);
    extractionLineEdit->setPlaceholderText(tr("输入正则表达式"));
    extractionLineEdit->setEnabled(false);
    optionsLayout->addWidget(extractionLineEdit, 4, 1);
    
    // 文件头选项
    headerCheckBox = new QCheckBox(tr("使用文件头"), optionsGroupBox);
    optionsLayout->addWidget(headerCheckBox, 5, 0);
    
    headerLineEdit = new QLineEdit(tr("// {filename} - {path}"), optionsGroupBox);
    headerLineEdit->setEnabled(false);
    optionsLayout->addWidget(headerLineEdit, 5, 1);
    
    mainLayout->addWidget(optionsGroupBox);
    
    // 创建按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    startButton = new QPushButton(tr("开始"), this);
    buttonLayout->addWidget(startButton);
    
    cancelButton = new QPushButton(tr("取消"), this);
    cancelButton->setEnabled(false);
    buttonLayout->addWidget(cancelButton);
    
    exportButton = new QPushButton(tr("导出"), this);
    exportButton->setEnabled(false);
    buttonLayout->addWidget(exportButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // 创建进度区域
    QHBoxLayout *progressLayout = new QHBoxLayout();
    
    progressBar = new QProgressBar(this);
    progressBar->setRange(0, 100);
    progressBar->setValue(0);
    progressLayout->addWidget(progressBar);
    
    statusLabel = new QLabel(tr("就绪"), this);
    progressLayout->addWidget(statusLabel);
    
    mainLayout->addLayout(progressLayout);
    
    // 创建文本显示区域
    mergedTextDisplay = new QTextEdit(this);
    mergedTextDisplay->setReadOnly(true);
    mergedTextDisplay->setLineWrapMode(QTextEdit::NoWrap);
    mainLayout->addWidget(mergedTextDisplay);
}

void FileMergerWidget::createConnections()
{
    // 连接信号和槽
    connect(browseButton, &QPushButton::clicked, this, &FileMergerWidget::browseDirectory);
    connect(startButton, &QPushButton::clicked, this, &FileMergerWidget::startMerging);
    connect(cancelButton, &QPushButton::clicked, this, &FileMergerWidget::cancelMerging);
    connect(exportButton, &QPushButton::clicked, this, &FileMergerWidget::exportMergedText);
    
    connect(filterCheckBox, &QCheckBox::toggled, this, &FileMergerWidget::toggleFilterOptions);
    connect(separatorCheckBox, &QCheckBox::toggled, this, &FileMergerWidget::toggleSeparatorOptions);
    connect(extractionCheckBox, &QCheckBox::toggled, this, &FileMergerWidget::toggleExtractionOptions);
    connect(headerCheckBox, &QCheckBox::toggled, this, &FileMergerWidget::toggleHeaderOptions);
    
    connect(filterRuleListWidget, &FilterRuleListWidget::rulesChanged, this, &FileMergerWidget::handleFilterRulesChanged);
}

void FileMergerWidget::browseDirectory()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("选择目录"),
                                                  directoryLineEdit->text(),
                                                  QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    
    if (!dir.isEmpty()) {
        directoryLineEdit->setText(dir);
    }
}

void FileMergerWidget::startMerging()
{
    QString rootPath = directoryLineEdit->text();
    if (rootPath.isEmpty()) {
        QMessageBox::warning(this, tr("警告"), tr("请选择一个目录"));
        return;
    }
    
    // 设置选项
    fileMerger->setMaxDepth(depthSpinBox->value());
    
    // 设置过滤规则
    if (filterCheckBox->isChecked()) {
        fileMerger->setFilterRules(filterRuleListWidget->getFilterRules());
    } else {
        fileMerger->setFilterRules(QList<FileFilterUtil::FilterRule>());
    }
    
    // 设置分隔符
    fileMerger->setSeparator(separatorCheckBox->isChecked(), separatorLineEdit->text());
    
    // 设置内容提取
    fileMerger->setExtractionRule(extractionLineEdit->text(), extractionCheckBox->isChecked());
    
    // 设置文件头
    fileMerger->setHeaderTemplate(headerLineEdit->text(), headerCheckBox->isChecked());
    
    // 更新UI状态
    startButton->setEnabled(false);
    cancelButton->setEnabled(true);
    exportButton->setEnabled(false);
    progressBar->setValue(0);
    statusLabel->setText(tr("正在搜索文件..."));
    mergedTextDisplay->clear();
    
    // 开始合并
    fileMerger->start(rootPath);
}

void FileMergerWidget::cancelMerging()
{
    if (fileMerger) {
        fileMerger->cancel();
    }
    
    statusLabel->setText(tr("已取消"));
    startButton->setEnabled(true);
    cancelButton->setEnabled(false);
}

void FileMergerWidget::updateProgress(int value)
{
    progressBar->setValue(value);
}

void FileMergerWidget::mergeFinished()
{
    // 显示合并结果
    mergedTextDisplay->setPlainText(fileMerger->getMergedText());
    
    // 更新UI状态
    statusLabel->setText(tr("合并完成"));
    startButton->setEnabled(true);
    cancelButton->setEnabled(false);
    exportButton->setEnabled(true);
}

void FileMergerWidget::handleProcessingFile(const QString &filePath)
{
    statusLabel->setText(tr("正在处理: %1").arg(filePath));
}

void FileMergerWidget::exportMergedText()
{
    QString documentsPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getSaveFileName(this, tr("导出合并文本"),
                                                 documentsPath,
                                                 tr("文本文件 (*.txt);;所有文件 (*.*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("错误"), tr("无法打开文件进行写入: %1").arg(file.errorString()));
        return;
    }
    
    QTextStream out(&file);
    out << mergedTextDisplay->toPlainText();
    
    QMessageBox::information(this, tr("成功"), tr("合并文本已成功导出到: %1").arg(fileName));
}

void FileMergerWidget::toggleFilterOptions(bool enabled)
{
    filterRuleListWidget->setEnabled(enabled);
    
    // 更新FileMerger的过滤规则
    if (fileMerger) {
        if (enabled) {
            fileMerger->setFilterRules(filterRuleListWidget->getFilterRules());
        } else {
            fileMerger->setFilterRules(QList<FileFilterUtil::FilterRule>());
        }
    }
}

void FileMergerWidget::toggleSeparatorOptions(bool enabled)
{
    separatorLineEdit->setEnabled(enabled);
}

void FileMergerWidget::toggleExtractionOptions(bool enabled)
{
    extractionLineEdit->setEnabled(enabled);
}

void FileMergerWidget::toggleHeaderOptions(bool enabled)
{
    headerLineEdit->setEnabled(enabled);
}

void FileMergerWidget::handleFilterRulesChanged(const QList<FileFilterUtil::FilterRule> &rules)
{
    if (fileMerger) {
        fileMerger->setFilterRules(rules);
        
        // 如果启用了过滤，提示用户重新应用，但不自动启动
        if (filterCheckBox->isChecked()) {
            statusLabel->setText(tr("过滤规则已更新，请重新开始合并操作以应用"));
        }
    }
} 