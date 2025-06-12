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
    , isCancelled(false)
{
}

DirectoryTreeReader::~DirectoryTreeReader()
{
}

void DirectoryTreeReader::setTreeWidget(QTreeWidget *treeWidget)
{
    this->treeWidget = treeWidget;
}

void DirectoryTreeReader::setMaxDepth(int depth)
{
    maxDepth = qMax(1, depth);
}

void DirectoryTreeReader::setReadFiles(bool readFiles)
{
    this->readFiles = readFiles;
}

void DirectoryTreeReader::setFilterRules(const QList<FileFilterUtil::FilterRule> &rules)
{
    fileFilter.setFilterRules(rules);
}

QList<FileFilterUtil::FilterRule> DirectoryTreeReader::getFilterRules() const
{
    return fileFilter.getFilterRules();
}

void DirectoryTreeReader::read(const QString &rootPath)
{
    if (!treeWidget) {
        return;
    }
    
    // 重置状态
    isCancelled = false;
    treeWidget->clear();
    
    // 创建根项
    QDir rootDir(rootPath);
    QTreeWidgetItem *rootItem = new QTreeWidgetItem(QStringList() << rootDir.dirName() << "目录" << rootPath);
    rootItem->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_DirIcon));
    
    // 添加根项
    treeWidget->addTopLevelItem(rootItem);
    
    // 开始递归读取
    readDirectory(rootPath, rootItem, 1);
    
    // 发送完成信号
    emit readingFinished();
}

void DirectoryTreeReader::cancel()
{
    isCancelled = true;
}

QString DirectoryTreeReader::generateTextRepresentation()
{
    if (!treeWidget || treeWidget->topLevelItemCount() == 0) {
        return QString();
    }
    
    return generateTextRepresentation(treeWidget->topLevelItem(0));
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
    int excluded = 0;
    
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
        
        // 检查是否应该排除此文件/目录
        if (fileFilter.shouldExcludeFile(entryName, entryPath)) {
            // 仅在顶层目录输出排除信息，避免过多输出
            if (currentDepth == 1) {
                qDebug() << "排除:" << entryName << "(过滤规则匹配)";
            }
            excluded++;
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
            parent->addChild(item);
            
            // 递归处理子目录
            readDirectory(entryPath, item, currentDepth + 1);
        } else {
            item->setText(1, "文件");
            item->setIcon(0, QApplication::style()->standardIcon(QStyle::SP_FileIcon));
            parent->addChild(item);
        }
    }
    
    // 仅在顶层目录输出排除统计
    if (currentDepth == 1 && excluded > 0) {
        qDebug() << "共排除" << excluded << "个文件/目录";
    }
}

QString DirectoryTreeReader::generateTextRepresentation(QTreeWidgetItem *item, int level)
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