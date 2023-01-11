#pragma once


#include "header.h"
#include "my_abc.h"


class CutMan {
private:
    NetMan& net;
    std::vector<ObjList> disjCuts;
    ObjVec oneCuts;
    std::vector<ObjVec> cutNtks;
    std::vector<boost::dynamic_bitset<ull>> poMarks;
    IntVec topoIds;
    DblVec flows;
    
public:
    explicit CutMan(NetMan& _net): net(_net) {}
    ~CutMan() = default;
    CutMan(const CutMan &) = delete;
    CutMan(CutMan &&) = delete;
    CutMan & operator = (const CutMan &) = delete;
    CutMan & operator = (CutMan &&)  = delete;
    void FindDisjCut();
    void FindDisjCutOfNode(AbcObj* pObj, ObjList& disjCut);
    void ExpandCut(AbcObj* pObj, ObjList& disjCut);
    AbcObj* ExpandWhich(ObjList& disjCut);
    void UpdTopoIds(ObjVec& topoNodes);
    void UpdPoMarks(ObjVec& topoNodes);
    void BuildOneCutNtks();
    void FindOneCut(AbcObj* pPivot);
};