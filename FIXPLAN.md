# AI文档工具集 - 修复计划

## 过滤规则功能修复

### 1. 修复包含规则处理逻辑 [已修复]

**问题描述**：当使用包含规则（Include）时，文件过滤不正确。目前的实现可能导致包含规则被排除规则覆盖，或者包含规则无法正确应用。

**解决方案**：

✅ 已于2024-06-16实现修复，采用以下方法：

1. 重写了 `FileFilterUtil::shouldIncludeFile()` 方法，确保包含规则优先级正确，并区分文件和目录处理。
2. 在判断包含规则时，增加了对规则类型的检查，特别是对文件类型包含规则的特殊处理。
3. 使用规范化路径处理，确保路径格式一致性。
4. 添加了详细的调试输出，帮助诊断匹配过程。

实现细节见源码。

### 2. 修复目录过滤问题 [已修复]

**问题描述**：过滤规则对目录项的处理不正确，特别是当规则指定了特定目录时。当使用包含规则（如 *.cpp）时，由于目录不匹配这些规则，导致过早退出目录遍历，无法找到深层目录中的匹配文件。

**解决方案**：

✅ 已于2024-06-16实现修复，采用以下方法：

1. 使用C++17的std::filesystem增强路径处理能力
2. 添加了目录的特殊处理逻辑：
   - 当存在文件类型包含规则时，允许继续遍历目录以查找匹配文件
   - 对build目录实现了智能自动排除
   - 针对不同形式的目录路径规则（如build/、/build、build）进行了优化处理
3. 改进了目录匹配算法，对目录应用更适合的过滤逻辑，避免过早退出遍历
4. 在 `DirectoryTreeReader::readDirectory()` 方法中增强了目录过滤决策

实现后，程序现在能够正确处理以下情况：
- 当存在"*.cpp"等文件类型包含规则时，会继续遍历所有目录查找匹配文件
- 可以使用多种形式的路径规则正确过滤目录
- build目录会被自动排除（除非明确包含）

### 3. 优化通配符匹配算法

**问题描述**：某些复杂的通配符模式可能无法正确匹配。

**解决方案**：

1. 改进 `FileFilterUtil::matchesRule()` 方法中的通配符处理：

```cpp
bool FileFilterUtil::matchesRule(const QString &filePath, const FilterRule &rule) const {
    if (!rule.enabled) {
        return false;
    }
    
    if (rule.matchType == MatchType::Wildcard) {
        // 改进的通配符匹配
        QStringList patterns = rule.pattern.split(';', Qt::SkipEmptyParts);
        for (const QString &pattern : patterns) {
            QRegularExpression regex(QRegularExpression::wildcardToRegularExpression(pattern.trimmed()));
            if (regex.match(filePath).hasMatch()) {
                return true;
            }
        }
        return false;
    } else if (rule.matchType == MatchType::Regex) {
        // 正则表达式匹配
        QRegularExpression regex(rule.pattern);
        return regex.match(filePath).hasMatch();
    }
    
    return false;
}
```

## 界面重新设计实现计划

### 1. 创建新的类结构

#### 1.1 核心界面类

1. **SidebarWidget** - 侧边栏导航组件
```cpp
// include/sidebarwidget.h
#ifndef SIDEBARWIDGET_H
#define SIDEBARWIDGET_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QIcon>

class SidebarWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SidebarWidget(QWidget *parent = nullptr);
    ~SidebarWidget();

    // 添加模块到侧边栏
    int addModule(const QString &name, const QIcon &icon, QWidget *moduleWidget);
    
    // 获取当前选中的模块索引
    int currentIndex() const;
    
    // 设置当前模块
    void setCurrentIndex(int index);

signals:
    void moduleSelected(int index);

private:
    void setupUI();
    void createConnections();

    QListWidget *m_moduleList;
    QStackedWidget *m_detailsWidget;
};

#endif // SIDEBARWIDGET_H
```

2. **ModuleBase** - 所有功能模块的基类
```cpp
// include/modulebase.h
#ifndef MODULEBASE_H
#define MODULEBASE_H

#include <QWidget>
#include <QString>
#include <QIcon>

class ModuleBase : public QWidget
{
    Q_OBJECT

public:
    explicit ModuleBase(QWidget *parent = nullptr);
    virtual ~ModuleBase();

    // 获取模块名称
    virtual QString moduleName() const = 0;
    
    // 获取模块图标
    virtual QIcon moduleIcon() const = 0;
    
    // 模块激活时调用
    virtual void onActivate();
    
    // 模块停用时调用
    virtual void onDeactivate();
    
    // 保存模块状态
    virtual void saveState();
    
    // 恢复模块状态
    virtual void restoreState();
};

#endif // MODULEBASE_H
```

#### 1.2 功能模块类

1. **FileBrowserModule** - 文件浏览与分析模块
```cpp
// include/filebrowsermodule.h
#ifndef FILEBROWSERMODULE_H
#define FILEBROWSERMODULE_H

#include "modulebase.h"
#include "directorytreereader.h"
#include "filterrulelistwidget.h"

class FileBrowserModule : public ModuleBase
{
    Q_OBJECT

public:
    explicit FileBrowserModule(QWidget *parent = nullptr);
    ~FileBrowserModule();

    QString moduleName() const override { return tr("文件浏览与分析"); }
    QIcon moduleIcon() const override;
    
    void onActivate() override;
    void onDeactivate() override;
    void saveState() override;
    void restoreState() override;

private slots:
    void browseDirectory();
    void startReading();
    void cancelReading();
    void updateProgress(int value);
    void readingFinished();
    void toggleFilterOptions(bool enabled);
    void updateTextDisplay();
    void exportToTxtFile();

private:
    void setupUI();
    void createConnections();
    QString generateTextRepresentation(QTreeWidgetItem *item, int level = 0);

    // UI组件
    QLineEdit *m_directoryLineEdit;
    QPushButton *m_browseButton;
    QSpinBox *m_depthSpinBox;
    QCheckBox *m_filterCheckBox;
    FilterRuleListWidget *m_filterRuleListWidget;
    QCheckBox *m_readFilesCheckBox;
    QPushButton *m_startButton;
    QPushButton *m_cancelButton;
    QTreeWidget *m_directoryTreeWidget;
    QTextEdit *m_directoryTextDisplay;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    
    // 目录树读取器
    DirectoryTreeReader *m_directoryReader;
    
    // 过滤规则
    QList<FileFilterUtil::FilterRule> m_filterRules;
};

#endif // FILEBROWSERMODULE_H
```

2. **ContentProcessingModule** - 内容处理模块
```cpp
// include/contentprocessingmodule.h
#ifndef CONTENTPROCESSINGMODULE_H
#define CONTENTPROCESSINGMODULE_H

#include "modulebase.h"
#include "filemerger.h"
#include "filterrulelistwidget.h"

class ContentProcessingModule : public ModuleBase
{
    Q_OBJECT

public:
    explicit ContentProcessingModule(QWidget *parent = nullptr);
    ~ContentProcessingModule();

    QString moduleName() const override { return tr("内容处理"); }
    QIcon moduleIcon() const override;
    
    void onActivate() override;
    void onDeactivate() override;
    void saveState() override;
    void restoreState() override;

    // 设置和获取过滤规则
    void setFilterRules(const QList<FileFilterUtil::FilterRule> &rules);
    QList<FileFilterUtil::FilterRule> getFilterRules() const;

private slots:
    void browseDirectory();
    void startMerging();
    void cancelMerging();
    void updateProgress(int value);
    void mergeFinished();
    void handleProcessingFile(const QString &filePath);
    void exportMergedText();
    void toggleFilterOptions(bool enabled);
    void toggleSeparatorOptions(bool enabled);
    void toggleExtractionOptions(bool enabled);
    void toggleHeaderOptions(bool enabled);
    void handleFilterRulesChanged(const QList<FileFilterUtil::FilterRule> &rules);

private:
    void setupUI();
    void createConnections();

    // UI组件
    QLineEdit *m_directoryLineEdit;
    QPushButton *m_browseButton;
    QSpinBox *m_depthSpinBox;
    QCheckBox *m_filterCheckBox;
    FilterRuleListWidget *m_filterRuleListWidget;
    QCheckBox *m_separatorCheckBox;
    QLineEdit *m_separatorLineEdit;
    QCheckBox *m_extractionCheckBox;
    QLineEdit *m_extractionLineEdit;
    QCheckBox *m_headerCheckBox;
    QLineEdit *m_headerLineEdit;
    QPushButton *m_startButton;
    QPushButton *m_cancelButton;
    QPushButton *m_exportButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QTextEdit *m_mergedTextDisplay;
    
    // 文件合并器
    FileMerger *m_fileMerger;
};

#endif // CONTENTPROCESSINGMODULE_H
```

3. **CodeToolsModule** - 代码工具模块
```cpp
// include/codetoolsmodule.h
#ifndef CODETOOLSMODULE_H
#define CODETOOLSMODULE_H

#include "modulebase.h"

class CodeToolsModule : public ModuleBase
{
    Q_OBJECT

public:
    explicit CodeToolsModule(QWidget *parent = nullptr);
    ~CodeToolsModule();

    QString moduleName() const override { return tr("代码工具"); }
    QIcon moduleIcon() const override;
    
    void onActivate() override;
    void onDeactivate() override;

private slots:
    void openCodeStatsDialog();
    void openDocGeneratorDialog();

private:
    void setupUI();
    void createConnections();

    // UI组件
    QPushButton *m_codeStatsButton;
    QPushButton *m_docGeneratorButton;
};

#endif // CODETOOLSMODULE_H
```

4. **FileManagementModule** - 文件管理模块
```cpp
// include/filemanagementmodule.h
#ifndef FILEMANAGEMENTMODULE_H
#define FILEMANAGEMENTMODULE_H

#include "modulebase.h"

class FileManagementModule : public ModuleBase
{
    Q_OBJECT

public:
    explicit FileManagementModule(QWidget *parent = nullptr);
    ~FileManagementModule();

    QString moduleName() const override { return tr("文件管理"); }
    QIcon moduleIcon() const override;
    
    void onActivate() override;
    void onDeactivate() override;

private slots:
    void openBatchRenameDialog();
    void openFileCompareDialog();

private:
    void setupUI();
    void createConnections();

    // UI组件
    QPushButton *m_batchRenameButton;
    QPushButton *m_fileCompareButton;
};

#endif // FILEMANAGEMENTMODULE_H
```

5. **SettingsManager** - 设置管理器
```cpp
// include/settingsmanager.h
#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include "filefilterutil.h"

class SettingsManager : public QObject
{
    Q_OBJECT

public:
    static SettingsManager* instance();
    ~SettingsManager();

    // 过滤规则管理
    QList<FileFilterUtil::FilterRule> getFilterRules() const;
    void setFilterRules(const QList<FileFilterUtil::FilterRule> &rules);
    
    // 样式设置
    QString getCurrentTheme() const;
    void setCurrentTheme(const QString &themeName);
    
    // 预设管理
    QStringList getPresets() const;
    QList<FileFilterUtil::FilterRule> getPresetRules(const QString &presetName) const;
    void savePreset(const QString &presetName, const QList<FileFilterUtil::FilterRule> &rules);
    void deletePreset(const QString &presetName);
    
    // 加载和保存设置
    void loadSettings();
    void saveSettings();

signals:
    void filterRulesChanged(const QList<FileFilterUtil::FilterRule> &rules);
    void themeChanged(const QString &themeName);
    void presetsChanged();

private:
    SettingsManager(QObject *parent = nullptr);
    static SettingsManager *m_instance;

    QList<FileFilterUtil::FilterRule> m_filterRules;
    QString m_currentTheme;
    QMap<QString, QList<FileFilterUtil::FilterRule>> m_presets;
};

#endif // SETTINGSMANAGER_H
```

### 2. 修改现有类

#### 2.1 修改 MainWindow 类

```cpp
// include/mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QSplitter>
#include <QAction>

#include "sidebarwidget.h"
#include "filebrowsermodule.h"
#include "contentprocessingmodule.h"
#include "codetoolsmodule.h"
#include "filemanagementmodule.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onModuleSelected(int index);
    void openStyleSettings();
    void openFilterRulesDialog();
    void importFilterRules();
    void exportFilterRules();
    void toggleSidebar(bool visible);
    void toggleStatusBar(bool visible);
    void openSettingsDialog();
    void about();

private:
    void setupUI();
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void setupModules();
    void createConnections();
    void loadSettings();
    void saveSettings();

    // UI组件
    QMenuBar *m_menuBar;
    QToolBar *m_toolBar;
    QStatusBar *m_statusBar;
    QSplitter *m_mainSplitter;
    
    // 侧边栏
    SidebarWidget *m_sidebar;
    
    // 功能模块
    FileBrowserModule *m_fileBrowserModule;
    ContentProcessingModule *m_contentProcessingModule;
    CodeToolsModule *m_codeToolsModule;
    FileManagementModule *m_fileManagementModule;
    
    // 菜单和动作
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_toolsMenu;
    QMenu *m_settingsMenu;
    QMenu *m_helpMenu;
    
    QAction *m_openAction;
    QAction *m_exportAction;
    QAction *m_exitAction;
    
    QAction *m_copyAction;
    QAction *m_selectAllAction;
    QAction *m_findAction;
    
    QAction *m_toggleSidebarAction;
    QAction *m_toggleStatusBarAction;
    QAction *m_themeAction;
    
    QAction *m_settingsAction;
    QAction *m_filterRulesAction;
    QAction *m_presetsAction;
    
    QAction *m_aboutAction;
    QAction *m_helpAction;
};

#endif // MAINWINDOW_H
```

### 3. 实现步骤

1. **阶段一：创建基础框架**
   - 实现 SidebarWidget 类
   - 实现 ModuleBase 基类
   - 修改 MainWindow 类以支持新的界面布局

2. **阶段二：迁移现有功能**
   - 实现 FileBrowserModule 类，将目录树读取器功能迁移到该模块
   - 实现 ContentProcessingModule 类，将文件合并工具功能迁移到该模块
   - 实现 CodeToolsModule 类，整合代码统计和文档生成功能
   - 实现 FileManagementModule 类，整合批量重命名功能

3. **阶段三：统一设置管理**
   - 实现 SettingsManager 类
   - 将样式设置和过滤规则管理整合到统一的设置对话框

4. **阶段四：优化用户体验**
   - 添加全局状态栏和通知系统
   - 实现规则预设库
   - 优化各模块之间的数据共享

5. **阶段五：测试和完善**
   - 测试所有功能模块
   - 修复发现的问题
   - 完善文档 