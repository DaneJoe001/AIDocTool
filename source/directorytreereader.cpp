#include "directorytreereader.h"

#include <QtConcurrent/QtConcurrent>
#include <QRegularExpression>
#include <QDebug>
#include <QStyle>
#include <QApplication>

DirectoryTreeReader::DirectoryTreeReader(QObject *parent)
    : QObject(parent)
    , treeWidget(nullptr)
    , maxDepth(3)
    , readFiles(true)
    , useRegex(false)
    , watcher(new QFutureWatcher<void>(this))
    , isCancelled(false)
{
    connect(watcher, &QFutureWatcher<void>::finished, this, &DirectoryTreeReader::readingFinished);
}

DirectoryTreeReader::~DirectoryTreeReader()
{
    if (watcher->isRunning()) {
        watcher->cancel();
        watcher->waitForFinished();
    }
}

void DirectoryTreeReader::setTreeWidget(QTreeWidget *treeWidget)
{
    this->treeWidget = treeWidget;
}

void DirectoryTreeReader::setMaxDepth(int depth)
{
    maxDepth = depth;
}

void DirectoryTreeReader::setReadFiles(bool readFiles)
{
    this->readFiles = readFiles;
}

void DirectoryTreeReader::setFilterPattern(const QString &pattern, bool isRegex)
{
    filterPattern = pattern;
    useRegex = isRegex;
}

void DirectoryTreeReader::setFilterRules(const QStringList &rules)
{
    filterRules = rules;
}

void DirectoryTreeReader::startReading(const QString &rootPath)
{
    if (!treeWidget) {
        qWarning() << "TreeWidget is not set!";
        return;
    }

    // 清空之前的内容
    treeWidget->clear();
    isCancelled = false;
    
    // 在后台线程中执行目录读取
    QFuture<void> future = QtConcurrent::run([this, rootPath]() {
        populateTreeWidget(rootPath);
    });
    
    watcher->setFuture(future);
}

void DirectoryTreeReader::cancelReading()
{
    isCancelled = true;
    if (watcher->isRunning()) {
        watcher->cancel();
        watcher->waitForFinished();
    }
    emit readingFinished();
}

bool DirectoryTreeReader::isRunning() const
{
    return watcher->isRunning();
}

void DirectoryTreeReader::populateTreeWidget(const QString &rootPath)
{
    QDir rootDir(rootPath);
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(QStringList() << rootDir.dirName() << "目录" << rootPath);
    rootItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    
    // 在主线程中安全地添加根项
    QMetaObject::invokeMethod(treeWidget, [this, rootItem](){
        treeWidget->addTopLevelItem(rootItem);
    }, Qt::QueuedConnection);
    
    // 开始递归读取
    readDirectory(rootPath, rootItem, 1);
}

void DirectoryTreeReader::readDirectory(const QString &path, QTreeWidgetItem *parent, int currentDepth)
{
    if (isCancelled || currentDepth > maxDepth) {
        return;
    }

    QDir dir(path);
    QFileInfoList entries;
    
    // 根据用户选择决定是否读取文件
    if (readFiles) {
        entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
    } else {
        entries = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    }
    
    int total = entries.size();
    int processed = 0;
    
    for (const QFileInfo &info : entries) {
        if (isCancelled) {
            return;
        }
        
        // 更新进度
        processed++;
        int progressValue = (processed * 100) / total;
        emit progressUpdated(progressValue);
        
        QString entryName = info.fileName();
        QString entryPath = info.filePath();
        
        // 检查是否匹配过滤规则
        if (!filterRules.isEmpty() && matchesFilterRules(entryPath)) {
            continue;
        }
        
        // 如果是文件且启用了过滤，则检查是否匹配
        if (info.isFile() && !filterPattern.isEmpty() && !matchesFilter(entryName)) {
            continue;
        }
        
        // 创建树项
        QTreeWidgetItem *item = new QTreeWidgetItem();
        item->setText(0, entryName);
        item->setText(2, entryPath);
        item->setFlags(item->flags() | Qt::ItemIsEditable);
        
        if (info.isDir()) {
            item->setText(1, "目录");
            item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
            
            // 在主线程中安全地添加子项
            QMetaObject::invokeMethod(treeWidget, [parent, item](){
                parent->addChild(item);
            }, Qt::QueuedConnection);
            
            // 递归处理子目录
            readDirectory(entryPath, item, currentDepth + 1);
        } else {
            item->setText(1, "文件");
            item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_FileIcon));
            
            // 在主线程中安全地添加子项
            QMetaObject::invokeMethod(treeWidget, [parent, item](){
                parent->addChild(item);
            }, Qt::QueuedConnection);
        }
    }
}

bool DirectoryTreeReader::matchesFilter(const QString &fileName)
{
    if (filterPattern.isEmpty()) {
        return true;
    }
    
    if (!useRegex) {
        // 使用通配符匹配
        QRegularExpression wildcard(QRegularExpression::wildcardToRegularExpression(filterPattern));
        return wildcard.match(fileName).hasMatch();
    } else {
        // 使用正则表达式匹配
        QRegularExpression regex(filterPattern);
        return regex.match(fileName).hasMatch();
    }
}

bool DirectoryTreeReader::matchesFilterRules(const QString &path)
{
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
        if (regex.match(path).hasMatch()) {
            return true; // 匹配到过滤规则
        }
    }
    
    return false;
}

QString DirectoryTreeReader::generateTextRepresentation(QTreeWidgetItem *item, int level)
{
    if (!item && treeWidget && treeWidget->topLevelItemCount() > 0) {
        item = treeWidget->topLevelItem(0);
    }
    
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