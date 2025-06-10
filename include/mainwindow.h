/**
 * @file mainwindow.h
 * @brief 主窗口类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "directorytreereader.h"

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

private:
    // UI组件
    QMenuBar *menuBar;           ///< 菜单栏
    QMenu *fileMenu;             ///< 文件菜单
    QMenu *viewMenu;             ///< 视图菜单
    QAction *exportAction;       ///< 导出动作
    QAction *importFilterAction; ///< 导入过滤规则动作
    
    QStackedWidget *stackedWidget;   ///< 堆叠部件
    QWidget *directoryReaderPage;    ///< 目录读取页面
    
    QSplitter *mainSplitter;         ///< 主分割器
    QLineEdit *directoryLineEdit;    ///< 目录输入框
    QPushButton *browseButton;       ///< 浏览按钮
    QSpinBox *depthSpinBox;          ///< 深度选择框
    QCheckBox *filterCheckBox;       ///< 过滤复选框
    QRadioButton *wildcardRadioButton; ///< 通配符单选按钮
    QRadioButton *regexRadioButton;  ///< 正则表达式单选按钮
    QLineEdit *filterPatternLineEdit; ///< 过滤模式输入框
    QCheckBox *readFilesCheckBox;    ///< 读取文件复选框
    QPushButton *startButton;        ///< 开始按钮
    QPushButton *cancelButton;       ///< 取消按钮
    QTreeWidget *directoryTreeWidget; ///< 目录树控件
    QTextEdit *directoryTextDisplay;  ///< 目录文本显示
    QProgressBar *progressBar;       ///< 进度条
    QLabel *statusLabel;             ///< 状态标签
    QPlainTextEdit *filterRulesTextEdit; ///< 过滤规则文本框
    
    // 读取目录的工作线程相关
    QFutureWatcher<void> *watcher;
    bool isCancelled;
    
    // 过滤规则
    QStringList filterRules;

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
    bool matchesFilterRules(const QString &path);
};

#endif // MAINWINDOW_H 