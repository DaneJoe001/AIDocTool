/**
 * @file directorytreereader.h
 * @brief 目录树读取器类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef DIRECTORYTREEREADER_H
#define DIRECTORYTREEREADER_H

#include <QObject>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>

/**
 * @class DirectoryTreeReader
 * @brief 用于读取和显示目录结构的类
 * 
 * 该类负责递归读取指定目录的文件结构，并将其显示在QTreeWidget中。
 * 支持过滤规则、搜索深度限制和文本表示生成。
 */
class DirectoryTreeReader : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit DirectoryTreeReader(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~DirectoryTreeReader();

    /**
     * @brief 设置要显示目录结构的树形控件
     * @param treeWidget 树形控件指针
     */
    void setTreeWidget(QTreeWidget *treeWidget);
    
    /**
     * @brief 设置目录读取的最大深度
     * @param depth 最大深度值
     */
    void setMaxDepth(int depth);
    
    /**
     * @brief 设置是否读取文件（否则只读取目录）
     * @param readFiles 是否读取文件的标志
     */
    void setReadFiles(bool readFiles);
    
    /**
     * @brief 设置文件过滤模式
     * @param pattern 过滤模式字符串
     * @param isRegex 是否使用正则表达式（否则使用通配符）
     */
    void setFilterPattern(const QString &pattern, bool isRegex);
    
    /**
     * @brief 设置类似.gitignore的过滤规则
     * @param rules 过滤规则列表
     */
    void setFilterRules(const QStringList &rules);

    /**
     * @brief 开始读取指定目录
     * @param rootPath 要读取的根目录路径
     */
    void startReading(const QString &rootPath);
    
    /**
     * @brief 取消当前的读取操作
     */
    void cancelReading();
    
    /**
     * @brief 检查是否正在读取目录
     * @return 如果正在读取返回true，否则返回false
     */
    bool isRunning() const;

    /**
     * @brief 生成目录树的文本表示
     * @param item 要表示的树项，如果为nullptr则使用根项
     * @param level 当前项的层级，用于缩进
     * @return 格式化的文本表示
     */
    QString generateTextRepresentation(QTreeWidgetItem *item = nullptr, int level = 0);

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

private:
    QTreeWidget *treeWidget;     ///< 用于显示目录结构的树形控件
    int maxDepth;                ///< 最大读取深度
    bool readFiles;              ///< 是否读取文件（否则只读取目录）
    QString filterPattern;       ///< 文件过滤模式
    bool useRegex;               ///< 是否使用正则表达式过滤
    QStringList filterRules;     ///< 类似.gitignore的过滤规则
    QFutureWatcher<void> *watcher; ///< 用于异步读取的Future监视器
    bool isCancelled;            ///< 是否已取消读取操作

    /**
     * @brief 填充树形控件
     * @param rootPath 根目录路径
     */
    void populateTreeWidget(const QString &rootPath);
    
    /**
     * @brief 递归读取目录
     * @param path 当前目录路径
     * @param parent 父树项
     * @param currentDepth 当前深度
     */
    void readDirectory(const QString &path, QTreeWidgetItem *parent, int currentDepth);
    
    /**
     * @brief 检查文件名是否匹配过滤模式
     * @param fileName 文件名
     * @return 如果匹配返回true，否则返回false
     */
    bool matchesFilter(const QString &fileName);
    
    /**
     * @brief 检查路径是否匹配过滤规则
     * @param path 路径
     * @return 如果匹配返回true，否则返回false
     */
    bool matchesFilterRules(const QString &path);
};

#endif // DIRECTORYTREEREADER_H 