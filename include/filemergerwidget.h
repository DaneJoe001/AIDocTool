/**
 * @file filemergerwidget.h
 * @brief 文件合并器界面类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef FILEMERGERWIDGET_H
#define FILEMERGERWIDGET_H

#include <QWidget>
#include <QLineEdit>
#include <QSpinBox>
#include <QCheckBox>
#include <QTextEdit>
#include <QComboBox>
#include <QPushButton>
#include <QProgressBar>
#include <QLabel>
#include <QRadioButton>
#include <QGroupBox>
#include <QListWidget>

#include "filemerger.h"

/**
 * @class FileMergerWidget
 * @brief 文件合并器的用户界面类
 * 
 * 该类提供了一个用户界面，用于配置和使用文件合并器。
 */
class FileMergerWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口部件
     */
    explicit FileMergerWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~FileMergerWidget();

private slots:
    /**
     * @brief 选择根目录
     */
    void selectRootDirectory();
    
    /**
     * @brief 开始合并文件
     */
    void startMerging();
    
    /**
     * @brief 取消合并操作
     */
    void cancelMerging();
    
    /**
     * @brief 导出合并结果到文件
     */
    void exportToFile();
    
    /**
     * @brief 复制合并结果到剪贴板
     */
    void copyToClipboard();
    
    /**
     * @brief 添加过滤规则
     */
    void addFilterRule();
    
    /**
     * @brief 删除选中的过滤规则
     */
    void removeFilterRule();
    
    /**
     * @brief 处理合并完成事件
     * @param fileCount 处理的文件数量
     */
    void onMergingFinished(int fileCount);
    
    /**
     * @brief 处理进度更新事件
     * @param value 进度值
     */
    void onProgressUpdated(int value);
    
    /**
     * @brief 处理文件处理事件
     * @param filePath 当前处理的文件路径
     */
    void onProcessingFile(const QString &filePath);
    
    /**
     * @brief 更新过滤器类型选择
     * @param index 选择的索引
     */
    void onFilterTypeChanged(int index);
    
    /**
     * @brief 更新提取规则状态
     * @param checked 是否选中
     */
    void onExtractionToggled(bool checked);

private:
    /**
     * @brief 初始化UI组件
     */
    void setupUI();
    
    /**
     * @brief 创建连接
     */
    void createConnections();
    
    /**
     * @brief 更新UI状态
     * @param isRunning 是否正在运行
     */
    void updateUIState(bool isRunning);
    
    /**
     * @brief 获取当前过滤规则列表
     * @return 过滤规则列表
     */
    QStringList getFilterRules() const;

    // UI组件
    QLineEdit *rootPathEdit;          ///< 根目录输入框
    QPushButton *browseButton;        ///< 浏览按钮
    QSpinBox *depthSpinBox;           ///< 深度选择框
    QComboBox *filterTypeCombo;       ///< 过滤器类型选择框
    QLineEdit *filterPatternEdit;     ///< 过滤模式输入框
    QListWidget *filterRulesList;     ///< 过滤规则列表
    QLineEdit *filterRuleEdit;        ///< 过滤规则输入框
    QPushButton *addRuleButton;       ///< 添加规则按钮
    QPushButton *removeRuleButton;    ///< 删除规则按钮
    QCheckBox *separatorCheckBox;     ///< 分隔符选择框
    QLineEdit *separatorEdit;         ///< 分隔符输入框
    QCheckBox *extractionCheckBox;    ///< 内容提取选择框
    QLineEdit *extractionRegexEdit;   ///< 提取正则表达式输入框
    QLineEdit *headerTemplateEdit;    ///< 文件头模板输入框
    QPushButton *startButton;         ///< 开始按钮
    QPushButton *cancelButton;        ///< 取消按钮
    QPushButton *exportButton;        ///< 导出按钮
    QPushButton *copyButton;          ///< 复制按钮
    QProgressBar *progressBar;        ///< 进度条
    QLabel *statusLabel;              ///< 状态标签
    QTextEdit *resultTextEdit;        ///< 结果文本编辑器
    
    FileMerger *fileMerger;           ///< 文件合并器对象
};

#endif // FILEMERGERWIDGET_H 