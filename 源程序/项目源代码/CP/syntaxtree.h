/**************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: syntaxtree.h
 * @Brief: 语法树构建类，用于对源程序进行语法分析并构建语法树
 * @Module: 语法树构建模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/23   袁知本       模块化，从原先的widget.cpp中分离
 *************************************************************************/
#ifndef GENERATESYNTAXTREE_H
#define GENERATESYNTAXTREE_H

#include "root.h"
#include "grammar.h"

// 语法树节点结构
struct TreeNode {
    string symbol;                  // 符号名称（终结符或非终结符）
    string value;                   // 值（对于终结符，如ID的名称、NUMBER的值）
    vector<TreeNode*> children;     // 子节点
    int line;                       // 行号（可选）

    TreeNode(const string& sym, const string& val = "")
        : symbol(sym), value(val), line(0) {}

    ~TreeNode() {
        for (TreeNode* child : children) {
            delete child;
        }
    }

    // 添加子节点
    void addChild(TreeNode* child) {
        children.push_back(child);
    }
};

// 分析过程中的语法树节点栈（用于LR分析）
extern vector<TreeNode*> nodeStack;

// 语法树根节点（简化后的语法树）
extern TreeNode* syntaxTreeRoot;

// 分析树根节点（未简化的原始分析树）
extern TreeNode* parseTreeRoot;

class GenerateSyntaxTree
{
public:
    GenerateSyntaxTree();

    // 构建语法树（分析树 → 语法树）
    static TreeNode* buildSyntaxTree(TreeNode* parseTreeRoot);

    // 简化表达式树（移除冗余节点）
    static TreeNode* simplifyExpressionTree(TreeNode* node);

    // 简化语句序列（合并连续的语句节点）
    static TreeNode* simplifyStatementSequence(TreeNode* node);

    static TreeNode* simplifyNodeRecursive(TreeNode* node);

    // 检查节点是否为运算符
    static bool isOperator(const string& symbol);

    // 检查节点是否为标点符号
    static bool isPunctuation(const string& symbol);

    // 检查节点是否为类型声明相关
    static bool isTypeRelated(const string& symbol);

    // 格式化语法树显示
    static QString formatTreeForDisplay(TreeNode* root);

    // 创建语法树显示项（用于QTreeWidget）
    static QTreeWidgetItem* createTreeWidgetItem(TreeNode* node);

    // 递归简化节点
    static TreeNode* simplifyNode(TreeNode* node);

    // 处理表达式节点
    static TreeNode* handleExpression(TreeNode* node);

    // 处理语句节点
    static TreeNode* handleStatement(TreeNode* node);

    // 处理声明节点
    static TreeNode* handleDeclaration(TreeNode* node);

    // 合并二元表达式
    static TreeNode* mergeBinaryExpression(TreeNode* node);

    // 判断是否为冗余节点（如多余的括号、分隔符等）
    static bool isRedundantNode(TreeNode* node);

    // 获取运算符优先级
    static int getOperatorPrecedence(const string& op);

private:
    // 内部辅助：深拷贝并简化解析树节点（返回新分配的节点或 nullptr）
    static TreeNode* duplicateAndSimplify(TreeNode* node);
};

#endif // GENERATESYNTAXTREE_H
