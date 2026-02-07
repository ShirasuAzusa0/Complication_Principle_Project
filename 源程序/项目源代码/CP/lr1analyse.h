/**************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: lr1analyse.h
 * @Brief: LR(1)分析表构建类，用于根据文法构建对应的表格形式的LR(1)分析表
 * @Module: LR(1)分析表构建模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/23   袁知本       模块化，从原先的widget.cpp中分离
 *************************************************************************/
#ifndef LR1ANALYSE_H
#define LR1ANALYSE_H

#include "root.h"

// ACTION 表
extern map<pair<int, string>, string> ACTION;

// GOTO 表
extern map<pair<int, string>, int> GOTO;

// 所有终结符集合
extern set<string> allTerminals;

// 所有非终结符集合
extern set<string> allNonTerminals;

class LR1Analyse
{
public:
    LR1Analyse();

    // LR(1) 分析表构建
    static void Analyse();
};

#endif // LR1ANALYSE_H
