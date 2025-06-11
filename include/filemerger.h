/**
 * @file filemerger.h
 * @brief 文件合并器类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef FILEMERGER_H
#define FILEMERGER_H

#include <QObject>
#include <QStringList>
#include <QRegularExpression>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>

/**
 * @class FileMerger
 * @brief 用于搜索和合并文本文件的类
 * 
 * 该类负责递归搜索指定目录中的文本文件，
 * 并根据用户定义的规则合并它们的内容。
 */
class FileMerger : public QObject
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父对象指针
     */
    explicit FileMerger(QObject *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~FileMerger();

    /**
     * @brief 设置搜索的根目录
     * @param path 根目录路径
     */
    void setRootPath(const QString &path);
    
    /**
     * @brief 设置搜索深度
     * @param depth 最大搜索深度
     */
    void setMaxDepth(int depth);
    
    /**
     * @brief 设置文件过滤模式
     * @param pattern 过滤模式字符串
     * @param isRegex 是否使用正则表达式（否则使用通配符）
     */
    void setFileFilter(const QString &pattern, bool isRegex);
    
    /**
     * @brief 设置文件包含规则
     * @param rules 包含规则列表，匹配这些规则的文件将被包含
     */
    void setFilterRules(const QStringList &rules);
    
    /**
     * @brief 设置文件头模板
     * @param headerTemplate 文件头模板，可包含{filename}、{index}、{path}等占位符
     */
    void setHeaderTemplate(const QString &headerTemplate);
    
    /**
     * @brief 设置是否在文件间添加分隔符
     * @param enabled 是否启用
     * @param separator 分隔符文本
     */
    void setSeparator(bool enabled, const QString &separator = "----------");
    
    /**
     * @brief 设置内容提取规则
     * @param regex 正则表达式，用于提取文件的特定部分
     * @param enabled 是否启用正则提取
     */
    void setExtractionRule(const QString &regex, bool enabled);
    
    /**
     * @brief 开始搜索和合并文件
     */
    void startMerging();
    
    /**
     * @brief 取消当前操作
     */
    void cancelOperation();
    
    /**
     * @brief 检查是否正在处理
     * @return 如果正在处理返回true，否则返回false
     */
    bool isRunning() const;
    
    /**
     * @brief 获取合并后的文本
     * @return 合并后的文本内容
     */
    QString getMergedText() const;
    
    /**
     * @brief 导出合并后的文本到文件
     * @param filePath 文件路径
     * @return 是否成功导出
     */
    bool exportToFile(const QString &filePath) const;

signals:
    /**
     * @brief 进度更新信号
     * @param value 进度值（0-100）
     */
    void progressUpdated(int value);
    
    /**
     * @brief 处理完成信号
     * @param fileCount 处理的文件数量
     */
    void mergingFinished(int fileCount);
    
    /**
     * @brief 文件处理信号
     * @param filePath 当前处理的文件路径
     */
    void processingFile(const QString &filePath);

private:
    QString rootPath;                ///< 搜索的根目录
    int maxDepth;                    ///< 最大搜索深度
    QString fileFilter;              ///< 文件过滤模式
    bool useRegex;                   ///< 是否使用正则表达式过滤
    QStringList filterRules;         ///< 文件包含规则列表
    QString headerTemplate;          ///< 文件头模板
    bool useSeparator;               ///< 是否使用分隔符
    QString separator;               ///< 文件间分隔符
    QString extractionRegex;         ///< 内容提取正则表达式
    bool useExtraction;              ///< 是否使用内容提取
    QFutureWatcher<void> *watcher;   ///< 用于异步处理的Future监视器
    bool isCancelled;                ///< 是否已取消操作
    QString mergedText;              ///< 合并后的文本
    QStringList foundFiles;          ///< 找到的文件列表

    /**
     * @brief 递归搜索文件
     * @param path 当前目录路径
     * @param currentDepth 当前深度
     */
    void searchFiles(const QString &path, int currentDepth);
    
    /**
     * @brief 合并文件内容
     */
    void mergeFiles();
    
    /**
     * @brief 判断是否应该包含指定文件
     * @param fileName 文件名
     * @param filePath 文件完整路径
     * @return 如果应该包含返回true，否则返回false
     */
    bool shouldIncludeFile(const QString &fileName, const QString &filePath) const;
    
    /**
     * @brief 从文件中提取内容
     * @param content 原始文件内容
     * @return 提取后的内容
     */
    QString extractContent(const QString &content) const;
    
    /**
     * @brief 生成文件头
     * @param filePath 文件路径
     * @param index 文件索引
     * @return 生成的文件头
     */
    QString generateHeader(const QString &filePath, int index) const;
};

#endif // FILEMERGER_H 