/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: regex.h
 * @Brief: 正则表达式处理类，声明相关的全局变量以及定义对应的正则表达式处理类
 * @Module: 正则表达式处理模块
 *
 * @Current Version: 2.1.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/8    袁知本       模块化，从原先的widget.cpp中分离
 *   2.1.0    2026/1/26   袁知本       优化编码映射结构与编码处理函数
 ***********************************************************************/
#ifndef REGEX_H
#define REGEX_H

#include "root.h"

// 全局变量声明
// 编码映射结构
struct RegexEncoding {
    string regexName;      // 正则表达式名称（如_ID101）
    int encoding;          // 编码值
    bool isSequence;       // 是否为序列编码（带S后缀）
    string displayName;    // 正则表达式值（也即等号右部，如digit|letter）
};

// 存储所有正则表达式的编码信息
extern vector<RegexEncoding> regexEncodings;

// 显示名称到编码的映射
extern map<string, int> encodingMap;

// 序列编码映射
extern map<string, vector<pair<string, int>>> sequenceEncodings;

// EPSILON定义（用"#"符号表示空串）
extern const char EPSILON;

// m1和m2用于防止字符冲突，对需转义字符进行相应的替换操作
// 符号->字符串map
extern map<char, string> m1;

// 字符串->符号map
extern map<string, char> m2;

// 处理完成后的正则表达式，可用于后续的NFA、DFA构建生成
extern string finalRegex;

// 关键词集合
extern set<string> keyWords;

// 操作符映射
extern map<string,string> opMap;

// 注释符集合，0表示开始符，1表示结束符
extern string commentSymbol[2];

// 是否忽略大小写（默认不忽略，仅当checkbox被勾选才为true）
extern bool isLowerCase;

class AnalyseRegex
{
public:
    AnalyseRegex();

    // 辅助函数：获取编码信息
    const vector<RegexEncoding>& getRegexEncodings() const { return regexEncodings; }
    const map<string, int>& getEncodingMap() const { return encodingMap; }
    const map<string, vector<pair<string, int>>>& getSequenceEncodings() const { return sequenceEncodings; }

    // 编码查询函数
    static int findEncodingForString(const string& str);

    // 获取操作符编码
    static int findOperatorEncoding(const string& op);

    // 获取关键词列表
    static void getKeyWords(QString regex);

    // 获取操作符号的名称
    static string getOpName(QString regex1, QString regex2);

    // 获取注释符号
    static string getCommentSymbol(QString& regex);

    // 判断是不是字符
    // 由于前面已通过m1映射对下面的字符进行了转换，故其一定不会再次出现
    static bool isChar(char c);

    // 显示连接符添加辅助判断函数
    static bool shouldConnect(char current, char next, bool isCurrentEscaped);

    // 进一步处理正则表达式，处理[]、+等符号，添加显式连接符（用@符号表示）
    static QString afterHandleRegex(QString regex);

    // 正则表达式总体处理
    static string handleAllRegex(QString allRegex, bool isLowerCase);
};

#endif // REGEX_H
