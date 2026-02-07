/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: lr1dfa.cpp
 * @Brief: LR(1)DFA构建类的实现部分，用于根据文法构建对应的表格形式的LR(1)DFA
 * @Module: LR(1)DFA构建模块
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
#include "lr1dfa.h"
#include "grammar.h"
#include "first.h"

vector<lr1Item> lr1Items;
vector<lr1State> lr1States;
set<string> lr1VN;
set<string> lr1VT;
set<int> lr1VisitedStates;
int lr1ItemCount = 0;
int lr1StateCount = 0;

GenerateLR1DFA::GenerateLR1DFA()
{

}

// 辅助函数，判断两个项目集合是否相同
bool GenerateLR1DFA::sameItemSet(const vector<int>& A, const vector<int>& B) {
    if (A.size() != B.size()) return false;
    vector<int> a = A, b = B;
    sort(a.begin(), a.end());
    sort(b.begin(), b.end());
    return a == b;
}

// LR(1)DFA 核心生成入口函数
void GenerateLR1DFA::getLR1() {
    lr1VisitedStates.clear();
    lr1Items.clear();
    lr1States.clear();
    lr1VN.clear();
    lr1VT.clear();
    lr1ItemCount = 0;
    lr1StateCount = 0;

    createFirstLR1State();

    // 注意：初始状态 createFirstLR1State 已经放入第一个 item 到 state.itemV/originV
    generateLR1State(0);
}

// 创建 LR(1) 初始状态（增广项 E' -> .S, lookahead = $）
void GenerateLR1DFA::createFirstLR1State() {
    lr1States.clear();
    lr1Items.clear();
    lr1ItemCount = 0;
    lr1StateCount = 0;

    lr1State start;
    start.sid = lr1StateCount++;
    lr1States.push_back(start);

    // 找到增广文法的编号
    // 在 handleGrammar() 中把增广规则 push_front 到 grammarDeque
    // 与 LR0 一致，取 gid = 0 作为 E' -> S
    lr1Item firstItem;
    firstItem.gid = 0;
    firstItem.index = 0;
    firstItem.lookahead = "$";
    firstItem.itemid = lr1ItemCount++;
    lr1Items.push_back(firstItem);

    lr1States[0].itemV.push_back(firstItem.itemid);
    lr1States[0].originV.push_back(firstItem.itemid);
}

// LR(1) 状态生成（递归），与 LR0 实现类似但使用 lr1Items/lr1States，处理 lookahead
// 递归生成 LR(1) 状态
void GenerateLR1DFA::generateLR1State(int stateId) {
    if (lr1VisitedStates.count(stateId) > 0)
        return;

    lr1VisitedStates.insert(stateId);

    lr1State &currentState = lr1States[stateId];

    // 确保状态0包含起始产生式
    if (stateId == 0) {
        // 查找起始符号的产生式
        for (size_t i = 0; i < grammarDeque.size(); ++i)
            if (grammarDeque[i].left == startSymbol) {
                // 添加初始项目 [S' -> . program, $]
                lr1Item initialItem;
                initialItem.gid = i;
                initialItem.index = 0;
                initialItem.lookahead = "$";
                initialItem.itemid = lr1ItemCount++;
                lr1Items.push_back(initialItem);
                currentState.itemV.push_back(initialItem.itemid);
                break;
            }
    }

    // 先做闭包：对于状态中每个 item，如果点后是非终结符 B，则 for each B->γ, 对应 lookahead 由 FIRST(β a) 决定
    for (size_t i = 0; i < lr1States[stateId].itemV.size(); ++i) {
        lr1Item &it = lr1Items[lr1States[stateId].itemV[i]];
        grammarUnit &g = grammarDeque[it.gid];
        QStringList rightListQ = QString::fromStdString(g.right).split(" ", Qt::SkipEmptyParts);
        vector<string> rightList;
        for (int k = 0; k < rightListQ.size(); ++k)
            rightList.push_back(rightListQ[k].toStdString());

        // 如果 dot 在产生式末尾或产生式为 # 则标记为规约项（isEnd）
        if (it.index == (int)rightList.size() || g.right == "#") {
            lr1States[stateId].isEnd = true;
            continue;
        }

        string nextSymbol = rightList[it.index];

        // 若 nextSymbol 是非终结符，则对其进行闭包扩展
        if (GrammarAnalyse::isBigAlpha(nextSymbol)) {
            // β 是点后剩余符号（不包括第一个被处理的 nextSymbol）
            vector<string> beta;
            for (size_t p = it.index + 1; p < rightList.size(); ++p)
                beta.push_back(rightList[p]);

            // 对于当前 item 的 lookahead，计算 FIRST( β + lookahead )
            vector<string> seq = beta;
            seq.push_back(it.lookahead);

            set<string> firstSetSeq = firstOfSequence(seq);

            // 遍历 grammarMap[nextSymbol] 的所有产生式，按 FIRST 集合创建新 item
            for (const auto &prod : grammarMap[nextSymbol]) {
                int gid_new = grammarToInt[make_pair(nextSymbol, prod)];
                for (const string &b : firstSetSeq) {
                    string la = (b == "#") ? it.lookahead : b;  // 若能推出 ε，用原 lookahead
                    int existing = isNewLR1Item(gid_new, 0, la);
                    if (existing == -1) {
                        lr1Item newItem(gid_new, 0, la);
                        newItem.itemid = lr1ItemCount++;
                        lr1Items.push_back(newItem);
                        lr1States[stateId].itemV.push_back(newItem.itemid);
                    }
                    else
                        // 即使 item 已存在，也保证放入 state.itemV
                        if (find(lr1States[stateId].itemV.begin(), lr1States[stateId].itemV.end(), existing) == lr1States[stateId].itemV.end())
                            lr1States[stateId].itemV.push_back(existing);
                }
            }
        }
    }

    // 生成从当前状态出发的转移（按点后符号分组）
    map<string, lr1State> tempSave; // key: symbol -> 临时 lr1State
    for (size_t i = 0; i < lr1States[stateId].itemV.size(); ++i) {
        lr1Item &it = lr1Items[lr1States[stateId].itemV[i]];
        grammarUnit &g = grammarDeque[it.gid];
        QStringList rightListQ = QString::fromStdString(g.right).split(" ", Qt::SkipEmptyParts);
        vector<string> rightList;
        for (int k = 0; k < rightListQ.size(); ++k) rightList.push_back(rightListQ[k].toStdString());

        if (it.index == (int)rightList.size() || g.right == "#")
            // 点在末尾
            continue;

        string nextSymbol = rightList[it.index];
        // 准备下一个 item：gid 不变，index + 1，lookahead 保持不变
        lr1Item nextIt(it.gid, it.index + 1, it.lookahead);
        int nextItemId = isNewLR1Item(nextIt.gid, nextIt.index, nextIt.lookahead);
        if (nextItemId == -1) {
            nextIt.itemid = lr1ItemCount++;
            lr1Items.push_back(nextIt);
        }
        else
            nextIt.itemid = nextItemId;

        // 将 nextIt 放到临时状态 tempSave[nextSymbol] 中
        lr1State &tempSt = tempSave[nextSymbol];
        tempSt.sid = -1; // 临时
        tempSt.itemV.push_back(nextIt.itemid);
        tempSt.originV.push_back(nextIt.itemid);

        // record symbol sets
        if (GrammarAnalyse::isBigAlpha(nextSymbol))
            lr1VN.insert(nextSymbol);
        else if (GrammarAnalyse::isSmallAlpha(nextSymbol))
            lr1VT.insert(nextSymbol);
    }

    // 将每个临时状态归一化（闭包会在递归进入时补完）；检测是否与已有状态重复，若不重复则新建
    for (auto &p : tempSave) {
        string symbol = p.first;
        lr1State tmp = p.second;
        // 检查是否和已有 lr1States 相同（通过 originV）
        int existSid = isNewLR1State(tmp.originV);
        lr1State newState;
        if (existSid == -1) {
            newState.sid = lr1StateCount++;
            newState.itemV = tmp.itemV;
            newState.originV = tmp.originV;
            lr1States.push_back(newState);
        }
        else
            newState.sid = existSid;

        nextStateUnit n;
        n.c = symbol;
        n.sid = newState.sid;
        lr1States[stateId].nextStateVector.push_back(n);
    }

    // 递归处理下级状态
    int nsize = lr1States[stateId].nextStateVector.size();
    for (int i = 0; i < nsize; ++i) {
        auto &nextunit = lr1States[stateId].nextStateVector[i];
        generateLR1State(nextunit.sid);
    }
}

// 计算符号序列的 FIRST 集合
set<string> GenerateLR1DFA::firstOfSequence(const vector<string>& seq) {
    set<string> result;
    bool allNullable = true;
    for (size_t i = 0; i < seq.size(); ++i) {
        string sym = seq[i];
        if (sym == "#")
            // 若为空（#），则跳过继续查看下一符号
            continue;
        if (GrammarAnalyse::isSmallAlpha(sym) || sym == "$") {
            // 终结符或 $ 直接加入
            result.insert(sym);
            allNullable = false;
            break;
        }
        // 非终结符
        // 将非终结符的 first（仅包含终结符）加入
        for (const string &t : firstSets[sym].s)
            result.insert(t);

        // 若该非终结符不能推出 epsilon，则停止
        if (!firstSets[sym].isEpsilon) {
            allNullable = false;
            break;
        }
        // else 继续下一个符号
    }
    if (allNullable)
        // 用内部标志表示可以推出 epsilon
        // 内部使用，表示 nullable
        result.insert("#");

    return result;
}

// 判断是否为新的 LR(1) 项
int GenerateLR1DFA::isNewLR1Item(int gid, int index, const string &lookahead) {
    for (const lr1Item &it : lr1Items)
        if (it.gid == gid && it.index == index && it.lookahead == lookahead)
            return it.itemid;
    return -1;
}

// 判断是否为新的 LR(1) 状态
int GenerateLR1DFA::isNewLR1State(const vector<int>& items) {
    for (const lr1State& st : lr1States)
        // 使用 originV 判重
        if (sameItemSet(st.originV, items))
            return st.sid;
    return -1;

}

// 获取状态文法的字符串表示
string GenerateLR1DFA::getLR1StateGrammar(const lr1State& st) {
    string result = "";
    for (int itemid : st.itemV) {
        const lr1Item &it = lr1Items[itemid];
        const grammarUnit &g = grammarDeque[it.gid];
        string left = (g.left == "zengguang") ? "E'": g.left;
        string right = (g.right == "#") ? "" : g.right;
        QStringList rightList = QString::fromStdString(right).split(" ", Qt::SkipEmptyParts);
        string rightResult = "";
        for (int i = 0; i <= rightList.size(); ++i) {
            if (i == it.index)
                rightResult += ".";
            if (i == rightList.size())
                break;
            rightResult += rightList[i].toStdString();
        }
        // 拼接 lookahead
        rightResult += " , ";
        rightResult += it.lookahead;
        string r = left + "->" + rightResult;
        result += r + "    ";
    }
    return result;
}
