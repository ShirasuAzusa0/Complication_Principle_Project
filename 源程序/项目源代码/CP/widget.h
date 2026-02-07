/*******************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: widget.h
 * @Brief: 应用程序窗口界面声明文件，包含主窗口类，负责交互操作控件定义
 * @Module: 主界面模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/16   袁知本       整合任务一和任务二的交互控件函数
 ******************************************************************/

#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    void wrapPage(QWidget *page, QGraphicsView *view);
    bool eventFilter(QObject *watched, QEvent *event);
    ~Widget();

private slots:

    // 查看正则表达式的输入规则说明
    void on_pushButton_Help_clicked();

    // 导入正则表达式
    void on_pushButton_LoadRegex_clicked();

    // 导出正则表达式
    void on_pushButton_SaveRegex_clicked();

    // 正则表达式分析
    void on_pushButton_Analyse_clicked();

    // 生成 NFA
    void on_pushButton_ShowNFA_clicked();

    // 生成 DFA
    void on_pushButton_ShowDFA_clicked();

    // DFA最小化
    void on_pushButton_ShowMinDFA_clicked();

    // 生成词法分析程序
    void on_pushButton_GenerateCode_clicked();

    // 查看LEX 文件
    void on_pushButton_LoadLEXFile_clicked();

    // 查看文法输入规则说明
    void on_pushButton_Help_1_clicked();

    // 载入文法规则
    void on_pushButton_LoadGrammar_clicked();

    // 保存文法规则
    void on_pushButton_SaveGrammar_clicked();

    // 载入源程序
    void on_pushButton_LoadProgram_clicked();

    // 保存源程序
    void on_pushButton_SaveProgram_clicked();

    // 求 First 集
    void on_pushButton_GetFirstSet_clicked();

    // 求 Follow 集
    void on_pushButton_GetFollowSet_clicked();

    // 分析结果说明
    void on_pushButton_Help_2_clicked();

    // LR(0)DFA 生成
    void on_pushButton_GenLR0DFA_clicked();

    // SLR(1) 文法分析
    void on_pushButton_SLR1Analyse_clicked();

    // LR(1)DFA 生成
    void on_pushButton_GenLR1DFA_clicked();

    // LR(1) 分析表生成
    void on_pushButton_LR1Analyse_clicked();

    // 源程序分析
    void on_pushButton_AnalyseSentence_clicked();

    // 中间代码生成
    void on_pushButton_Interlanguage_clicked();

    // 可视化显示语法树
    void on_pushButton_ShowSyntaxTree_clicked();

    // 转到任务二界面
    void on_commandLinkButton_clicked();

    // 转到任务一界面
    void on_commandLinkButton_2_clicked();

private:
    Ui::Widget *ui;
};
#endif // WIDGET_H
