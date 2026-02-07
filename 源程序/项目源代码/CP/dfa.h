/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: dfa.h
 * @Brief: DFA构建类，根据构建的NFA构建对应的DFA图（表格形式），并进行最小化处理
 * @Module: DFA构建模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/8    袁知本       模块化，从原先的widget.cpp中分离
 *   2.1.0    2026/1/18   袁知本       新增辅助函数isMappedChar()
 ***********************************************************************/
#ifndef DFA_H
#define DFA_H

#include "root.h"
#include "nfa.h"

// DFA
extern set<char> dfaCharSet;

// dfa节点
struct dfaNode
{
    string flag;                        // 是否包含终态（+）或初态（-）
    set<int> nfaStates;                 // 该DFA状态包含的NFA状态的集合
    map<char, set<int>> transitions;    // 字符到下一状态的映射
    dfaNode() {
        flag = "";
    }
};

// dfa状态去重集
extern set<set<int>> dfaStatusSet;

// dfa最终结果
extern vector<dfaNode> dfaTable;

//下面用于DFA最小化
// dfa终态集合
extern set<int> dfaEndStatusSet;

// dfa非终态集合
extern set<int> dfaNotEndStatusSet;

// set对应序号MAP
extern map<set<int>, int> dfa2numberMap;

// 存储DFA字符集（从NFA继承）下
extern int startStaus;

// dfa最小化节点
struct dfaMinNode
{
    string flag;                // 是否包含终态（+）或初态（-）
    int id;
    map<char, int> transitions; // 字符到下一状态的映射
    dfaMinNode() {
        flag = "";
    }
};

extern vector<dfaMinNode> dfaMinTable;

// 用于分割集合
extern vector<set<int>> divideVector;

// 存下标
extern map<int, int> dfaMinMap;

class GenerateDFA
{
public:
    GenerateDFA();

    // 辅助函数，DFA最小化状态判断
    static string minSetHasStartOrEnd(set<int>& statusSet);

    // 辅助函数，状态分割函数
    static void splitSet(int i, char ch);

    // 辅助函数，判断是否含有初态、终态，含有则返回对应字符串
    static string setHasStartOrEnd(set<int>& statusSet);

    // 辅助函数，DFA 使用的字符显示函数
    static string getCharDisplayNameForDFA(char c);

    // DFA 格式化输出函数
    static void printDfaTable(const vector<dfaNode>& dfaTable);

    // 最小化 DFA 格式化输出函数
    static void printMinimizedDfaTable(const vector<dfaMinNode>& dfaMinTable);

    //计算 NFA 状态的ε闭包，确保正确处理映射字符
    static set<int> epsilonClosure(int id);

    // 计算字符转换闭包，确保正确处理映射字符
    static set<int> otherCharClosure(int id, char ch);

    // NFA 转 DFA 函数（格式化输出）
    static void NFA2DFA(NFA& nfa);

    // DFA 最小化函数
    static void DFAminimize();

};

#endif // DFA_H
