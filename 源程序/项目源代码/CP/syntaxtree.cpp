/**************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: syntaxtree.cpp
 * @Brief: 语法树构建类的实现部分，用于对源程序进行语法分析并构建语法树
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
#include "syntaxtree.h"

// 分析过程中的语法树节点栈（用于LR分析）
vector<TreeNode*> nodeStack;

// 语法树根节点（简化后的语法树）
TreeNode* syntaxTreeRoot = nullptr;

// 分析树根节点（未简化的原始分析树）
TreeNode* parseTreeRoot = nullptr;

// 用于存储中间代码的列表（如果语法树生成过程中需要）
vector<string> intermediateCodes;

GenerateSyntaxTree::GenerateSyntaxTree()
{

}

// ========================= 控制台打印语法树 =========================

void printSyntaxTree(TreeNode* node, int depth) {
    if (!node) return;

    for (int i = 0; i < depth; ++i)
        std::cout << "  ";

    std::cout << node->symbol;
    if (!node->value.empty() && node->value != node->symbol)
        std::cout << " : " << node->value;
    std::cout << std::endl;

    for (TreeNode* ch : node->children)
        printSyntaxTree(ch, depth + 1);
}

// ========================= 辅助判定函数 =========================

// 判断是否为运算符（接受显示符号和文法名两种形式）
bool GenerateSyntaxTree::isOperator(const string& symbol) {
    static const set<string> operators = {
        // 基础算术运算符
        "+", "-", "*", "/", "%",
        // 增强算术运算符
        "++", "--",
        // 关系运算符
        "<", ">", "<=", ">=", "==", "!=",
        // 逻辑运算符
        "&&", "||", "!",
        // 位运算符
        "&", "|", "^", "~", "<<", ">>",
        // 赋值运算符
        "=", ":=", "+=", "-=", "*=", "/=", "%=", "&=", "|=", "^=", "<<=", ">>=",
        // 条件运算符
        "?", ":",
        // 逗号运算符
        ",",
        // 成员访问运算符
        ".", "->",
        // 指针运算符
        "*", "&",

        // 文法中的运算符名称（对应终结符）
        // 算术
        "PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MOD",
        "INCREMENT", "DECREMENT",
        // 关系
        "LT", "GT", "LTEQ", "GTEQ", "EQ", "NE",
        // 逻辑
        "AND", "OR", "NOT",
        // 位运算
        "BITAND", "BITOR", "BITXOR", "BITNOT", "LSHIFT", "RSHIFT",
        // 赋值
        "ASSIGN", "PLUS_ASSIGN", "MINUS_ASSIGN", "MULTIPLY_ASSIGN",
        "DIVIDE_ASSIGN", "MOD_ASSIGN", "BITAND_ASSIGN", "BITOR_ASSIGN",
        "BITXOR_ASSIGN", "LSHIFT_ASSIGN", "RSHIFT_ASSIGN",
        // 其他
        "QUESTION", "COLON",
        "DOT", "ARROW",
        "POINTER", "ADDRESS"
    };
    return operators.find(symbol) != operators.end();
}

// 判断是否为标点符号（包括符号显示和文法名）
bool GenerateSyntaxTree::isPunctuation(const string& symbol) {
    static const set<string> punctuations = {
        ";", ",",
        "(", ")", "{", "}", "[", "]",
        "SEMI", "DOU", "LLM", "RLM", "LBM", "RBM", "LMM", "RMM"
    };
    return punctuations.find(symbol) != punctuations.end();
}

// 判断是否与类型相关
bool GenerateSyntaxTree::isTypeRelated(const string& symbol) {
    static const set<string> typeSymbols = {
        "type", "int", "double", "void", "float", "char", "bool"
    };
    return typeSymbols.find(symbol) != typeSymbols.end();
}

// 判断是否为"结构性列表节点"（AST中应该保留为层次结构，而不是扁平化）
static bool isListLikeNode(const string& symbol) {
    static const set<string> listNodes = {
        "definition-list",
        "statement-list",
        "local-definition-list",
        "parameter-list",
        "argument-list",
        "arguments"
    };
    return listNodes.find(symbol) != listNodes.end();
}

// 判断节点是否冗余（保守策略）
// 更新冗余节点判断，确保运算符不被视为冗余
bool GenerateSyntaxTree::isRedundantNode(TreeNode* node) {
    if (!node) return true;

    // 标点直接冗余
    if (isPunctuation(node->symbol)) return true;

    // 运算符不是冗余节点
    if (isOperator(node->symbol) || node->symbol == "addop" ||
        node->symbol == "mulop" || node->symbol == "relop") {
        return false;
    }

    // 表达式相关节点不是冗余节点
    if (node->symbol == "exp" || node->symbol == "simple-exp" ||
        node->symbol == "term" || node->symbol == "factor" ||
        node->symbol == "assign-stmt") {
        return false;
    }

    static const set<string> potentiallyRedundant = {
        "stmt", "stmt-sequence",
        "term-tail", "factor-tail",
        "expression-stmt",
        "matched-stmt",
        "unmatched-stmt"
    };

    return potentiallyRedundant.find(node->symbol) != potentiallyRedundant.end();
}

// ========================= AST 核心构建逻辑 =========================

// 核心：深拷贝并简化解析树
TreeNode* GenerateSyntaxTree::duplicateAndSimplify(TreeNode* node) {
    if (!node) return nullptr;

    // ---------- 第一步：先过滤标点符号节点 ----------
    if (isPunctuation(node->symbol)) {
        if (node->children.empty()) {
            return nullptr;
        }

        if (node->children.size() == 1) {
            return duplicateAndSimplify(node->children[0]);
        }

        TreeNode* seqNode = new TreeNode("seq");
        for (TreeNode* ch : node->children) {
            TreeNode* c = duplicateAndSimplify(ch);
            if (c) seqNode->addChild(c);
        }

        if (seqNode->children.empty()) {
            delete seqNode;
            return nullptr;
        }

        if (seqNode->children.size() == 1) {
            TreeNode* only = seqNode->children[0];
            seqNode->children.clear();
            delete seqNode;
            return only;
        }

        return seqNode;
    }

    if (node->children.empty()) {
        TreeNode* leaf = new TreeNode(node->symbol, node->value);
        leaf->line = node->line;
        return leaf;
    }

    // ---------- 特殊处理：表达式相关节点 ----------
    // 表达式节点需要特别处理，保留运算符节点
    if (node->symbol == "simple-exp" || node->symbol == "term" ||
        node->symbol == "factor" || node->symbol == "exp") {

        // 对于表达式节点，需要检查是否有运算符子节点
        // 如果有运算符，则需要保留完整的结构

        // 先处理所有子节点
        vector<TreeNode*> simplifiedChildren;
        for (TreeNode* ch : node->children) {
            TreeNode* c = duplicateAndSimplify(ch);
            if (c) simplifiedChildren.push_back(c);
        }

        if (simplifiedChildren.empty()) {
            return nullptr;
        }

        // 如果只有一个子节点，直接返回
        if (simplifiedChildren.size() == 1) {
            // 特殊处理：如果子节点是运算符，需要包装
            if (isOperator(simplifiedChildren[0]->symbol) ||
                simplifiedChildren[0]->symbol == "addop" ||
                simplifiedChildren[0]->symbol == "mulop") {
                TreeNode* exprNode = new TreeNode(node->symbol);
                exprNode->addChild(simplifiedChildren[0]);
                return exprNode;
            }
            return simplifiedChildren[0];
        }

        // 如果有多个子节点，检查是否包含运算符
        // 如果包含运算符，保留完整的表达式结构
        bool hasOperator = false;
        for (TreeNode* ch : simplifiedChildren) {
            if (isOperator(ch->symbol) || ch->symbol == "addop" || ch->symbol == "mulop") {
                hasOperator = true;
                break;
            }
        }

        if (hasOperator) {
            // 保留表达式节点和所有子节点（包括运算符）
            TreeNode* exprNode = new TreeNode(node->symbol);
            for (TreeNode* ch : simplifiedChildren) {
                exprNode->addChild(ch);
            }
            return exprNode;
        } else {
            // 没有运算符，可以简化
            // 如果有多个子节点，创建seq节点
            if (simplifiedChildren.size() > 1) {
                TreeNode* seqNode = new TreeNode("seq");
                for (TreeNode* ch : simplifiedChildren) {
                    seqNode->addChild(ch);
                }
                return seqNode;
            }
        }
    }

    // ---------- 特殊处理：运算符节点 ----------
    // 运算符节点需要保留，并处理其子节点
    if (node->symbol == "addop" || node->symbol == "mulop" ||
        node->symbol == "relop" || node->symbol == "ASSIGN") {

        TreeNode* opNode = new TreeNode(node->symbol);

        // 如果运算符节点有子节点，可能是具体的运算符符号（如"PLUS", "MOD"等）
        if (!node->children.empty()) {
            for (TreeNode* ch : node->children) {
                TreeNode* c = duplicateAndSimplify(ch);
                if (c) {
                    // 如果子节点是具体的运算符，将其值提升到当前节点
                    if (isOperator(c->symbol) || isOperator(c->value)) {
                        if (!c->value.empty()) {
                            opNode->value = c->value;
                        } else {
                            opNode->value = c->symbol;
                        }
                        delete c;
                    } else {
                        opNode->addChild(c);
                    }
                }
            }
        } else if (!node->value.empty()) {
            // 如果当前节点有值，保留它
            opNode->value = node->value;
        }

        return opNode;
    }

    // ---------- 特殊处理：赋值语句 ----------
    if (node->symbol == "assign-stmt") {
        TreeNode* assignNode = new TreeNode("assign-stmt");

        for (TreeNode* ch : node->children) {
            // 跳过标点符号
            if (isPunctuation(ch->symbol)) continue;

            TreeNode* c = duplicateAndSimplify(ch);
            if (c) assignNode->addChild(c);
        }

        // 如果assign-stmt有ID和exp两个子节点，可以简化结构
        if (assignNode->children.size() == 2) {
            // 检查是否为ID和exp
            bool hasID = false, hasExp = false;
            for (TreeNode* ch : assignNode->children) {
                if (ch->symbol == "ID") hasID = true;
                if (ch->symbol == "exp" || ch->symbol == "simple-exp" ||
                    ch->symbol == "term" || ch->symbol == "factor") hasExp = true;
            }

            if (hasID && hasExp) {
                // 保持当前结构
                return assignNode;
            }
        }

        return assignNode;
    }

    // ---------- 特殊处理：definition-list ----------
    if (node->symbol == "definition-list") {
        TreeNode* listNode = new TreeNode("definition-list");
        bool isTopLevel = true;

        for (TreeNode* ch : node->children) {
            if (isPunctuation(ch->symbol)) continue;

            TreeNode* c = duplicateAndSimplify(ch);
            if (!c) continue;

            if (c->symbol == "definition") {
                listNode->addChild(c);
            } else if (c->symbol == "definition-list") {
                isTopLevel = false;
                for (TreeNode* gc : c->children) {
                    listNode->addChild(gc);
                }
                c->children.clear();
                delete c;
            } else {
                TreeNode* defNode = new TreeNode("definition");
                defNode->addChild(c);
                listNode->addChild(defNode);
            }
        }

        if (isTopLevel && listNode->children.size() == 1) {
            TreeNode* only = listNode->children[0];
            listNode->children.clear();
            delete listNode;
            return only;
        }

        return listNode;
    }

    // ---------- 其他特殊节点的处理 ----------
    // 这些节点保持原有结构
    static const set<string> keepStructureNodes = {
        "program", "function-definition", "compound-stmt",
        "if", "do-while-stmt", "while-stmt", "for-stmt",
        "return-stmt", "local-definition-list"
    };

    if (keepStructureNodes.find(node->symbol) != keepStructureNodes.end()) {
        TreeNode* newNode = new TreeNode(node->symbol, node->value);
        newNode->line = node->line;

        for (TreeNode* ch : node->children) {
            if (isPunctuation(ch->symbol)) continue;

            TreeNode* c = duplicateAndSimplify(ch);
            if (c) newNode->addChild(c);
        }

        return newNode;
    }

    // ---------- 默认处理：普通节点 ----------
    TreeNode* newNode = new TreeNode(node->symbol, node->value);
    newNode->line = node->line;

    // 处理子节点
    vector<TreeNode*> validChildren;
    for (TreeNode* ch : node->children) {
        // 跳过标点符号
        if (isPunctuation(ch->symbol)) continue;

        TreeNode* c = duplicateAndSimplify(ch);
        if (c) validChildren.push_back(c);
    }

    // 根据节点类型决定如何处理子节点
    if (isOperator(node->symbol) || node->symbol == "addop" || node->symbol == "mulop") {
        // 运算符节点：保留所有子节点
        for (TreeNode* ch : validChildren) {
            newNode->addChild(ch);
        }
    } else if (isListLikeNode(node->symbol)) {
        // 列表节点：平铺处理
        for (TreeNode* ch : validChildren) {
            if (ch->symbol == node->symbol) {
                // 展开嵌套的列表
                for (TreeNode* gc : ch->children) {
                    newNode->addChild(gc);
                }
                ch->children.clear();
                delete ch;
            } else {
                newNode->addChild(ch);
            }
        }
    } else {
        // 普通节点：简化处理
        for (TreeNode* ch : validChildren) {
            newNode->addChild(ch);
        }

        // 冗余节点提升（保守策略）
        static const set<string> noPromoteNodes = {
            "function-definition", "compound-stmt", "global-variable-definition",
            "definition", "definition-list", "program", "if", "do-while-stmt",
            "while-stmt", "for-stmt", "return-stmt", "assignment-expression",
            "assign-stmt", "exp", "simple-exp", "term", "factor",
            "ID", "NUMBER", "FLOAT_NUMBER"
        };

        if (isRedundantNode(node) && newNode->children.size() == 1) {
            if (noPromoteNodes.find(node->symbol) == noPromoteNodes.end()) {
                TreeNode* only = newNode->children[0];
                newNode->children.clear();
                delete newNode;
                return only;
            }
        }
    }

    // 如果节点没有子节点，但需要保留
    if (newNode->children.empty()) {
        static const set<string> keepEmptyNodes = {
            "ID", "NUMBER", "FLOAT_NUMBER", "int", "double", "void", "float", "char", "bool",
            "return", "if", "else", "while", "do", "for",
            "PLUS", "MINUS", "MULTIPLY", "DIVIDE", "MOD",
            "LT", "RT", "LTEQ", "RTEQ", "EQ", "NE", "ASSIGN"
        };

        if (keepEmptyNodes.find(node->symbol) == keepEmptyNodes.end()) {
            delete newNode;
            return nullptr;
        }
    }

    return newNode;
}

// ========================= 对外接口 =========================

// 构建语法树
TreeNode* GenerateSyntaxTree::buildSyntaxTree(TreeNode* parseRoot) {
    if (syntaxTreeRoot) {
        delete syntaxTreeRoot;
        syntaxTreeRoot = nullptr;
    }

    if (!parseRoot) return nullptr;

    syntaxTreeRoot = duplicateAndSimplify(parseRoot);

    // 如果语法树根是program，但只有一个definition-list子节点，可以简化
    if (syntaxTreeRoot && syntaxTreeRoot->symbol == "program" &&
        syntaxTreeRoot->children.size() == 1) {
        TreeNode* child = syntaxTreeRoot->children[0];
        if (child->symbol == "definition-list") {
            TreeNode* temp = syntaxTreeRoot;
            syntaxTreeRoot = child;
            temp->children.clear();
            delete temp;
        }
    }

    // ===== 控制台打印简化后的语法树 =====
    QTextStream(stdout) << "\n================ Syntax Tree (Simplified) ================\n";
    printSyntaxTree(syntaxTreeRoot, 0);
    QTextStream(stdout) << "=========================================================\n";

    return syntaxTreeRoot;
}

// 兼容接口
TreeNode* GenerateSyntaxTree::simplifyNode(TreeNode* node) {
    return duplicateAndSimplify(node);
}

TreeNode* GenerateSyntaxTree::simplifyNodeRecursive(TreeNode* node) {
    return duplicateAndSimplify(node);
}

// ========================= 显示辅助 =========================

// 格式化语法树为字符串
QString GenerateSyntaxTree::formatTreeForDisplay(TreeNode* root) {
    if (!root) return "";

    QString result;
    stack<pair<TreeNode*, int>> st;
    st.push({root, 0});

    while (!st.empty()) {
        auto pr = st.top(); st.pop();
        TreeNode* node = pr.first;
        int depth = pr.second;

        QString indent(depth * 2, ' ');
        QString nodeInfo = QString::fromStdString(node->symbol);
        if (!node->value.empty() && node->value != node->symbol) {
            nodeInfo += " : " + QString::fromStdString(node->value);
        }
        result += indent + nodeInfo + "\n";

        for (auto it = node->children.rbegin(); it != node->children.rend(); ++it)
            st.push({*it, depth + 1});
    }
    return result;
}

// 创建 QTreeWidgetItem
QTreeWidgetItem* GenerateSyntaxTree::createTreeWidgetItem(TreeNode* node) {
    if (!node) return nullptr;

    QString symbolText = QString::fromStdString(node->symbol);
    QString valueText  = QString::fromStdString(node->value);
    if (valueText == symbolText) valueText.clear();

    QTreeWidgetItem* item = new QTreeWidgetItem(QStringList() << symbolText << valueText);
    for (TreeNode* ch : node->children) {
        QTreeWidgetItem* ci = createTreeWidgetItem(ch);
        if (ci) item->addChild(ci);
    }
    return item;
}
