/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: code.h
 * @Brief: 词法分析程序代码生成类，用于根据正则表达式生成相对应的词法分析程序
 * @Module: 词法分析程序代码生成模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/19   袁知本       模块化，从原先的widget.cpp中分离
 ***********************************************************************/
#ifndef GENERATECODE_H
#define GENERATECODE_H

#include "root.h"

class GenerateCode
{
public:
    GenerateCode();

    // 生成完整词法分析器的C代码
    static bool genLexCodeCase(QList<QString> tmpList, QString& codeStr, int idx, bool flag);

    // 生成状态转移的 case 代码片段
    static QString generateCode(QString filePath);
};

#endif // GENERATECODE_H
