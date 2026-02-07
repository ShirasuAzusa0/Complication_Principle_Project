/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: code.cpp
 * @Brief: 词法分析程序代码生成类的实现部分，用于根据正则表达式生成相对应的词法分析程序
 * @Module: 词法分析程序代码生成模块
 *
 * @Current Version: 2.1.3
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------------------
 *   1.0.0    2025/10/5   袁知本      初始版本创建
 *   2.0.0    2026/1/19   袁知本      模块化，从原先的widget.cpp中分离
 *   2.1.0    2026/1/28   袁知本      优化字母数字匹配，直接生成可用lex文件
 *   2.1.1    2026/1/29   袁知本      修复宏定义和switch语句的错误
 *   2.1.2    2026/1/29   袁知本      简化宏定义
 *   2.1.3    2026/1/29   袁知本      修复转义操作符识别问题
 ***********************************************************************/
#include "code.h"
#include "regex.h"
#include "nfa.h"
#include "dfa.h"

// 声明外部变量
extern map<string, int> encodingMap;
extern map<string, vector<pair<string, int>>> sequenceEncodings;

GenerateCode::GenerateCode() {}

// 工具函数
bool IsDigit(char c) { return c >= '0' && c <= '9'; }
bool IsAlpha(char c) { return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'; }

// 生成 DFA case 代码
bool GenerateCode::genLexCodeCase(QList<QString> tmpList,
                                 QString& codeStr,
                                 int idx,
                                 bool flag)
{
    for (auto& chStr : tmpList) {
        char ch = chStr.toUtf8().constData()[0];

        if (dfaMinTable[idx].transitions.count(ch) == 0)
            continue;

        int nextState = dfaMinTable[idx].transitions[ch];
        QString caseStr;

        char realChar = 0;

        auto it = m1.find(ch);
        if (it != m1.end()) {
            const string& cls = it->second;

            // 跳过字符类
            if (cls == "digit" || cls == "letter" ||
                cls == "num"   || cls == "float") {
                continue;
            }

            // 只要是转义形式：\+
            if (!cls.empty() && cls[0] == '\\' && cls.size() >= 2) {
                realChar = cls[1];   // '+' '*' '(' ')'
            }
            // 普通单字符
            else if (cls.size() == 1) {
                realChar = cls[0];
            }
            else {
                continue;
            }
        }
        else {
            realChar = ch;
        }

        // 防御：绝不生成 '\0'
        if (realChar == 0)
            continue;

        // 生成合法的 C 字符常量
        switch (realChar) {
            case '\'': caseStr = "'\\''"; break;
            case '\\': caseStr = "'\\\\'"; break;
            case '\n': caseStr = "'\\n'"; break;
            case '\t': caseStr = "'\\t'"; break;
            case '\r': caseStr = "'\\r'"; break;
            default:
                caseStr = "'" + QString(QChar(realChar)) + "'";
                break;
        }

        codeStr += "\t\t\tcase " + caseStr + ": ";
        if (flag)
            codeStr += "state=" + QString::number(nextState) + "; ";
        codeStr += "break;\n";
    }
    return false;
}


// 生成完整词法分析程序
// 简化只生成必要的宏定义
QString GenerateCode::generateCode(QString filePath) {
    QString lexCode;
    lexCode += "#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n#include <stdbool.h>\n#include <ctype.h>\n\n";
    lexCode += "bool IsDigit(char c){ return c>='0'&&c<='9'; }\n";
    lexCode += "bool IsAlpha(char c){ return (c>='a'&&c<='z')||(c>='A'&&c<='Z')||c=='_'; }\n\n";

    // 输出正则编码宏 - 只输出非序列编码的宏定义
    for (const auto& reg : regexEncodings) {
        // 只输出非序列编码的宏定义（如_ID400, _num500等）
        if (!reg.isSequence) {
            string cleanName = reg.regexName;
            // 确保宏名是有效的C标识符
            if (!cleanName.empty() && cleanName[0] == '_') {
                // 已经是有效的宏名
                lexCode += "#define " + QString::fromStdString(cleanName) + " " + QString::number(reg.encoding) + "\n";
            }
        }
    }

    // 为关键字和操作符生成简洁的宏定义

    // 1. 关键字宏定义 - 从关键词key序列获取
    auto key_it = sequenceEncodings.lower_bound("_key");
    if (key_it != sequenceEncodings.end()) {
        for (const auto& item : key_it->second) {
            string keyword = item.first;
            int encoding = item.second;
            string macroName = "KEYWORD_" + keyword;
            lexCode += "#define " + QString::fromStdString(macroName) + " " + QString::number(encoding) + "\n";
        }
    }

    // 2. 操作符宏定义 - 使用 specialName 中的名称和 special 中的编码
    auto special_it = sequenceEncodings.lower_bound("_special");
    auto specialName_it = sequenceEncodings.lower_bound("_specialName");

    if (special_it != sequenceEncodings.end() && specialName_it != sequenceEncodings.end()) {
        const auto& special_items = special_it->second;
        const auto& specialName_items = specialName_it->second;

        // 假设两个序列的顺序是一致的
        for (size_t i = 0; i < special_items.size() && i < specialName_items.size(); i++) {
            string operatorName = specialName_items[i].first;
            int encoding = special_items[i].second;
            string macroName = "OPERATOR_" + operatorName;
            lexCode += "#define " + QString::fromStdString(macroName) + " " + QString::number(encoding) + "\n";
        }
    }

    lexCode += "\n";

    // 输出关键字表 - 直接从关键词序列获取，确保编码正确
    lexCode += "static struct { const char* keyword; int encoding; } keywordTable[] = {\n";
    if (key_it != sequenceEncodings.end()) {
        for (const auto& item : key_it->second) {
            string keyword = item.first;
            int encoding = item.second;
            lexCode += "\t{\"" + QString::fromStdString(keyword) + "\", " + QString::number(encoding) + "},\n";
        }
    }
    lexCode += "};\n\n";

    // 输出操作符表 - 直接从操作符 special 序列获取并清理转义字符
    lexCode += "static struct { const char* op; int encoding; } operatorTable[] = {\n";
    if (special_it != sequenceEncodings.end()) {
        for (const auto& item : special_it->second) {
            string op = item.first;
            int encoding = item.second;

            // 彻底清理操作符字符串，移除所有转义反斜杠
            string cleanOp;
            for (size_t i = 0; i < op.length(); i++) {
                if (op[i] == '\\') {
                    // 跳过转义反斜杠，取下一个字符
                    if (i + 1 < op.length()) {
                        i++; // 跳过反斜杠
                        cleanOp += op[i]; // 取转义后的字符
                    }
                } else {
                    cleanOp += op[i];
                }
            }

            // 如果清理后的操作符为空，跳过
            if (cleanOp.empty()) {
                continue;
            }

            // 在C字符串中转义特殊字符
            QString escapedOp;
            for (char c : cleanOp) {
                // 对C字符串中的特殊字符进行转义
                switch(c) {
                    case '\\': escapedOp += "\\\\"; break; // 反斜杠
                    case '\"': escapedOp += "\\\""; break; // 双引号
                    case '\'': escapedOp += "\\\'"; break; // 单引号
                    case '\n': escapedOp += "\\n"; break;  // 换行
                    case '\t': escapedOp += "\\t"; break;  // 制表
                    case '\r': escapedOp += "\\r"; break;  // 回车
                    default: escapedOp += QChar(c); break;
                }
            }

            lexCode += "\t{\"" + escapedOp + "\", " + QString::number(encoding) + "},\n";
        }
    }
    lexCode += "};\n\n";

    // 辅助函数 - 添加调试信息
    lexCode +=
        "void concat(char str[], char tmp){ size_t len=strlen(str); str[len]=tmp; str[len+1]='\\0'; }\n"
        "int findKeywordEncoding(const char* str){ \n"
        "    printf(\"查找关键字: %s\\n\", str);\n"
        "    for(int i=0;i<sizeof(keywordTable)/sizeof(keywordTable[0]);i++) { \n"
        "        printf(\"  比较: %s vs %s\\n\", str, keywordTable[i].keyword);\n"
        "        if(strcmp(str,keywordTable[i].keyword)==0) { \n"
        "            printf(\"  找到关键字: %s, 编码: %d\\n\", str, keywordTable[i].encoding);\n"
        "            return keywordTable[i].encoding; \n"
        "        }\n"
        "    } \n"
        "    printf(\"  未找到关键字: %s\\n\", str);\n"
        "    return -1; \n"
        "}\n"
        "int findOperatorEncoding(const char* str){ \n"
        "    printf(\"查找操作符: %s\\n\", str);\n"
        "    for(int i=0;i<sizeof(operatorTable)/sizeof(operatorTable[0]);i++) { \n"
        "        printf(\"  比较: %s vs %s\\n\", str, operatorTable[i].op);\n"
        "        if(strcmp(str,operatorTable[i].op)==0) { \n"
        "            printf(\"  找到操作符: %s, 编码: %d\\n\", str, operatorTable[i].encoding);\n"
        "            return operatorTable[i].encoding; \n"
        "        }\n"
        "    } \n"
        "    printf(\"  未找到操作符: %s\\n\", str);\n"
        "    return -1; \n"
        "}\n"
        "void outputToken(FILE* fp,int encoding,const char* val){ \n"
        "    if(val&&strlen(val)>0){ \n"
        "        fprintf(fp,\"%d \\\"%s\\\" \",encoding,val); \n"
        "        printf(\"%d \\\"%s\\\" \",encoding,val); \n"
        "    } else { \n"
        "        fprintf(fp,\"%d \",encoding); \n"
        "        printf(\"%d \",encoding); \n"
        "    } \n"
        "}\n"
        "int isFloatNumber(const char* str){ for(int i=0;str[i];i++) if(str[i]=='.') return 1; return 0; }\n\n";

    // coding函数 - 添加调试信息
    lexCode += "void coding(FILE* input_fp, FILE* output_fp){\n"
               "    char tmp=fgetc(input_fp);\n"
               "if(tmp == EOF) return;\n"
               "\n"
               "/* -------- 注释处理 -------- */\n"
               "    if(tmp == '/'){ \n"
               "        char tmp2 = fgetc(input_fp);\n"
               "        if(tmp2 == '/'){  // 行注释\n"
               "            char c;\n"
               "            while((c = fgetc(input_fp)) != EOF && c != '\\n');\n"
               "            return;\n"
               "        }else{\n"
               "            ungetc(tmp2, input_fp);\n"
               "        }\n"
               "    }\n"
               "    else if(tmp == '{'){  // Tiny 风格注释\n"
               "        char c;\n"
               "        while((c = fgetc(input_fp)) != EOF){\n"
               "            if(c == '}') break;  // 注释结束\n"
               "        }\n"
               "        return;  // 跳过整个注释\n"
               "    }\n"
               "\n"
               "    // 跳过空白字符\n"
               "    if(tmp==' '||tmp=='\\n'||tmp=='\\t'||tmp=='\\r') { return; }\n"
               "    ungetc(tmp,input_fp);\n"
               "    \n"
               "    int state=0;\n"
               "    bool flag=false;\n"
               "    bool isIdentifier=false;\n"
               "    bool isDigit=false;\n"
               "    bool isFloat=false;\n"
               "    char value[1024]; value[0]='\\0';\n"
               "    \n"
               "    while(!flag){\n"
               "        tmp=fgetc(input_fp);\n"
               "        if(tmp == EOF) { flag=true; break; }\n"
               "        \n"
               "        // 检查字符类型\n"
               "        if(tmp >= '0' && tmp <= '9') isDigit = true;\n"
               "        else if((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') isIdentifier = true;\n"
               "        else if(tmp == '.') isFloat = true;\n"
               "        \n"
               "        printf(\"状态 %d, 读取字符 '%c' (ASCII %d)\\n\", state, tmp, tmp);\n"
               "        \n"
               "        switch(state){\n";

    // 为每个状态生成转移代码
    for (size_t i = 0; i < dfaMinTable.size(); i++) {
        if (!dfaMinTable[i].transitions.empty()) {
            lexCode += "\t\tcase " + QString::number(dfaMinTable[i].id) + ":{\n";

            // 首先处理特殊字符类（数字和字母）
            bool hasDigitTransition = false;
            bool hasLetterTransition = false;

            for (auto& pair : dfaMinTable[i].transitions) {
                char ch = pair.first;
                auto it = m1.find(ch);
                if (it != m1.end()) {
                    if (it->second == "digit" || it->second == "num") {
                        hasDigitTransition = true;
                    }
                    else if (it->second == "letter") {
                        hasLetterTransition = true;
                    }
                }
            }

            if (hasDigitTransition) {
                lexCode += "\t\t\tif(tmp >= '0' && tmp <= '9') {\n";
                // 找到对应的数字转移
                for (auto& pair : dfaMinTable[i].transitions) {
                    char ch = pair.first;
                    auto it = m1.find(ch);
                    if (it != m1.end() && (it->second == "digit" || it->second == "num")) {
                        lexCode += "\t\t\t\tstate=" + QString::number(pair.second) + ";\n";
                        break;
                    }
                }
                lexCode += "\t\t\t\tprintf(\"数字转移: %d -> %d\\n\", " + QString::number(dfaMinTable[i].id) + ", state);\n";
                lexCode += "\t\t\t\tbreak;\n";
                lexCode += "\t\t\t}\n";
            }

            if (hasLetterTransition) {
                lexCode += "\t\t\tif((tmp >= 'a' && tmp <= 'z') || (tmp >= 'A' && tmp <= 'Z') || tmp == '_') {\n";
                // 找到对应的字母转移
                for (auto& pair : dfaMinTable[i].transitions) {
                    char ch = pair.first;
                    auto it = m1.find(ch);
                    if (it != m1.end() && it->second == "letter") {
                        lexCode += "\t\t\t\tstate=" + QString::number(pair.second) + ";\n";
                        break;
                    }
                }
                lexCode += "\t\t\t\tprintf(\"字母转移: %d -> %d\\n\", " + QString::number(dfaMinTable[i].id) + ", state);\n";
                lexCode += "\t\t\t\tbreak;\n";
                lexCode += "\t\t\t}\n";
            }

            // 处理普通字符
            lexCode += "\t\t\tswitch(tmp){\n";
            QList<QString> tmpList;
            for (auto& pair : dfaMinTable[i].transitions) {
                char ch = pair.first;
                auto it = m1.find(ch);
                if (it != m1.end()) {
                    const string& cls = it->second;

                    if (cls == "\\n") {
                        continue;
                    }

                    // 真正的字符类，交给 if(tmp>=...) 处理
                    if (cls == "digit" || cls == "letter" ||
                        cls == "num"   || cls == "float") {
                        continue;
                    }

                    // 转义的单字符操作符：\+ \* \( \)
                    if (cls.size() >= 2 && cls[0] == '\\') {
                        tmpList.append(QString(ch));
                        continue;
                    }

                    // 其他情况忽略
                    continue;
                }
                tmpList.append(QString(ch));
            }
            genLexCodeCase(tmpList, lexCode, i, true);
            lexCode += "\t\t\tdefault: \n";
            lexCode += "\t\t\t\tprintf(\"默认转移: 未匹配字符 '%c'\\n\", tmp);\n";
            lexCode += "\t\t\t\tflag=true; ungetc(tmp,input_fp); break;\n";
            lexCode += "\t\t\t}\n";
            lexCode += "\t\t\tbreak;\n\t\t}\n";
        } else {
            // 没有转移的状态（通常是终态）
            lexCode += "\t\tcase " + QString::number(dfaMinTable[i].id) + ":\n";
            lexCode += "\t\t\tprintf(\"到达终态 %d, 回退字符 '%c'\\n\", state, tmp);\n";
            lexCode += "\t\t\tungetc(tmp, input_fp);\n";
            lexCode += "\t\t\tflag = true;\n";
            lexCode += "\t\t\tbreak;\n";
        }
    }

    lexCode += "\t\tdefault:\n";
    lexCode += "\t\t\tprintf(\"默认状态: 回退字符 '%c'\\n\", tmp);\n";
    lexCode += "\t\t\tungetc(tmp, input_fp);\n";
    lexCode += "\t\t\tflag = true;\n";
    lexCode += "\t\t\tbreak;\n";
    lexCode += "\t\t}\n";  // 结束switch(state)

    lexCode += "\t\tif(!flag) { \n";
    lexCode += "\t\t\tconcat(value,tmp); \n";
    lexCode += "\t\t\tprintf(\"当前value: %s\\n\", value);\n";
    lexCode += "\t\t}\n";
    lexCode += "\t}\n";  // 结束while循环

    // 终态处理逻辑
    lexCode += "\tprintf(\"词法单元识别完成: value=%s, state=%d\\n\", value, state);\n";

    lexCode += "\tif(strlen(value) > 0){\n";
    lexCode += "\t\t// 首先检查是否是关键字\n";
    lexCode += "\t\tint kEnc = findKeywordEncoding(value);\n";
    lexCode += "\t\tif(kEnc != -1) {\n";
    lexCode += "\t\t\t// 关键字只输出编码\n";
    lexCode += "\t\t\toutputToken(output_fp, kEnc, \"\");\n";
    lexCode += "\t\t}\n";
    lexCode += "\t\telse {\n";
    lexCode += "\t\t\t// 其次检查是否是操作符\n";
    lexCode += "\t\t\tint opEnc = findOperatorEncoding(value);\n";
    lexCode += "\t\t\tif(opEnc != -1) {\n";
    lexCode += "\t\t\t\t// 操作符只输出编码\n";
    lexCode += "\t\t\t\toutputToken(output_fp, opEnc, \"\");\n";
    lexCode += "\t\t\t}\n";
    lexCode += "\t\t\telse {\n";
    lexCode += "\t\t\t\t// 标识符、数字等输出编码和值\n";
    lexCode += "\t\t\t\tint enc = -1;\n";

    lexCode += "\t\t\t\t// 标识符处理\n";
    lexCode += "\t\t\t\tif(isIdentifier) {\n";
    for (const auto& reg : regexEncodings) {
        if (!reg.isSequence &&
            (reg.regexName.find("ID") != string::npos ||
             reg.regexName.find("id") != string::npos)) {
            lexCode += "\t\t\t\t\tenc = " + QString::fromStdString(reg.regexName) + ";\n";
            break;
        }
    }
    lexCode += "\t\t\t\t\tif(enc == -1) enc = 400; // 默认标识符编码\n";
    lexCode += "\t\t\t\t}\n";

    lexCode += "\t\t\t\t// 数字处理\n";
    lexCode += "\t\t\t\telse if(isDigit) {\n";
    lexCode += "\t\t\t\t\tif(isFloat || isFloatNumber(value)) {\n";
    for (const auto& reg : regexEncodings) {
        if (!reg.isSequence &&
            (reg.regexName.find("FLOAT") != string::npos ||
             reg.regexName.find("Float") != string::npos ||
             reg.regexName.find("float") != string::npos)) {
            lexCode += "\t\t\t\t\t\tenc = " + QString::fromStdString(reg.regexName) + ";\n";
            break;
        }
    }
    lexCode += "\t\t\t\t\t\tif(enc == -1) enc = 500; // 默认浮点数编码\n";
    lexCode += "\t\t\t\t\t}\n";
    lexCode += "\t\t\t\t\telse {\n";
    lexCode += "\t\t\t\t\t\tenc = 500; // 默认整数编码\n";
    lexCode += "\t\t\t\t\t}\n";
    lexCode += "\t\t\t\t}\n";

    lexCode += "\t\t\t\toutputToken(output_fp, enc, value);\n";
    lexCode += "\t\t\t}\n";  // 结束else（既不是关键字也不是操作符）
    lexCode += "\t\t}\n";    // 结束else（不是关键字）
    lexCode += "\t}\n";      // 结束if(strlen(value) > 0)
    lexCode += "}\n\n";


    // main函数
    lexCode += "int main(int argc,char* argv[]){\n"
               "    FILE* input_fp=fopen(\"" + filePath + "/_sample.tny\",\"r\");\n"
               "    if(input_fp==NULL){printf(\"Failed to open input file\\n\");return 1;}\n"
               "    FILE* output_fp=fopen(\"" + filePath + "/output.lex\",\"w\");\n"
               "    if(output_fp==NULL){printf(\"Failed to open output file\\n\");fclose(input_fp);return 1;}\n"
               "    \n"
               "    printf(\"开始词法分析...\\n\");\n"
               "    \n"
               "    char c;\n"
               "    while((c=fgetc(input_fp))!=EOF){ \n"
               "        ungetc(c,input_fp); \n"
               "        printf(\"--- 开始处理下一个词法单元 ---\\n\");\n"
               "        coding(input_fp,output_fp); \n"
               "    }\n"
               "    fprintf(output_fp,\"-1\"); printf(\"\\n词法分析完成！\\n\"); fclose(input_fp); fclose(output_fp);\n"
               "    return 0;\n"
               "}\n";

    return lexCode;
}
