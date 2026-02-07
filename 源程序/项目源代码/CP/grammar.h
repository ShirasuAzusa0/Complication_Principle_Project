/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: grammar.h
 * @Brief: 文法处理类，用于对用户输入的文法进行预处理
 * @Module: 文法处理模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/22    袁知本       模块化，从原先的widget.cpp中分离
 ***********************************************************************/
#ifndef GRAMMARANALYSE_H
#define GRAMMARANALYSE_H

#include "root.h"

// 非终结符数组
extern vector<string> bigAlpha;

// 终结符数组
extern vector<string> smallAlpha;

// 全局文法变量
extern string grammarStr;

// 结构化后的文法map
extern unordered_map<string, set<string>> grammarMap;

// 文法unit（用于LR0）
struct grammarUnit
{
    int gid;
    string left;
    string right;
    grammarUnit(string l, string r)
    {
        left = l;
        right = r;
    }
};

// 文法数组（用于LR0）
extern deque<grammarUnit> grammarDeque;

// LR0结果提示字符串
extern QString LR0Result;

// 文法查找下标
extern map<pair<string, string>, int> grammarToInt;

// 开始符号
extern string startSymbol;

// 增广后开始符号
extern string trueStartSymbol;

class GrammarAnalyse
{
public:
    GrammarAnalyse();

    // 判断字符 c 是否是终结符
    static bool isSmallAlpha(string c);

    // 判断字符 c 是否是非终结符
    static bool isBigAlpha(string c);

    // 文法初始化处理主函数
    static void handleGrammar();
};

#endif // GRAMMARANALYSE_H
