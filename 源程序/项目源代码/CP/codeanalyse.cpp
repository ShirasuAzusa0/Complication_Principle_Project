/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: codeanalyse.cpp
 * @Brief: 源程序语法分析类的实现部分，用于进行源程序的语法分析
 * @Module: 源程序语法分析模块
 *
 * @Current Version: 3.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/19   袁知本       模块化，从原先的widget.cpp中分离
 *   3.0.0    2026/2/4    袁知本       优化LEX文件解析代码
 ***********************************************************************/
#include "codeanalyse.h"
#include "regex.h"
#include "grammar.h"
#include "first.h"
#include "lr1dfa.h"
#include "utils.h"

// 全局编码映射
map<int, string> encodingToTerminalMap;

// 与 inputTokens 对齐的 lexeme 序列
vector<string> tokenLexemes;

CodeAnalyse::CodeAnalyse()
{

}

// ==================== 语法树打印工具 ====================

// 递归打印语法树（控制台）
static void printSyntaxTree(TreeNode* node, int depth = 0)
{
    if (!node) return;

    QString indent;
    for (int i = 0; i < depth; ++i)
        indent += "  ";

    qDebug().noquote() << indent + QString::fromStdString(node->symbol);

    for (TreeNode* child : node->children) {
        printSyntaxTree(child, depth + 1);
    }
}

// 执行LEX文件语法分析
bool CodeAnalyse::analyseLEXContent(const QString& lexContent,
                                    const QString& grammarContent,
                                    QTableWidget* tableWidget)
{
    // 1. 初始化清理
    if (tableWidget == nullptr) {
        qDebug() << "错误：表格控件为空";
        return false;
    }

    // 清理表格
    tableWidget->clear();
    tableWidget->setRowCount(0);
    tableWidget->setColumnCount(5);
    QStringList headers;
    headers << "步骤" << "状态栈" << "符号栈" << "剩余输入" << "动作";
    tableWidget->setHorizontalHeaderLabels(headers);

    // 2. 检查输入内容
    if (lexContent.trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "提示", "LEX文件内容为空");
        return false;
    }

    if (grammarContent.trimmed().isEmpty()) {
        QMessageBox::warning(nullptr, "错误", "文法内容为空");
        return false;
    }

    // 3. 初始化编码映射
    if (!initializeEncodingMapping()) {
        qDebug() << "警告：编码映射初始化不完整，可能影响分析";
    }

    // 4. 解析文法
    grammarStr = grammarContent.toStdString();
    init_2();                           // 清空初始化
    GrammarAnalyse::handleGrammar();    // 文法处理
    AnalyseFIRST::getFirstSets();       // 计算FIRST集
    GenerateLR1DFA::getLR1();           // 构建LR(1)自动机

    // 5. 构建ACTION/GOTO表
    map<pair<int, string>, string> ACTION;
    map<pair<int, string>, int> GOTO;
    if (!buildAnalysisTables(ACTION, GOTO)) {
        QMessageBox::warning(nullptr, "错误", "分析表构建失败");
        return false;
    }

    // 6. 解析LEX内容
    vector<string> rawTokens = parseLEXContent(lexContent);
    if (rawTokens.empty()) {
        QMessageBox::warning(nullptr, "错误", "无法从LEX内容解析Token序列");
        return false;
    }

    vector<string> inputTokens;
    for (const string& tok : rawTokens) {

        if (tok == "$") {
            inputTokens.push_back("$");
            continue;
        }

        if (tok == "ID" || tok == "NUMBER" || tok == "FLOAT") {
            inputTokens.push_back(tok);
            continue;
        }

        bool mapped = false;

        // 在 sequenceEncodings 中查 displayName -> grammar终结符
        for (const auto& seq : sequenceEncodings) {
            int hit = 0;  // 命中计数
            for (const auto& p : seq.second) {
                // p.first  = grammar终结符名（ASSIGN / PLUS / MOD）
                // p.second = 编码
                if (encodingToTerminalMap[p.second] == tok) {
                    hit++;
                    if (hit == 2) {      // 匹配后者为语法终结符
                        inputTokens.push_back(p.first);
                        mapped = true;
                        break;
                    }
                }
            }
            if (mapped) break;
        }

        if (!mapped) {
            qDebug() << "错误：无法将Token规范化：" << QString::fromStdString(tok);
            inputTokens.push_back(tok); // 保底，方便你看错误
        }
    }

    qDebug() << "解析到Token序列:" << tokensToString(inputTokens);

    // 7. 执行LR(1)分析
    bool success = executeLRAnalysis(inputTokens, ACTION, GOTO, tableWidget);

    // 8. 配置表格显示
    configureTableDisplay(tableWidget);

    return success;
}

// 解析LEX文件内容
vector<string> CodeAnalyse::parseLEXContent(const QString& lexContent)
{
    tokenLexemes.clear();

    vector<string> tokens;

    QStringList parts = lexContent.split(QRegExp("\\s+"),
                                         Qt::SkipEmptyParts);

    for (int i = 0; i < parts.size(); ++i) {
        QString part = parts[i];

        // 结束符
        if (part == "-1") {
            tokens.push_back("$");
            tokenLexemes.push_back("");
            break;
        }

        bool ok = false;
        int encoding = part.toInt(&ok);

        if (!ok) {
            // 非数字（只能是 "xxx"），作为上一个 token 的 lexeme
            if (part.startsWith('"') && part.endsWith('"')) {
                QString lex = part.mid(1, part.length() - 2); // 去掉引号
                if (!tokenLexemes.empty()) {
                    tokenLexemes.back() = lex.toStdString();
                }
            }
            continue;
        }

        auto it = encodingToTerminalMap.find(encoding);
        if (it != encodingToTerminalMap.end()) {
            tokens.push_back(it->second);
            tokenLexemes.push_back("");   // 先占位，等后面的 "xxx"
        } else {
            qDebug() << "警告：未识别的编码：" << encoding;
            tokens.push_back("UNKNOWN");
        }

        // 跳过后面的 "lexeme"
        /*
        if (i + 1 < parts.size() &&
            parts[i + 1].startsWith('"') &&
            parts[i + 1].endsWith('"')) {
            i++;
        }*/
    }

    // 保证以 $ 结尾
    if (tokens.empty() || tokens.back() != "$")
        tokens.push_back("$");

    return tokens;
}

// 初始化编码映射
bool CodeAnalyse::initializeEncodingMapping()
{
    encodingToTerminalMap.clear();

    // 1. 普通正则（如 _ID400, _num500）
    for (const auto& enc : regexEncodings) {
        if (!enc.isSequence) {
            string name = enc.regexName;

            // 去掉前导下划线
            if (!name.empty() && name[0] == '_') {
                name.erase(0, 1);
            }

            // 去掉末尾数字
            while (!name.empty() && isdigit(name.back())) {
                name.pop_back();
            }

            // grammar 里用的是 ID / NUMBER
            encodingToTerminalMap[enc.encoding] = name;
        }
    }

    // 2. 序列编码（special -> specialName）
    // sequenceEncodings: regexName -> [(terminalName, encoding)]
    for (const auto& seq : sequenceEncodings) {
        for (const auto& p : seq.second) {
            // p.first  本来就是 PLUS / ASSIGN / MOD ...
            encodingToTerminalMap[p.second] = p.first;
        }
    }

    // 3. 文件结束符
    encodingToTerminalMap[-1] = "$";

    qDebug() << "编码映射初始化完成，共"
             << encodingToTerminalMap.size() << "项";

    return true;
}

// 构建分析表
bool CodeAnalyse::buildAnalysisTables(map<pair<int, string>, string>& ACTION,
                                      map<pair<int, string>, int>& GOTO)
{
    if (lr1States.empty()) {
        qDebug() << "错误：LR(1)状态为空";
        return false;
    }

    // 1. 处理状态转移
    for (const lr1State &st : lr1States) {
        int sid = st.sid;
        for (const nextStateUnit &n : st.nextStateVector) {
            string sym = n.c;
            int toSid = n.sid;
            if (GrammarAnalyse::isSmallAlpha(sym) || sym == "$") {
                ACTION[make_pair(sid, sym)] = "s" + to_string(toSid);
            } else {
                GOTO[make_pair(sid, sym)] = toSid;
            }
        }
    }

    // 2. 构建完整的GOTO表
    for (const lr1State &fromState : lr1States) {
        int fromSid = fromState.sid;
        for (const string &nonTerminal : bigAlpha) {
            if (GOTO.find(make_pair(fromSid, nonTerminal)) != GOTO.end()) {
                continue;
            }
            for (const nextStateUnit &n : fromState.nextStateVector) {
                if (n.c == nonTerminal) {
                    GOTO[make_pair(fromSid, nonTerminal)] = n.sid;
                    break;
                }
            }
        }
    }

    // 3. 处理归约动作
    for (const lr1State &st : lr1States) {
        int sid = st.sid;
        for (int itemid : st.itemV) {
            const lr1Item &it = lr1Items[itemid];
            const grammarUnit &g = grammarDeque[it.gid];
            QStringList rightList = QString::fromStdString(g.right).split(" ", Qt::SkipEmptyParts);
            int rightSize = rightList.size();

            if (it.index == rightSize || g.right == "#") {
                if (g.left == "program" && it.lookahead == "$") {
                    ACTION[make_pair(sid, it.lookahead)] = "acc";
                } else {
                    string rLabel = "r" + to_string(g.gid);
                    ACTION[make_pair(sid, it.lookahead)] = rLabel;
                }
            }
        }
    }

    return !ACTION.empty();
}

// 执行LR分析
bool CodeAnalyse::executeLRAnalysis(const vector<string>& inputTokens,
                                    const map<pair<int, string>, string>& ACTION,
                                    const map<pair<int, string>, int>& GOTO,
                                    QTableWidget* tableWidget)
{
    int tableRow = 0;
    int stepCounter = 0;

    const int MAX_STEPS = 5000;   // 防止无限规约导致内存爆炸

    // 初始化栈
    vector<int> stateStack;
    vector<string> symbolStack;
    stateStack.push_back(0);

    // ===== 语法树构建初始化 =====
    nodeStack.clear();
    parseTreeRoot = nullptr;
    syntaxTreeRoot = nullptr;

    // 显示初始状态
    addAnalysisStep(tableWidget, stepCounter++,
                   stateStackToString(stateStack),
                   symbolStackToString(symbolStack),
                   tokensToString(inputTokens),
                   "初始化",
                   tableRow);

    size_t ip = 0;
    bool accept = false;
    bool error = false;

    while (!accept && !error) {

        // ====== 硬性步数保护 ======
        if (stepCounter > MAX_STEPS) {
            addAnalysisStep(tableWidget, stepCounter,
                           stateStackToString(stateStack),
                           symbolStackToString(symbolStack),
                           "",
                           "错误：分析步骤过多，可能存在无限规约或分析死循环",
                           tableRow);
            return false;
        }

        int curState = stateStack.back();
        string a = inputTokens[ip];

        auto actIt = ACTION.find(make_pair(curState, a));
        string act = (actIt != ACTION.end()) ? actIt->second : "";

        if (act.empty()) {
            QString remaining = tokensToString(
                vector<string>(inputTokens.begin() + ip, inputTokens.end()));
            addAnalysisStep(tableWidget, stepCounter++,
                           stateStackToString(stateStack),
                           symbolStackToString(symbolStack),
                           remaining,
                           QString("错误：状态%1遇到'%2'无定义动作")
                           .arg(curState)
                           .arg(QString::fromStdString(a)),
                           tableRow);
            error = true;
            break;
        }

        if (act == "acc") {
            addAnalysisStep(tableWidget, stepCounter++,
                           stateStackToString(stateStack),
                           symbolStackToString(symbolStack),
                           "$",
                           "接受：分析成功完成",
                           tableRow);

            // ACC: 定语法树根
            if (!nodeStack.empty()) {
                TreeNode* root = nodeStack.back();

                // 确保根节点是program或definition-list
                // 如果最外层是function-definition，说明分析结构有误
                if (root->symbol == "function-definition") {
                    qDebug() << "警告：根节点为function-definition，可能分析结构错误";
                    // 包装成definition-list
                    TreeNode* definitionListNode = new TreeNode("definition-list");
                    TreeNode* definitionNode = new TreeNode("definition");
                    definitionNode->addChild(root);
                    definitionListNode->addChild(definitionNode);
                    root = definitionListNode;
                }

                parseTreeRoot = root;
                // ===== 控制台打印原始语法树（用于调试） =====
                qDebug().noquote() << "================ Parse Tree ================";
                printSyntaxTree(parseTreeRoot, 0);
                qDebug().noquote() << "============================================";
                syntaxTreeRoot = GenerateSyntaxTree::buildSyntaxTree(parseTreeRoot);
            }

            accept = true;
            break;
        }

        if (act[0] == 's') {
            int toState = stoi(act.substr(1));
            QString remaining = tokensToString(
                vector<string>(inputTokens.begin() + ip, inputTokens.end()));

            addAnalysisStep(tableWidget, stepCounter++,
                           stateStackToString(stateStack),
                           symbolStackToString(symbolStack),
                           remaining,
                           QString("移进：状态%1遇到'%2'，移进到状态%3")
                           .arg(curState)
                           .arg(QString::fromStdString(a))
                           .arg(toState),
                           tableRow);

            // Shift：构造语法树叶子结点
            symbolStack.push_back(a);
            stateStack.push_back(toState);

            TreeNode* leaf = new TreeNode(a);

            // 只给真正有值的终结符塞 value
            if ((a == "ID" || a == "NUMBER" || a == "FLOAT") &&
                ip < tokenLexemes.size()) {
                leaf->value = tokenLexemes[ip];
            }

            nodeStack.push_back(leaf);
            ip++;
            continue;
        }

        if (act[0] == 'r') {
            int gid = stoi(act.substr(1));
            if (gid < 0 || gid >= (int)grammarDeque.size()) {
                QString remaining = tokensToString(
                    vector<string>(inputTokens.begin() + ip, inputTokens.end()));
                addAnalysisStep(tableWidget, stepCounter++,
                               stateStackToString(stateStack),
                               symbolStackToString(symbolStack),
                               remaining,
                               QString("错误：无效的规约编号%1").arg(gid),
                               tableRow);
                error = true;
                break;
            }

            const grammarUnit &prod = grammarDeque[gid];
            QStringList rightList =
                QString::fromStdString(prod.right).split(" ", Qt::SkipEmptyParts);
            int rcount = (prod.right == "#") ? 0 : rightList.size();

            QString remaining = tokensToString(
                vector<string>(inputTokens.begin() + ip, inputTokens.end()));
            addAnalysisStep(tableWidget, stepCounter++,
                           stateStackToString(stateStack),
                           symbolStackToString(symbolStack),
                           remaining,
                           QString("规约：使用产生式 %1 -> %2")
                           .arg(QString::fromStdString(prod.left))
                           .arg(QString::fromStdString(prod.right)),
                           tableRow);

            // 调试输出：显示规约时的节点栈
            qDebug() << "规约前nodeStack大小:" << nodeStack.size();
            for (size_t i = 0; i < nodeStack.size(); ++i) {
                qDebug() << "  nodeStack[" << i << "]: " << QString::fromStdString(nodeStack[i]->symbol);
            }

            for (int k = 0; k < rcount; k++) {
                if (!symbolStack.empty()) symbolStack.pop_back();
                if (!stateStack.empty()) stateStack.pop_back();
            }

            // Reduce：构造语法树子树
            // 1. 收集右部对应的子树
            vector<TreeNode*> children;
            for (int k = 0; k < rcount; ++k) {
                if (!nodeStack.empty()) {
                    children.push_back(nodeStack.back());
                    nodeStack.pop_back();
                }
            }
            reverse(children.begin(), children.end());

            // 创建父节点
            TreeNode* parent = nullptr;

            // 先根据产生式类型创建父节点
            if (prod.left == "program" && children.size() == 1) {
                qDebug() << "规约program -> definition-list";
                parent = new TreeNode("program");
                parent->addChild(children[0]);
            }
            else if (prod.left == "definition-list" && children.size() == 2) {
                qDebug() << "规约definition-list -> definition-list definition";

                TreeNode* listNode = children[0];
                TreeNode* defNode = children[1];

                // 检查listNode是否为definition-list
                if (listNode->symbol != "definition-list") {
                    qDebug() << "错误：第一个子节点不是definition-list，而是" << QString::fromStdString(listNode->symbol);
                    // 创建新的definition-list
                    TreeNode* newListNode = new TreeNode("definition-list");
                    if (listNode->symbol == "definition") {
                        newListNode->addChild(listNode);
                    }
                    newListNode->addChild(defNode);
                    nodeStack.push_back(newListNode);
                } else {
                    // 正确情况：listNode已经是definition-list
                    // 检查defNode是否为definition
                    if (defNode->symbol != "definition") {
                        qDebug() << "第二个子节点不是definition，而是" << QString::fromStdString(defNode->symbol);
                        TreeNode* wrapDef = new TreeNode("definition");
                        wrapDef->addChild(defNode);
                        listNode->addChild(wrapDef);
                    } else {
                        listNode->addChild(defNode);
                    }
                    nodeStack.push_back(listNode);
                }
            }

            // 5. definition-list -> definition
            else if (prod.left == "definition-list" && children.size() == 1) {
                qDebug() << "规约definition-list -> definition";

                TreeNode* defNode = children[0];
                TreeNode* listNode = new TreeNode("definition-list");

                // 检查defNode是否为definition
                if (defNode->symbol != "definition") {
                    qDebug() << "子节点不是definition，而是" << QString::fromStdString(defNode->symbol);
                    TreeNode* wrapDef = new TreeNode("definition");
                    wrapDef->addChild(defNode);
                    listNode->addChild(wrapDef);
                } else {
                    listNode->addChild(defNode);
                }

                nodeStack.push_back(listNode);
            }

            // 6. definition -> function-definition
            else if (prod.left == "definition" && children.size() == 1) {
                qDebug() << "规约definition -> function-definition";

                TreeNode* funcNode = children[0];
                TreeNode* defNode = new TreeNode("definition");
                defNode->addChild(funcNode);
                nodeStack.push_back(defNode);
            }

            // 7. definition -> global-variable-definition
            else if (prod.left == "definition" && children.size() == 1) {
                qDebug() << "规约definition -> global-variable-definition";

                TreeNode* globalVarNode = children[0];
                TreeNode* defNode = new TreeNode("definition");
                defNode->addChild(globalVarNode);
                nodeStack.push_back(defNode);
            }

            // 8. function-definition产生式（防止嵌套）
            else if (prod.left == "function-definition") {
                qDebug() << "规约function-definition产生式";

                // 检查是否有definition-list混入
                bool hasDefinitionList = false;
                for (TreeNode* child : children) {
                    if (child->symbol == "definition-list") {
                        hasDefinitionList = true;
                        qDebug() << "警告：function-definition包含definition-list，这应该是错误！";
                    }
                }

                if (hasDefinitionList) {
                    // 错误情况：function-definition不应该包含definition-list
                    // 应该只包含：type-indicator, ID, parameters, compound-stmt等
                    // 过滤掉definition-list
                    vector<TreeNode*> filteredChildren;
                    for (TreeNode* child : children) {
                        if (child->symbol != "definition-list") {
                            filteredChildren.push_back(child);
                        } else {
                            qDebug() << "丢弃definition-list节点";
                            // 这个definition-list应该被推到nodeStack后面独立处理
                            nodeStack.push_back(child);
                        }
                    }

                    if (!filteredChildren.empty()) {
                        TreeNode* funcNode = new TreeNode("function-definition");
                        for (TreeNode* child : filteredChildren) {
                            funcNode->addChild(child);
                        }
                        nodeStack.push_back(funcNode);
                    }
                } else {
                    // 正常情况
                    TreeNode* funcNode = new TreeNode("function-definition");
                    for (TreeNode* child : children) {
                        funcNode->addChild(child);
                    }
                    nodeStack.push_back(funcNode);
                }
            }

            // 9. 其他产生式：通用处理
            else {
                TreeNode* parent = new TreeNode(prod.left);
                for (TreeNode* ch : children) {
                    parent->addChild(ch);
                }
                nodeStack.push_back(parent);
            }

            // 调试输出：显示收集到的子节点
            qDebug() << "规约" << QString::fromStdString(prod.left) << "->" << QString::fromStdString(prod.right);
            qDebug() << "收集到" << children.size() << "个子节点";
            for (size_t i = 0; i < children.size(); ++i) {
                qDebug() << "  子节点" << i << ":" << QString::fromStdString(children[i]->symbol);
            }

            int cur = stateStack.back();
            string A = prod.left;
            auto gotoIt = GOTO.find(make_pair(cur, A));

            if (gotoIt == GOTO.end()) {
                addAnalysisStep(tableWidget, stepCounter++,
                               stateStackToString(stateStack),
                               symbolStackToString(symbolStack),
                               remaining,
                               QString("错误：状态%1遇到非终结符%2无GOTO转移")
                               .arg(cur)
                               .arg(QString::fromStdString(A)),
                               tableRow);
                error = true;
                break;
            }

            int gotoState = gotoIt->second;
            symbolStack.push_back(A);
            stateStack.push_back(gotoState);

            addAnalysisStep(tableWidget, stepCounter++,
                           stateStackToString(stateStack),
                           symbolStackToString(symbolStack),
                           remaining,
                           "规约完成，执行GOTO",
                           tableRow);
            continue;
        }
    }

    addAnalysisStep(tableWidget, stepCounter++,
                   "", "", "",
                   error ? "分析失败" : "分析成功完成",
                   tableRow);

    return !error;
}

// 辅助函数：Token序列转字符串
QString CodeAnalyse::tokensToString(const vector<string>& tokens)
{
    QStringList list;
    for (const auto& token : tokens) {
        list << QString::fromStdString(token);
    }
    return list.join(" ");
}

// 辅助函数：状态栈转字符串
QString CodeAnalyse::stateStackToString(const vector<int>& states)
{
    QStringList list;
    for (int state : states) {
        list << QString::number(state);
    }
    return list.join(" ");
}

// 辅助函数：符号栈转字符串
QString CodeAnalyse::symbolStackToString(const vector<string>& symbols)
{
    QStringList list;
    for (const string& sym : symbols) {
        list << QString::fromStdString(sym);
    }
    return list.join(" ");
}

// 添加分析步骤到表格
void CodeAnalyse::addAnalysisStep(QTableWidget* tableWidget,
                                  int step,
                                  const QString& stateStack,
                                  const QString& symbolStack,
                                  const QString& remainingInput,
                                  const QString& action,
                                  int& row)
{
    tableWidget->insertRow(row);
    tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(step)));
    tableWidget->setItem(row, 1, new QTableWidgetItem(stateStack));
    tableWidget->setItem(row, 2, new QTableWidgetItem(symbolStack));
    tableWidget->setItem(row, 3, new QTableWidgetItem(remainingInput));
    tableWidget->setItem(row, 4, new QTableWidgetItem(action));
    row++;
}

// 配置表格显示
void CodeAnalyse::configureTableDisplay(QTableWidget* tableWidget)
{
    // 列宽配置
    tableWidget->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    tableWidget->setColumnWidth(0, 60);
    tableWidget->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Interactive);
    tableWidget->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    tableWidget->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Interactive);

    // 滚动条优化
    tableWidget->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);
    tableWidget->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    QScrollBar* hBar = tableWidget->horizontalScrollBar();
    if (hBar) {
        hBar->setSingleStep(8);
        hBar->setPageStep(150);
    }

    QScrollBar* vBar = tableWidget->verticalScrollBar();
    if (vBar) {
        vBar->setSingleStep(8);
        vBar->setPageStep(150);
    }
}
