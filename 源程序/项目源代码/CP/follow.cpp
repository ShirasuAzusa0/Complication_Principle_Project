/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: follow.cpp
 * @Brief: FOLLOW集合构建类的实现部分，用于根据文法构建对应的FOLLOW集合
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
#include "follow.h"
#include "first.h"
#include "grammar.h"

map<string, followUnit> followSets;

AnalyseFOLLOW::AnalyseFOLLOW()
{

}

// 计算 Follow 集合
bool AnalyseFOLLOW::calculateFollowSets() {
    bool flag = false;
    for (auto& grammar : grammarMap) {
        string nonTerminal = grammar.first;

        for (auto& g : grammar.second) {
            QStringList gList = QString::fromStdString(g).split(" ");
            for (int i = 0; i < gList.size(); ++ i) {
                string t = gList[i].toStdString();
                if (GrammarAnalyse::isSmallAlpha(t) || t == "#")
                    continue;   // 跳过终结符
                set<string> follow_k;
                size_t originalSize = followSets[t].s.size();

                if (i == gList.size() - 1)
                    // Case A: A -> αB, add Follow(A) to Follow(B)
                    follow_k.insert(followSets[nonTerminal].s.begin(), followSets[nonTerminal].s.end());
                else {
                    // Case B: A -> αBβ
                    int j = i + 1;
                    while (j < gList.size()) {
                        string t2 = gList[j].toStdString();
                        if (GrammarAnalyse::isSmallAlpha(t2)) {
                            // 若为终结符则直接加入并跳出循环
                            follow_k.insert(t2);
                            break;
                        }
                        else {
                            // 若为非终结符则加入 FIRST 集合
                            set<string> first_beta = firstSets[t2].s;
                            follow_k.insert(first_beta.begin(), first_beta.end());

                            // 若没有空串在FIRST集合中，则停止
                            if (!firstSets[t2].isEpsilon)
                                break;
                            ++ j;
                        }
                    }

                    // 若 β 是 空串（ε）或者 β 是 all nullable, add Follow(A) to Follow(B)
                    if (j == gList.size())
                        follow_k.insert(followSets[nonTerminal].s.begin(), followSets[nonTerminal].s.end());
                }

                addToFollow(t, follow_k);
                // 检查是否发生变化
                if (originalSize != followSets[t].s.size())
                    flag = true;
            }
        }
    }

    return flag;
}

// 添加 Follow 集合
void AnalyseFOLLOW::addToFollow(string nonTerminal, const set<string>& elements) {
    followSets[nonTerminal].s.insert(elements.begin(), elements.end());
}

// Follow 集合核心计算函数入口
void AnalyseFOLLOW::getFollowSets() {
    // 开始符号加入 $ 符号
    addToFollow(startSymbol, { "$" });

    // 不停迭代，直到FOLLOW集合不再发生变化
    bool flag = false;
    do
        flag = calculateFollowSets();
    while (flag);
}
