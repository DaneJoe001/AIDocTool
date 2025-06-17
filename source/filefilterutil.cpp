#include "filefilterutil.h"

#include <QRegularExpression>
#include <QFileInfo>
#include <QDebug>
#include <QDir>

namespace fs = std::filesystem;

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
    
    // 打印当前规则列表，方便调试
    qDebug() << "设置过滤规则列表:";
    for (const FilterRule &rule : rules) {
        QString modeStr = (rule.filterMode == FilterMode::Include) ? "包含" : "排除";
        QString typeStr = (rule.matchType == MatchType::Wildcard) ? "通配符" : "正则表达式";
        QString enabledStr = rule.enabled ? "启用" : "禁用";
        qDebug() << "  - 规则:" << rule.pattern << "|" << modeStr << "|" << typeStr << "|" << enabledStr;
    }
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

QString FileFilterUtil::normalizePath(const QString &path) const
{
    // 将路径转换为标准格式，统一使用正斜杠
    QString normalized = QDir::cleanPath(path);
    normalized.replace('\\', '/');
    
    // 确保目录路径以/结尾
    if (!normalized.isEmpty()) {
        QFileInfo fileInfo(normalized);
        if (fileInfo.isDir() && !normalized.endsWith('/')) {
            normalized += '/';
        }
    }
    
    return normalized;
}

bool FileFilterUtil::isSubPath(const QString &path, const QString &basePath) const
{
    QString normalizedPath = normalizePath(path);
    QString normalizedBasePath = normalizePath(basePath);
    
    // 如果基础路径不以/结尾，添加/以确保完整匹配目录名
    if (!normalizedBasePath.endsWith('/')) {
        normalizedBasePath += '/';
    }
    
    // 检查path是否是basePath的子路径
    return normalizedPath.startsWith(normalizedBasePath, Qt::CaseInsensitive);
}

bool FileFilterUtil::shouldIncludeFile(const QString &fileName, const QString &filePath) const
{
    // 如果没有过滤规则，则包含所有文件
    if (m_filterRules.isEmpty()) {
        return true;
    }
    
    // 是否为目录的判断，用于调试输出
    bool isDirectory = false;
    QString normalizedPath = filePath;
    if (!filePath.isEmpty()) {
        QFileInfo fileInfo(filePath);
        isDirectory = fileInfo.isDir();
        normalizedPath = normalizePath(filePath);
    }
    
    // 仅在深度较小时输出调试信息，避免过多输出
    bool enableDebug = normalizedPath.count('/') < 3;
    QString entryType = isDirectory ? "目录" : "文件";
    if (enableDebug) {
        qDebug() << "检查" << entryType << ":" << (normalizedPath.isEmpty() ? fileName : normalizedPath);
    }
    
    bool shouldInclude = true; // 默认包含
    bool hasIncludeRules = false;
    
    // 首先处理build目录的特殊情况，为了提高性能
    if (isDirectory) {
        // 检查是否是build目录或其子目录
        QString name = QFileInfo(normalizedPath).fileName().toLower();
        
        // 特殊处理build目录
        if (name == "build" || normalizedPath.contains("/build/", Qt::CaseInsensitive)) {
            // 检查是否有明确包含build目录的规则
            bool hasBuildIncludeRule = false;
            for (const FilterRule &rule : m_filterRules) {
                if (!rule.enabled) continue;
                
                if (rule.filterMode == FilterMode::Include) {
                    if (rule.pattern.contains("build", Qt::CaseInsensitive)) {
                        hasBuildIncludeRule = true;
                        break;
                    }
                }
            }
            
            // 如果没有明确包含build目录的规则，直接排除
            if (!hasBuildIncludeRule) {
                if (enableDebug) {
                    qDebug() << "  -> 特殊处理: build目录，结果: 排除";
                }
                return false;
            }
        }
    }
    
    // 首先检查是否有包含规则
    for (const FilterRule &rule : m_filterRules) {
        if (!rule.enabled) continue;
        
        if (rule.filterMode == FilterMode::Include) {
            hasIncludeRules = true;
            if (matchesRule(fileName, normalizedPath, rule)) {
                if (enableDebug) {
                    qDebug() << "  -> 匹配包含规则:" << rule.pattern << "，结果: 包含";
                }
                return true; // 匹配到包含规则，直接包含
            } else if (enableDebug) {
                qDebug() << "  -> 不匹配包含规则:" << rule.pattern;
            }
        }
    }
    
    // 如果有包含规则但都不匹配，则默认排除
    if (hasIncludeRules) {
        shouldInclude = false;
        if (enableDebug) {
            qDebug() << "  -> 有包含规则但都不匹配，默认排除";
        }
    }
    
    // 目录的特殊处理
    if (isDirectory && hasIncludeRules) {
        // 检查是否有文件类型的包含规则（如*.cpp）
        bool hasFileTypeIncludeRule = false;
        for (const FilterRule &rule : m_filterRules) {
            if (!rule.enabled || rule.filterMode != FilterMode::Include) continue;
            
            if (rule.pattern.startsWith("*.") || 
                (rule.pattern.contains('.') && !rule.pattern.contains('/') && !rule.pattern.contains('\\'))) {
                hasFileTypeIncludeRule = true;
                break;
            }
        }
        
        // 如果有文件类型包含规则，应该允许目录被遍历
        if (hasFileTypeIncludeRule) {
            if (enableDebug) {
                qDebug() << "  -> 存在文件类型包含规则，允许遍历目录";
            }
            return true;
        }
    }
    
    // 然后检查排除规则
    for (const FilterRule &rule : m_filterRules) {
        if (!rule.enabled) continue;
        
        if (rule.filterMode == FilterMode::Exclude) {
            if (matchesRule(fileName, normalizedPath, rule)) {
                if (enableDebug) {
                    qDebug() << "  -> 匹配排除规则:" << rule.pattern << "，结果: 排除";
                }
                return false; // 匹配到排除规则，直接排除
            } else if (enableDebug) {
                qDebug() << "  -> 不匹配排除规则:" << rule.pattern;
            }
        }
    }
    
    if (enableDebug) {
        qDebug() << "  -> 最终结果:" << (shouldInclude ? "包含" : "排除");
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
    textToMatch = normalizePath(textToMatch);
    
    // 获取文件/目录信息
    bool isDirectory = false;
    if (!textToMatch.isEmpty()) {
        QFileInfo fileInfo(textToMatch);
        isDirectory = fileInfo.isDir();
    }
    
    // 仅在深度较小时输出调试信息，避免过多输出
    bool enableDebug = textToMatch.count('/') < 3;
    
    // 处理特殊的目录匹配规则
    if (isDirectory) {
        // 如果规则是针对目录的，使用更强大的路径匹配
        QString pattern = rule.pattern;
        pattern = pattern.replace('\\', '/');
        
        // 处理各种形式的目录路径规则
        if (pattern.endsWith('/') || pattern == "build" || pattern == "build/") {
            // 目录名称匹配
            QString dirName = QFileInfo(textToMatch).fileName();
            if (pattern.endsWith('/')) {
                pattern = pattern.left(pattern.length() - 1);
            }
            
            // 检查是否是指定目录或其子目录
            if (dirName.compare(pattern, Qt::CaseInsensitive) == 0 || 
                textToMatch.contains("/" + pattern + "/", Qt::CaseInsensitive)) {
                if (enableDebug) {
                    qDebug() << "    [目录匹配] 目录名" << dirName << "匹配规则" << pattern;
                }
                return true;
            }
        }
        
        // 处理包含规则的特殊情况
        if (rule.filterMode == FilterMode::Include) {
            // 如果是文件扩展名匹配规则（如*.cpp），目录默认应该被包含
            if (rule.pattern.startsWith("*.") || rule.pattern.startsWith(".")) {
                if (enableDebug) {
                    qDebug() << "    [规则分析] 目录" << textToMatch << "匹配文件扩展名规则" << rule.pattern << "-> 默认包含";
                }
                return true;
            }
            
            // 如果是明确的目录过滤规则（以/结尾），则按规则匹配
            if (rule.pattern.endsWith('/') || rule.pattern.endsWith('\\')) {
                // 继续使用下面的匹配逻辑
                if (enableDebug) {
                    qDebug() << "    [规则分析] 目录" << textToMatch << "遇到目录匹配规则" << rule.pattern << "-> 按规则匹配";
                }
            }
            // 其他情况下的包含规则，如果不是明确针对目录的，目录应默认包含
            else if (!rule.pattern.contains('/') && !rule.pattern.contains('\\')) {
                if (enableDebug) {
                    qDebug() << "    [规则分析] 目录" << textToMatch << "遇到非目录规则" << rule.pattern << "-> 默认包含";
                }
                return true;
            }
        }
    }
    
    bool result = false;
    
    // 使用std::filesystem进行路径匹配
    try {
        // 对于路径模式，尝试使用filesystem进行匹配
        if (rule.pattern.contains('/') || rule.pattern.contains('\\')) {
            QString normalizedPattern = rule.pattern;
            normalizedPattern.replace('\\', '/');
            
            // 如果模式是目录路径（以/结尾）
            if (normalizedPattern.endsWith('/')) {
                normalizedPattern = normalizedPattern.left(normalizedPattern.length() - 1);
                
                // 检查当前路径是否包含此目录
                if (textToMatch.contains("/" + normalizedPattern + "/", Qt::CaseInsensitive) ||
                    textToMatch.endsWith("/" + normalizedPattern, Qt::CaseInsensitive)) {
                    if (enableDebug) {
                        qDebug() << "    [路径匹配] 路径" << textToMatch << "包含目录" << normalizedPattern;
                    }
                    return true;
                }
            }
            
            // 检查当前路径是否匹配或是子路径
            fs::path fsPath = textToMatch.toStdString();
            fs::path fsPattern = normalizedPattern.toStdString();
            
            try {
                // 检查相对路径
                if (normalizedPattern.startsWith('/') || normalizedPattern.startsWith("./") || normalizedPattern.startsWith("../")) {
                    result = (fs::relative(fsPath, fsPattern).empty() || 
                             textToMatch.contains(normalizedPattern, Qt::CaseInsensitive));
                    
                    if (enableDebug) {
                        qDebug() << "    [路径匹配] 相对路径检查:" << result;
                    }
                }
            } catch (const std::exception&) {
                // 相对路径计算失败，回退到字符串匹配
                result = textToMatch.contains(normalizedPattern, Qt::CaseInsensitive);
                if (enableDebug) {
                    qDebug() << "    [路径匹配] 回退到字符串匹配:" << result;
                }
            }
        }
    } catch (const std::exception& e) {
        qDebug() << "路径匹配错误:" << e.what();
    }
    
    // 如果路径匹配已经成功，直接返回
    if (result) {
        return true;
    }
    
    // 标准通配符匹配
    if (rule.matchType == MatchType::Wildcard) {
        // 对于通配符，简单使用字符串比较方法
        if (rule.pattern == "*") {
            // 通配符 * 匹配所有内容
            result = true;
            if (enableDebug) {
                qDebug() << "    [规则匹配] 通配符 * -> 匹配所有内容";
            }
        }
        // 处理常见的通配符情况
        else if (rule.pattern.startsWith('*') && rule.pattern.endsWith('*')) {
            // *xxx* 形式，匹配包含xxx的字符串
            QString pattern = rule.pattern.mid(1, rule.pattern.length() - 2);
            result = textToMatch.contains(pattern, Qt::CaseInsensitive);
            if (enableDebug) {
                qDebug() << "    [规则匹配] 通配符 *" << pattern << "* -> " << (result ? "匹配" : "不匹配");
            }
        } else if (rule.pattern.startsWith('*')) {
            // *xxx 形式，匹配以xxx结尾的字符串
            QString pattern = rule.pattern.mid(1);
            result = textToMatch.endsWith(pattern, Qt::CaseInsensitive);
            if (enableDebug) {
                qDebug() << "    [规则匹配] 通配符 *" << pattern << " -> " << (result ? "匹配" : "不匹配");
            }
        } else if (rule.pattern.endsWith('*')) {
            // xxx* 形式，匹配以xxx开头的字符串
            QString pattern = rule.pattern.left(rule.pattern.length() - 1);
            result = textToMatch.startsWith(pattern, Qt::CaseInsensitive);
            if (enableDebug) {
                qDebug() << "    [规则匹配] 通配符 " << pattern << "* -> " << (result ? "匹配" : "不匹配");
            }
        } else if (rule.pattern.contains('*')) {
            // 复杂通配符，使用QRegularExpression
            QString pattern = QRegularExpression::escape(rule.pattern);
            pattern.replace("\\*", ".*");
            QRegularExpression regex(pattern, QRegularExpression::CaseInsensitiveOption);
            result = regex.match(textToMatch).hasMatch();
            if (enableDebug) {
                qDebug() << "    [规则匹配] 复杂通配符 " << rule.pattern << " -> " << (result ? "匹配" : "不匹配");
            }
        } else {
            // 没有通配符，直接比较
            result = textToMatch.compare(rule.pattern, Qt::CaseInsensitive) == 0;
            if (enableDebug) {
                qDebug() << "    [规则匹配] 精确匹配 " << rule.pattern << " -> " << (result ? "匹配" : "不匹配");
            }
        }
    } else {
        // 使用正则表达式匹配
        QRegularExpression regex(rule.pattern, QRegularExpression::CaseInsensitiveOption);
        result = regex.match(textToMatch).hasMatch();
        if (enableDebug) {
            qDebug() << "    [规则匹配] 正则表达式 " << rule.pattern << " -> " << (result ? "匹配" : "不匹配");
        }
    }
    
    return result;
}
