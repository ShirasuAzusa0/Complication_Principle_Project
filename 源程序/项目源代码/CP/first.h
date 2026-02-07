/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: first.h
 * @Brief: FIRST集合构建类，用于根据文法构建对应的FIRST集合
 * @Module: FIRST集合构建模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/22   袁知本       模块化，从原先的widget.cpp中分离
 ***********************************************************************/
#ifndef ANALYSEFIRST_H
#define ANALYSEFIRST_H

#include "root.h"

// First集合单元
struct firstUnit
{
    set<string> s;
    bool isEpsilon = false;
};

// 非终结符的First集合
extern map<string, firstUnit> firstSets;

class AnalyseFIRST
{
public:
    AnalyseFIRST();

    // 计算 First 集合
    static bool calculateFirstSets();

    // First 集合核心计算函数
    static void getFirstSets();
};

#endif // ANALYSEFIRST_H
