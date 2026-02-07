/**************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: midcode.cpp
 * @Brief: 中间代码生成类的实现部分，用于对源程序进行语法分析并生成对应的中间代码
 * @Module: 中间代码生成模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 * Version Date       Author    Description
 * ------- ---------- --------- -------------------
 * 1.0.0   2025/10/5  袁知本     初始版本创建
 * 2.0.0   2026/1/23  袁知本     模块化，从原先的widget.cpp中分离
 *************************************************************************/

#include "midcode.h"
#include "syntaxtree.h"
#include <QDebug>
#include <fstream>
#include <sstream>

using namespace std;

// 全局变量定义
vector<IntermediateCode> intermediateCodes;
int tempVarCounter = 0;
int labelCounter = 0;

// IntermediateCode的toString方法实现
string IntermediateCode::toString() const {
    stringstream ss;
    switch (type) {
    case IC_ASSIGN:
        ss << result << " = " << op1;
        break;
    case IC_READ:
        ss << "read " << result;
        break;
    case IC_WRITE:
        ss << "write " << op1;
        break;
    case IC_ADD:
        ss << result << " = " << op1 << " + " << op2;
        break;
    case IC_SUB:
        ss << result << " = " << op1 << " - " << op2;
        break;
    case IC_MUL:
        ss << result << " = " << op1 << " * " << op2;
        break;
    case IC_DIV:
        ss << result << " = " << op1 << " / " << op2;
        break;
    case IC_MOD:
        ss << result << " = " << op1 << " % " << op2;
        break;
    case IC_LT:
        ss << "if " << op1 << " < " << op2 << " goto " << label;
        break;
    case IC_LE:
        ss << "if " << op1 << " <= " << op2 << " goto " << label;
        break;
    case IC_GT:
        ss << "if " << op1 << " > " << op2 << " goto " << label;
        break;
    case IC_GE:
        ss << "if " << op1 << " >= " << op2 << " goto " << label;
        break;
    case IC_EQ:
        ss << "if " << op1 << " == " << op2 << " goto " << label;
        break;
    case IC_NE:
        ss << "if " << op1 << " != " << op2 << " goto " << label;
        break;
    case IC_JUMP:
        ss << "goto " << label;
        break;
    case IC_JUMP_TRUE:
        ss << "if " << op1 << " goto " << label;
        break;
    case IC_JUMP_FALSE:
        ss << "ifFalse " << op1 << " goto " << label;
        break;
    case IC_LABEL:
        ss << label << ":";
        break;
    case IC_FUNC_DECL:
        ss << "function " << result;
        break;
    case IC_FUNC_END:
        ss << "end function";
        break;
    case IC_PARAM:
        ss << "param " << result;
        break;
    case IC_CALL:
        if (!result.empty())
            ss << result << " = ";
        ss << "call " << op1;
        break;
    case IC_RETURN:
        ss << "return " << op1;
        break;
    case IC_ARRAY_DECL:
        ss << "array " << result << "[" << op1 << "]";
        break;
    case IC_ARRAY_LOAD:
        ss << result << " = " << op1 << "[" << op2 << "]";
        break;
    case IC_ARRAY_STORE:
        ss << op1 << "[" << op2 << "] = " << result;
        break;
    default:
        ss << "unknown instruction";
    }
    return ss.str();
}

GenerateMidCode::GenerateMidCode() {}

// 清空中间代码和计数器
void GenerateMidCode::reset() {
    intermediateCodes.clear();
    tempVarCounter = 0;
    labelCounter = 0;
}

// 根据语法树节点生成中间代码
string GenerateMidCode::generateFromSyntaxTree(TreeNode* node) {
    if (!node) return "";

    const string& sym = node->symbol;

    /* ========= 值节点 ========= */
    if (sym == "ID" || sym == "NUMBER") {
        return node->value;
    }

    /* ========= 语句序列 ========= */
    if (sym == "stmt-sequence" || sym == "statement" || sym == "program") {
        for (TreeNode* ch : node->children)
            generateFromSyntaxTree(ch);
        return "";
    }

    /* ========= 赋值语句 ========= */
    if (sym == "assign-stmt") {
        string lhs = generateFromSyntaxTree(node->children[0]);
        string rhs = handleExpression(node->children[2]);
        addCode(IC_ASSIGN, lhs, rhs);
        return lhs;
    }

    /* ========= 读写 ========= */
    if (sym == "read-stmt") {
        string id = generateFromSyntaxTree(node->children[1]);
        addCode(IC_READ, id);
        return "";
    }

    if (sym == "write-stmt") {
        string v = handleExpression(node->children[1]);
        addCode(IC_WRITE, "", v);
        return "";
    }

    /* ========= 表达式 ========= */
    if (sym == "expression" || sym == "simple-exp" || sym == "term" || sym == "factor") {
        return handleExpression(node);
    }

    /* ========= 兜底 ========= */
    for (TreeNode* ch : node->children)
        generateFromSyntaxTree(ch);

    return "";
}

// 处理赋值语句节点
string GenerateMidCode::handleAssignment(TreeNode* node) {
    if (node->children.size() < 2) {
        return "";
    }

    // 查找ID节点和表达式节点
    TreeNode* idNode = nullptr;
    TreeNode* exprNode = nullptr;
    for (TreeNode* child : node->children) {
        if (child->symbol == "ID") {
            idNode = child;
        } else if (child->symbol == "expression") {
            exprNode = child;
        }
    }

    if (idNode && exprNode) {
        string idValue = idNode->value.empty() ? "var" : idNode->value;
        string exprValue = generateFromSyntaxTree(exprNode);
        if (!exprValue.empty()) {
            addCode(IC_ASSIGN, idValue, exprValue);
            return idValue;
        }
    }
    return "";
}

// 处理表达式节点
string GenerateMidCode::handleExpression(TreeNode* node) {
    // 值节点
    if (node->symbol == "ID" || node->symbol == "NUMBER") {
        return node->value;
    }

    // factor：下沉
    if (node->symbol == "factor") {
        return handleExpression(node->children[0]);
    }

    // expression / simple-exp / term
    if (node->symbol == "expression" || node->symbol == "simple-exp" || node->symbol == "term") {
        if (node->children.size() == 1) {
            return handleExpression(node->children[0]);
        }
        return handleArithmetic(node, "");
    }

    // 默认递归
    for (TreeNode* ch : node->children) {
        string v = handleExpression(ch);
        if (!v.empty())
            return v;
    }
    return "";
}

// 处理算术运算节点
string GenerateMidCode::handleArithmetic(TreeNode* node, const string&) {
    // node: operand (op operand)*
    // op: 当前运算符（来自 mulop / addop / orop 的 value）
    // 第一个操作数
    string left = handleExpression(node->children[0]);

    for (size_t i = 1; i + 1 < node->children.size(); i += 2) {
        TreeNode* opNode = node->children[i];
        TreeNode* rhsNode = node->children[i + 1];
        string right = handleExpression(rhsNode);
        string t = newTemp();
        string oper;

        // 情况 1：运算符直接挂在 value
        if (!opNode->value.empty()) {
            oper = opNode->value;
        }
        // 情况 2：子节点才是真正的符号
        else if (!opNode->children.empty()) {
            oper = opNode->children[0]->symbol;
        } else {
            throw runtime_error("运算符节点无有效内容");
        }

        /* 算术运算 */
        if (oper == "PLUS")
            addCode(IC_ADD, t, left, right);
        else if (oper == "MINUS")
            addCode(IC_SUB, t, left, right);
        else if (oper == "MULTI" || oper == "MULTIPLY" || oper == "TIMES")
            addCode(IC_MUL, t, left, right);
        else if (oper == "DIV")
            addCode(IC_DIV, t, left, right);
        else if (oper == "MOD")
            addCode(IC_MOD, t, left, right);
        /* 关系运算（产生 0/1） */
        else if (oper == "LT")
            addCode(IC_LT, t, left, right);
        else if (oper == "LE" || oper == "LTEQ")
            addCode(IC_LE, t, left, right);
        else if (oper == "GT")
            addCode(IC_GT, t, left, right);
        else if (oper == "GE" || oper == "GTEQ")
            addCode(IC_GE, t, left, right);
        else if (oper == "EQ")
            addCode(IC_EQ, t, left, right);
        else if (oper == "NE")
            addCode(IC_NE, t, left, right);
        /* Tiny / mini-C 的 |（统一视为非短路 OR） */
        else if (oper == "OR" || oper == "BOR" || oper == "|") {
            // 简化为算术 OR：非零即真
            addCode(IC_ADD, t, left, right);
        } else {
            throw runtime_error("不支持的算术/逻辑运算符: " + oper);
        }

        left = t;
    }
    return left;
}

// 处理比较运算节点
string GenerateMidCode::handleComparison(TreeNode* node, const string& op) {
    // 这里使用重载版本处理传入的节点
    if (node->children.size() < 2) {
        return "";
    }

    string leftValue = generateFromSyntaxTree(node->children[0]);
    string rightValue = generateFromSyntaxTree(node->children[1]);

    // 为比较结果生成临时变量
    string result = newTemp();
    string trueLabel = newLabel();
    string falseLabel = newLabel();
    string endLabel = newLabel();

    // 生成比较跳转指令
    if (op == "<") {
        addCode(IC_LT, "", leftValue, rightValue, trueLabel);
    } else if (op == "<=") {
        addCode(IC_LE, "", leftValue, rightValue, trueLabel);
    } else if (op == ">") {
        addCode(IC_GT, "", leftValue, rightValue, trueLabel);
    } else if (op == ">=") {
        addCode(IC_GE, "", leftValue, rightValue, trueLabel);
    } else if (op == "==") {
        addCode(IC_EQ, "", leftValue, rightValue, trueLabel);
    } else if (op == "!=") {
        addCode(IC_NE, "", leftValue, rightValue, trueLabel);
    }

    // false分支：结果为0
    addCode(IC_ASSIGN, result, "0");
    addCode(IC_JUMP, "", "", "", endLabel);

    // true分支：结果为1
    addCode(IC_LABEL, "", "", "", trueLabel);
    addCode(IC_ASSIGN, result, "1");

    addCode(IC_LABEL, "", "", "", endLabel);

    return result;
}

// 处理条件语句节点
string GenerateMidCode::handleConditional(TreeNode* node) {
    if (node->children.size() < 2) {
        return "";
    }

    // 查找条件表达式和 then 部分
    TreeNode* conditionNode = nullptr;
    TreeNode* thenNode = nullptr;
    TreeNode* elseNode = nullptr;

    for (TreeNode* child : node->children) {
        if (child->symbol == "expression") {
            conditionNode = child;
        } else if (child->symbol == "stmt-sequence" || child->symbol == "statement") {
            if (!thenNode) {
                thenNode = child;
            } else if (!elseNode) {
                elseNode = child;
            }
        }
    }

    if (!conditionNode || !thenNode) {
        return "";
    }

    // 生成条件表达式代码
    string conditionResult = generateFromSyntaxTree(conditionNode);
    string trueLabel = newLabel();
    string falseLabel = elseNode ? newLabel() : "";
    string endLabel = newLabel();

    // 条件跳转
    addCode(IC_JUMP_TRUE, "", conditionResult, "", trueLabel);

    if (elseNode) {
        // 有 else 分支
        addCode(IC_JUMP, "", "", "", falseLabel);

        // then 部分
        addCode(IC_LABEL, "", "", "", trueLabel);
        generateFromSyntaxTree(thenNode);
        addCode(IC_JUMP, "", "", "", endLabel);

        // else部分
        addCode(IC_LABEL, "", "", "", falseLabel);
        generateFromSyntaxTree(elseNode);
    } else {
        // 只有 then 部分
        addCode(IC_JUMP, "", "", "", endLabel);
        addCode(IC_LABEL, "", "", "", trueLabel);
        generateFromSyntaxTree(thenNode);
    }

    addCode(IC_LABEL, "", "", "", endLabel);

    return "";  // if语句无返回值
}

// 处理循环语句节点
string GenerateMidCode::handleLoop(TreeNode* node) {
    if (node->children.size() < 2) {
        return "";
    }

    string nodeSymbol = node->symbol;

    if (nodeSymbol == "while-stmt") {
        // while 循环
        TreeNode* conditionNode = nullptr;
        TreeNode* bodyNode = nullptr;

        for (TreeNode* child : node->children) {
            if (child->symbol == "expression") {
                conditionNode = child;
            } else if (child->symbol == "stmt-sequence") {
                bodyNode = child;
            }
        }

        if (!conditionNode || !bodyNode) {
            return "";
        }

        string startLabel = newLabel();
        string condLabel = newLabel();
        string endLabel = newLabel();

        // 跳转到条件判断
        addCode(IC_JUMP, "", "", "", condLabel);

        // 循环体开始
        addCode(IC_LABEL, "", "", "", startLabel);
        generateFromSyntaxTree(bodyNode);

        // 条件判断
        addCode(IC_LABEL, "", "", "", condLabel);
        string conditionResult = generateFromSyntaxTree(conditionNode);
        addCode(IC_JUMP_TRUE, "", conditionResult, "", startLabel);

        // 循环结束
        addCode(IC_LABEL, "", "", "", endLabel);

    } else if (nodeSymbol == "repeat-stmt") {
        // repeat 循环
        TreeNode* bodyNode = nullptr;
        TreeNode* conditionNode = nullptr;

        for (TreeNode* child : node->children) {
            if (child->symbol == "stmt-sequence") {
                bodyNode = child;
            } else if (child->symbol == "expression") {
                conditionNode = child;
            }
        }

        if (!bodyNode || !conditionNode) {
            return "";
        }

        string startLabel = newLabel();
        string endLabel = newLabel();

        // 循环体开始
        addCode(IC_LABEL, "", "", "", startLabel);
        generateFromSyntaxTree(bodyNode);

        // 条件判断：直到条件为真
        string conditionResult = generateFromSyntaxTree(conditionNode);
        addCode(IC_JUMP_FALSE, "", conditionResult, "", startLabel);

        // 循环结束
        addCode(IC_LABEL, "", "", "", endLabel);
    }

    return "";
}

// 处理函数声明节点
string GenerateMidCode::handleFunctionDecl(TreeNode* node) {
    if (node->children.empty()) {
        return "";
    }

    // 查找函数名
    string funcName;
    for (TreeNode* child : node->children) {
        if (child->symbol == "ID") {
            funcName = child->value.empty() ? "func" : child->value;
            break;
        }
    }

    if (!funcName.empty()) {
        addCode(IC_FUNC_DECL, funcName);
    }

    // 处理参数和函数体
    for (TreeNode* child : node->children) {
        generateFromSyntaxTree(child);
    }

    return funcName;
}

// 处理函数调用节点
string GenerateMidCode::handleFunctionCall(TreeNode* node) {
    if (node->children.empty()) {
        return "";
    }

    // 查找函数名
    string funcName;
    for (TreeNode* child : node->children) {
        if (child->symbol == "ID") {
            funcName = child->value.empty() ? "func" : child->value;
            break;
        }
    }

    if (!funcName.empty()) {
        // 假设是有返回值的函数调用
        string result = newTemp();
        addCode(IC_CALL, result, funcName);
        return result;
    }
    return "";
}

// 生成新的临时变量名
string GenerateMidCode::newTemp() {
    tempVarCounter++;
    return "t" + to_string(tempVarCounter);
}

// 生成新的标签名
string GenerateMidCode::newLabel() {
    labelCounter++;
    return "L" + to_string(labelCounter);
}

// 添加中间代码指令
void GenerateMidCode::addCode(IntermediateCodeType type, const string& result, const string& op1, const string& op2, const string& label) {
    intermediateCodes.push_back(IntermediateCode(type, result, op1, op2, label));
    qDebug() << "添加中间代码:" << QString::fromStdString(intermediateCodes.back().toString());
}

// 获取中间代码列表
const vector<IntermediateCode>& GenerateMidCode::getIntermediateCodes() {
    return intermediateCodes;
}

// 将中间代码输出到文件
bool GenerateMidCode::outputToFile(const string& filename) {
    ofstream outFile(filename);
    if (!outFile.is_open()) {
        qDebug() << "无法打开文件:" << QString::fromStdString(filename);
        return false;
    }

    outFile << "========== 中间代码 ==========" << endl;
    for (size_t i = 0; i < intermediateCodes.size(); i++) {
        outFile << i << ": " << intermediateCodes[i].toString() << endl;
    }
    outFile << "==============================" << endl;

    outFile.close();
    qDebug() << "中间代码已输出到文件:" << QString::fromStdString(filename);
    return true;
}

// 将中间代码输出为字符串
string GenerateMidCode::outputToString() {
    stringstream ss;
    ss << "========== 中间代码 ==========" << endl;
    for (size_t i = 0; i < intermediateCodes.size(); i++) {
        ss << i << ": " << intermediateCodes[i].toString() << endl;
    }
    ss << "==============================" << endl;
    return ss.str();
}
