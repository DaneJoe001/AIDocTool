#include "filefilterutil.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QDebug>

FileFilterUtil::FileFilterUtil()
{
}

void FileFilterUtil::addFilterRule(const FilterRule &rule)
{
    m_filterRules.append(rule);
}

void FileFilterUtil::addFilterRule(const QString &pattern, MatchType matchType, FilterMode filterMode, bool enabled)
{
    FilterRule rule(pattern, matchType, filterMode, enabled);
    m_filterRules.append(rule);
}

void FileFilterUtil::setFilterRules(const QList<FilterRule> &rules)
{
    m_filterRules = rules;
}

QList<FileFilterUtil::FilterRule> FileFilterUtil::getFilterRules() const
{
    return m_filterRules;
}

bool FileFilterUtil::removeFilterRule(int index)
{
    if (index >= 0 && index < m_filterRules.size()) {
        m_filterRules.removeAt(index);
        return true;
    }
    return false;
}

bool FileFilterUtil::setRuleEnabled(int index, bool enabled)
{
    if (index >= 0 && index < m_filterRules.size()) {
        m_filterRules[index].enabled = enabled;
        return true;
    }
    return false;
}

void FileFilterUtil::clearFilterRules()
{
    m_filterRules.clear();
}

bool FileFilterUtil::shouldIncludeFile(const QString &fileName, const QString &filePath) const
{
    // 如果没有过滤规则，则包含所有文件
    if (m_filterRules.isEmpty()) {
        return true;
    }
    
    bool shouldInclude = true; // 默认包含
    bool hasIncludeRules = false;
    
    // 首先检查是否有包含规则
    for (const FilterRule &rule : m_filterRules) {
        if (!rule.enabled) continue;
        
        if (rule.filterMode == FilterMode::Include) {
            hasIncludeRules = true;
            if (matchesRule(fileName, filePath, rule)) {
                return true; // 匹配到包含规则，直接包含
            }
        }
    }
    
    // 如果有包含规则但都不匹配，则默认排除
    if (hasIncludeRules) {
        shouldInclude = false;
    }
    
    // 然后检查排除规则
    for (const FilterRule &rule : m_filterRules) {
        if (!rule.enabled) continue;
        
        if (rule.filterMode == FilterMode::Exclude) {
            if (matchesRule(fileName, filePath, rule)) {
                return false; // 匹配到排除规则，直接排除
            }
        }
    }
    
    return shouldInclude;
}

bool FileFilterUtil::shouldExcludeFile(const QString &fileName, const QString &filePath) const
{
    return !shouldIncludeFile(fileName, filePath);
}

bool FileFilterUtil::matchesRule(const QString &fileName, const QString &filePath, const FilterRule &rule) const
{
    // 优先使用完整路径进行匹配
    QString textToMatch = filePath.isEmpty() ? fileName : filePath;
    
    // 对于目录路径，确保路径以斜杠结尾
    if (!textToMatch.isEmpty() && QFileInfo(textToMatch).isDir() && !textToMatch.endsWith('/') && !textToMatch.endsWith('\\')) {
        textToMatch += '/';
    }
    
    if (rule.matchType == MatchType::Wildcard) {
        // 对于通配符，简单使用字符串比较方法
        if (rule.pattern == "*") {
            // 通配符 * 匹配所有内容
            return true;
        }
        
        // 处理常见的通配符情况
        if (rule.pattern.startsWith('*') && rule.pattern.endsWith('*')) {
            // *xxx* 形式，匹配包含xxx的字符串
            QString pattern = rule.pattern.mid(1, rule.pattern.length() - 2);
            return textToMatch.contains(pattern, Qt::CaseInsensitive);
        } else if (rule.pattern.startsWith('*')) {
            // *xxx 形式，匹配以xxx结尾的字符串
            QString pattern = rule.pattern.mid(1);
            return textToMatch.endsWith(pattern, Qt::CaseInsensitive);
        } else if (rule.pattern.endsWith('*')) {
            // xxx* 形式，匹配以xxx开头的字符串
            QString pattern = rule.pattern.left(rule.pattern.length() - 1);
            return textToMatch.startsWith(pattern, Qt::CaseInsensitive);
        } else if (rule.pattern.contains('*')) {
            // 复杂通配符，使用QRegularExpression
            QString pattern = QRegularExpression::escape(rule.pattern);
            pattern.replace("\\*", ".*");
            QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
            return regex.match(textToMatch).hasMatch();
        } else {
            // 没有通配符，直接比较
            return textToMatch.compare(rule.pattern, Qt::CaseInsensitive) == 0;
        }
    } else {
        // 使用正则表达式匹配
        QRegularExpression regex(rule.pattern, QRegularExpression::CaseInsensitiveOption);
        return regex.match(textToMatch).hasMatch();
    }
}
