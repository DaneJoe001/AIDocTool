/**
 * @file filterrulelistwidget.h
 * @brief 过滤规则列表部件类的定义
 * @author AIDocTools
 * @date 2023
 */

#ifndef FILTERRULELISTWIDGET_H
#define FILTERRULELISTWIDGET_H

#include "filefilterutil.h"

#include <QWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLabel>

/**
 * @class FilterRuleListWidget
 * @brief 过滤规则列表部件类
 * 
 * 该类提供了一个用于显示和管理过滤规则的列表部件。
 */
class FilterRuleListWidget : public QWidget
{
    Q_OBJECT

public:
    /**
     * @brief 构造函数
     * @param parent 父窗口部件
     */
    explicit FilterRuleListWidget(QWidget *parent = nullptr);
    
    /**
     * @brief 析构函数
     */
    ~FilterRuleListWidget();
    
    /**
     * @brief 设置过滤规则
     * @param rules 过滤规则列表
     */
    void setFilterRules(const QList<FileFilterUtil::FilterRule> &rules);
    
    /**
     * @brief 获取过滤规则
     * @return 过滤规则列表
     */
    QList<FileFilterUtil::FilterRule> getFilterRules() const;
    
    /**
     * @brief 清空规则列表
     */
    void clearRules();

signals:
    /**
     * @brief 规则列表变更信号
     * @param rules 更新后的规则列表
     */
    void rulesChanged(const QList<FileFilterUtil::FilterRule> &rules);

private slots:
    /**
     * @brief 添加规则
     */
    void addRule();
    
    /**
     * @brief 删除选中的规则
     */
    void removeSelectedRule();
    
    /**
     * @brief 打开过滤规则对话框
     */
    void openFilterRulesDialog();
    
    /**
     * @brief 更新添加按钮状态
     */
    void updateAddButtonState();

private:
    /**
     * @brief 设置UI组件
     */
    void setupUI();
    
    /**
     * @brief 创建连接
     */
    void createConnections();
    
    /**
     * @brief 更新规则列表显示
     */
    void updateRulesList();
    
    /**
     * @brief 生成规则项显示文本
     * @param rule 过滤规则
     * @return 显示文本
     */
    QString generateRuleItemText(const FileFilterUtil::FilterRule &rule) const;

    // UI组件
    QLineEdit *patternEdit;           ///< 模式输入框
    QRadioButton *wildcardRadioButton; ///< 通配符单选按钮
    QRadioButton *regexRadioButton;   ///< 正则表达式单选按钮
    QButtonGroup *matchTypeGroup;     ///< 匹配类型按钮组
    QComboBox *filterModeCombo;       ///< 过滤模式下拉框
    QPushButton *addButton;           ///< 添加按钮
    QPushButton *removeButton;        ///< 删除按钮
    QPushButton *manageButton;        ///< 管理按钮
    QListWidget *rulesListWidget;     ///< 规则列表部件
    
    QList<FileFilterUtil::FilterRule> m_rules; ///< 过滤规则列表
};

#endif // FILTERRULELISTWIDGET_H 