/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: nfa.h
 * @Brief: NFA构建类的实现部分，根据预处理的正则表达式构建对应的NFA图（表格形式）
 * @Module: NFA构建模块
 *
 * @Current Version: 2.2.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/8    袁知本       模块化，从原先的widget.cpp中分离
 *   2.1.0    2026/1/18   袁知本       新增辅助函数isMappedChar()
 *   2.2.0    2026/1/26   袁知本       修复映射字符处理，保留映射名称
 ***********************************************************************/
#include "nfa.h"
#include "regex.h"
#include "dfa.h"

int nodeCount = 0;

// NFA
set<char> nfaCharSet;
unordered_map<int, statusTableNode> statusTable;
vector<int> insertionOrder;
set<int> startNFAstatus;
set<int> endNFAstatus;

// 映射字符表（保存映射字符到显示名称的关系）
unordered_map<char, string> mappedCharTable;

// 构建完成后的 NFA，可用于后续的 DFA 构建生成
NFA final_nfa;

GenerateNFA::GenerateNFA()
{

}

// 辅助函数，获取字符（串）的显示名称
string GenerateNFA::getCharDisplayName(char c) {
    if (c == EPSILON) return "ε";

    // 检查是否是映射字符
    if (isMappedChar(c)) {
        auto it = mappedCharTable.find(c);
        if (it != mappedCharTable.end()) {
            return it->second;
        }

        // 从m1中获取映射名称
        auto m1_it = m1.find(c);
        if (m1_it != m1.end()) {
            string displayName = m1_it->second;

            // 对于特殊的映射字符，返回更友好的显示名称
            if (displayName == "num" || displayName == "digit") return "digit";
            if (displayName == "letter") return "letter";
            if (displayName == "float") return "float_num";

            // 返回原始映射名称
            return displayName;
        }
        return string(1, c);
    }

    if (c >= 32 && c <= 126) {
        return string(1, c);
    }

    return "[ASCII:" + to_string((int)c) + "]";
}

// 辅助函数，判断是否为映射字符
bool GenerateNFA::isMappedChar(char c) {
    // 检查是否是已知的映射字符
    if (mappedCharTable.find(c) != mappedCharTable.end()) {
        return true;
    }

    // 检查是否在m1映射表中
    return m1.find(c) != m1.end();
}

// 辅助函数，注册映射字符
void GenerateNFA::registerMappedChar(char mappedChar, const string& displayName) {
    if (mappedCharTable.find(mappedChar) == mappedCharTable.end()) {
        mappedCharTable[mappedChar] = displayName;
        qDebug() << "注册映射字符: ASCII" << (int)mappedChar << " -> "
                 << QString::fromStdString(displayName);
    }
}

// 辅助函数，创建映射字符的NFA
NFA GenerateNFA::CreateMappedCharNFA(char mappedChar) {
    // 查找m1映射表获取映射名称
    auto m1_it = m1.find(mappedChar);
    if (m1_it == m1.end()) {
        qDebug() << "错误：未找到映射字符的映射关系 ASCII" << (int)mappedChar;
        // 当作普通字符处理
        NFA nfa = CreateBasicNFA(mappedChar);
        nfaCharSet.insert(mappedChar);
        dfaCharSet.insert(mappedChar);
        return nfa;
    }

    string mappedStr = m1_it->second;
    string displayName;

    // 确定显示名称
    if (mappedStr == "num" || mappedStr == "digit") {
        displayName = "digit";
    } else if (mappedStr == "letter") {
        displayName = "letter";
    } else if (mappedStr == "float") {
        displayName = "float_num";
    } else {
        // 对于转义字符，直接使用原始字符
        if (mappedStr.size() == 3 && mappedStr[0] == '\\' && mappedStr[2] == '\\') {
            displayName = string(1, mappedStr[1]);
        } else {
            displayName = mappedStr;
        }
    }

    // 注册映射字符
    registerMappedChar(mappedChar, displayName);

    qDebug() << "处理映射字符: ASCII" << (int)mappedChar
             << " -> " << QString::fromStdString(displayName);

    // 创建基本的映射字符NFA
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge;
    edge.c = mappedChar;  // 使用映射字符本身作为转换字符
    edge.next = end;
    start->edges.push_back(edge);

    // 将映射字符添加到字符集
    nfaCharSet.insert(mappedChar);
    dfaCharSet.insert(mappedChar);

    NFA nfa(start, end);
    return nfa;
}

// 辅助函数，获取运算符优先级
int GenerateNFA::Precedence(char op) {
    switch(op) {
        case '|': return 1;
        case '@': return 2;
        case '*': return 3;
        case '?': return 3;
    default: return 0;
    }
}

// 辅助函数，将集合转换为字符串，用于状态集合的可视化输出
string GenerateNFA::set2string(set<int> s) {
    string result;
    for (int i : s) {
        result.append(to_string(i));
        result.append(",");
    }

    if (result.size() != 0)
        result.pop_back();

    return result;
}

// 辅助函数，进行字符串修剪
string GenerateNFA::trim(const string& str) {
    if (str.empty() || str == "\\n")
        return str;
    string ret(str);
    ret.erase(0, ret.find_first_not_of("\\"));
    ret.erase(ret.find_last_not_of("\\") + 1);
    return ret;
}

// 辅助函数，调试输出状态表（使用全局变量）
void GenerateNFA::printStatusTable() {
    qDebug() << "========== NFA状态表 ==========";

    // 显示映射字符表
    qDebug() << "\n映射字符表:";
    for (const auto& entry : mappedCharTable) {
        qDebug() << "  ASCII" << (int)entry.first << " -> "
                 << QString::fromStdString(entry.second);
    }

    qDebug() << "\n状态转换表:";

    // 动态确定列顺序：收集所有出现的字符类别
    set<string> columnSet;

    // 先添加固定列
    columnSet.insert("digit");
    columnSet.insert("letter");
    columnSet.insert(".");
    columnSet.insert("#");  // 空转移

    // 收集所有出现的转移字符类别
    for (int stateId : insertionOrder) {
        if (statusTable.find(stateId) != statusTable.end()) {
            const statusTableNode& node = statusTable[stateId];
            for (const auto& entry : node.m) {
                char transitionChar = entry.first;
                string columnName = getColumnForChar(transitionChar);
                columnSet.insert(columnName);
            }
        }
    }

    // 转换为有序向量
    vector<string> columnOrder(columnSet.begin(), columnSet.end());

    // 确保 digit 和 letter 在前
    vector<string> finalColumnOrder;
    finalColumnOrder.push_back("digit");
    finalColumnOrder.push_back("letter");
    finalColumnOrder.push_back("#");

    for (const string& col : columnOrder) {
        if (col != "digit" && col != "letter" && col != "#") {
            finalColumnOrder.push_back(col);
        }
    }

    columnOrder = finalColumnOrder;

    // 打印表头
    QString header = "| 标志 | ID |";
    for (const string& col : columnOrder) {
        header += QString::fromStdString(col) + " |";
    }
    qDebug() << header;

    // 打印分隔线
    QString separator = "|---|---|";
    for (size_t i = 0; i < columnOrder.size(); i++) {
        separator += "---|";
    }
    qDebug() << separator;

    // 按插入顺序输出状态（这是表格的行序号）
    int rowNum = 1;
    for (int stateId : insertionOrder) {
        if (statusTable.find(stateId) == statusTable.end()) {
            qDebug() << QString("| %1 | %2 | 状态未定义 |").arg(rowNum).arg(stateId);
            rowNum++;
            continue;
        }

        const statusTableNode& node = statusTable[stateId];

        // 获取标志
        QString flag;
        if (startNFAstatus.find(stateId) != startNFAstatus.end()) {
            flag = "-";
        } else if (endNFAstatus.find(stateId) != endNFAstatus.end()) {
            flag = "+";
        } else {
            flag = " ";
        }

        // 初始化列值（全部为空）
        unordered_map<string, QString> columnValues;
        for (const string& col : columnOrder) {
            columnValues[col] = "   ";
        }

        // 填充实际转移
        for (const auto& entry : node.m) {
            char transitionChar = entry.first;
            const set<int>& targetStates = entry.second;

            // 获取列名
            string columnName = getColumnForChar(transitionChar);

            // 构建目标状态字符串
            QString targets;
            for (int targetId : targetStates) {
                targets += QString::number(targetId) + ",";
            }
            if (!targets.isEmpty()) {
                targets.chop(1);  // 移除最后的逗号
            }

            // 合并相同列的转移（用逗号分隔）
            if (columnValues[columnName] != "   ") {
                columnValues[columnName] += "," + targets;
            } else {
                columnValues[columnName] = targets;
            }
        }

        // 构建行字符串
        QString rowStr = QString("| %1 | %2 |").arg(flag).arg(stateId);

        for (const string& col : columnOrder) {
            rowStr += columnValues[col] + " |";
        }

        qDebug() << rowStr;
        rowNum++;
    }

    // 输出统计信息
    qDebug() << "\n统计信息:";
    qDebug() << "总状态数:" << statusTable.size();

    // 找出所有终态
    set<int> allEndStates;
    for (int stateId : insertionOrder) {
        if (statusTable.find(stateId) != statusTable.end()) {
            if (endNFAstatus.find(stateId) != endNFAstatus.end()) {
                allEndStates.insert(stateId);
            }
        }
    }
    qDebug() << "初态数:" << startNFAstatus.size();
    qDebug() << "终态数:" << allEndStates.size();

    if (!allEndStates.empty()) {
        QString endStatesStr;
        for (int endState : allEndStates) {
            endStatesStr += QString::number(endState) + ",";
        }
        endStatesStr.chop(1);
        qDebug() << "终态ID:" << endStatesStr;
    }

    qDebug() << "=============================";
}

// 辅助函数，获取字符对应的列名
string GenerateNFA::getColumnForChar(char c) {
    if (c == EPSILON) return "#";

    if (c == '.') return ".";

    // 检查是否是映射字符
    if (isMappedChar(c)) {
        string displayName = getCharDisplayName(c);
        if (displayName == "digit" || displayName == "num") return "digit";
        if (displayName == "letter") return "letter";
        if (displayName == "float_num") return "float";
        return displayName;
    }

    // 普通字符分类
    if (c >= '0' && c <= '9') return "digit";
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) return "letter";

    // 特殊字符直接返回
    return string(1, c);
}

// 创建基本字符的NFA
NFA GenerateNFA::CreateBasicNFA(char character) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge;
    edge.c = character;
    edge.next = end;
    start->edges.push_back(edge);

    // 如果是普通可打印字符，添加到字符集
    if (character >= 32 && character <= 126) {
        nfaCharSet.insert(character);
        dfaCharSet.insert(character);
    }

    NFA nfa(start, end);

    return nfa;
}

// 创建可选运算（?）的NFA
NFA GenerateNFA::CreateOptionalNFA(NFA nfa1) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;

    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = end;
    start->edges.push_back(edge2);

    nfa1.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = end;
    nfa1.end->edges.push_back(edge3);

    NFA nfa(start, end);

    return nfa;
}

// 创建闭包运算（*）的NFA
NFA GenerateNFA::CreateZeroOrMoreNFA(NFA nfa1) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;

    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = end;
    start->edges.push_back(edge2);

    nfa1.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = nfa1.start;
    nfa1.end->edges.push_back(edge3);

    nfaEdge edge4;
    edge4.c = EPSILON;
    edge4.next = end;
    nfa1.end->edges.push_back(edge4);

    NFA nfa{ start,end };

    return nfa;
}

// 创建选择运算（|）的NFA
NFA GenerateNFA::CreateUnionNFA(NFA nfa1, NFA nfa2) {
    nfaNode* start = new nfaNode();
    nfaNode* end = new nfaNode();

    start->isStart = true;
    end->isEnd = true;

    nfaEdge edge1;
    edge1.c = EPSILON;
    edge1.next = nfa1.start;
    start->edges.push_back(edge1);
    nfa1.start->isStart = false;

    nfaEdge edge2;
    edge2.c = EPSILON;
    edge2.next = nfa2.start;
    start->edges.push_back(edge2);
    nfa2.start->isStart = false;

    nfa1.end->isEnd = false;
    nfa2.end->isEnd = false;

    nfaEdge edge3;
    edge3.c = EPSILON;
    edge3.next = end;
    nfa1.end->edges.push_back(edge3);

    nfaEdge edge4;
    edge4.c = EPSILON;
    edge4.next = end;
    nfa2.end->edges.push_back(edge4);

    NFA nfa{start, end};

    return nfa;
}

// 创建连接运算（@）的NFA
NFA GenerateNFA::CreateConcatenationNFA(NFA nfa1, NFA nfa2) {
    nfa1.end->isEnd = false;
    nfa2.start->isStart = false;

    nfaEdge edge;
    edge.c = EPSILON;
    edge.next = nfa2.start;
    nfa1.end->edges.push_back(edge);

    NFA nfa;
    nfa.start = nfa1.start;
    nfa.end = nfa2.end;

    return nfa;
}

// 正则表达式转NFA的主入口函数
NFA GenerateNFA::regex2NFA(string regex) {
    // 清空映射字符表
    mappedCharTable.clear();

    // 双栈法，创建两个栈opStack（运算符栈）,nfaStack（nfa图栈）
    stack<char> opStack;
    stack<NFA> nfaStack;

    qDebug() << "开始构建NFA，输入正则表达式:";
    string debugStr;
    for (char c : regex) {
        if (isMappedChar(c)) {
            auto it = m1.find(c);
            if (it != m1.end()) {
                debugStr += "{" + it->second + "}";
            } else {
                debugStr += string(1, c);
            }
        } else if (c >= 32 && c <= 126) {
            debugStr += string(1, c);
        } else {
            debugStr += "[ASCII:" + to_string((int)c) + "]";
        }
    }
    qDebug() << QString::fromStdString(debugStr);

    // 主处理循环
    for (char c : regex)
    {
        switch (c)
        {
        case ' ': // 空格跳过
            break;
        case '(':
            opStack.push(c);
            break;
        case ')':
            while (!opStack.empty() && opStack.top() != '(')
            {
                char op = opStack.top();
                opStack.pop();

                if (op == '|') {
                    if (nfaStack.size() < 2) {
                        qDebug() << "正则表达式语法错误：不足以处理运算符 |";
                        exit(-1);
                    }
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    NFA resultNFA = CreateUnionNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
                else if (op == '@') {
                    if (nfaStack.size() < 2) {
                        qDebug() << "正则表达式语法错误：不足以处理运算符 @";
                        exit(-1);
                    }
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    NFA resultNFA = CreateConcatenationNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
            }
            if (opStack.empty())
            {
                qDebug() << "括号未闭合，请检查正则表达式！";
                exit(-1);
            }
            else
            {
                opStack.pop(); // 弹出(
            }
            break;
        case '|':
        case '@':
            while (!opStack.empty() && (opStack.top() == '|' || opStack.top() == '@') &&
                Precedence(opStack.top()) >= Precedence(c))
            {
                char op = opStack.top();
                opStack.pop();

                if (op == '|') {
                    if (nfaStack.size() < 2) {
                        qDebug() << "正则表达式语法错误：不足以处理运算符 |";
                        exit(-1);
                    }
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    NFA resultNFA = CreateUnionNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
                else if (op == '@') {
                    if (nfaStack.size() < 2) {
                        qDebug() << "正则表达式语法错误：不足以处理运算符 @";
                        exit(-1);
                    }
                    NFA nfa2 = nfaStack.top();
                    nfaStack.pop();
                    NFA nfa1 = nfaStack.top();
                    nfaStack.pop();

                    NFA resultNFA = CreateConcatenationNFA(nfa1, nfa2);
                    nfaStack.push(resultNFA);
                }
            }
            opStack.push(c);
            break;
        case '?':
        case '*':
            if (!nfaStack.empty()) {
                NFA nfa = nfaStack.top();
                nfaStack.pop();
                if (c == '?') {
                    NFA resultNFA = CreateOptionalNFA(nfa);
                    nfaStack.push(resultNFA);
                }
                else if (c == '*') {
                    NFA resultNFA = CreateZeroOrMoreNFA(nfa);
                    nfaStack.push(resultNFA);
                }
            }
            else {
                qDebug() << "正则表达式语法错误：闭包操作没有NFA可用！";
                exit(-1);
            }
            break;
        default:
            if (isMappedChar(c)) {
                // 处理映射字符（保留映射名称）
                NFA nfa = CreateMappedCharNFA(c);
                nfaStack.push(nfa);
            } else {
                // 处理普通字符
                NFA nfa = CreateBasicNFA(c);
                nfaStack.push(nfa);
            }
            break;
        }
    }

    while (!opStack.empty())
    {
        char op = opStack.top();
        opStack.pop();

        if (op == '|' || op == '@')
        {
            if (nfaStack.size() < 2)
            {
                qDebug() << "正则表达式语法错误：不足以处理运算符 " << op << "！";
                exit(-1);
            }

            NFA nfa2 = nfaStack.top();
            nfaStack.pop();
            NFA nfa1 = nfaStack.top();
            nfaStack.pop();

            if (op == '|')
            {
                NFA resultNFA = CreateUnionNFA(nfa1, nfa2);
                nfaStack.push(resultNFA);
            }
            else if (op == '@')
            {
                NFA resultNFA = CreateConcatenationNFA(nfa1, nfa2);
                nfaStack.push(resultNFA);
            }
        }
        else
        {
            qDebug() << "正则表达式语法错误：未知的运算符 " << op << "！";
            exit(-1);
        }
    }

    if (nfaStack.empty()) {
        qDebug() << "错误：NFA栈为空，无法构建NFA！";
        exit(-1);
    }

    NFA result = nfaStack.top();
    qDebug() << "NFA图构建完毕";

    // 输出映射字符表信息
    qDebug() << "\n映射字符统计:";
    qDebug() << "映射字符数量:" << mappedCharTable.size();
    for (const auto& entry : mappedCharTable) {
        qDebug() << "  " << (int)entry.first << " -> " << QString::fromStdString(entry.second);
    }

    qDebug() << "\n字符集统计:";
    qDebug() << "NFA字符集大小:" << nfaCharSet.size();
    qDebug() << "DFA字符集大小:" << dfaCharSet.size();

    createNFAStatusTable(result);
    qDebug() << "状态转换表构建完毕";

    // 打印状态表
    printStatusTable();

    return result;
}

// 生成 NFA 状态转换表，使用 DFS 算法
void GenerateNFA::createNFAStatusTable(NFA& nfa) {
    stack<nfaNode*> nfaStack;
    set<nfaNode*> visitedNodes;

    // 创建节点ID到节点的映射
    unordered_map<int, nfaNode*> idToNode;

    // 清空现有数据
    statusTable.clear();
    insertionOrder.clear();
    startNFAstatus.clear();
    endNFAstatus.clear();

    nfaNode* startNode = nfa.start;

    // 深度优先遍历，收集所有节点
    nfaStack.push(startNode);
    while (!nfaStack.empty()) {
        nfaNode* currentNode = nfaStack.top();
        nfaStack.pop();

        if (visitedNodes.find(currentNode) != visitedNodes.end()) {
            continue;
        }

        visitedNodes.insert(currentNode);
        idToNode[currentNode->id] = currentNode;

        for (const nfaEdge& edge : currentNode->edges) {
            if (visitedNodes.find(edge.next) == visitedNodes.end()) {
                nfaStack.push(edge.next);
            }
        }
    }

    // 创建状态表条目
    for (nfaNode* node : visitedNodes) {
        int nodeId = node->id;

        // 确保每个节点都有状态表条目
        if (statusTable.find(nodeId) == statusTable.end()) {
            statusTableNode newNode;
            newNode.id = nodeId;

            // 设置标志
            if (node->isStart) {
                newNode.flag = "-";
                startNFAstatus.insert(nodeId);
            }
            if (node->isEnd) {
                if (!newNode.flag.empty()) newNode.flag += ",";
                newNode.flag += "+";
                endNFAstatus.insert(nodeId);
            }
            if (newNode.flag.empty()) {
                newNode.flag = " ";
            }

            statusTable[nodeId] = newNode;

            // 添加到插入顺序（除了终态在最后）
            if (!node->isEnd) {
                insertionOrder.push_back(nodeId);
            }
        }
    }

    // 添加终态到插入顺序的末尾
    for (nfaNode* node : visitedNodes) {
        if (node->isEnd) {
            insertionOrder.push_back(node->id);
        }
    }

    // 再次遍历，填充转移关系
    for (nfaNode* node : visitedNodes) {
        int nodeId = node->id;

        for (const nfaEdge& edge : node->edges) {
            char transitionChar = edge.c;
            int targetId = edge.next->id;

            // 确保目标节点在状态表中
            if (statusTable.find(targetId) == statusTable.end()) {
                qDebug() << "警告：目标状态" << targetId << "不在状态表中，正在添加...";
                statusTableNode targetNode;
                targetNode.id = targetId;

                if (edge.next->isStart) {
                    targetNode.flag = "-";
                    startNFAstatus.insert(targetId);
                }
                if (edge.next->isEnd) {
                    if (!targetNode.flag.empty()) targetNode.flag += ",";
                    targetNode.flag += "+";
                    endNFAstatus.insert(targetId);
                }
                if (targetNode.flag.empty()) {
                    targetNode.flag = " ";
                }

                statusTable[targetId] = targetNode;

                // 添加到插入顺序
                if (!edge.next->isEnd) {
                    insertionOrder.push_back(targetId);
                } else {
                    // 终态添加到末尾
                    insertionOrder.push_back(targetId);
                }
            }

            // 添加转移
            statusTable[nodeId].m[transitionChar].insert(targetId);
        }
    }

    // 确保终态被正确标记
    for (nfaNode* node : visitedNodes) {
        if (node->isEnd && endNFAstatus.find(node->id) == endNFAstatus.end()) {
            endNFAstatus.insert(node->id);
            if (statusTable.find(node->id) != statusTable.end()) {
                statusTableNode& nodeEntry = statusTable[node->id];
                if (nodeEntry.flag.find('+') == string::npos) {
                    if (nodeEntry.flag != " ") {
                        nodeEntry.flag += ",+";
                    } else {
                        nodeEntry.flag = "+";
                    }
                }
            }
        }
    }

    qDebug() << "NFA状态表构建完成";
    qDebug() << "共收集到" << visitedNodes.size() << "个节点";
    qDebug() << "状态表中有" << statusTable.size() << "个状态";
    qDebug() << "初态:" << startNFAstatus.size() << "个";
    qDebug() << "终态:" << endNFAstatus.size() << "个";
}

// 辅助函数，获取NFA状态转换表（用于显示）
const unordered_map<int, statusTableNode>& GenerateNFA::getStatusTable() {
    return statusTable;
}

// 辅助函数，获取插入顺序的状态ID列表
const vector<int>& GenerateNFA::getInsertionOrder() {
    return insertionOrder;
}

// 辅助函数，获取映射字符表
const unordered_map<char, string>& GenerateNFA::getMappedCharTable() {
    return mappedCharTable;
}
