# AI文档工具集 - 待办事项

## 紧急修复

### 过滤规则功能

1. **修复包含规则处理逻辑**
   - 问题：当使用包含规则（Include）时，文件过滤不正确
   - 文件：`source/filefilterutil.cpp`
   - 方法：`shouldIncludeFile()` 和 `shouldExcludeFile()`
   - 解决方案：重新检查规则应用的优先级和逻辑，确保包含规则正确覆盖排除规则

2. **修复目录过滤问题**
   - 问题：过滤规则对目录项的处理不正确，特别是当规则指定了特定目录时
   - 文件：`source/directorytreereader.cpp`
   - 方法：`readDirectory()`
   - 解决方案：在处理目录项时添加特殊逻辑，确保正确应用过滤规则

3. **优化通配符匹配算法**
   - 问题：某些复杂的通配符模式可能无法正确匹配
   - 文件：`source/filefilterutil.cpp`
   - 方法：`matchesRule()`
   - 解决方案：改进通配符处理逻辑，支持更复杂的模式匹配

## 功能改进

### 用户界面优化

1. **规则列表增强**
   - 为规则列表项添加上下文菜单（右键菜单）
   - 添加编辑、禁用/启用、删除等操作
   - 实现拖放功能，允许用户调整规则优先级

2. **规则导入/导出功能**
   - 添加批量导入/导出规则的功能
   - 支持多种格式（如.gitignore, JSON等）

3. **规则预设功能**
   - 添加常用规则预设（如忽略编译文件、忽略版本控制文件等）
   - 允许用户保存和加载自定义预设

### 性能优化

1. **优化大目录处理**
   - 改进大目录的读取和过滤性能
   - 添加进度反馈和取消机制

2. **优化内存使用**
   - 减少不必要的内存分配
   - 改进大文件处理方式

## 测试计划

1. **单元测试**
   - 为`FileFilterUtil`类添加更多单元测试
   - 测试各种过滤规则组合和边缘情况

2. **集成测试**
   - 测试过滤规则在目录树读取和文件合并中的应用
   - 验证UI组件和后端逻辑的正确交互

## 文档

1. **更新用户文档**
   - 添加过滤规则使用指南
   - 提供常见问题解答和最佳实践

2. **更新开发者文档**
   - 记录过滤规则系统的设计和实现
   - 提供扩展指南 