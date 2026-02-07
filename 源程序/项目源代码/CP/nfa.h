/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: nfa.h
 * @Brief: NFA构建类，根据预处理的正则表达式构建对应的NFA图（表格形式）
 * @Module: NFA构建模块
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
#ifndef GENERATENFA_H
#define GENERATENFA_H

#include "root.h"

// 全局结点计数器
extern int nodeCount;

// 全局字符统计
// NFA
extern set<char> nfaCharSet;

// nfaNode和nfaEdge互为成员，故需在此作提前声明
struct nfaNode;

// 结构体，NFA图的边
struct nfaEdge
{
    char c;                     // 转移字符
    nfaNode* next;              // 指向目标节点的指针
};

// 结构体，NFA图的结点
struct nfaNode
{
    int id;                     // 结点唯一编号
    bool isStart;               // 初态标识
    bool isEnd;                 // 终态标识
    vector<nfaEdge> edges;      // 边，用vector因为有可能一个结点有多条边可走
    nfaNode() {
        id = nodeCount++;
        isStart = false;
        isEnd = false;
    }
};

// 结构体，NFA图
struct NFA
{
    nfaNode* start;
    nfaNode* end;
    NFA() {}
    NFA(nfaNode* s, nfaNode* e)
    {
        start = s;
        end = e;
    }
};

// 结构体，状态转换表单个结点
struct statusTableNode
{
    string flag;            // 标记初态还是终态
    int id;                 // 唯一id值
    map<char, set<int>> m;  // 对应字符能到达的状态
    statusTableNode()
    {
        flag = "";          // 默认为空
    }
};

// 状态转换表
extern unordered_map<int, statusTableNode> statusTable;

// statusTable 插入顺序记录，方便后续输出
extern vector<int> insertionOrder;

// 初态集合
extern set<int> startNFAstatus;

// 终态集合
extern set<int> endNFAstatus;

// 映射字符表（保存映射字符到显示名称的关系）
extern unordered_map<char, string> mappedCharTable;

// 构建完成后的 NFA，可用于后续的 DFA 构建生成
extern NFA final_nfa;

class GenerateNFA
{
public:
    // 辅助函数，获取字符对应的列名
    static string getColumnForChar(char c);

    // 辅助函数，获取NFA状态转换表（用于显示）
    static const unordered_map<int, statusTableNode>& getStatusTable();

    // 辅助函数，获取插入顺序的状态ID列表
    static const vector<int>& getInsertionOrder();

    // 辅助函数，获取映射字符表
    static const unordered_map<char, string>& getMappedCharTable();

    // 辅助函数，注册映射字符
    static void registerMappedChar(char mappedChar, const string& displayName);

    // 辅助函数，获取字符（串）的显示名称
    static string getCharDisplayName(char c);

    // 辅助函数，判断是否为映射字符
    static bool isMappedChar(char c);

    // 辅助函数，创建映射字符的NFA
    static NFA CreateMappedCharNFA(char mappedChar);

    // 辅助函数，获取运算符优先级
    static int Precedence(char op);

    // 辅助函数，将集合转换为字符串
    static string set2string(set<int> s);

    // 辅助函数，进行字符串修剪
    static string trim(const string& str);

    // 辅助函数，调试输出状态表
    static void printStatusTable();

    // 创建基本字符的NFA
    static NFA CreateBasicNFA(char character);

    // 创建可选运算（?）的NFA
    static NFA CreateOptionalNFA(NFA nfa1);

    // 创建闭包运算（*）的NFA
    static NFA CreateZeroOrMoreNFA(NFA nfa1);

    // 创建选择运算（|）的NFA
    static NFA CreateUnionNFA(NFA nfa1, NFA nfa2);

    // 创建连接运算（@）的NFA
    static NFA CreateConcatenationNFA(NFA nfa1, NFA nfa2);

    // 正则表达式转NFA的主入口函数
    static NFA regex2NFA(string regex);

    // 生成NFA状态转换表
    static void createNFAStatusTable(NFA& nfa);

    GenerateNFA();
};

#endif // GENERATENFA_H
