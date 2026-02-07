/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: lr1dfa.h
 * @Brief: LR(1)DFA构建类，用于根据文法构建对应的表格形式的LR(1)DFA
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
#ifndef GENERATELR1DFA_H
#define GENERATELR1DFA_H

#include "root.h"
#include "lr0dfa.h"

// LR(1) item：使用单一 lookahead（canonical LR(1) 中每个 item 携带一个终结符或 $）
struct lr1Item
{
    int itemid;                             // 唯一 id
    int gid;                                // 文法编号（grammarDeque 中的下标）
    int index;                              // 点的位置
    string lookahead;                       // 单个 lookahead 符号，例如 "$" 或某终结符
    lr1Item(int g = 0, int i = 0, const string &la = "")
    {
        gid = g; index = i; lookahead = la; itemid = -1;
    }
};

// LR(1) 状态结构（类似 LR0 的 dfaState）
struct lr1State
{
    int sid;
    vector<int> originV;                    // 初始 core item id 列表（用于状态判重）
    vector<int> itemV;                      // 包含的 lr1Item 的 id
    bool isEnd = false;
    bool isSpecial = false;
    vector<nextStateUnit> nextStateVector;  // 复用 nextStateUnit { string c; int sid; }
    set<string> right_VNs;                  // 已处理过的非终结符（闭包时使用）
};

// 全局 LR(1) 容器（reset() 中已经清除）
extern vector<lr1Item> lr1Items;
extern vector<lr1State> lr1States;
extern set<string> lr1VN;
extern set<string> lr1VT;
extern set<int> lr1VisitedStates;
extern int lr1ItemCount;
extern int lr1StateCount;

class GenerateLR1DFA
{
public:
    GenerateLR1DFA();

    // 辅助函数，判断两个项目集合是否相同
    static bool sameItemSet(const vector<int>& A, const vector<int>& B);

    // LR(1)DFA 核心生成入口函数
    static void getLR1();

    // 创建 LR(1) 初始状态
    static void createFirstLR1State();

    // 递归生成 LR(1) 状态
    static void generateLR1State(int stateId);

    // 计算符号序列的 FIRST 集合
    static set<string> firstOfSequence(const vector<string>& seq);

    // 判断是否为新的 LR(1) 项
    static int isNewLR1Item(int gid, int index, const string &lookahead);

    // 判断是否为新的 LR(1) 状态
    static int isNewLR1State(const vector<int>& items);

    // 获取状态文法的字符串表示
    static string getLR1StateGrammar(const lr1State& st);
};

#endif // GENERATELR1DFA_H
