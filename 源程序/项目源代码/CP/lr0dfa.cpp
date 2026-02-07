/************************************************************************
 * @Copyright (c) 2025 LDL. All rights reserved.
 *
 * @FileName: lr0dfa.cpp
 * @Brief: LR(0)DFA构建类的实现部分，用于根据文法构建对应的表格形式的LR(0)DFA
 * @Module: LR(0)DFA构建模块
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
#include "lr0dfa.h"
#include "grammar.h"

int scnt = 0;
int ccnt = 0;
vector<dfaCell> dfaCellVector;
vector<dfaState> dfaStateVector;
set<string> VN;
set<string> VT;
set<int> visitedStates;

GenerateLR0DFA::GenerateLR0DFA()
{

}

// 创建 LR(0) 初始状态
void GenerateLR0DFA::createFirstState() {
    // 由于增广，故一定只会有一个入口
    dfaState zero = dfaState();
    // 给一个id
    zero.sid = scnt++;
    // 放入到 vector 数组中
    dfaStateVector.push_back(zero);

    // 添加初始的LR0项，即E' -> .S
    dfaCell startCell;
    // 此处设增广文法的编号为0
    startCell.gid = 0;
    startCell.index = 0;
    startCell.cellid = ccnt++;

    dfaCellVector.push_back(startCell);

    // 把初始 LR(0) 项放入初始状态
    dfaStateVector[0].cellV.push_back(startCell.cellid);
    dfaStateVector[0].originV.push_back(startCell.cellid);
}

// 递归设生成 LR(0) 状态
void GenerateLR0DFA::generateLR0State(int stateId) {
    // 采用 DFS 算法
    if (visitedStates.count(stateId) > 0)
        return ;

    // 若标记已走过
    visitedStates.insert(stateId);

    qDebug() << stateId << endl;

    // 求闭包
    for (size_t i = 0; i < dfaStateVector[stateId].cellV.size(); ++ i) {
        dfaCell& currentCell = dfaCellVector[dfaStateVector[stateId].cellV[i]];

        qDebug() << QString::fromStdString(grammarDeque[currentCell.gid].left) << QString::fromStdString("->") << QString::fromStdString(grammarDeque[currentCell.gid].right) << endl;
        qDebug() << "current index:" << currentCell.index << endl;

        QStringList rightList = QString::fromStdString(grammarDeque[currentCell.gid].right).split(" ");

        // 若点号在产生式末尾或者空串，则跳过（LR0不需要结束）
        if (currentCell.index == rightList.size() || grammarDeque[currentCell.gid].right == "#") {
            dfaStateVector[stateId].isEnd = true;
            continue;
        }

        string nextSymbol = rightList[currentCell.index].toStdString();

        // 如果nextSymbol是非终结符，则将新项添加到状态中
        if (GrammarAnalyse::isBigAlpha(nextSymbol) && dfaStateVector[stateId].right_VNs.find(nextSymbol) == dfaStateVector[stateId].right_VNs.end()) {
            dfaStateVector[stateId].right_VNs.insert(nextSymbol);
            for (auto& grammar : grammarMap[nextSymbol]) {
                // 获取通过nextSymbol转移的新LR0项
                dfaCell nextCell = dfaCell();
                nextCell.gid = grammarToInt[make_pair(nextSymbol, grammar)];
                nextCell.index = 0;
                int nextcellid = isNewCell(nextCell.gid, nextCell.index);
                if (nextcellid == -1) {
                    nextCell.cellid = ccnt ++;
                    dfaCellVector.push_back(nextCell);
                    dfaStateVector[stateId].cellV.push_back(nextCell.cellid);
                }
                else
                    dfaStateVector[stateId].cellV.push_back(nextcellid);
            }
        }
    }

    // 暂存新状态
    map<string, dfaState> tempSave;
    // 生成新状态，但还不能直接存到dfaStateVector中，我们要校验他是否和之前的状态一样
    for (size_t i = 0; i < dfaStateVector[stateId].cellV.size(); ++i) {
        dfaCell& currentCell = dfaCellVector[dfaStateVector[stateId].cellV[i]];
        QStringList rightList = QString::fromStdString(grammarDeque[currentCell.gid].right).split(" ");

        // 如果点号在产生式末尾，则跳过（LR0不需要结束）
        if (currentCell.index == rightList.size() || grammarDeque[currentCell.gid].right == "#")
            continue;

        // 下一个字符
        string nextSymbol = rightList[currentCell.index].toStdString();

        // 创建下一个状态（临时的）
        dfaState& nextState = tempSave[nextSymbol];
        dfaCell nextStateCell = dfaCell();
        nextStateCell.gid = currentCell.gid;
        nextStateCell.index = currentCell.index + 1;

        // 看项目是否有重复的，若重复则取之前的，不重复生成
        int nextStateCellid = isNewCell(nextStateCell.gid, nextStateCell.index);
        if (nextStateCellid == -1) {
            nextStateCell.cellid = ccnt ++;
            dfaCellVector.push_back(nextStateCell);
        }
        else
            nextStateCell.cellid = nextStateCellid;
        nextState.cellV.push_back(nextStateCell.cellid);
        nextState.originV.push_back(nextStateCell.cellid);

        // 收集后方便后续的通过表格形式构建 LR(0)DFA
        if (GrammarAnalyse::isBigAlpha(nextSymbol))
            VN.insert(nextSymbol);
        else if (GrammarAnalyse::isSmallAlpha(nextSymbol))
            VT.insert(nextSymbol);
    }

    // 校验是否存在重复状态
    for (auto& t : tempSave) {
        dfaState nextState = dfaState();
        int newStateId = isNewState(t.second.originV);

        // 不重复就新开一个状态
        if (newStateId == -1) {
            nextState.sid = scnt++;
            nextState.cellV = t.second.cellV;
            nextState.originV = t.second.originV;
            dfaStateVector.push_back(nextState);
        }
        else
            nextState.sid = newStateId;

        // 存入现在这个状态的 nextStateVector
        nextStateUnit n = nextStateUnit();
        n.sid = nextState.sid;
        n.c = t.first;
        dfaStateVector[stateId].nextStateVector.push_back(n);
    }

    // 对每个下一状态进行递归
    int nsize = dfaStateVector[stateId].nextStateVector.size();
    for (int i = 0; i < nsize; i++) {
        auto& nextunit = dfaStateVector[stateId].nextStateVector[i];
        generateLR0State(nextunit.sid);
    }
}

// LR(0) 核心生成函数入口
void GenerateLR0DFA::getLR0() {
    // set初始化清空
    visitedStates.clear();

    // 首先生成第一个状态
    createFirstState();

    // 再递归生成其他状态
    generateLR0State(0);

}

// 判断是否为新的项目
int GenerateLR0DFA::isNewCell(int gid, int index) {
    for (const dfaCell& cell : dfaCellVector)
        // 检查 dfaCellVector 中是否存在相同的 gid 和 index 的 dfaCell
        if (cell.gid == gid && cell.index == index)
            // 若不是新项目
            return cell.cellid;
    // 若是新项目
    return -1;
}

// 判断是否为新的状态
int GenerateLR0DFA::isNewState(const vector<int>& cellIds) {
    for (const dfaState& state : dfaStateVector)
        // 检查状态中的 originV 是否相同
        if (state.originV.size() == cellIds.size() && equal(state.originV.begin(), state.originV.end(), cellIds.begin()))
            // 若不是新状态
            return state.sid;
    // 若是新状态
    return -1;
}

// 获取状态内文法的字符串表示
string GenerateLR0DFA::getStateGrammar(const dfaState& d) {
    string result = "";
    for (auto cell : d.cellV) {
        const dfaCell& dfaCell = dfaCellVector[cell];

        // 拿到文法
        int gid = dfaCell.gid;
        grammarUnit g = grammarDeque[gid];

        // 拿到位置
        int index = dfaCell.index;

        // 拼接结果
        string r = "";
        r += g.left == "zengguang" ? "E\'->" : g.left + "->";
        string right = g.right == "#" ? "" : g.right;
        string rightResult = "";
        QStringList rightList = QString::fromStdString(right).split(" ");
        for (int i = 0; i <= rightList.size(); i ++) {
            if (i == index)
                rightResult += ".";
            if (i == rightList.size())
                break;
            rightResult += rightList[i].toStdString();
        }
        r += rightResult;
        result += r + " ";
    }
    return result;
}
