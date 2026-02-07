/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: lr0dfa.h
 * @Brief: LR(0)DFA构建类，用于根据文法构建对应的表格形式的LR(0)DFA
 * @Module: LR(0)DFA构建模块
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
#ifndef GENERATELR0DFA_H
#define GENERATELR0DFA_H

#include "root.h"

// 状态编号
extern int scnt;

// 项目编号
extern int ccnt;

// DFA表每一项项目的结构
struct dfaCell
{
    int cellid; // 这一项的编号，便于后续判断状态相同
    int gid; // 文法编号
    int index = 0; // .在第几位，如i=3, xxx.x，i=0, .xxxx, i=4, xxxx
};

// 用于通过编号快速找到对应结构
extern vector<dfaCell> dfaCellVector;

struct nextStateUnit
{
    string c;       // 通过什么字符进入这个状态
    int sid;        // 下一个状态id是什么
};

// DFA表状态
struct dfaState
{
    int sid;                                // 状态id
    vector<int> originV;                    // 未闭包前的cell
    vector<int> cellV;                      // 存储这个状态的cellid
    bool isEnd = false;                     // 是否为规约状态
    bool isSpecial = false;                 // 是否是规约移进冲突
    vector<nextStateUnit> nextStateVector;  // 下一个状态集合
    set<string> right_VNs;                  // 判断是否已经处理过这个非终结符
};

// 用于通过编号快速找到对应结构
extern vector<dfaState> dfaStateVector;

// 非终结符集合
extern set<string> VN;

// 终结符集合
extern set<string> VT;

// DFS标记数组
extern set<int> visitedStates;

class GenerateLR0DFA
{
public:
    GenerateLR0DFA();

    // 创建 LR(0) 初始状态
    static void createFirstState();

    // 递归设生成 LR(0) 状态
    static void generateLR0State(int stateId);

    // LR(0) 核心生成函数
    static void getLR0();

    // 判断是否为新的项目
    static int isNewCell(int gid, int index);

    // 判断是否为新的状态
    static int isNewState(const vector<int>& cellIds);

    // 获取状态内文法的字符串表示
    static string getStateGrammar(const dfaState& d);
};

#endif // GENERATELR0DFA_H
