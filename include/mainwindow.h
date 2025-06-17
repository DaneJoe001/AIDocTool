/**
 * @file mainwindow.h
 * @brief 主窗口类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "directorytreereader.h"
#include "filemergerwidget.h"
#include "filterrulelistwidget.h"

#include <QMainWindow>
#include <QTreeWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include <QProgressBar>
#include <QRadioButton>
#include <QButtonGroup>
#include <QDir>
#include <QFuture>
#include <QFutureWatcher>
#include <QTextEdit>
#include <QSplitter>
#include <QStackedWidget>
#include <QMenuBar>
#include <QPlainTextEdit>
#include <QAction>

/**
 * @class MainWindow
 * @brief 应用程序的主窗口类
 * 
 * 该类提供了应用程序的用户界面和交互逻辑，
 * 包括目录树读取、文本显示、文件导出等功能。
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口指针
     */
    explicit MainWindow(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~MainWindow();

private slots:
    /**
     * @brief 浏览目录槽函数
     */
    void browseDirectory();
    
    /**
     * @brief 开始读取目录槽函数
     */
    void startReading();
    
    /**
     * @brief 取消读取目录槽函数
     */
    void cancelReading();
    
    /**
     * @brief 更新进度条槽函数
     * @param value 进度值（0-100）
     */
    void updateProgress(int value);
    
    /**
     * @brief 读取完成槽函数
     */
    void readingFinished();
    
    /**
     * @brief 切换过滤选项槽函数
     * @param enabled 是否启用过滤
     */
    void toggleFilterOptions(bool enabled);
    
    /**
     * @brief 导出为TXT文件槽函数
     */
    void exportToTxtFile();
    
    /**
     * @brief 导入过滤规则槽函数
     */
    void importFilterRules();
    
    /**
     * @brief 更新文本显示槽函数
     */
    void updateTextDisplay();
    
    /**
     * @brief 切换页面槽函数
     * @param index 页面索引
     */
    void switchToPage(int index);

    /**
     * @brief 打开样式设置对话框
     */
    void openStyleSettings();

    /**
     * @brief 主题变更处理函数
     * @param themeName 主题名称
     */
    void onThemeChanged(const QString &themeName);

    /**
     * @brief 打开过滤规则对话框
     */
    void openFilterRulesDialog();

    /**
     * @brief 打开批量文件重命名工具
     */
    void openBatchRenameDialog();

    /**
     * @brief 打开代码统计工具
     */
    void openCodeStatsDialog();

    /**
     * @brief 打开文档生成工具
     */
    void openDocGeneratorDialog();

    /**
     * @brief 处理过滤规则变更
     * @param rules 更新后的规则列表
     */
    void handleFilterRulesChanged(const QList<FileFilterUtil::FilterRule> &rules);

    /**
     * @brief 显示关于对话框
     */
    void showAboutDialog();

    /**
     * @brief 显示帮助文档
     */
    void showHelpDocument();

private:
    // UI组件
    QMenuBar *menuBar;           ///< 菜单栏
    QMenu *fileMenu;             ///< 文件菜单
    QMenu *viewMenu;             ///< 视图菜单
    QMenu *settingsMenu;         ///< 设置菜单
    QMenu *toolsMenu;            ///< 工具菜单
    QMenu *helpMenu;             ///< 帮助菜单
    QAction *exportAction;       ///< 导出动作
    QAction *importFilterAction; ///< 导入过滤规则动作
    QAction *styleSettingsAction; ///< 样式设置动作
    QAction *filterRulesAction;  ///< 过滤规则动作
    QAction *batchRenameAction;  ///< 批量重命名动作
    QAction *codeStatsAction;    ///< 代码统计动作
    QAction *docGeneratorAction; ///< 文档生成动作
    QAction *aboutAction;        ///< 关于动作
    QAction *helpAction;         ///< 帮助动作
    
    QStackedWidget *stackedWidget;   ///< 堆叠部件
    QWidget *directoryReaderPage;    ///< 目录读取页面
    FileMergerWidget *fileMergerPage; ///< 文件合并页面
    
    QSplitter *mainSplitter;         ///< 主分割器
    QLineEdit *directoryLineEdit;    ///< 目录输入框
    QPushButton *browseButton;       ///< 浏览按钮
    QSpinBox *depthSpinBox;
    QCheckBox *filterCheckBox;       ///< 过滤复选框
    FilterRuleListWidget *filterRuleListWidget; ///< 过滤规则列表部件
    QCheckBox *readFilesCheckBox;    ///< 读取文件复选框
    QPushButton *startButton;        ///< 开始按钮
    QPushButton *cancelButton;       ///< 取消按钮
    QTreeWidget *directoryTreeWidget; ///< 目录树控件
    QTextEdit *directoryTextDisplay;  ///< 目录文本显示
    QProgressBar *progressBar;       ///< 进度条
    QLabel *statusLabel;             ///< 状态标签
    
    // 过滤规则
    QList<FileFilterUtil::FilterRule> filterRules;

    // 目录树读取器
    DirectoryTreeReader *directoryReader; ///< 目录树读取器

    /**
     * @brief 设置UI组件
     */
    void setupUI();
    
    /**
     * @brief 设置菜单
     */
    void setupMenus();
    void readDirectory(const QString &path, QTreeWidgetItem *parent, int currentDepth, int maxDepth);
    bool matchesFilter(const QString &fileName);
    void populateTreeWidget(const QString &rootPath, int maxDepth);
    QString generateTextRepresentation(QTreeWidgetItem *item, int level = 0);
    
    /**
     * @brief 检查文件是否匹配过滤规则
     * @param path 文件路径
     * @return 如果文件匹配规则则返回true，否则返回false
     */
    bool matchesFilterRules(const QString &path);
};

#endif // MAINWINDOW_H 