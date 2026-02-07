/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: widget.cpp
 * @Brief: Widget窗口类的实现部分，用于实现界面交互
 * @Module: 主界面模块
 *
 * @Current Version: 2.0.0
 * @Author: 袁知本
 * @Created Date: 2025/10/5
 *
 * @Revision History:
 *   Version  Date        Author      Description
 *   -------  ----------  ----------  -------------------
 *   1.0.0    2025/10/5   袁知本       初始版本创建
 *   2.0.0    2026/1/16   袁知本       重构代码逻辑，只保留与界面交互部分
 ***********************************************************************/
#include "widget.h"
#include "ui_widget.h"
#include "utils.h"
#include "regex.h"
#include "nfa.h"
#include "dfa.h"
#include "code.h"
#include "grammar.h"
#include "first.h"
#include "follow.h"
#include "lr0dfa.h"
#include "slr1analyse.h"
#include "lr1dfa.h"
#include "lr1analyse.h"
#include "codeanalyse.h"
#include "syntaxtree.h"
#include "midcode.h"

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QFileDialog>
#include <QTextCodec>
#include <QFileDialog>
#include <sstream>
#include <fstream>
#include <iostream>

#pragma execution_character_set("utf-8")
using namespace std;

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    if (!this->layout()) {
        QVBoxLayout *rootLayout = new QVBoxLayout(this);
        rootLayout->setContentsMargins(0, 0, 0, 0);
        rootLayout->setSpacing(0);
        rootLayout->addWidget(ui->stackedWidget);
    }
    wrapPage(ui->page_1, ui->graphicsView_1);
    wrapPage(ui->page_2, ui->graphicsView_2);
}

void Widget::wrapPage(QWidget *page, QGraphicsView *view)
{
    page->setMinimumSize(0, 0);
    page->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    view->setMinimumSize(0, 0);

    auto *scene = new QGraphicsScene(view);
    view->setScene(scene);

    auto *proxy = scene->addWidget(page);

    // scene 大小 = page 实际大小
    scene->setSceneRect(proxy->boundingRect());

    view->setAlignment(Qt::AlignCenter);
    view->setResizeAnchor(QGraphicsView::AnchorViewCenter);
    view->setTransformationAnchor(QGraphicsView::AnchorViewCenter);

    // 保存 proxy
    view->setProperty("proxy", QVariant::fromValue<void*>(proxy));

    // 监听 resize
    view->viewport()->installEventFilter(this);

    // 第一次直接铺满
    view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Resize) {
        auto *viewport = qobject_cast<QWidget*>(watched);
        if (!viewport) return false;

        auto *view = qobject_cast<QGraphicsView*>(viewport->parent());
        if (!view) return false;

        view->fitInView(view->scene()->sceneRect(), Qt::KeepAspectRatio);
    }
    return QWidget::eventFilter(watched, event);
}


Widget::~Widget()
{
    delete ui;
}

// 查看正则表达式的输入规则说明
void Widget::on_pushButton_Help_clicked()
{
    QString message = R"(
                    输入格式要求
    (1)通过命名中加下划线()来表示该正则表达式需要生成DFA图。
    (2)命名中的名字后的数值为对应单词的编码。
    (3)命名中的名字中数值后加上S表示后面有多个单词，但对应的单词编码从这个数值开始编码。
    (4)由于整数中的正号(+),在系统已经被用作正比闭包运算符号，所以需要通过引入转义符号\来区分。同理，如算术运算符号(*),在系统已经被用作闭包运算符号，所以也需要通过引入转义符号\来区分。

                        注意事项
    (1)输入/导入正则表达式后，先点击“开始分析”按钮后，在点击其他按钮查看各项分析生成结果。
    (2)生成词法分析程序时，需要把sample.tny也存放到选择的文件夹中。
    )";

    QMessageBox::information(this, "输入规则", message);
}

// 导入正则表达式
void Widget::on_pushButton_LoadRegex_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("文本文件 (*.txt);;所有文件 (*.*)"));

    if (!filePath.isEmpty()) {
        ifstream inputFile;
        QTextCodec* code = QTextCodec::codecForName("GB2312");

        string selectedFile = code->fromUnicode(filePath.toStdString().c_str()).data();
        inputFile.open(selectedFile.c_str(), ios::in);


        //        cout<<filePath.toStdString();
        //        ifstream inputFile(filePath.toStdString());
        if (!inputFile) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening file." << endl;
        }
        // 读取文件内容并显示在 plainTextEdit_2
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContents = QString::fromStdString(buffer.str());
        ui->plainTextEdit_2->setPlainText(fileContents);
    }
}

// 导出正则表达式
void Widget::on_pushButton_SaveRegex_clicked()
{
    // 保存结果到文本文件
    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("保存结果文件"), QDir::homePath(), tr("文本文件 (*.txt)"));
    if (!saveFilePath.isEmpty() && !ui->plainTextEdit_2->toPlainText().isEmpty()) {
        QFile outputFile(saveFilePath);
        if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&outputFile);
            stream << ui->plainTextEdit_2->toPlainText();
            outputFile.close();
            QMessageBox::about(this, "提示", "导出成功！");
        }
    }
    else if (ui->plainTextEdit_2->toPlainText().isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("输入框为空，请重试！"));
    }
}

// 正则表达式分析
void Widget::on_pushButton_Analyse_clicked()
{
    // 清空全局变量
    init_1();

    // 拿到所有的正则表达式
    QString allRegex = ui->plainTextEdit_2->toPlainText();

    // isLowerCase = ui->checkBox->isChecked();
    // qDebug() <<"是否区分大小写："<< isLowerCase;

    string result = AnalyseRegex::handleAllRegex(allRegex, isLowerCase);
    // 如果字符串不为空就是报错了，退出
    if (!result.empty()) {
        QMessageBox::critical(this, "错误信息", QString::fromStdString(result));
        return;
    }

    QMessageBox::about(this, "提示", "分析成功！请点击其余按钮查看结果！");
}

// 生成 NFA
void Widget::on_pushButton_ShowNFA_clicked()
{
    // 表格内容初始化
    ui->tableWidget->clearContents();           // 清除表格中的数据
    ui->tableWidget->setRowCount(0);            // 清除所有行
    ui->tableWidget->setColumnCount(0);         // 清除所有列

    //正则表达式转换成NFA图
    final_nfa = GenerateNFA::regex2NFA(finalRegex);

    // 设置列数
    int n = 2 + nfaCharSet.size(); // 默认两列：Flag 和 ID
    ui->tableWidget->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "标志" << "ID";
    int headerCount = 3;
    for (const auto& ch : nfaCharSet) {
        if (m1.find(ch) != m1.end()) {
            headerLabels << QString::fromStdString(GenerateNFA::trim(m1[ch]));
        }
        else {
            if (ch == '~') {
                char tt = commentSymbol[1][0];
                string res = commentSymbol[1];
                if (m1.find(tt) != m1.end()) {
                    res = GenerateNFA::trim(m1[tt]);
                }
                headerLabels << QString::fromStdString("非" + res);
            }
            else {
                headerLabels << QString(ch);
            }
        }

        headerCharNum[ch] = headerCount++;
    }
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = statusTable.size();
    ui->tableWidget->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto id : insertionOrder) {
        const statusTableNode& node = statusTable[id];

        // Flag 列
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(node.flag)));

        // ID 列
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(node.id)));

        // TransitionChar 列
        int col = 2;
        for (const auto& transitionEntry : node.m) {
            string resutlt = GenerateNFA::set2string(transitionEntry.second);

            // 放到指定列数据
            ui->tableWidget->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(QString::fromStdString(resutlt)));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 显示表格
    ui->tableWidget->show();
}

// 生成 DFA
void Widget::on_pushButton_ShowDFA_clicked()
{
    // 表格内容初始化
    ui->tableWidget->clearContents();           // 清除表格中的数据
    ui->tableWidget->setRowCount(0);            // 清除所有行
    ui->tableWidget->setColumnCount(0);         // 清除所有列

    // NFA转DFA
    GenerateDFA::NFA2DFA(final_nfa);

    // 设置列数
    int n = 2 + dfaCharSet.size(); // 默认两列：Flag 和 状态集合
    ui->tableWidget->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "标志" << "状态集合";
    int headerCount = 3;
    for (const auto& ch : dfaCharSet) {
        if (m1.find(ch) != m1.end()) {
            headerLabels << QString::fromStdString(GenerateNFA::trim(m1[ch]));
        }
        else {
            if (ch == '~') {
                char tt = commentSymbol[1][0];
                string res = commentSymbol[1];
                if (m1.find(tt) != m1.end()) {
                    res = GenerateNFA::trim(m1[tt]);
                }
                headerLabels << QString::fromStdString("非" + res);
            }
            else {
                headerLabels << QString(ch);
            }
        }

        headerCharNum[ch] = headerCount++;
    }
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);
    // 设置行数
    int rowCount = dfaTable.size();
    ui->tableWidget->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaTable) {

        // Flag 列
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::fromStdString("{" + GenerateNFA::set2string(dfaNode.nfaStates) + "}")));

        // 状态转换 列
        int col = 2;
        for (const auto& transitionEntry : dfaNode.transitions) {
            string re = GenerateNFA::set2string(transitionEntry.second);

            // 放到指定列数据
            ui->tableWidget->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(QString::fromStdString("{" + re + "}")));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 显示表格
    ui->tableWidget->show();
}

// DFA 最小化
void Widget::on_pushButton_ShowMinDFA_clicked()
{
    // 表格内容初始化
    ui->tableWidget->clearContents();           // 清除表格中的数据
    ui->tableWidget->setRowCount(0);            // 清除所有行
    ui->tableWidget->setColumnCount(0);         // 清除所有列

    // DFA最小化
    GenerateDFA::DFAminimize();

    // 设置列数
    int n = 2 + dfaCharSet.size();              // 默认两列：Flag 和 状态集合
    ui->tableWidget->setColumnCount(n);

    // 字符和第X列存起来对应
    map<char, int> headerCharNum;

    // 设置表头
    QStringList headerLabels;
    headerLabels << "标志" << "ID";
    int headerCount = 3;
    for (const auto& ch : dfaCharSet) {
        if (m1.find(ch) != m1.end()) {
            headerLabels << QString::fromStdString(GenerateNFA::trim(m1[ch]));
        }
        else {
            if (ch == '~') {
                char tt = commentSymbol[1][0];
                string res = commentSymbol[1];
                if (m1.find(tt) != m1.end()) {
                    res = GenerateNFA::trim(m1[tt]);
                }
                headerLabels << QString::fromStdString("非" + res);
            }
            else {
                headerLabels << QString(ch);
            }
        }

        headerCharNum[ch] = headerCount++;
    }
    ui->tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    int rowCount = dfaMinTable.size();
    ui->tableWidget->setRowCount(rowCount);

    // 填充数据
    int row = 0;
    for (auto& dfaNode : dfaMinTable) {

        // Flag 列
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(dfaNode.flag)));

        // 状态集合 列
        ui->tableWidget->setItem(row, 1, new QTableWidgetItem(QString::number(dfaNode.id)));

        // 状态转换 列
        int col = 2;
        for (const auto& transitionEntry : dfaNode.transitions) {
            // 放到指定列数据
            ui->tableWidget->setItem(row, headerCharNum[transitionEntry.first] - 1, new QTableWidgetItem(transitionEntry.second == -1 ? QString::fromStdString("") : QString::number(transitionEntry.second)));
            col++;
        }

        row++;
    }

    // 调整列宽
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    // 显示表格
    ui->tableWidget->show();
}

// 生成词法分析程序
void Widget::on_pushButton_GenerateCode_clicked()
{
    QString srcFilePath;
    QString t_filePath=QFileDialog::getExistingDirectory(this,"选择词法分析程序生成路径，并将sample.tny存放在该路径",QDir::currentPath());
    if(t_filePath.isEmpty())
        return;
    else
        srcFilePath=t_filePath;

    qDebug() << "生成词法分析程序...";
    QString res = GenerateCode::generateCode(srcFilePath);//调用主函数
    qDebug() << "词法分析程序生成完成...";

    /*==========文件处理=================*/
    QFile tgtFile(srcFilePath+"/_lexer.c");
    if(!tgtFile.open(QIODevice::ReadWrite|QIODevice::Text|QIODevice::Truncate))
    {
        QMessageBox::warning(NULL, "文件", "文件打开/写入失败");
        return;
    }
    QTextStream outputFile(&tgtFile);
    outputFile<< res;
    tgtFile.close();

    ui->tableWidget->hide();
    ui->plainTextEdit->show();

    ui->plainTextEdit->setPlainText(res);
}

// 查看LEX文件
void Widget::on_pushButton_LoadLEXFile_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("LEX文件 (*.lex)"));

    if (!filePath.isEmpty())
    {
        ifstream inputFile;
        QTextCodec* code = QTextCodec::codecForName("GB2312");

        string selectedFile = code->fromUnicode(filePath.toStdString().c_str()).data();
        inputFile.open(selectedFile.c_str(), ios::in);

        if (!inputFile) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening file." << endl;
        }
        // 读取文件内容并显示在 plainTextEdit_2
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContents = QString::fromStdString(buffer.str());
        ui->tableWidget->hide();
        ui->plainTextEdit->show();
        ui->plainTextEdit->setPlainText(fileContents);
    }
}

// 查看文法输入规则说明
void Widget::on_pushButton_Help_1_clicked()
{
    QString message = R"(
    第一行输入非终结符，用|隔开
    第二行输入终结符，用|隔开
    输入时不同的单词、标识符请用空格隔开；
    用#表示空串，默认左边出现的第一个单词、标识符为文法的开始符号
    同时，文法中含有或(|)，需分开两条输入)";

    QMessageBox::information(this, "输入规则", message);
}

// 载入文法规则
void Widget::on_pushButton_LoadGrammar_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("文本文件 (*.txt);;所有文件 (*.*)"));

    if (!filePath.isEmpty()) {
        ifstream inputFile;
        QTextCodec* code = QTextCodec::codecForName("GB2312");

        string selectedFile = code->fromUnicode(filePath.toStdString().c_str()).data();
        inputFile.open(selectedFile.c_str(), ios::in);

        if (!inputFile) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening file." << endl;
        }
        // 读取文件内容并显示在 plainTextEdit_3
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContents = QString::fromStdString(buffer.str());
        ui->plainTextEdit_3->setPlainText(fileContents);
    }
}

// 保存文法规则
void Widget::on_pushButton_SaveGrammar_clicked()
{
    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("保存文法文件"), QDir::homePath(), tr("文本文件 (*.txt)"));
    if (!saveFilePath.isEmpty() && !ui->plainTextEdit_3->toPlainText().isEmpty()) {
        QFile outputFile(saveFilePath);
        if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&outputFile);
            stream << ui->plainTextEdit_3->toPlainText();
            outputFile.close();
            QMessageBox::about(this, "提示", "导出成功！");
        }
    }
    else if (ui->plainTextEdit_3->toPlainText().isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("输入框为空，请重试！"));
    }
}

// 载入LEX
void Widget::on_pushButton_LoadProgram_clicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, tr("选择文件"), QDir::homePath(), tr("LEX文件 (*.lex)"));

    if (!filePath.isEmpty())
    {
        ifstream inputFile;
        QTextCodec* code = QTextCodec::codecForName("GB2312");

        string selectedFile = code->fromUnicode(filePath.toStdString().c_str()).data();
        inputFile.open(selectedFile.c_str(), ios::in);

        if (!inputFile) {
            QMessageBox::critical(this, "错误信息", "导入错误！无法打开文件，请检查路径和文件是否被占用！");
            cerr << "Error opening file." << endl;
        }
        // 读取文件内容并显示在 plainTextEdit_11
        stringstream buffer;
        buffer << inputFile.rdbuf();
        QString fileContents = QString::fromStdString(buffer.str());
        ui->tableWidget->hide();
        ui->plainTextEdit_11->show();
        ui->plainTextEdit_11->setPlainText(fileContents);
    }
}

// 保存LEX
void Widget::on_pushButton_SaveProgram_clicked()
{
    QString saveFilePath = QFileDialog::getSaveFileName(this, tr("保存句子文件"), QDir::homePath(), tr("文本文件 (*.txt)"));
    if (!saveFilePath.isEmpty() && !ui->plainTextEdit_11->toPlainText().isEmpty()) {
        QFile outputFile(saveFilePath);
        if (outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream stream(&outputFile);
            stream << ui->plainTextEdit_11->toPlainText();
            outputFile.close();
            QMessageBox::about(this, "提示", "导出成功！");
        }
    }
    else if (ui->plainTextEdit_11->toPlainText().isEmpty())
    {
        QMessageBox::warning(this, tr("提示"), tr("输入框为空，请重试！"));
    }
}

// 求 FIRST 集
void Widget::on_pushButton_GetFirstSet_clicked()
{
    init_2();
    QString grammar_q = ui->plainTextEdit_3->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    GrammarAnalyse::handleGrammar();
    AnalyseFIRST::getFirstSets();

    QTableWidget* tableWidget = ui->tableWidget_3;

    // 清空表格内容
    tableWidget->clearContents();

    // 设置表格的列数
    tableWidget->setColumnCount(2);

    // 设置表头
    QStringList headerLabels;
    headerLabels << "非终结符" << "First集合";
    tableWidget->setHorizontalHeaderLabels(headerLabels);

    // 设置行数
    tableWidget->setRowCount(firstSets.size());

    // 遍历非终结符的First集合，将其展示在表格中
    int row = 0;
    for (const auto& entry : firstSets) {
        string nonTerminal = entry.first;
        const set<string>& firstSet = entry.second.s;

        // 在表格中设置非终结符
        QTableWidgetItem* nonTerminalItem = new QTableWidgetItem(QString::fromStdString(nonTerminal));
        tableWidget->setItem(row, 0, nonTerminalItem);

        // 在表格中设置First集合，将set<char>转换为逗号分隔的字符串
        QString firstSetString;
        for (string symbol : firstSet)
            firstSetString += QString::fromStdString(symbol) + ",";
        if (entry.second.isEpsilon)
            firstSetString += QString('#') + ",";

        // 去掉最后一个逗号
        if (!firstSetString.isEmpty())
            firstSetString.chop(1);

        QTableWidgetItem* firstSetItem = new QTableWidgetItem(firstSetString);
        tableWidget->setItem(row, 1, firstSetItem);

        // 增加行数
        ++row;
    }
}

// 求 FOLLOW 集
void Widget::on_pushButton_GetFollowSet_clicked()
{
    init_2();
    QString grammar_q = ui->plainTextEdit_3->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    GrammarAnalyse::handleGrammar();
    AnalyseFIRST::getFirstSets();
    AnalyseFOLLOW::getFollowSets();

    // 清空TableWidget
    ui->tableWidget_4->clear();

    // 设置表格的行数和列数
    int rowCount = followSets.size();
    int columnCount = 2; // 两列
    ui->tableWidget_4->setRowCount(rowCount);
    ui->tableWidget_4->setColumnCount(columnCount);

    // 设置表头
    QStringList headers;
    headers << "非终结符" << "Follow集合";
    ui->tableWidget_4->setHorizontalHeaderLabels(headers);

    // 遍历followSets，将数据填充到TableWidget中
    int row = 0;
    for (const auto& entry : followSets) {
        // 获取非终结符和对应的followUnit
        string nonTerminal = entry.first;
        const followUnit& followSet = entry.second;

        // 在第一列设置非终结符
        QTableWidgetItem* nonTerminalItem = new QTableWidgetItem(QString::fromStdString(nonTerminal));
        ui->tableWidget_4->setItem(row, 0, nonTerminalItem);

        // 在第二列设置followUnit，使用逗号拼接
        QString followSetStr = "";
        for (string c : followSet.s) {
            followSetStr += QString::fromStdString(c);
            followSetStr += ",";
        }
        followSetStr.chop(1); // 移除最后一个逗号
        QTableWidgetItem* followSetItem = new QTableWidgetItem(followSetStr);
        ui->tableWidget_4->setItem(row, 1, followSetItem);

        // 移动到下一行
        ++row;
    }
}

// 分析结果说明
void Widget::on_pushButton_Help_2_clicked()
{
    QString message = R"(
    点击上方的6个按钮，即可进行不同的文法分析
    在下方即可查看对应的分析结果)";

    QMessageBox::information(this, "分析结果说明", message);
}

// LR(0)DFA 生成
void Widget::on_pushButton_GenLR0DFA_clicked()
{
    init_2();
    ui->tableWidget_2->clear();
    QString grammar_q = ui->plainTextEdit_3->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    GrammarAnalyse::handleGrammar();
    ui->plainTextEdit_4->setPlainText(LR0Result);
    GenerateLR0DFA::getLR0();

    int numRows = dfaStateVector.size();
    int numCols = 2 + VT.size() + VN.size();

    ui->tableWidget_2->setRowCount(numRows);
    ui->tableWidget_2->setColumnCount(numCols);

    // Set the table headers
    QStringList headers;
    headers << "状态" << "状态内文法";
    map<string, int> c2int;
    int cnt = 0;
    for (string vt : VT) {
        headers << QString::fromStdString(vt);
        c2int[vt] = cnt++;
    }
    for (string vn : VN) {
        headers << QString::fromStdString(vn);
        c2int[vn] = cnt++;
    }
    ui->tableWidget_2->setHorizontalHeaderLabels(headers);

    // Populate the table with data
    for (int i = 0; i < numRows; ++i)
    {
        ui->tableWidget_2->setItem(i, 0, new QTableWidgetItem(QString::number(dfaStateVector[i].sid)));
        ui->tableWidget_2->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(GenerateLR0DFA::getStateGrammar(dfaStateVector[i]))));

        // Display nextStateVector
        for (size_t j = 0; j < dfaStateVector[i].nextStateVector.size(); ++j)
        {
            ui->tableWidget_2->setItem(i, 2 + c2int[dfaStateVector[i].nextStateVector[j].c], new QTableWidgetItem(QString::number(dfaStateVector[i].nextStateVector[j].sid)));
        }
    }
}

// SLR(1) 文法分析
void Widget::on_pushButton_SLR1Analyse_clicked()
{
    QString grammar_q = ui->plainTextEdit_3->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }
    GrammarAnalyse::handleGrammar();
    AnalyseFIRST::getFirstSets();
    AnalyseFOLLOW::getFollowSets();
    GenerateLR0DFA::getLR0();
    QString result;
    int analysisResult = SLR1Analyse::SLR1_Analyse();
    switch (analysisResult)
    {
    case 0:
        result = "该文法是SLR(1)文法\n";
        result += "说明：没有发现移进-规约冲突和规约-规约冲突";
        break;
    case 1:
        result = "该文法不是SLR(1)文法\n";
        result += "原因：存在移进-规约冲突\n";
        result += "在部分DFA状态中，同一个终结符既可以移进也可以规约";
        break;
    case 2:
        result = "该文法不是SLR(1)文法\n";
        result += "原因：存在规约-规约冲突\n";
        result += "在部分DFA状态中，存在多个可用的规约项目，且它们的Follow集合有交集";
        break;
    case 3:
        result = "该文法不是SLR(1)文法\n";
        result += "原因：同时存在移进-规约冲突和规约-规约冲突";
        break;
    default:
        result = "分析过程中出现未知错误";
        break;
    }
    ui->plainTextEdit_4->setPlainText(result);
}

// LR(1)DFA 生成
void Widget::on_pushButton_GenLR1DFA_clicked()
{
    init_2();
    ui->tableWidget_2->clear();
    QString grammar_q = ui->plainTextEdit_3->toPlainText();
    grammarStr = grammar_q.toStdString();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }

    // 解析文法，计算 FIRST（LR(1) 需要 FIRST）
    GrammarAnalyse::handleGrammar();
    AnalyseFIRST::getFirstSets();

    // 生成 LR(1)
    GenerateLR1DFA::getLR1();

    // 构建表格，与 LR(0) 类似：列 = "状态", "状态内文法" + VT + VN
    int numRows = lr1States.size();
    int numCols = 2 + lr1VT.size() + lr1VN.size();

    ui->tableWidget_2->setRowCount(numRows);
    ui->tableWidget_2->setColumnCount(numCols);

    QStringList headers;
    headers << "状态" << "状态内文法";
    map<string, int> c2int;
    int cnt = 0;
    for (const string& vt : lr1VT) {
        headers << QString::fromStdString(vt);
        c2int[vt] = cnt ++;
    }
    for (const string &vn : lr1VN) {
        headers << QString::fromStdString(vn);
        c2int[vn] = cnt++;
    }
    ui->tableWidget_2->setHorizontalHeaderLabels(headers);

    // 填充表格
    for (int i = 0; i < numRows; ++i) {
        ui->tableWidget_2->setItem(i, 0, new QTableWidgetItem(QString::number(lr1States[i].sid)));
        ui->tableWidget_2->setItem(i, 1, new QTableWidgetItem(QString::fromStdString(GenerateLR1DFA::getLR1StateGrammar(lr1States[i]))));
        for (size_t j = 0; j < lr1States[i].nextStateVector.size(); ++j) {
            string sym = lr1States[i].nextStateVector[j].c;
            int sid = lr1States[i].nextStateVector[j].sid;
            auto it = c2int.find(sym);
            if (it != c2int.end())
                ui->tableWidget_2->setItem(i, 2 + it->second, new QTableWidgetItem(QString::number(sid)));
        }
    }
}

// LR(1) 分析表生成
void Widget::on_pushButton_LR1Analyse_clicked()
{
    // 清理表格
    ui->tableWidget_2->clear();
    ui->tableWidget_2->setRowCount(0);
    ui->tableWidget_2->setColumnCount(0);

    // 读取文法文本并检查
    QString grammar_q = ui->plainTextEdit_3->toPlainText();
    if (grammar_q.isEmpty()) {
        QMessageBox::critical(this, "错误信息", "请先输入文法");
        return;
    }

    // 解析文法并准备 FIRST/LR(1)
    grammarStr = grammar_q.toStdString();
    init_2();                               // 清理并准备
    GrammarAnalyse::handleGrammar();        // 填充 bigAlpha / smallAlpha / grammarDeque / grammarMap / grammarToInt
    AnalyseFIRST::getFirstSets();
    GenerateLR1DFA::getLR1();

    LR1Analyse::Analyse();

    // 设置表格列数：状态列 + 所有终结符 + 所有非终结符
    int numCols = 1 + allTerminals.size() + allNonTerminals.size();
    ui->tableWidget_2->setColumnCount(numCols);

    // 设置表头
    QStringList headers;
    headers << "状态";

    // 终结符列
    for (const string &term : allTerminals) {
        headers << QString::fromStdString(term);
    }

    // 非终结符列
    for (const string &nonterm : allNonTerminals) {
        headers << QString::fromStdString(nonterm);
    }

    ui->tableWidget_2->setHorizontalHeaderLabels(headers);

    // 设置行数：状态数量
    int numRows = lr1States.size();
    ui->tableWidget_2->setRowCount(numRows);

    // 创建符号到列索引的映射
    map<string, int> symbolToColumn;
    int colIndex = 1;
    for (const string &term : allTerminals) {
        symbolToColumn[term] = colIndex++;
    }
    for (const string &nonterm : allNonTerminals) {
        symbolToColumn[nonterm] = colIndex++;
    }

    // 填充表格
    for (int i = 0; i < numRows; ++i) {
        // 状态列
        ui->tableWidget_2->setItem(i, 0, new QTableWidgetItem(QString::number(i)));

        // 填充ACTION表（终结符部分）
        for (const string &term : allTerminals) {
            auto it = ACTION.find(make_pair(i, term));
            if (it != ACTION.end()) {
                int col = symbolToColumn[term];
                ui->tableWidget_2->setItem(i, col, new QTableWidgetItem(QString::fromStdString(it->second)));
            }
        }

        // 填充GOTO表（非终结符部分）
        for (const string &nonterm : allNonTerminals) {
            auto it = GOTO.find(make_pair(i, nonterm));
            if (it != GOTO.end()) {
                int col = symbolToColumn[nonterm];
                ui->tableWidget_2->setItem(i, col, new QTableWidgetItem(QString::number(it->second)));
            }
        }
    }

    // 调整列宽
    ui->tableWidget_2->resizeColumnsToContents();

    QMessageBox::information(this, "提示", "LR(1)分析表生成完成！");
}

// LEX 分析表生成
void Widget::on_pushButton_AnalyseSentence_clicked()
{
    // 检查是否有输入的文法
    QString grammar_q = ui->plainTextEdit_3->toPlainText();
    if (grammar_q.trimmed().isEmpty()) {
        QMessageBox::critical(this, "错误", "请先输入文法");
        return;
    }

    // 检查是否有LEX内容
    QString lexContent = ui->plainTextEdit_11->toPlainText();
    if (lexContent.trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请在“句子输入”处填写LEX文件内容");
        return;
    }

    // 显示分析进度
    cerr << "正在分析LEX文件...";

    // 直接调用静态方法执行LEX文件语法分析
    bool success = CodeAnalyse::analyseLEXContent(lexContent, grammar_q, ui->tableWidget_2);

    // 显示分析结果
    if (success) {
        // 统计分析步骤
        int totalSteps = ui->tableWidget_2->rowCount();

        // 添加总结行
        ui->tableWidget_2->insertRow(totalSteps);
        ui->tableWidget_2->setItem(totalSteps, 0, new QTableWidgetItem("✓"));
        ui->tableWidget_2->setItem(totalSteps, 4, new QTableWidgetItem("分析成功完成"));

        // 合并单元格使总结更美观
        ui->tableWidget_2->setSpan(totalSteps, 0, 1, 4);

        // 设置总结行样式
        for (int col = 0; col < ui->tableWidget_2->columnCount(); col++) {
            QTableWidgetItem* item = ui->tableWidget_2->item(totalSteps, col);
            if (item) {
                item->setBackground(QBrush(QColor(240, 255, 240))); // 浅绿色背景
                item->setForeground(QBrush(QColor(0, 128, 0)));     // 深绿色文字
                item->setFont(QFont("Arial", 10, QFont::Bold));
            }
        }

        // 显示成功信息
        QString message = QString("LEX分析成功完成，共 %1 步").arg(totalSteps);
        QMessageBox::about(this, "分析完成", message);

        // 滚动到顶部
        ui->tableWidget_2->scrollToTop();

    } else {
        // 添加错误总结行
        int totalSteps = ui->tableWidget_2->rowCount();
        ui->tableWidget_2->insertRow(totalSteps);
        ui->tableWidget_2->setItem(totalSteps, 0, new QTableWidgetItem("✗"));
        ui->tableWidget_2->setItem(totalSteps, 4, new QTableWidgetItem("分析失败"));

        // 合并单元格
        ui->tableWidget_2->setSpan(totalSteps, 0, 1, 4);

        // 设置错误行样式
        for (int col = 0; col < ui->tableWidget_2->columnCount(); col++) {
            QTableWidgetItem* item = ui->tableWidget_2->item(totalSteps, col);
            if (item) {
                item->setBackground(QBrush(QColor(255, 240, 240))); // 浅红色背景
                item->setForeground(QBrush(QColor(255, 0, 0)));     // 红色文字
                item->setFont(QFont("Arial", 10, QFont::Bold));
            }
        }

        QMessageBox::warning(this, "分析失败", "LEX文件分析过程中出现错误");
    }

    // 调整列宽
    ui->tableWidget_2->resizeColumnsToContents();
}

// 中间代码生成
void Widget::on_pushButton_Interlanguage_clicked()
{
    ui->codeText->clear();

    if (syntaxTreeRoot == nullptr) {
        QMessageBox::warning(this, "提示", "请先进行语法分析生成语法树");
        return;
    }

    // 重置中间代码模块
    GenerateMidCode::reset();

    try {
        // 从语法树根开始生成中间代码
        GenerateMidCode::generateFromSyntaxTree(syntaxTreeRoot);

        const vector<IntermediateCode>& codes =
            GenerateMidCode::getIntermediateCodes();

        if (codes.empty()) {
            ui->codeText->setPlainText("未生成中间代码");
            return;
        }

        QString text;
        text += "========== 中间代码 ==========\n";

        for (size_t i = 0; i < codes.size(); ++i) {
            text += QString("%1: %2\n")
                        .arg((int)i, 3)
                        .arg(QString::fromStdString(codes[i].toString()));
        }

        text += "==============================";
        ui->codeText->setPlainText(text);

        // 输出文件
        GenerateMidCode::outputToFile("intermediate_code.txt");

        QMessageBox::information(
            this,
            "成功",
            QString("成功生成 %1 条中间代码").arg((int)codes.size())
        );

    } catch (const exception& e) {
        ui->codeText->setPlainText(e.what());
        QMessageBox::critical(this, "错误", e.what());
    }
}

// 辅助函数：统计语法树节点数量
int countTreeNodes(const TreeNode* root)
{
    if (!root) return 0;

    int count = 1;
    for (const TreeNode* child : root->children) {
        count += countTreeNodes(child);
    }
    return count;
}

// 可视化显示语法树
void Widget::on_pushButton_ShowSyntaxTree_clicked()
{
    // 清理 treeWidget
    ui->treeWidget->clear();

    // 检查是否有LEX内容
    QString lexContent = ui->plainTextEdit_11->toPlainText();
    if (lexContent.trimmed().isEmpty()) {
        QMessageBox::warning(this, "提示", "请在“句子输入”处填写LEX文件内容");
        return;
    }

    // 语法树必须已经在语法分析阶段构建完成
    if (parseTreeRoot == nullptr) {
        QMessageBox::information(
            this,
            "提示",
            "尚未生成语法树，请先完成语法分析"
        );
        return;
    }

    // 设置 treeWidget 表头
    ui->treeWidget->setColumnCount(2);
    ui->treeWidget->setHeaderLabels(QStringList() << "语法符号" << "值");

    // 创建语法树显示项
    QTreeWidgetItem* rootItem =
        GenerateSyntaxTree::createTreeWidgetItem(syntaxTreeRoot);

    if (!rootItem) {
        QMessageBox::warning(this, "错误", "语法树显示项创建失败");
        return;
    }

    // 显示
    ui->treeWidget->addTopLevelItem(rootItem);
    ui->treeWidget->expandAll();
    ui->treeWidget->resizeColumnToContents(0);
    ui->treeWidget->resizeColumnToContents(1);

    // 显示统计信息
    int nodeCount = countTreeNodes(syntaxTreeRoot);
    QString info = QString("语法树构建完成，共 %1 个节点").arg(nodeCount);
    QMessageBox::about(this, "成功", info);
}

// 转到任务二界面
void Widget::on_commandLinkButton_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_2);
}

// 转到任务一界面
void Widget::on_commandLinkButton_2_clicked()
{
    ui->stackedWidget->setCurrentWidget(ui->page_1);
}
