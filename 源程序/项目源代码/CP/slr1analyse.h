/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: slr1analyse.h
 * @Brief: SLR(1)分析类，用于分析判断用户提供的文法是否符合SLR(1)文法
 * @Module: SLR(1)分析模块
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
#ifndef SLR1ANALYSE_H
#define SLR1ANALYSE_H


class SLR1Analyse
{
public:
    SLR1Analyse();

    // 检查“移进-规约”冲突
    static bool SLR1Fun1();

    // 检查“规约-规约”冲突
    static bool SLR1Fun2();

    // 检查 SLR(1) 分析入口
    static int SLR1_Analyse();
};

#endif // SLR1ANALYSE_H
