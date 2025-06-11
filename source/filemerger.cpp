#include "filemerger.h"

#include <QtConcurrent/QtConcurrent>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDebug>

FileMerger::FileMerger(QObject *parent)
    : QObject(parent)
    , maxDepth(3)
    , useRegex(false)
    , useSeparator(true)
    , separator("----------")
    , useExtraction(false)
    , watcher(new QFutureWatcher<void>(this))
    , isCancelled(false)
{
    connect(watcher, &QFutureWatcher<void>::finished, this, [this]() {
        emit mergingFinished(foundFiles.size());
    });
}

FileMerger::~FileMerger()
{
    if (watcher->isRunning()) {
        watcher->cancel();
        watcher->waitForFinished();
    }
}

void FileMerger::setRootPath(const QString &path)
{
    rootPath = path;
}

void FileMerger::setMaxDepth(int depth)
{
    maxDepth = depth;
}

void FileMerger::setFileFilter(const QString &pattern, bool isRegex)
{
    fileFilter = pattern;
    useRegex = isRegex;
}

void FileMerger::setFilterRules(const QStringList &rules)
{
    filterRules = rules;
}

void FileMerger::setHeaderTemplate(const QString &headerTemplate)
{
    this->headerTemplate = headerTemplate;
}

void FileMerger::setSeparator(bool enabled, const QString &separator)
{
    useSeparator = enabled;
    this->separator = separator;
}

void FileMerger::setExtractionRule(const QString &regex, bool enabled)
{
    extractionRegex = regex;
    useExtraction = enabled;
}

void FileMerger::startMerging()
{
    if (rootPath.isEmpty()) {
        qWarning() << "Root path is not set!";
        return;
    }

    // 清空之前的结果
    foundFiles.clear();
    mergedText.clear();
    isCancelled = false;
    
    // 在后台线程中执行搜索和合并
    QFuture<void> future = QtConcurrent::run([this]() {
        // 首先搜索文件
        searchFiles(rootPath, 0);
        
        // 然后合并文件内容
        if (!isCancelled && !foundFiles.isEmpty()) {
            mergeFiles();
        }
    });
    
    watcher->setFuture(future);
}

void FileMerger::cancelOperation()
{
    isCancelled = true;
    if (watcher->isRunning()) {
        watcher->cancel();
        watcher->waitForFinished();
    }
}

bool FileMerger::isRunning() const
{
    return watcher->isRunning();
}

QString FileMerger::getMergedText() const
{
    return mergedText;
}

bool FileMerger::exportToFile(const QString &filePath) const
{
    if (mergedText.isEmpty()) {
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }
    
    QTextStream out(&file);
    out << mergedText;
    file.close();
    
    return true;
}

void FileMerger::searchFiles(const QString &path, int currentDepth)
{
    if (isCancelled || currentDepth > maxDepth) {
        return;
    }

    QDir dir(path);
    QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    
    for (const QFileInfo &info : entries) {
        if (isCancelled) {
            return;
        }
        
        QString entryPath = info.filePath();
        
        if (info.isDir()) {
            // 递归处理子目录
            searchFiles(entryPath, currentDepth + 1);
        } else if (info.isFile()) {
            // 检查文件是否匹配过滤模式
            if (shouldIncludeFile(info.fileName(), entryPath)) {
                foundFiles.append(entryPath);
                emit processingFile(entryPath);
            }
        }
    }
}

bool FileMerger::shouldIncludeFile(const QString &fileName, const QString &filePath) const
{
    // 如果没有设置过滤模式和规则，则包含所有文件
    if (fileFilter.isEmpty() && filterRules.isEmpty()) {
        return true;
    }
    
    // 首先检查文件是否匹配过滤模式
    bool matchesPattern = true;
    if (!fileFilter.isEmpty()) {
        if (!useRegex) {
            // 使用通配符匹配
            QRegularExpression wildcard(QRegularExpression::wildcardToRegularExpression(fileFilter));
            matchesPattern = wildcard.match(fileName).hasMatch();
        } else {
            // 使用正则表达式匹配
            QRegularExpression regex(fileFilter);
            matchesPattern = regex.match(fileName).hasMatch();
        }
    }
    
    // 如果没有设置过滤规则，则只根据过滤模式判断
    if (filterRules.isEmpty()) {
        return matchesPattern;
    }
    
    // 如果设置了过滤规则，则检查是否匹配任一规则
    for (const QString &rule : filterRules) {
        QString trimmedRule = rule.trimmed();
        if (trimmedRule.isEmpty() || trimmedRule.startsWith(QChar('#'))) {
            continue; // 跳过空行和注释
        }
        
        // 处理目录规则
        bool isDir = trimmedRule.endsWith('/');
        if (isDir) {
            trimmedRule.chop(1);
        }
        
        // 将gitignore风格的规则转换为通配符
        QString pattern = trimmedRule;
        if (pattern.startsWith(QStringLiteral("**/"))) {
            pattern.remove(0, 3); // 移除 **/ 前缀
        }
        
        // 转换为正则表达式
        QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern));
        if (regex.match(filePath).hasMatch() || regex.match(fileName).hasMatch()) {
            return true; // 匹配到包含规则
        }
    }
    
    // 如果有过滤规则但都不匹配，则根据过滤模式判断
    return matchesPattern;
}

void FileMerger::mergeFiles()
{
    int totalFiles = foundFiles.size();
    if (totalFiles == 0) {
        return;
    }
    
    QStringList contentList;
    
    for (int i = 0; i < totalFiles; ++i) {
        if (isCancelled) {
            return;
        }
        
        QString filePath = foundFiles.at(i);
        QFile file(filePath);
        
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);
            QString content = in.readAll();
            file.close();
            
            // 如果启用了内容提取，则提取指定内容
            if (useExtraction && !extractionRegex.isEmpty()) {
                content = extractContent(content);
            }
            
            // 生成文件头
            QString header = generateHeader(filePath, i + 1);
            
            // 添加到内容列表
            if (!header.isEmpty()) {
                contentList.append(header);
            }
            contentList.append(content);
            
            // 添加分隔符（如果不是最后一个文件）
            if (useSeparator && i < totalFiles - 1) {
                contentList.append(separator);
            }
        }
        
        // 更新进度
        int progressValue = ((i + 1) * 100) / totalFiles;
        emit progressUpdated(progressValue);
    }
    
    // 合并所有内容
    mergedText = contentList.join("\n");
}

QString FileMerger::extractContent(const QString &content) const
{
    QRegularExpression regex(extractionRegex);
    QRegularExpressionMatchIterator matches = regex.globalMatch(content);
    
    QStringList extractedParts;
    while (matches.hasNext()) {
        QRegularExpressionMatch match = matches.next();
        if (match.hasMatch()) {
            if (match.lastCapturedIndex() > 0) {
                // 如果有捕获组，则使用第一个捕获组
                extractedParts.append(match.captured(1));
            } else {
                // 否则使用整个匹配
                extractedParts.append(match.captured(0));
            }
        }
    }
    
    return extractedParts.join("\n");
}

QString FileMerger::generateHeader(const QString &filePath, int index) const
{
    if (headerTemplate.isEmpty()) {
        return QString();
    }
    
    QFileInfo fileInfo(filePath);
    QString header = headerTemplate;
    
    // 替换占位符
    header.replace("{filename}", fileInfo.fileName());
    header.replace("{index}", QString::number(index));
    header.replace("{path}", filePath);
    header.replace("{basename}", fileInfo.baseName());
    header.replace("{suffix}", fileInfo.suffix());
    header.replace("{size}", QString::number(fileInfo.size()));
    header.replace("{date}", fileInfo.lastModified().toString("yyyy-MM-dd"));
    header.replace("{time}", fileInfo.lastModified().toString("HH:mm:ss"));
    
    return header;
} 