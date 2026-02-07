/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: codeanalyse.h
 * @Brief: 源程序语法分析类，用于进行源程序的语法分析
 * @Module: 源程序语法分析模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/19   袁知本       模块化，从原先的widget.cpp中分离
 ***********************************************************************/
#ifndef CODEANALYSE_H
#define CODEANALYSE_H

#include "root.h"
#include "syntaxtree.h"
#include "midcode.h"

class CodeAnalyse
{
public:
    CodeAnalyse();

    // LEX 文件语法分析核心函数
    static bool analyseLEXContent(const QString& lexContent,
                                      const QString& grammarContent,
                                      QTableWidget* tableWidget);

    // 解析 LEX 文件内容，将编码转换为 Token 序列
    static vector<string> parseLEXContent(const QString& lexContent);

    // 初始化编码到终结符的映射
    static bool initializeEncodingMapping();

    // 构建 ACTION 和 GOTO 分析表
    static bool buildAnalysisTables(map<pair<int, string>, string>& ACTION,
                                    map<pair<int, string>, int>& GOTO);

    // Token 序列转换为可显示的字符串
    static QString tokensToString(const vector<string>& tokens);

    // 将状态栈转换为显示字符串
    static QString stateStackToString(const vector<int>& states);

    // 将符号栈转换为显示字符串
    static QString symbolStackToString(const vector<string>& symbols);

    // 执行LR(1)语法分析
    static bool executeLRAnalysis(const vector<string>& inputTokens,
                                      const map<pair<int, string>, string>& ACTION,
                                      const map<pair<int, string>, int>& GOTO,
                                      QTableWidget* tableWidget);

    // 向表中添加分析步骤
    static void addAnalysisStep(QTableWidget* tableWidget,
                                    int step,
                                    const QString& stateStack,
                                    const QString& symbolStack,
                                    const QString& remainingInput,
                                    const QString& action,
                                    int& row);

    // 配置表格显示属性
    static void configureTableDisplay(QTableWidget* tableWidget);
};

#endif // CODEANALYSE_H
