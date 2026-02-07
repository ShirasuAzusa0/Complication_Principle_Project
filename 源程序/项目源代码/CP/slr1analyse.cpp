/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: slr1analyse.cpp
 * @Brief: SLR(1)分析类的实现部分，用于分析判断用户提供的文法是否符合SLR(1)文法
 * @Module: SLR(1)分析模块
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
 ***********************************************************************/
#include "slr1analyse.h"
#include "grammar.h"
#include "follow.h"
#include "lr0dfa.h"

SLR1Analyse::SLR1Analyse()
{

}

// 检查“移进-规约”冲突
bool SLR1Analyse::SLR1Fun1() {
    bool flag = false;
    for (dfaState& state : dfaStateVector) {
        // 规约项目的左边集合
        set<string> a;
        // 终结符
        set<string> rVT;
        // 不是规约状态不考虑
        if (!state.isEnd) continue;
        // 规约状态
        for (int cellid : state.cellV) {
            // 拿到这个cell
            const dfaCell& cell = dfaCellVector[cellid];
            // 获取文法
            const grammarUnit gm = grammarDeque[cell.gid];

            QStringList rightList = QString::fromStdString(grammarDeque[cell.gid].right).split(" ");
            // 判断是不是规约项目
            if (cell.index == rightList.size() || gm.right == "#")
                a.insert(gm.left);
            // 判断是不是终结符
            else
                if (GrammarAnalyse::isSmallAlpha(rightList[cell.index].toStdString()))
                    rVT.insert(rightList[cell.index].toStdString());
        }
        for (string c : a)
            for (string v : rVT)
                if (followSets[c].s.find(v) != followSets[c].s.end()) {
                    flag = true;
                    state.isSpecial = true;
                }
    }
    return flag;
}

// 检查“规约-规约”冲突
bool SLR1Analyse::SLR1Fun2() {
    for (const auto& state : dfaStateVector) {
        // 规约项目的左边集合
        set<string> a;
        // 不是规约状态不考虑
        if (!state.isEnd) continue;

        // 规约状态
        for (int cellid : state.cellV) {
            // 拿到这个cell
            const dfaCell& cell = dfaCellVector[cellid];
            // 获取文法
            const grammarUnit gm = grammarDeque[cell.gid];

            QStringList rightList = QString::fromStdString(grammarDeque[cell.gid].right).split(" ");
            // 判断是不是规约项目
            if (cell.index == rightList.size() || gm.right == "#")
                a.insert(gm.left);
        }

        for (string c1 : a)
            for (string c2 : a)
                if (c1 != c2) {
                    // 判断followSets[c1]和followSets[c2]是否有交集
                    set<string> followSetC1 = followSets[c1].s;
                    set<string> followSetC2 = followSets[c2].s;
                    set<string> intersection;

                    // 利用STL算法求交集
                    set_intersection(
                        followSetC1.begin(), followSetC1.end(),
                        followSetC2.begin(), followSetC2.end(),
                        inserter(intersection, intersection.begin())
                    );

                    // 如果交集非空，说明存在规约-规约冲突
                    if (!intersection.empty())
                        return true;
                }
    }

    return false;
}

// 检查 SLR(1) 分析入口
int SLR1Analyse::SLR1_Analyse() {
    // 开始符号添加follow集合
    followSets["zengguang"].s.insert("$");

    bool flag1 = SLR1Fun1();
    bool flag2 = SLR1Fun2();

    if (flag1 && flag2)
        return 3;
    else if (flag1)
        return 1;
    else if (flag2)
        return 2;

    // 没有冲突，是SLR(1)文法
    return 0;
}
