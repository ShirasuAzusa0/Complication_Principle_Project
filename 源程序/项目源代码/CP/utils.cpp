/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: utils.cpp
 * @Brief: 辅助方法函数的实现部分，可用于初始化等辅助操作
 * @Module: 辅助方法模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/18    袁知本       模块化，从原先的widget.cpp中分离
 ***********************************************************************/
#include "utils.h"
#include "regex.h"
#include "nfa.h"
#include "dfa.h"
#include "grammar.h"
#include "first.h"
#include "follow.h"
#include "lr0dfa.h"
#include "slr1analyse.h"
#include "lr1dfa.h"

void init_1() {
    // 全局变量清空
    keyWords.clear();
    finalRegex.clear();
    commentSymbol->clear();
    nodeCount = 0;
    nfaCharSet.clear();
    dfaCharSet.clear();
    statusTable.clear();
    insertionOrder.clear();
    startNFAstatus.clear();
    endNFAstatus.clear();
    dfaStatusSet.clear();
    dfaEndStatusSet.clear();
    dfaNotEndStatusSet.clear();
    dfaMinTable.clear();
    divideVector.clear();
    dfaMinMap.clear();
    dfaTable.clear();
    nfaCharSet.insert(EPSILON); // 放入epsilon
}

void init_2() {
    grammarMap.clear();

    bigAlpha.clear();
    smallAlpha.clear();


    firstSets.clear();
    followSets.clear();
    LR0Result.clear();
    grammarDeque.clear();
    dfaStateVector.clear();
    dfaCellVector.clear();
    VT.clear();
    VN.clear();
    scnt = 0;
    ccnt = 0;

    // 清理LR(1)相关变量
    lr1Items.clear();
    lr1States.clear();
    lr1VT.clear();
    lr1VN.clear();
    lr1ItemCount = 0;
    lr1StateCount = 0;
}

