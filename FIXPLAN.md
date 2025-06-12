# 过滤规则功能修复计划

## 问题分析

在当前实现中，过滤规则功能存在以下主要问题：

1. **包含规则（Include）不正确工作**
   - 当添加包含规则时，预期只有匹配该规则的文件会被显示，但实际上没有正确应用
   - 包含规则与排除规则的优先级处理不正确

2. **目录过滤逻辑不正确**
   - 当规则指定目录时，没有正确处理目录路径的匹配
   - 子目录的过滤规则应用不一致

3. **通配符匹配算法存在问题**
   - 某些复杂通配符模式无法正确匹配
   - 通配符转正则表达式的处理有问题

## 修复方案

### 1. 修复包含规则处理逻辑

修改 `FileFilterUtil::shouldIncludeFile()` 方法，实现以下逻辑：

```cpp
bool FileFilterUtil::shouldIncludeFile(const QString &fileName, const QString &filePath) const
{
    // 如果没有过滤规则，则包含所有文件
    if (m_filterRules.isEmpty()) {
        return true;
    }
    
    // 首先检查是否有任何包含规则
    bool hasIncludeRules = false;
    for (const FilterRule &rule : m_filterRules) {
        if (rule.enabled && rule.filterMode == FilterMode::Include) {
            hasIncludeRules = true;
            break;
        }
    }
    
    // 如果有包含规则，默认排除所有文件，除非匹配某个包含规则
    bool shouldInclude = !hasIncludeRules;
    
    // 首先应用所有包含规则
    for (const FilterRule &rule : m_filterRules) {
        if (!rule.enabled) continue;
        
        if (rule.filterMode == FilterMode::Include && matchesRule(fileName, filePath, rule)) {
            shouldInclude = true;
            break; // 一旦匹配某个包含规则，就立即包含此文件
        }
    }
    
    // 如果文件被包含规则匹配，再检查是否被任何排除规则匹配
    if (shouldInclude) {
        for (const FilterRule &rule : m_filterRules) {
            if (!rule.enabled) continue;
            
            if (rule.filterMode == FilterMode::Exclude && matchesRule(fileName, filePath, rule)) {
                shouldInclude = false;
                break; // 一旦匹配某个排除规则，就立即排除此文件
            }
        }
    }
    
    return shouldInclude;
}
```

### 2. 修复目录过滤问题

修改 `DirectoryTreeReader::readDirectory()` 方法中的目录处理逻辑：

```cpp
void DirectoryTreeReader::readDirectory(const QString &path, QTreeWidgetItem *parent, int currentDepth)
{
    // ... 现有代码 ...
    
    for (const QFileInfo &info : entries) {
        // ... 现有代码 ...
        
        QString entryName = info.fileName();
        QString entryPath = info.filePath();
        
        // 特殊处理目录路径，确保以斜杠结尾
        if (info.isDir() && !entryPath.endsWith('/') && !entryPath.endsWith('\\')) {
            entryPath += '/';
        }
        
        // 检查是否应该排除此文件/目录
        if (fileFilter.shouldExcludeFile(entryName, entryPath)) {
            // ... 现有代码 ...
            continue;
        }
        
        // ... 现有代码 ...
    }
}
```

### 3. 优化通配符匹配算法

修改 `FileFilterUtil::matchesRule()` 方法，实现更健壮的通配符匹配：

```cpp
bool FileFilterUtil::matchesRule(const QString &fileName, const QString &filePath, const FilterRule &rule) const
{
    // 优先使用完整路径进行匹配
    QString textToMatch = filePath.isEmpty() ? fileName : filePath;
    
    if (rule.matchType == MatchType::Wildcard) {
        QString pattern = rule.pattern;
        
        // 处理特殊通配符情况
        if (pattern == "*") {
            return true; // 匹配所有
        }
        
        // 处理目录通配符
        if (pattern.endsWith("/*") || pattern.endsWith("\\*")) {
            // 移除末尾的/*或\*
            QString dirPattern = pattern.left(pattern.length() - 2);
            
            // 检查是否为目录匹配
            if (textToMatch.startsWith(dirPattern)) {
                return true;
            }
        }
        
        // 处理常见通配符模式
        if (pattern.startsWith('*') && pattern.endsWith('*')) {
            QString subPattern = pattern.mid(1, pattern.length() - 2);
            return textToMatch.contains(subPattern, Qt::CaseInsensitive);
        } else if (pattern.startsWith('*')) {
            QString subPattern = pattern.mid(1);
            return textToMatch.endsWith(subPattern, Qt::CaseInsensitive);
        } else if (pattern.endsWith('*')) {
            QString subPattern = pattern.left(pattern.length() - 1);
            return textToMatch.startsWith(subPattern, Qt::CaseInsensitive);
        }
        
        // 对于更复杂的通配符，使用QRegularExpression
        QString regexPattern = QRegularExpression::escape(pattern);
        regexPattern.replace("\\*", ".*");
        regexPattern.replace("\\?", ".");
        QRegularExpression regex("^" + regexPattern + "$", QRegularExpression::CaseInsensitiveOption);
        return regex.match(textToMatch).hasMatch();
    } else {
        // 正则表达式匹配
        QRegularExpression regex(rule.pattern, QRegularExpression::CaseInsensitiveOption);
        return regex.match(textToMatch).hasMatch();
    }
}
```

## 测试计划

1. **包含规则测试**
   - 添加只包含`.cpp`文件的规则，验证只有`.cpp`文件被显示
   - 添加包含和排除规则的组合，验证优先级处理

2. **目录过滤测试**
   - 添加排除特定目录的规则，验证整个目录被排除
   - 添加包含特定目录的规则，验证只有该目录下的文件被显示

3. **通配符匹配测试**
   - 测试各种通配符模式：`*`, `*.cpp`, `src/*`, `*test*.*`等
   - 测试复杂通配符：`src/*/test_*.cpp`

## 实施计划

1. 首先修复`FileFilterUtil`类中的规则处理逻辑
2. 然后修复`DirectoryTreeReader`中的目录处理逻辑
3. 最后优化通配符匹配算法
4. 进行全面测试，确保所有问题都已解决 