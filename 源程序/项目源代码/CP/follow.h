/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: follow.h
 * @Brief: FOLLOW集合构建类，用于根据文法构建对应的FOLLOW集合
 * @Module: FOLLOW集合构建模块
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
#ifndef ANALYSEFOLLOW_H
#define ANALYSEFOLLOW_H

#include "root.h"

// Follow集合单元
struct followUnit
{
    set<string> s;
};

// 非终结符的Follow集合
extern map<string, followUnit> followSets;

class AnalyseFOLLOW
{
public:
    AnalyseFOLLOW();

    // 计算 Follow 集合
    static bool calculateFollowSets();

    // 添加 Follow 集合
    static void addToFollow(string nonTerminal, const set<string>& elements);

    // Follow 集合核心计算函数
    static void getFollowSets();
};

#endif // ANALYSEFOLLOW_H
