#include "cut.h"


using abc::Abc_Ntk_t;
using boost::dynamic_bitset;
using std::bitset;
using std::cout;
using std::endl;
using std::string;
using std::thread;
using std::unordered_map;
using std::vector;


void CutMan::FindDisjCut() {
    clock_t start = clock();
    // init
    // reset data
    cutNtks.resize(net.GetIdMaxPlus1());
    disjCuts.resize(net.GetIdMaxPlus1());
    poMarks.resize(net.GetIdMaxPlus1(), dynamic_bitset<ull>(net.GetPoNum(), 0));
    // topological sort and update PO marks
    auto topoNodes = net.CalcTopoOrd();
    UpdTopoIds(topoNodes);
    UpdPoMarks(topoNodes);
    cout<< "initial time = "<< clock() - start<< endl;
    // collect disjoint cuts and the corresponding cut networks
    int travTime = 0, recTime = 0;
    int _size = 0;
    for (const auto & pTarg: topoNodes) {
        int i = net.GetId(pTarg);
        assert(net.IsNode(pTarg));
        Abc_NtkIncrementTravId(net.GetNet());
        start = clock();
        FindDisjCutOfNode(pTarg, disjCuts[i]);
        travTime += (clock() - start);
        start = clock();
        for (const auto & node: topoNodes) {
            if (Abc_NodeIsTravIdCurrent(node))
                cutNtks[i].emplace_back(node);
        }
        for (int j = 0; j < net.GetPoNum(); ++j) {
            auto pPo = net.GetPo(j);
            if (Abc_NodeIsTravIdCurrent(pPo))
                cutNtks[i].emplace_back(pPo);
        }
        recTime += (clock() - start);
        // cout<< pTarg<< ":"<< endl;
        // PrintVect(vector<AbcObj*>(disjCuts[i].begin(), disjCuts[i].end()), "\n");
        // PrintVect(cutNtks[i], "\n");
        _size += cutNtks[i].size();
    }
    cout<< "traverse time = "<< travTime<< endl;
    cout<< "record time = "<< recTime<< endl;
    cout<< static_cast<double>(_size) / topoNodes.size()<< endl;
}


void CutMan::FindDisjCutOfNode(AbcObj* pObj, ObjList& disjCut) {
    disjCut.clear();
    ExpandCut(pObj, disjCut);
    AbcObj* pObjExpd = nullptr;
    while ((pObjExpd = ExpandWhich(disjCut)) != nullptr)
        ExpandCut(pObjExpd, disjCut);
}


void CutMan::ExpandCut(AbcObj* pObj, ObjList& disjCut) {
    AbcObj* pFanout = nullptr;
    int i = 0;
    Abc_ObjForEachFanout(pObj, pFanout, i) {
        if (poMarks[pFanout->Id].none())
            continue;
        if (!abc::Abc_NodeIsTravIdCurrent(pFanout)) {
            if (abc::Abc_ObjFanoutNum(pFanout) || abc::Abc_ObjIsPo(pFanout)) {
                abc::Abc_NodeSetTravIdCurrent(pFanout);
                disjCut.emplace_back(pFanout);
            }
        }
    } 
}


AbcObj* CutMan::ExpandWhich(ObjList& disjCut) {
    for (auto ppAbcObj1 = disjCut.begin(); ppAbcObj1 != disjCut.end(); ++ppAbcObj1) {
        auto ppAbcObj2 = ppAbcObj1;
        for (++ppAbcObj2; ppAbcObj2 != disjCut.end(); ++ppAbcObj2) {
            #ifdef DEBUG
            assert(poMarks[(*ppAbcObj1)->Id].size() == poMarks[(*ppAbcObj2)->Id].size());
            assert((*ppAbcObj1)->Id != (*ppAbcObj2)->Id);
            if (topoIds[(*ppAbcObj1)->Id] == topoIds[(*ppAbcObj2)->Id]) {
                cout<< topoIds[(*ppAbcObj1)->Id]<< ","<< topoIds[(*ppAbcObj2)->Id]<< ","<<*ppAbcObj1<< ","<<*ppAbcObj2<< endl;
                assert(0);
            }
            #endif
            auto isJoint = poMarks[(*ppAbcObj1)->Id] & poMarks[(*ppAbcObj2)->Id];
            if (isJoint.any()) {
                AbcObj* pRet = nullptr;
                if (topoIds[(*ppAbcObj1)->Id]< topoIds[(*ppAbcObj2)->Id]) {
                    pRet = *ppAbcObj1;
                    disjCut.erase(ppAbcObj1);
                }
                else {
                    pRet = *ppAbcObj2;
                    disjCut.erase(ppAbcObj2);
                }
                return pRet;
            }
        }
    }
    return nullptr;
}


void CutMan::UpdTopoIds(ObjVec& topoNodes) {
    topoIds.resize(net.GetIdMaxPlus1());
    for (int i = 0; i < topoNodes.size(); ++i) {
        topoIds[topoNodes[i]->Id] = i;
        // cout << topoNodes[i] << ">>>>>>" << i << endl;
    }
    int topoId = -1;
    for (int i = 0; i < net.GetPiNum(); ++i)
        topoIds[net.GetPiId(i)] = topoId--;
    topoId = topoNodes.size();
    for (int i = 0; i < net.GetPoNum(); ++i)
        topoIds[net.GetPoId(i)] = topoId++;
}


void CutMan::UpdPoMarks(ObjVec& topoNodes) {
    for (int i = 0; i < net.GetPoNum(); ++i)
        poMarks[net.GetPoId(i)].set(i);
    for (auto it = topoNodes.rbegin(); it != topoNodes.rend(); ++it) {
        auto pObj = *it;
        if (pObj == nullptr)
            continue;
        int i = net.GetId(pObj);
        for (int j = 0; j < net.GetFanoutNum(pObj); ++j)
            poMarks[i] |= poMarks[net.GetFanoutId(pObj, j)];
    }
}


void CutMan::BuildOneCutNtks() {
    // reset data
    flows.resize(net.GetIdMaxPlus1(), 0.0);
    oneCuts.resize(net.GetIdMaxPlus1());
    cutNtks.resize(net.GetIdMaxPlus1());
    poMarks.resize(net.GetIdMaxPlus1(), dynamic_bitset<ull>(net.GetPoNum(), 0));
    // topological sort and update PO marks
    auto topoNodes = net.CalcTopoOrd();
    UpdTopoIds(topoNodes);
    // collect one cuts and cut networks
    int cutNtkSize = 0, count = 0;
    for (auto pNode: topoNodes) {
        if (Abc_NodeIsConst(pNode))
            continue;
        if (net.GetName(pNode).find("_app") != -1)
            continue;
        ++count;
        FindOneCut(pNode);
    //     SortCutNtkNodes(pObj, cutNtkNodes);
        cutNtkSize += cutNtks[pNode->Id].size();
        cout << pNode << "->" << oneCuts[pNode->Id] << endl;
    }
    cout<< "average cut network size " << cutNtkSize / static_cast<double>(count) << endl;
}


void CutMan::FindOneCut(AbcObj* pPivot) {
    // init
    assert(net.GetPoNum() == 1);
    std::fill(flows.begin(), flows.end(), 0);
    oneCuts[pPivot->Id] = nullptr;
    // bfs
    auto comp = [this](const AbcObj* pA, const AbcObj* pB) {return topoIds[pA->Id] > topoIds[pB->Id];};
    std::priority_queue<AbcObj*, vector<AbcObj*>, decltype(comp)> q(comp);
    q.push(pPivot);
    flows[pPivot->Id] = 1.0;
    // cout << pPivot << "-------" << endl;
    do {
        auto pObj = q.top();
        q.pop();
        // cout << "pop " << pObj << endl;
        int nFanout = net.GetFanoutNum(pObj);
        double subFlow = flows[pObj->Id] / nFanout;
        flows[pObj->Id] = 0.0;
        bool fFound = false;
        for (int i = 0; i < nFanout; ++i) {
            auto pFanout = net.GetFanout(pObj, i);
            // update flow
            flows[pFanout->Id] += subFlow;
            if (DoubleEqual(flows[pFanout->Id], 1.0)) {
                oneCuts[pPivot->Id] = pFanout;
                fFound = true;
                break;
            }
            // update queue
            bool fAdd = true;
            for (int j = 0; j < net.GetFaninNum(pFanout); ++j) {
                if (flows[net.GetFaninId(pFanout, j)] != 0.0) {
                    fAdd = false;
                    break;
                }
            }
            if (fAdd) {
                q.push(pFanout);
                cutNtks[pPivot->Id].emplace_back(pFanout);
            }
        }
        if (fFound) break;
    } while (!q.empty());
    assert(oneCuts[pPivot->Id] != nullptr);
}