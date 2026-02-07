/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: dfa.cpp
 * @Brief: DFA构建类的实现部分，根据构建的NFA构建对应的DFA图（表格形式），并进行最小化处理
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
#include "regex.h"
#include "dfa.h"

// DFA
set<char> dfaCharSet;
set<set<int>> dfaStatusSet;
vector<dfaNode> dfaTable;

//下面用于DFA最小化
set<int> dfaEndStatusSet;
set<int> dfaNotEndStatusSet;
map<set<int>, int> dfa2numberMap;
int startStaus;
vector<dfaMinNode> dfaMinTable;
vector<set<int>> divideVector;
map<int, int> dfaMinMap;

GenerateDFA::GenerateDFA()
{

}

// 辅助函数，DFA最小化状态判断
// 判断是否含有初态终态，含有则返回对应字符串
string GenerateDFA::minSetHasStartOrEnd(set<int>& statusSet)
{
    string result = "";
    if (statusSet.count(startStaus) > 0) {
        result += "-";
    }

    for (const int& element : dfaEndStatusSet) {
        if (statusSet.count(element) > 0) {
            result += "+";
            break;  // 可能会有多个终态同时包含，但只要一个
        }
    }

    return result;
}

// 辅助函数，状态分割函数
// 根据字符 ch 将状态集合 node 分成两个子集合
void GenerateDFA::splitSet(int i, char ch)
{
    set<int> result;
    auto& node = divideVector[i];
    int s = -2;

    for (auto state : node)
    {
        int thisNum;
        if (dfaTable[state - 1].transitions.find(ch) == dfaTable[state - 1].transitions.end())
            thisNum = -1; // 空集
        else {
            // 根据字符 ch 找到下一个状态
            int next_state = dfa2numberMap[dfaTable[state - 1].transitions[ch]];
            thisNum = dfaMinMap[next_state];    // 这个状态的下标是多少
        }

        if (s == -2)             // 初始下标
            s = thisNum;
        else if (thisNum != s)   // 若下标不同，则有问题，需要分出来
            result.insert(state);
    }

    // 删除要删除的元素
    for (int state : result)
        node.erase(state);

    // 都遍历完了，如果result不是空，证明有新的，加入vector中
    if (!result.empty()) {
        divideVector.push_back(result);
        // 同时更新下标
        for (auto a : result)
            dfaMinMap[a] = divideVector.size() - 1;
    }
}

// 辅助函数，判断是否含有初态、终态，含有则返回对应字符串
string GenerateDFA::setHasStartOrEnd(set<int>& statusSet)
{
    string result = "";
    for (const int& element : startNFAstatus)
        if (statusSet.count(element) > 0)
            result += "-";

    for (const int& element : endNFAstatus)
        if (statusSet.count(element) > 0)
            result += "+";

    return result;
}

// 辅助函数，DFA 使用的字符显示函数
string GenerateDFA::getCharDisplayNameForDFA(char c) {
    return GenerateNFA::getCharDisplayName(c);
}

// DFA 格式化输出函数
void GenerateDFA::printDfaTable(const vector<dfaNode>& dfaTable) {
    qDebug() << "\n========== DFA状态转换表 ==========";
    for (size_t i = 0; i < dfaTable.size(); ++i) {
        const dfaNode& node = dfaTable[i];

        // 构建状态标志字符串
        string flagStr;
        if (node.flag.find('-') != string::npos) flagStr += "初态 ";
        if (node.flag.find('+') != string::npos) flagStr += "终态 ";
        if (flagStr.empty()) flagStr = "中间态";

        qDebug() << "DFA状态" << (i+1) << " (" << QString::fromStdString(flagStr) << "):";

        // 显示包含的NFA状态
        string nfaStatesStr;
        for (int state : node.nfaStates) {
            nfaStatesStr += to_string(state) + ",";
        }
        if (!nfaStatesStr.empty()) {
            nfaStatesStr.pop_back();
            qDebug() << "  包含NFA状态: {" << QString::fromStdString(nfaStatesStr) << "}";
        }

        // 显示状态转换
        if (node.transitions.empty()) {
            qDebug() << "  无转换";
        } else {
            for (const auto& transition : node.transitions) {
                char inputChar = transition.first;
                const set<int>& nextStates = transition.second;

                // 获取字符显示名称
                string displayChar = getCharDisplayNameForDFA(inputChar);

                // 构建下一状态字符串
                string nextStatesStr;
                for (int nextState : nextStates) {
                    // 查找对应的DFA状态编号
                    for (const auto& pair : dfa2numberMap) {
                        if (pair.first == nextStates) {
                            nextStatesStr += to_string(pair.second) + ",";
                            break;
                        }
                    }
                }

                if (!nextStatesStr.empty()) {
                    nextStatesStr.pop_back();
                    qDebug() << "  " << QString::fromStdString(displayChar) << " -> {" << QString::fromStdString(nextStatesStr) << "}";
                } else {
                    qDebug() << "  " << QString::fromStdString(displayChar) << " -> 空集";
                }
            }
        }
        qDebug() << "  --------------------";
    }

    // 显示统计信息
    qDebug() << "\n========== DFA统计信息 ==========";
    qDebug() << "DFA总状态数:" << dfaTable.size();
    qDebug() << "初态编号:" << startStaus;

    // 显示终态集合
    string endStatesStr;
    for (int state : dfaEndStatusSet) {
        endStatesStr += to_string(state) + ",";
    }
    if (!endStatesStr.empty()) {
        endStatesStr.pop_back();
        qDebug() << "终态集合: {" << QString::fromStdString(endStatesStr) << "}";
    }

    // 显示字符集
    qDebug() << "DFA字符集大小:" << dfaCharSet.size();
    string charSetStr;
    for (char c : dfaCharSet) {
        charSetStr += getCharDisplayNameForDFA(c) + " ";
    }
    qDebug() << "字符集:" << QString::fromStdString(charSetStr);
}

// 最小化 DFA 格式化输出函数
void GenerateDFA::printMinimizedDfaTable(const vector<dfaMinNode>& dfaMinTable) {
    qDebug() << "\n========== 最小化DFA状态转换表 ==========";
    for (size_t i = 0; i < dfaMinTable.size(); ++i) {
        const dfaMinNode& node = dfaMinTable[i];

        // 构建状态标志字符串
        string flagStr;
        if (node.flag.find('-') != string::npos) flagStr += "初态 ";
        if (node.flag.find('+') != string::npos) flagStr += "终态 ";
        if (flagStr.empty()) flagStr = "中间态";

        qDebug() << "最小化状态" << node.id << " (" << QString::fromStdString(flagStr) << "):";

        // 显示包含的原始DFA状态
        for (size_t j = 0; j < divideVector.size(); ++j) {
            if (j == (size_t)node.id) {
                string originalStatesStr;
                for (int state : divideVector[j]) {
                    originalStatesStr += to_string(state) + ",";
                }
                if (!originalStatesStr.empty()) {
                    originalStatesStr.pop_back();
                    qDebug() << "  包含原始DFA状态: {" << QString::fromStdString(originalStatesStr) << "}";
                }
                break;
            }
        }

        // 显示状态转换
        if (node.transitions.empty()) {
            qDebug() << "  无转换";
        } else {
            for (const auto& transition : node.transitions) {
                char inputChar = transition.first;
                int nextState = transition.second;

                // 获取字符显示名称
                string displayChar = getCharDisplayNameForDFA(inputChar);

                if (nextState == -1) {
                    qDebug() << "  " << QString::fromStdString(displayChar) << " -> 空集";
                } else {
                    qDebug() << "  " << QString::fromStdString(displayChar) << " -> 状态" << nextState;
                }
            }
        }
        qDebug() << "  --------------------";
    }

    // 显示统计信息
    qDebug() << "\n========== 最小化DFA统计信息 ==========";
    qDebug() << "最小化后状态数:" << dfaMinTable.size();

    // 查找初态
    int minStartState = -1;
    for (const auto& node : dfaMinTable) {
        if (node.flag.find('-') != string::npos) {
            minStartState = node.id;
            break;
        }
    }
    if (minStartState != -1) {
        qDebug() << "初态编号:" << minStartState;
    }

    // 显示终态集合
    string minEndStatesStr;
    for (const auto& node : dfaMinTable) {
        if (node.flag.find('+') != string::npos) {
            minEndStatesStr += to_string(node.id) + ",";
        }
    }
    if (!minEndStatesStr.empty()) {
        minEndStatesStr.pop_back();
        qDebug() << "终态集合: {" << QString::fromStdString(minEndStatesStr) << "}";
    }
}

//计算 NFA 状态的ε闭包，确保正确处理映射字符
set<int> GenerateDFA::epsilonClosure(int id) {
    set<int> eResult{ id };
    stack<int> stack;
    stack.push(id);

    while (!stack.empty())
    {
        int current = stack.top();
        stack.pop();

        // 获取ε转换
        set<int> eClosure = statusTable[current].m[EPSILON];
        for (auto t : eClosure)
        {
            if (eResult.find(t) == eResult.end())
            {
                eResult.insert(t);
                stack.push(t);
            }
        }
    }

    return eResult;
}

// 计算字符转换闭包，确保正确处理映射字符
set<int> GenerateDFA::otherCharClosure(int id, char ch) {
    set<int> otherResult{};
    set<int> processed;
    stack<int> stack;
    stack.push(id);

    while (!stack.empty())
    {
        int current = stack.top();
        stack.pop();

        if (processed.find(current) != processed.end())
            continue;

        processed.insert(current);

        // 获取字符ch的转换
        set<int> otherClosure = statusTable[current].m[ch];
        for (auto o : otherClosure)
        {
            auto tmp = epsilonClosure(o);
            otherResult.insert(tmp.begin(), tmp.end());
            stack.push(o);
        }
    }

    return otherResult;
}

// NFA 转 DFA 函数（格式化输出）
void GenerateDFA::NFA2DFA(NFA& nfa) {
    // 清空之前的DFA数据
    dfaStatusSet.clear();
    dfaTable.clear();
    dfaEndStatusSet.clear();
    dfaNotEndStatusSet.clear();
    dfa2numberMap.clear();

    int dfaStatusCount = 1;
    auto start = nfa.start;                                         // 获得NFA图的起始位置
    auto startId = start->id;                                       // 获得起始编号
    dfaNode startDFANode;
    startDFANode.nfaStates = epsilonClosure(startId);               // 初始闭包
    startDFANode.flag = setHasStartOrEnd(startDFANode.nfaStates);   // 判断初态终态
    deque<set<int>> newStatus{};
    dfa2numberMap[startDFANode.nfaStates] = dfaStatusCount;
    startStaus = dfaStatusCount;

    if (setHasStartOrEnd(startDFANode.nfaStates).find("+") != string::npos) {
        dfaEndStatusSet.insert(dfaStatusCount++);
    }
    else
    {
        dfaNotEndStatusSet.insert(dfaStatusCount++);
    }

    // 对每个字符进行遍历
    for (auto ch : dfaCharSet)
    {
        set<int> thisChClosure{};
        for (auto c : startDFANode.nfaStates)
        {
            set<int> tmp = otherCharClosure(c, ch);
            thisChClosure.insert(tmp.begin(), tmp.end());
        }
        if (thisChClosure.empty())                                  // 如果这个闭包是空集没必要继续下去了
        {
            continue;
        }
        int presize = dfaStatusSet.size();
        dfaStatusSet.insert(thisChClosure);
        int lastsize = dfaStatusSet.size();
        // 不管一不一样都是该节点这个字符的状态
        startDFANode.transitions[ch] = thisChClosure;
        // 若大小不一样，则证明是新状态
        if (lastsize > presize)
        {
            dfa2numberMap[thisChClosure] = dfaStatusCount;
            newStatus.push_back(thisChClosure);
            if (setHasStartOrEnd(thisChClosure).find("+") != string::npos) {
                dfaEndStatusSet.insert(dfaStatusCount++);
            }
            else
            {
                dfaNotEndStatusSet.insert(dfaStatusCount++);
            }
        }
    }
    dfaTable.push_back(startDFANode);

    // 对后面的新状态进行不停遍历
    while (!newStatus.empty())
    {
        // 拿出一个新状态
        set<int> ns = newStatus.front();
        newStatus.pop_front();
        dfaNode DFANode;
        DFANode.nfaStates = ns;                     // 该节点状态集合
        DFANode.flag = setHasStartOrEnd(ns);

        for (auto ch : dfaCharSet)
        {
            set<int> thisChClosure{};
            for (auto c : ns)
            {
                set<int> tmp = otherCharClosure(c, ch);
                thisChClosure.insert(tmp.begin(), tmp.end());
            }
            if (thisChClosure.empty())              // 如果这个闭包是空集没必要继续下去了
            {
                continue;
            }
            int presize = dfaStatusSet.size();
            dfaStatusSet.insert(thisChClosure);
            int lastsize = dfaStatusSet.size();
            // 不管一不一样都是该节点这个字符的状态
            DFANode.transitions[ch] = thisChClosure;
            // 如果大小不一样，证明是新状态
            if (lastsize > presize)
            {
                dfa2numberMap[thisChClosure] = dfaStatusCount;
                newStatus.push_back(thisChClosure);
                if (setHasStartOrEnd(thisChClosure).find("+") != string::npos) {
                    dfaEndStatusSet.insert(dfaStatusCount++);
                }
                else
                {
                    dfaNotEndStatusSet.insert(dfaStatusCount++);
                }
            }
        }
        dfaTable.push_back(DFANode);
    }

    // 输出DFA表
    printDfaTable(dfaTable);
    qDebug() << "NFA转DFA完成！";
}

// DFA 最小化函数
void GenerateDFA::DFAminimize() {
    divideVector.clear();
    dfaMinMap.clear();
    dfaMinTable.clear();

    // 存入非终态、终态集合
    if (dfaNotEndStatusSet.size() != 0)
        divideVector.push_back(dfaNotEndStatusSet);

    // 初始化map
    for (auto t : dfaNotEndStatusSet)
        dfaMinMap[t] = divideVector.size() - 1;

    divideVector.push_back(dfaEndStatusSet);

    for (auto t : dfaEndStatusSet)
        dfaMinMap[t] = divideVector.size() - 1;

    // 当flag为1时，一直循环
    int continueFlag = 1;

    while (continueFlag) {
        continueFlag = 0;
        int size1 = divideVector.size();

        for (int i = 0; i < size1; i++)
            // 逐字符尝试分割状态集合
            for (char ch : dfaCharSet)
                splitSet(i, ch);

        int size2 = divideVector.size();
        if (size2 > size1)
            continueFlag = 1;
    }

    for (size_t dfaMinCount = 0; dfaMinCount < divideVector.size(); dfaMinCount++) {
        auto& v = divideVector[dfaMinCount];
        dfaMinNode d;
        d.flag = minSetHasStartOrEnd(v);
        d.id = dfaMinCount;
        // 逐字符
        for (char ch : dfaCharSet) {
            if (v.size() == 0) {
                d.transitions[ch] = -1;             // 空集特殊判断
                continue;
            }
            int i = *(v.begin());                   // 拿出一个来
            if (dfaTable[i - 1].transitions.find(ch) == dfaTable[i - 1].transitions.end()) {
                d.transitions[ch] = -1;             // 空集特殊判断
                continue;
            }
            int next_state = dfa2numberMap[dfaTable[i - 1].transitions[ch]];
            int thisNum = dfaMinMap[next_state];    // 状态下标
            d.transitions[ch] = thisNum;
        }
        dfaMinTable.push_back(d);
    }

    // 输出最小化DFA表
    printMinimizedDfaTable(dfaMinTable);
    qDebug() << "DFA最小化完成！";
}
