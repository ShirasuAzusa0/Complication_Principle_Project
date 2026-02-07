/**************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: midcode.h
 * @Brief: 中间代码生成类的头文件
 * @Module: 中间代码生成模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 * Version Date       Author    Description
 * ------- ---------- --------- -------------------
 * 1.0.0   2025/10/5  袁知本   初始版本创建
 * 2.0.0   2026/1/23  袁知本   模块化，从原先的widget.cpp中分离
 *************************************************************************/

#ifndef MIDCODE_H
#define MIDCODE_H

#include "root.h"
#include "syntaxtree.h"

using namespace std;

// 中间代码类型枚举
enum IntermediateCodeType {
    IC_ASSIGN,      // 赋值
    IC_READ,        // 读
    IC_WRITE,       // 写
    IC_ADD,         // 加法
    IC_SUB,         // 减法
    IC_MUL,         // 乘法
    IC_DIV,         // 除法
    IC_MOD,         // 取模
    IC_LT,          // 小于
    IC_LE,          // 小于等于
    IC_GT,          // 大于
    IC_GE,          // 大于等于
    IC_EQ,          // 等于
    IC_NE,          // 不等于
    IC_JUMP,        // 无条件跳转
    IC_JUMP_TRUE,   // 为真跳转
    IC_JUMP_FALSE,  // 为假跳转
    IC_LABEL,       // 标签
    IC_FUNC_DECL,   // 函数声明
    IC_FUNC_END,    // 函数结束
    IC_PARAM,       // 参数
    IC_CALL,        // 函数调用
    IC_RETURN,      // 返回
    IC_ARRAY_DECL,  // 数组声明
    IC_ARRAY_LOAD,  // 数组加载
    IC_ARRAY_STORE  // 数组存储
};

// 中间代码结构体
struct IntermediateCode {
    IntermediateCodeType type;  // 指令类型
    string result;              // 结果
    string op1;                 // 操作数1
    string op2;                 // 操作数2
    string label;               // 标签

    IntermediateCode(IntermediateCodeType t = IC_ASSIGN,
                     const string& r = "",
                     const string& o1 = "",
                     const string& o2 = "",
                     const string& l = "")
        : type(t), result(r), op1(o1), op2(o2), label(l) {}

    // 转换为字符串表示
    string toString() const;
};

// 全局中间代码列表
extern vector<IntermediateCode> intermediateCodes;
extern int tempVarCounter;
extern int labelCounter;

// 中间代码生成类
class GenerateMidCode {
public:
    GenerateMidCode();

    // 从语法树生成中间代码
    static string generateFromSyntaxTree(TreeNode* node);

    // 清空中间代码和计数器
    static void reset();

    // 获取中间代码列表
    static const vector<IntermediateCode>& getIntermediateCodes();

    // 将中间代码输出到文件
    static bool outputToFile(const string& filename);

    // 将中间代码输出为字符串
    static string outputToString();

private:
    // 处理不同类型的节点
    static string handleAssignment(TreeNode* node);
    static string handleExpression(TreeNode* node);
    static string handleArithmetic(TreeNode* node, const string& op);
    static string handleComparison(TreeNode* node, const string& op);
    static string handleConditional(TreeNode* node);
    static string handleLoop(TreeNode* node);
    static string handleFunctionDecl(TreeNode* node);
    static string handleFunctionCall(TreeNode* node);

    // 生成新的临时变量名
    static string newTemp();

    // 生成新的标签名
    static string newLabel();

    // 添加中间代码指令
    static void addCode(IntermediateCodeType type,
                 const string& result = "",
                 const string& op1 = "",
                 const string& op2 = "",
                 const string& label = "");
};

#endif // MIDCODE_H
