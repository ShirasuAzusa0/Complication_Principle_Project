/**************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: lr1analyse.cpp
 * @Brief: LR(1)分析表构建类的实现部分，用于根据文法构建对应的表格形式的LR(1)分析表
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
#include "lr1analyse.h"
#include "grammar.h"
#include "first.h"
#include "lr1dfa.h"

map<pair<int, string>, string> ACTION;
map<pair<int, string>, int> GOTO;
set<string> allTerminals;
set<string> allNonTerminals;

LR1Analyse::LR1Analyse()
{

}

void LR1Analyse::Analyse() {
    // 清除之前的表
    ACTION.clear();
    GOTO.clear();

    // 1. 处理所有通过 nextStateVector 的转移
    for (const lr1State &st : lr1States) {
        int sid = st.sid;
        for (const nextStateUnit &n : st.nextStateVector) {
            string sym = n.c;
            int toSid = n.sid;

            // 判断是否为终结符（包括$）
            if (GrammarAnalyse::isSmallAlpha(sym) || sym == "$") {
                // 检查是否存在移进-归约冲突
                auto key = make_pair(sid, sym);
                if (ACTION.find(key) != ACTION.end()) {
                    // 冲突处理：这里需要根据策略解决（如移进优先）
                    cerr << "移进-归约冲突: 状态" << sid << ", 符号" << sym
                         << ", 已有动作: " << ACTION[key] << ", 新动作: s" << toSid << endl;
                    // 移进优先策略
                    if (ACTION[key].substr(0, 1) == "r") {
                        ACTION[key] = "s" + to_string(toSid);
                    }
                } else {
                    ACTION[key] = "s" + to_string(toSid);
                }
            } else {
                // 非终结符直接添加到GOTO表
                GOTO[make_pair(sid, sym)] = toSid;
            }
        }
    }

    // 2. 添加归约动作
    for (const lr1State &st : lr1States) {
        int sid = st.sid;
        for (int itemid : st.itemV) {
            const lr1Item &it = lr1Items[itemid];
            const grammarUnit &g = grammarDeque[it.gid];
            QStringList rightListQ = QString::fromStdString(g.right).split(" ", Qt::SkipEmptyParts);
            int rightSize = rightListQ.size();

            // 检查是否可以规约
            if (it.index == rightSize || g.right == "#") {
                string actionValue;

                // 检查是否是起始产生式的归约
                if (g.left == trueStartSymbol && it.lookahead == "$") {
                    actionValue = "acc";
                } else {
                    actionValue = "r" + to_string(g.gid);
                }

                // 添加归约动作
                auto key = make_pair(sid, it.lookahead);

                // 检查冲突
                if (ACTION.find(key) != ACTION.end()) {
                    string existingAction = ACTION[key];

                    if (actionValue == "acc") {
                        // 接受动作冲突
                        cerr << "错误: 状态" << sid << "存在接受动作冲突" << endl;
                        continue;
                    }

                    if (existingAction.substr(0, 1) == "s") {
                        // 移进-归约冲突
                        cerr << "移进-归约冲突: 状态" << sid << ", 符号" << it.lookahead
                             << ", 移进: " << existingAction << ", 归约: " << actionValue << endl;
                        // 这里可以实现冲突解决策略
                        // 移进优先：保持原有移进动作
                    } else if (existingAction.substr(0, 1) == "r") {
                        // 归约-归约冲突
                        cerr << "归约-归约冲突: 状态" << sid << ", 符号" << it.lookahead
                             << ", 归约" << existingAction << "和" << actionValue << endl;
                        // 选择产生式编号较小的规约（或实现其他策略）
                        int existingGid = stoi(existingAction.substr(1));
                        int newGid = stoi(actionValue.substr(1));
                        if (newGid < existingGid) {
                            ACTION[key] = actionValue;
                        }
                    }
                } else {
                    ACTION[key] = actionValue;
                }
            }
        }
    }

    // 3. 准备表格显示 - 收集所有符号
    // 收集所有终结符（包括$）
    allTerminals.clear();
    for (const string &vt : smallAlpha) {
        allTerminals.insert(vt);
    }
    allTerminals.insert("$");

    // 收集所有非终结符
    allNonTerminals.clear();
    for (const string &vn : bigAlpha) {
        allNonTerminals.insert(vn);
    }

    // 4. 验证分析表的完整性（可选但推荐）
    // 检查每个状态对每个终结符是否有定义的动作
    for (int sid = 0; sid < (int)lr1States.size(); ++sid) {
        for (const string &term : allTerminals) {
            auto key = make_pair(sid, term);
            if (ACTION.find(key) == ACTION.end()) {
                // 未定义动作，可能是错误（对于LR(1)这是正常的）
                cerr << "警告: 状态" << sid << "对终结符'" << term << "'没有定义动作" << endl;
            }
        }
    }
}
