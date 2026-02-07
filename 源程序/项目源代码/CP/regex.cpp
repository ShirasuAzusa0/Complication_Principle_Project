/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: regex.cpp
 * @Brief: 正则表达式处理类的实现部分
 * @Module: 正则表达式处理模块
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
 *   2.1.0    2026/1/18   袁知本       扩充m1和m2的映射内容
 *   2.2.0    2026/1/26   袁知本       优化编码映射结构与编码处理函数
 *   2.2.1    2026/1/27   袁知本       修复afterHandleRegex函数添加显式连接符
 ***********************************************************************/
#include "regex.h"

// 全局变量定义
// EPSILON定义（用"#"符号表示空串）
const char EPSILON = '#';

// m1和m2用于防止字符冲突，对需转义字符进行相应的替换操作
// 符号->字符串map
map<char, string> m1 = {
    {(char)1,"num"},
    {(char)2, "digit"},
    {(char)3, "float"},
    {(char)4, "letter"},
    {(char)11, "\\+\\"},
    {(char)12, "\\|\\"},
    {(char)13, "\\(\\"},
    {(char)14, "\\)\\"},
    {(char)15, "\\*\\"},
    {(char)16, "\\?\\"},
    {(char)17, "\\[\\"},
    {(char)18, "\\]\\"},
    {(char)19,"\\~\\"},
    {(char)20,"\\n"}
};
// 字符串->符号map
map<string, char> m2 = {
    {"num", (char)1},
    {"digit", (char)2},
    {"float", (char)3},
    {"letter", (char)4},
    {"\\+\\", (char)11},
    {"\\|\\", (char)12},
    { "\\(\\", (char)13},
    { "\\)\\", (char)14},
    {"\\*\\", (char)15},
    { "\\?\\", (char)16},
    {"\\[\\", (char)17},
    {"\\]\\", (char)18},
    {"\\~\\", (char)19},
    {"\\n", (char)20}
};

// 处理完成后的正则表达式，可用于后续的NFA、DFA构建生成
string finalRegex;

// 关键词集合
set<string> keyWords;

// 操作符映射
map<string,string> opMap;

// 注释符集合，0表示开始符，1表示结束符
string commentSymbol[2];

// 是否忽略大小写（默认不忽略，仅当checkbox被勾选才为true）
bool isLowerCase = false;

// 存储所有正则表达式的编码信息
vector<RegexEncoding> regexEncodings;

// 显示名称到编码的映射
map<string, int> encodingMap;

// 序列编码映射
map<string, vector<pair<string, int>>> sequenceEncodings;

AnalyseRegex::AnalyseRegex()
{

}

// 编码查询函数
int AnalyseRegex::findEncodingForString(const string& str) {
    auto it = encodingMap.find(str);
    if (it != encodingMap.end())
        return it->second;
    return -1;
}

// 获取操作符编码
int AnalyseRegex::findOperatorEncoding(const string& op) {
    // 先尝试直接查找
    auto it = encodingMap.find(op);
    if (it != encodingMap.end())
        return it->second;

    // 在序列编码中查找
    for (const auto& seqPair : sequenceEncodings)
        for (const auto& opPair : seqPair.second)
            if (opPair.first == op)
                return opPair.second;

    return -1;
}

// 获取关键词列表
void AnalyseRegex::getKeyWords(QString regex) {
    // 使用 split 按等号进行分割
    QStringList parts = regex.split('=');

    // 获取关键词的前提是该行是关键词行且符合输入格式要求
    if (parts.size() == 2 && parts[0].trimmed().contains("keys")) {
        // regex复用，获取等号右部并进行处理
        regex = parts[1].trimmed();
        // 按符号 “|” 划分出每一个关键词，并逐个插入到keyWords容器中
        QStringList keys = regex.split("|");
        for (QString key : keys)
            keyWords.insert(key.toStdString());
    }
}

// 获取操作符号的名称
string AnalyseRegex::getOpName(QString regex1, QString regex2) {
    QStringList parts1 = regex1.split('=');
    QStringList parts2 = regex2.split('=');
    if (parts1.size() == 2 && parts1[0].trimmed().contains("specials") && parts2.size() == 2 && parts2[0].trimmed().contains("specialNames")) {
        regex1 = parts1[1].trimmed();
        regex2 = parts2[1].trimmed();
        QStringList ops = regex1.split("|");
        QStringList opNames = regex2.split("|");
        // ops 和 opNames 需完全一一对应才能进行后续的操作符处理
        if (ops.size() != opNames.size())
            return "操作符和操作符名称个数不一致！";
        for (int i = 0; i < ops.size(); i ++)
            opMap[ops[i].toStdString()] = opNames[i].toStdString();
        return "";
    } else {
        return "操作符号输入格式不合规";
    }
}

// 获取注释符号（以{~}形式实现，其中“~”符号是代指注释内容）
string AnalyseRegex::getCommentSymbol(QString& regex) {
    QStringList parts = regex.split('=');
    if (parts.size() == 2 && parts[0].trimmed().contains("annotation")) {
        regex = parts[1].trimmed();
        QStringList a = regex.split("~");
        if (a.size() != 2)
            return "注释输入格式错误";
        commentSymbol[0] = a[0].toStdString();
        commentSymbol[1] = a[1].toStdString();
        regex = a[0] + "~*" + a[1];
        return "";
    } else {
        return "注释输入格式错误";
    }
}

// 判断是不是字符
// 由于前面已通过m1映射对下面的字符进行了转换，故其一定不会再次出现
bool AnalyseRegex::isChar(char c) {
    if (c == '+' || c == '[' || c == ']' || c == '|' || c == '*' || c == '(' || c == ')' || c == '?' || c == '@')
        return false;
    return true;
}

// 辅助函数：判断字符是否需要连接（考虑转义），处理示例效果如下：
// 原始: "ab"       → 结果: "a@b"
// 原始: "a(b|c)"   → 结果: "a@(b|c)"
// 原始: "(a)b"     → 结果: "(a)@b"
// 原始: "a*b"      → 结果: "a*@b"
// 原始: "a\\db"    → 结果: "a@\\d@b"
// 原始: "a.b"      → 结果: "a@.@b"
// 原始: "(a)(b)"   → 结果: "(a)@(b)"
// 原始: "a?b"      → 结果: "a?@b"
// 原始: "a+(b|c)"  → 结果: "a+@(b|c)"
bool AnalyseRegex::shouldConnect(char current, char next, bool isCurrentEscaped)
{
    // 如果当前字符是转义序列的一部分，视为普通字符
    if (isCurrentEscaped) {
        return isChar(next) || next == '(' || next == '\\';
    }

    // 后一个字符是不能作为连接起点的
    if (next == ')' || next == '|' || next == '*' ||
        next == '?' || next == '+') {
        return false;
    }

    // 当前字符不能作为连接终点
    if (current == '|' || current == '(') {
        return false;
    }

    // 空括号 ()
    if (current == '(' && next == ')') {
        return false;
    }

    // ===== 核心连接规则 =====

    // 1. 普通字符 / 右括号 / 量词 后接：
    //    普通字符 / 左括号 / 转义序列
    if (isChar(current) || current == ')' ||
        current == '*' || current == '?' || current == '+') {

        if (isChar(next) || next == '(' || next == '\\') {
            return true;
        }
    }

    return false;
}

// 进一步处理正则表达式，处理[]、+等符号，添加显式连接符（用@符号表示）
// [a-c] -> (a|b|c)
// a+ -> aa*
// (ab)+ -> (ab)(ab)*
// ab -> a@b
QString AnalyseRegex::afterHandleRegex(QString regex)
{
    string src = regex.toStdString();

    /************ 1. 处理 + ：X+ → XX* ************/
    for (size_t i = 0; i < src.size(); ++i) {
        if (src[i] != '+') continue;

        size_t end = i;
        size_t start = i - 1;

        // 情况 1：(...) +
        if (src[start] == ')') {
            int bal = 1;
            start--;
            while (start > 0 && bal) {
                if (src[start] == ')') bal++;
                else if (src[start] == '(') bal--;
                start--;
            }
            start++;
        }
        // 情况 2：\x+
        else if (src[start] == '\\') {
            start--;
        }

        string X = src.substr(start, end - start);
        src = src.substr(0, start) + X + X + "*" + src.substr(end + 1);
        i = start + X.size() * 2;
    }

    /************ 2. 逐字符插入显式连接符 @ ************/
    string out;
    bool escaped = false;

    for (size_t i = 0; i < src.size(); ++i) {
        char cur = src[i];
        out.push_back(cur);

        if (escaped) {
            escaped = false;
            continue;
        }

        if (cur == '\\') {
            escaped = true;
            continue;
        }

        if (i + 1 >= src.size()) continue;

        char next = src[i + 1];

        if (shouldConnect(cur, next, false)) {
            out.push_back('@');
        }
    }

    return QString::fromStdString(out);
}

// 正则表达式总体处理
string AnalyseRegex::handleAllRegex(QString allRegex, bool isLowerCase)
{
    regexEncodings.clear();
    encodingMap.clear();
    sequenceEncodings.clear();
    keyWords.clear();
    opMap.clear();

    commentSymbol[0] = "";
    commentSymbol[1] = "";

    if (isLowerCase)
        allRegex = allRegex.toLower();

    QStringList lines = allRegex.split("\n", Qt::SkipEmptyParts);

    struct Def {
        QString name;        // 等号左边
        QString rhsRaw;      // 未编码 RHS（用于判断引用）
        string  rhsEncoded;  // 编码后 RHS（用于构建 NFA）
    };
    vector<Def> defs;

    // m2：字符串 → 控制字符，按长度降序
    vector<pair<string, char>> m2list;
    for (auto& p : m2)
        m2list.emplace_back(p.first, p.second);

    sort(m2list.begin(), m2list.end(),
         [](const pair<string, char>& a,
            const pair<string, char>& b){
            return a.first.size() > b.first.size();
         });

    /************ Step 1：收集所有定义 ************/
    for (QString line : lines) {
        line = line.trimmed();
        if (line.isEmpty()) continue;

        int pos = line.indexOf('=');
        if (pos < 0) continue;

        QString left  = line.left(pos).trimmed();
        QString right = line.mid(pos + 1).trimmed();

        // 编码 RHS
        string rhs = right.toStdString();
        for (auto& p : m2list) {
            size_t k = 0;
            while ((k = rhs.find(p.first, k)) != string::npos) {
                if (k > 0 && rhs[k - 1] == '\\') {
                    k += p.first.size();
                    continue;
                }
                rhs.replace(k, p.first.size(), string(1, p.second));
                k++;
            }
        }

        defs.push_back({ left, right, rhs });

        // 处理编码逻辑：_xxxNNN 和 _xxxNNNS（序列编码）
        if (left.startsWith("_")) {
            string name = left.toStdString();

            // 检查是否是特殊符号名称（以_specialName开头）
            bool isSpecialName = (name.find("_specialName") == 0);

            // 提取数字部分
            string numStr;
            for (char c : name) {
                if (isdigit(c)) numStr.push_back(c);
                else if (!numStr.empty()) break;
            }

            if (!numStr.empty()) {
                int baseEncoding = stoi(numStr);

                // 检查是否是序列编码（以S结尾）
                bool isSequence = (name.back() == 'S');

                if (isSequence) {
                    // 处理序列编码
                    QStringList items = right.split("|", Qt::SkipEmptyParts);
                    vector<pair<string, int>> seq;
                    int cur = baseEncoding;

                    for (QString item : items) {
                        string token = item.trimmed().toStdString();
                        seq.emplace_back(token, cur);
                        encodingMap[token] = cur;

                        // 为序列中的每个项创建RegexEncoding记录
                        RegexEncoding enc;
                        enc.regexName = name + "_" + token;
                        enc.displayName = token;
                        enc.encoding = cur;
                        enc.isSequence = true;
                        regexEncodings.push_back(enc);

                        cur++;
                    }

                    sequenceEncodings[name] = seq;

                    // 同时为序列整体创建一个RegexEncoding记录
                    // 特殊符号名称不参与NFA构建，但需要记录编码信息
                    if (!isSpecialName) {
                        RegexEncoding seqEnc;
                        seqEnc.regexName = name;
                        seqEnc.displayName = right.toStdString();
                        seqEnc.encoding = baseEncoding;
                        seqEnc.isSequence = true;
                        regexEncodings.push_back(seqEnc);
                    }
                } else {
                    // 非序列编码
                    // 特殊符号名称不参与NFA构建
                    if (!isSpecialName) {
                        RegexEncoding enc;
                        enc.regexName = name;
                        enc.displayName = right.toStdString();
                        enc.encoding = baseEncoding;
                        enc.isSequence = false;
                        regexEncodings.push_back(enc);
                        encodingMap[enc.displayName] = enc.encoding;
                    }
                }
            }
        }
    }

    /************ Step 2：基于"原始 RHS"判断引用关系 ************/
    set<QString> referenced;

    for (size_t i = 0; i < defs.size(); ++i) {
        for (size_t j = 0; j < defs.size(); ++j) {
            if (i == j) continue;

            if (defs[i].rhsRaw.contains(defs[j].name)) {
                referenced.insert(defs[j].name);
            }
        }
    }

    /************ Step 3：只拼接"入口规则"（排除纯字符类和特殊符号名称） ************/
    auto isPureCharClass = [](const string& s) -> bool {
        // 编码后为单控制字符（如 digit / letter）
        if (s.size() == 1)
            return true;

        // 原始形式 [x-y]
        if (s.size() >= 3 && s.front() == '[' && s.back() == ']')
            return true;

        return false;
    };

    QStringList finalList;
    for (auto& d : defs) {
        // 被引用的规则不作为入口
        if (referenced.count(d.name))
            continue;

        // 纯字符类不作为 NFA 起点
        if (isPureCharClass(d.rhsEncoded))
            continue;

        // 特殊符号名称（以_specialName开头）不参与NFA构建
        if (d.name.startsWith("_specialName"))
            continue;

        finalList.push_back(QString::fromStdString(d.rhsEncoded));
    }

    if (finalList.isEmpty())
        return "未找到可用正则规则";

    /************ Step 4：拼接总正则 ************/
    QString merged;
    for (int i = 0; i < finalList.size(); ++i) {
        merged += "(" + finalList[i] + ")";
        if (i + 1 < finalList.size())
            merged += "|";
    }

    qDebug() << "拼接后的正则：" << merged;

    finalRegex = afterHandleRegex(merged).toStdString();

    qDebug() << "处理后的正则：" << QString::fromStdString(finalRegex);

    return "";
}
