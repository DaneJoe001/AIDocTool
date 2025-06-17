/**
 * @file directorytreereader.h
 * @brief 目录树读取器类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef DIRECTORYTREEREADER_H
#define DIRECTORYTREEREADER_H

#include "filefilterutil.h"

#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDir>
#include <QFileInfo>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QApplication>
#include <QStyle>

/**
 * @class DirectoryTreeReader
 * @brief 目录树读取器类
 * 
 * 该类用于递归读取目录结构，并将其显示在树形控件中。
 * 使用多线程技术在后台执行目录读取操作，避免UI阻塞。
 */
class DirectoryTreeReader : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象
     */
    explicit DirectoryTreeReader(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~DirectoryTreeReader();
    
    /**
     * @brief 设置树形控件
     * @param treeWidget 树形控件指针
     */
    void setTreeWidget(QTreeWidget *treeWidget);
    
    /**
     * @brief 设置最大搜索深度
     * @param depth 最大深度
     */
    void setMaxDepth(int depth);
    
    /**
     * @brief 设置是否读取文件
     * @param readFiles 是否读取文件
     */
    void setReadFiles(bool readFiles);
    
    /**
     * @brief 设置过滤规则
     * @param rules 过滤规则列表
     */
    void setFilterRules(const QList<FileFilterUtil::FilterRule> &rules);
    
    /**
     * @brief 获取过滤规则
     * @return 过滤规则列表
     */
    QList<FileFilterUtil::FilterRule> getFilterRules() const;
    
    /**
     * @brief 读取目录
     * @param rootPath 根目录路径
     */
    void read(const QString &rootPath);
    
    /**
     * @brief 取消读取
     */
    void cancel();
    
    /**
     * @brief 生成文本表示
     * @return 目录结构的文本表示
     */
    QString generateTextRepresentation();

signals:
    /**
     * @brief 进度更新信号
     * @param value 进度值（0-100）
     */
    void progressUpdated(int value);
    
    /**
     * @brief 读取完成信号
     */
    void readingFinished();

private slots:
    /**
     * @brief 读取完成槽函数
     */
    void onReadingFinished();

private:
    QTreeWidget *treeWidget;      ///< 树形控件
    QTreeWidgetItem *rootItem;    ///< 根节点项
    int maxDepth;                 ///< 最大搜索深度
    bool readFiles;               ///< 是否读取文件
    bool isCancelled;             ///< 是否已取消
    FileFilterUtil fileFilter;    ///< 文件过滤工具
    QFutureWatcher<void> *watcher; ///< 异步任务监视器
    
    /**
     * @brief 递归读取目录
     * @param path 目录路径
     * @param parent 父树项
     * @param currentDepth 当前深度
     */
    void readDirectory(const QString &path, QTreeWidgetItem *parent, int currentDepth);
    
    /**
     * @brief 生成文本表示
     * @param item 树项
     * @param level 缩进级别
     * @return 目录结构的文本表示
     */
    QString generateTextRepresentation(QTreeWidgetItem *item, int level = 0);
};

#endif // DIRECTORYTREEREADER_H 