#include "filterrulelistwidget.h"
#include "filterrulesdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QMessageBox>
#include <QApplication>

FilterRuleListWidget::FilterRuleListWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUI();
    createConnections();
}

FilterRuleListWidget::~FilterRuleListWidget()
{
}

void FilterRuleListWidget::setupUI()
{
    // 创建主布局
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    
    // 创建输入区域
    QGridLayout *inputLayout = new QGridLayout();
    
    // 模式输入框
    QLabel *patternLabel = new QLabel(QStringLiteral("过滤模式:"), this);
    patternEdit = new QLineEdit(this);
    patternEdit->setPlaceholderText(QStringLiteral("输入过滤模式 (例如: *.txt 或 .*\\.txt)"));
    inputLayout->addWidget(patternLabel, 0, 0);
    inputLayout->addWidget(patternEdit, 0, 1, 1, 3);
    
    // 匹配类型选择
    wildcardRadioButton = new QRadioButton(QStringLiteral("通配符"), this);
    regexRadioButton = new QRadioButton(QStringLiteral("正则表达式"), this);
    matchTypeGroup = new QButtonGroup(this);
    matchTypeGroup->addButton(wildcardRadioButton, static_cast<int>(FileFilterUtil::MatchType::Wildcard));
    matchTypeGroup->addButton(regexRadioButton, static_cast<int>(FileFilterUtil::MatchType::Regex));
    wildcardRadioButton->setChecked(true);
    
    // 过滤模式选择
    QLabel *filterModeLabel = new QLabel(QStringLiteral("过滤模式:"), this);
    filterModeCombo = new QComboBox(this);
    filterModeCombo->addItem(QStringLiteral("包含"), static_cast<int>(FileFilterUtil::FilterMode::Include));
    filterModeCombo->addItem(QStringLiteral("排除"), static_cast<int>(FileFilterUtil::FilterMode::Exclude));
    filterModeCombo->setCurrentIndex(1); // 默认选择排除模式
    
    inputLayout->addWidget(wildcardRadioButton, 1, 0);
    inputLayout->addWidget(regexRadioButton, 1, 1);
    inputLayout->addWidget(filterModeLabel, 1, 2);
    inputLayout->addWidget(filterModeCombo, 1, 3);
    
    // 按钮区域
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    addButton = new QPushButton(QStringLiteral("添加"), this);
    addButton->setEnabled(false);
    buttonLayout->addWidget(addButton);
    
    removeButton = new QPushButton(QStringLiteral("删除"), this);
    removeButton->setEnabled(false);
    buttonLayout->addWidget(removeButton);
    
    manageButton = new QPushButton(QStringLiteral("管理规则"), this);
    buttonLayout->addWidget(manageButton);
    
    inputLayout->addLayout(buttonLayout, 2, 0, 1, 4);
    
    mainLayout->addLayout(inputLayout);
    
    // 规则列表
    rulesListWidget = new QListWidget(this);
    rulesListWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    rulesListWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(rulesListWidget);
}

void FilterRuleListWidget::createConnections()
{
    connect(patternEdit, &QLineEdit::textChanged, this, &FilterRuleListWidget::updateAddButtonState);
    connect(addButton, &QPushButton::clicked, this, &FilterRuleListWidget::addRule);
    connect(removeButton, &QPushButton::clicked, this, &FilterRuleListWidget::removeSelectedRule);
    connect(manageButton, &QPushButton::clicked, this, &FilterRuleListWidget::openFilterRulesDialog);
    
    connect(rulesListWidget, &QListWidget::itemSelectionChanged, [this]() {
        removeButton->setEnabled(!rulesListWidget->selectedItems().isEmpty());
    });
}

void FilterRuleListWidget::setFilterRules(const QList<FileFilterUtil::FilterRule> &rules)
{
    m_rules = rules;
    updateRulesList();
}

QList<FileFilterUtil::FilterRule> FilterRuleListWidget::getFilterRules() const
{
    return m_rules;
}

void FilterRuleListWidget::clearRules()
{
    m_rules.clear();
    updateRulesList();
    emit rulesChanged(m_rules);
}

void FilterRuleListWidget::addRule()
{
    QString pattern = patternEdit->text();
    if (pattern.isEmpty()) {
        return;
    }
    
    // 获取匹配类型
    FileFilterUtil::MatchType matchType;
    if (wildcardRadioButton->isChecked()) {
        matchType = FileFilterUtil::MatchType::Wildcard;
    } else {
        matchType = FileFilterUtil::MatchType::Regex;
    }
    
    // 获取过滤模式
    FileFilterUtil::FilterMode filterMode = static_cast<FileFilterUtil::FilterMode>(
        filterModeCombo->currentData().toInt());
    
    // 添加规则
    FileFilterUtil::FilterRule rule(pattern, matchType, filterMode, true);
    m_rules.append(rule);
    
    // 更新界面
    updateRulesList();
    patternEdit->clear();
    patternEdit->setFocus();
    
    // 发送信号
    emit rulesChanged(m_rules);
}

void FilterRuleListWidget::removeSelectedRule()
{
    int currentRow = rulesListWidget->currentRow();
    if (currentRow >= 0 && currentRow < m_rules.size()) {
        m_rules.removeAt(currentRow);
        updateRulesList();
        emit rulesChanged(m_rules);
    }
}

void FilterRuleListWidget::openFilterRulesDialog()
{
    FilterRulesDialog dialog(this);
    dialog.setFilterRules(m_rules);
    
    if (dialog.exec() == QDialog::Accepted) {
        m_rules = dialog.getFilterRules();
        updateRulesList();
        emit rulesChanged(m_rules);
    }
}

void FilterRuleListWidget::updateAddButtonState()
{
    addButton->setEnabled(!patternEdit->text().isEmpty());
}

void FilterRuleListWidget::updateRulesList()
{
    rulesListWidget->clear();
    
    for (const FileFilterUtil::FilterRule &rule : m_rules) {
        QListWidgetItem *item = new QListWidgetItem(generateRuleItemText(rule), rulesListWidget);
        
        // 设置项的颜色，根据规则是否启用
        if (!rule.enabled) {
            item->setForeground(Qt::gray);
        }
        
        // 设置图标，根据规则类型
        if (rule.filterMode == FileFilterUtil::FilterMode::Include) {
            item->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogApplyButton));
        } else {
            item->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogCancelButton));
        }
        
        // 存储规则数据
        item->setData(Qt::UserRole, QVariant::fromValue(rule));
        
        rulesListWidget->addItem(item);
    }
}

QString FilterRuleListWidget::generateRuleItemText(const FileFilterUtil::FilterRule &rule) const
{
    QString typeStr = (rule.matchType == FileFilterUtil::MatchType::Wildcard) ? 
                     QStringLiteral("通配符") : QStringLiteral("正则");
    QString modeStr = (rule.filterMode == FileFilterUtil::FilterMode::Include) ? 
                     QStringLiteral("包含") : QStringLiteral("排除");
    QString enabledStr = rule.enabled ? QStringLiteral("") : QStringLiteral("(禁用)");
    
    return QString("[%1, %2] %3 %4").arg(typeStr, modeStr, rule.pattern, enabledStr);
} 