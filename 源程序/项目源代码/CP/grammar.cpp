/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: grammar.cpp
 * @Brief: 文法处理类的实现部分，用于对用户输入的文法进行预处理
 * @Module: 文法处理模块
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
#include "grammar.h"

vector<string> bigAlpha;
vector<string> smallAlpha;
string grammarStr;
unordered_map<string, set<string>> grammarMap;
deque<grammarUnit> grammarDeque;
QString LR0Result;
map<pair<string, string>, int> grammarToInt;
string startSymbol;
string trueStartSymbol;

GrammarAnalyse::GrammarAnalyse()
{

}

// 判断字符 c 是否是终结符
bool GrammarAnalyse::isSmallAlpha(string c) {
    for (const auto& symbol : smallAlpha)
        if (symbol == c)
            return true;
    return false;
}

// 判断字符 c 是否是非终结符
bool GrammarAnalyse::isBigAlpha(string c) {
    for (const auto& symbol : bigAlpha)
        if (symbol == c)
            return true;
    return false;
}

// 文法初始化处理主函数
// 文法处理说明如下：
/*
 * （1）用 # 号表示空串
 * （2）第一行输入非终结符，用 | 号分隔
 * （3）第二行输入终结符，用 | 号分隔
 * （4）其他行均为文法，输入文法时，单词间用空格进行分隔，文法开头必须为非终结符
 * （5）默认左边出现的第一个字符串为文法的开始符号
 * （6）当文法中含有符号 | 时，要分成两条进行输入
 */
void GrammarAnalyse::handleGrammar() {
    vector<string> lines;
    istringstream iss(grammarStr);
    string line;

    // 防止中间有换行符
    while(getline(iss, line))
        if (!line.empty())
            lines.push_back(line);

    // 非终结符
    QString line1 = QString::fromStdString(lines[0]);
    QStringList tokens1 = line1.split("|");

    for (const QString& token : tokens1) {
        // 去除空格
        QString trimmedToken = token.trimmed();
        if (!trimmedToken.isEmpty())
            bigAlpha.push_back(trimmedToken.toStdString());
    }

    // 非终结符
    QString line2 = QString::fromStdString(lines[1]);
    QStringList tokens2 = line2.split("|");

    for (const QString& token : tokens2) {
        // 去除空格
        QString trimmedToken = token.trimmed();
        if (!trimmedToken.isEmpty())
            smallAlpha.push_back(trimmedToken.toStdString());
    }

    for (size_t i = 2; i < lines.size(); i ++) {
        string rule = lines[i];
        istringstream ruleStream(rule);
        string nonTerminal;
        // 读取非终结符
        ruleStream >> nonTerminal;

        // 验证非终结符的格式
        if (!isBigAlpha(nonTerminal)) {
            QMessageBox::critical(nullptr, "Error", "文法开头必须是非终结符！");
            continue;
        }

        // 跳过箭头符号 “->”
        string arrow;
        ruleStream >> arrow;

        string rightHandSide;
        getline(ruleStream, rightHandSide);

        // 去除开头的空格
        rightHandSide = rightHandSide.substr(1);

        // 若为第一条规则，则认为是开始符号
        if (grammarMap.empty()) {
            startSymbol = nonTerminal;
            trueStartSymbol = startSymbol;
        }

        // 将文法结构化
        grammarMap[nonTerminal].insert(QString::fromStdString(rightHandSide).trimmed().toStdString());

        // 为 LR(0) 做准备
        grammarDeque.push_back(grammarUnit(nonTerminal, rightHandSide));
    }

    // 增广处理
    // 如果开始符号多于2个，说明需要增广，为了避免出现字母重复，采用 ^ 作为增广后的字母，后期输出特殊处理
    if (grammarMap[startSymbol].size() > 1) {
        grammarDeque.push_front(grammarUnit("zengguang", startSymbol));
        LR0Result += QString::fromStdString("进行了增广处理\n");

        trueStartSymbol = startSymbol;
    }

    // 开始编号
    int gid = 0;
    for (auto& g : grammarDeque) {
        g.gid = gid++;
        LR0Result += QString::number(g.gid) + QString::fromStdString(":") + QString::fromStdString(g.left == "zengguang" ? "E\'" : g.left) + QString::fromStdString("->") + QString::fromStdString(g.right) + "\n";
        // 存入 map 中
        grammarToInt[make_pair(g.left, g.right)] = g.gid;
    }
}
