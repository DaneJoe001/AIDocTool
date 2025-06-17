/**
 * @file filefilterutil.h
 * @brief 文件过滤工具类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef FILEFILTERUTIL_H
#define FILEFILTERUTIL_H

#include <QString>
#include <QStringList>
#include <QRegularExpression>
#include <QMap>
#include <filesystem>

/**
 * @class FileFilterUtil
 * @brief 文件过滤工具类
 * 
 * 该类提供了文件过滤相关的功能，包括通配符和正则表达式匹配，
 * 以及基于规则的过滤和包含功能。
 */
class FileFilterUtil
{
public:
    /**
     * @brief 过滤模式类型
     */
    enum class FilterMode {
        Include,    ///< 包含模式（匹配则包含）
        Exclude     ///< 排除模式（匹配则排除）
    };

    /**
     * @brief 匹配类型
     */
    enum class MatchType {
        Wildcard,   ///< 通配符匹配
        Regex       ///< 正则表达式匹配
    };

    /**
     * @brief 过滤规则结构体
     */
    struct FilterRule {
        QString pattern;     ///< 过滤模式
        MatchType matchType; ///< 匹配类型
        FilterMode filterMode; ///< 过滤模式
        bool enabled;        ///< 是否启用

        FilterRule() : matchType(MatchType::Wildcard), filterMode(FilterMode::Exclude), enabled(true) {}
        FilterRule(const QString &p, MatchType mt, FilterMode fm, bool en = true) 
            : pattern(p), matchType(mt), filterMode(fm), enabled(en) {}
    };

    /**
     * @brief 构造函数
     */
    FileFilterUtil();

    /**
     * @brief 添加过滤规则
     * @param rule 过滤规则
     */
    void addFilterRule(const FilterRule &rule);

    /**
     * @brief 添加过滤规则
     * @param pattern 过滤模式字符串
     * @param matchType 匹配类型
     * @param filterMode 过滤模式
     * @param enabled 是否启用
     */
    void addFilterRule(const QString &pattern, MatchType matchType, FilterMode filterMode, bool enabled = true);

    /**
     * @brief 设置过滤规则列表
     * @param rules 过滤规则列表
     */
    void setFilterRules(const QList<FilterRule> &rules);

    /**
     * @brief 获取过滤规则列表
     * @return 过滤规则列表
     */
    QList<FilterRule> getFilterRules() const;

    /**
     * @brief 移除过滤规则
     * @param index 规则索引
     * @return 是否成功移除
     */
    bool removeFilterRule(int index);

    /**
     * @brief 启用或禁用过滤规则
     * @param index 规则索引
     * @param enabled 是否启用
     * @return 是否成功设置
     */
    bool setRuleEnabled(int index, bool enabled);

    /**
     * @brief 清除所有过滤规则
     */
    void clearFilterRules();

    /**
     * @brief 检查文件是否应该被包含
     * @param fileName 文件名
     * @param filePath 文件路径
     * @return 如果文件应该被包含则返回true，否则返回false
     */
    bool shouldIncludeFile(const QString &fileName, const QString &filePath = QString()) const;

    /**
     * @brief 检查文件是否应该被排除
     * @param fileName 文件名
     * @param filePath 文件路径
     * @return 如果文件应该被排除则返回true，否则返回false
     */
    bool shouldExcludeFile(const QString &fileName, const QString &filePath = QString()) const;

private:
    QList<FilterRule> m_filterRules;   ///< 过滤规则列表

    /**
     * @brief 检查文件是否匹配指定规则
     * @param fileName 文件名
     * @param filePath 文件路径
     * @param rule 过滤规则
     * @return 如果文件匹配规则则返回true，否则返回false
     */
    bool matchesRule(const QString &fileName, const QString &filePath, const FilterRule &rule) const;
    
    /**
     * @brief 规范化路径
     * @param path 输入路径
     * @return 规范化后的路径
     */
    QString normalizePath(const QString &path) const;
    
    /**
     * @brief 检查路径是否是另一个路径的子路径
     * @param path 要检查的路径
     * @param basePath 基础路径
     * @return 如果是子路径则返回true
     */
    bool isSubPath(const QString &path, const QString &basePath) const;
};

#endif // FILEFILTERUTIL_H 