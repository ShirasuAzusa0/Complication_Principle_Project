/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: first.cpp
 * @Brief: FIRST集合构建类的实现部分，用于根据文法构建对应的FIRST集合
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
#include "first.h"
#include "grammar.h"

map<string, firstUnit> firstSets;

AnalyseFIRST::AnalyseFIRST()
{

}

// 计算 First 集合，用于单轮FIRST推导
// 若任何FIRST集发生变化，返回true，否则返回false
bool AnalyseFIRST::calculateFirstSets() {
    bool flag = false;
    for (auto& grammar : grammarMap) {
        string nonTerminal = grammar.first;
        // 保存当前FIRST集合的大小以用于检查是否发生变化
        size_t originalSize = firstSets[nonTerminal].s.size();
        bool originalE = firstSets[nonTerminal].isEpsilon;
        for (auto& g : grammar.second) {
            QStringList gList = QString::fromStdString(g).split(" ");
            int k = 0;
            while (k <= gList.size() - 1) {
                string t = gList[k].toStdString();
                set<string> first_k;
                if (t == "#") {
                    k ++;
                    continue;
                }
                else if (GrammarAnalyse::isSmallAlpha(t))
                    first_k.insert(t);
                else
                    first_k = firstSets[t].s;

                firstSets[nonTerminal].s.insert(first_k.begin(), first_k.end());
                // 若是终结符或没有空串再非终结符中，则直接跳出
                if (GrammarAnalyse::isSmallAlpha(t) || !firstSets[t].isEpsilon)
                    break;
                k ++;
            }
            if ((size_t)k == g.size())
                firstSets[nonTerminal].isEpsilon = true;
        }

        // 看原始大小和是否变化epsilon，如果变化说明要重新再来一次
        if (originalSize != firstSets[nonTerminal].s.size() || originalE != firstSets[nonTerminal].isEpsilon)
            flag = true;
    }
    return flag;
}

// First 集合核心计算函数入口
void AnalyseFIRST::getFirstSets() {
    // 不停迭代，直到FIRST集合不再发生变化
    bool flag = false;
    do
        flag = calculateFirstSets();
    while (flag);
}
