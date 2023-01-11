#include "simulator.h"


using namespace abc;
using namespace std;
using namespace boost;
using namespace random;


Simulator::Simulator(NetMan & net_man, unsigned _seed, int n_frame): NetMan(net_man.GetNet(), false), seed(_seed), nFrame(n_frame) {
    #ifdef DEBUG
    auto type = NetMan::GetNetType();
    assert(type == NET_TYPE::AIG || type == NET_TYPE::GATE || type == NET_TYPE::SOP || type == NET_TYPE::STRASH);
    #endif
    dat.resize(NetMan::GetIdMaxPlus1(), dynamic_bitset <ull> (nFrame, 0));
}


void Simulator::InpUnif() {
    uniform_int <> unif01(0, 1);
    random::mt19937 eng(seed);
    variate_generator < random::mt19937, uniform_int <> > rand01(eng, unif01);

    for (int i = 0; i < NetMan::GetPiNum(); ++i) {
        auto piId = NetMan::GetPiId(i);
        dat[piId].reset();
        for (int j = 0; j < nFrame; ++j) {
            if (rand01())
                dat[piId].set(j);
        }
    }
    
    for (int i = 0; i < NetMan::GetIdMaxPlus1(); ++i) {
        if (NetMan::IsConst0(i))
            dat[i].reset();
        else if (NetMan::IsConst1(i))
            dat[i].set();
    }
}


void Simulator::InpUnifFast() {
    random::uniform_int_distribution <ull> unifUll;
    random::mt19937 eng(seed);
    variate_generator <random::mt19937, random::uniform_int_distribution <ull> > randUll(eng, unifUll);
    const int unitLength = 64;
    assert((nFrame & (unitLength - 1)) == 0);
    int nUnit = nFrame / unitLength;

    for (int i = 0; i < NetMan::GetPiNum(); ++i) {
        auto piId = NetMan::GetPiId(i);
        dat[piId].resize(0);
        for (int j = 0; j < nUnit; ++j) {
            ull numb = randUll();
            dat[piId].append(numb);
        }
    }

    auto type = NetMan::GetNetType();
    if (type == NET_TYPE::AIG || type == NET_TYPE::GATE || type == NET_TYPE::SOP) {
        for (int i = 0; i < NetMan::GetIdMaxPlus1(); ++i) {
            if (NetMan::IsConst0(i))
                dat[i].reset();
            else if (NetMan::IsConst1(i))
                dat[i].set();
        }
    }
    else if (type == NET_TYPE::STRASH) {
        auto pConst1 = Abc_AigConst1(NetMan::GetNet());
        dat[pConst1->Id].set();
    }
    else
        assert(0);
}


void Simulator::InpEnum() {
    #ifdef DEBUG
    assert(GetPiNum() < 30);
    assert(1ll << GetPiNum() == nFrame);
    #endif
    for (int i = 0; i < GetPiNum(); ++i) {
        bool phase = 1;
        auto piId = GetPiId(i);
        dat[piId].reset();
        for (int j = 0; j < nFrame; ++j) {
            if (j % (1 << i) == 0)
                phase = !phase;
            if (phase)
                dat[piId].set(j);
        }
    }

    auto type = NetMan::GetNetType();
    if (type == NET_TYPE::AIG || type == NET_TYPE::GATE || type == NET_TYPE::SOP) {
        for (int i = 0; i < NetMan::GetIdMaxPlus1(); ++i) {
            if (NetMan::IsConst0(i))
                dat[i].reset();
            else if (NetMan::IsConst1(i))
                dat[i].set();
        }
    }
    else if (type == NET_TYPE::STRASH) {
        auto pConst1 = Abc_AigConst1(NetMan::GetNet());
        dat[pConst1->Id].set();
    }
    else
        assert(0);
}


void Simulator::Sim() {
    auto type = GetNetType();
    auto nodes = CalcTopoOrd();
    for (const auto & pObj: nodes) {
        if (type == NET_TYPE::AIG)
            UpdAigNode(pObj);
        else if (type == NET_TYPE::SOP)
            UpdSopNode(pObj);
        else if (type == NET_TYPE::GATE)
            UpdGateNode(pObj);
        else if (type == NET_TYPE::STRASH)
            UpdStrashNode(pObj);
        else
            assert(0);
    }
    for (int i = 0; i < GetPoNum(); ++i) {
        auto pPo = GetPo(i);
        auto drivId = GetFaninId(pPo, 0);
        #ifdef DEBUG
        assert(!Abc_ObjIsComplement(pPo));
        #endif
        if (type == NET_TYPE::AIG || type == NET_TYPE::GATE || type == NET_TYPE::SOP)
            dat[GetId(pPo)] = dat[drivId];
        else if (type == NET_TYPE::STRASH)
            dat[GetId(pPo)] = Abc_ObjFaninC0(pPo)? ~dat[drivId]: dat[drivId];
        else
            assert(0);
    }
}


void Simulator::UpdAigNode(Abc_Obj_t * pObj) {
    #ifdef DEBUG
    assert(Abc_ObjIsNode(pObj));
    #endif
    auto pNtk = NetMan::GetNet();
    auto pMan = static_cast <Hop_Man_t *> (pNtk->pManFunc);
    auto pRoot = static_cast <Hop_Obj_t *> (pObj->pData);
    auto pRootR = Hop_Regular(pRoot);

    // skip constant node
    if (Hop_ObjIsConst1(pRootR))
        return;

    // get topological order of subnetwork in aig
    Vec_Ptr_t * vHopNodes = Hop_ManDfsNode(pMan, pRootR);

    // init internal hop nodes
    int maxHopId = -1;
    int i = 0;
    Hop_Obj_t * pHopObj = nullptr;
    Vec_PtrForEachEntry(Hop_Obj_t *, vHopNodes, pHopObj, i)
        maxHopId = max(maxHopId, pHopObj->Id);
    Vec_PtrForEachEntry( Hop_Obj_t *, pMan->vPis, pHopObj, i )
        maxHopId = max(maxHopId, pHopObj->Id);
    vector < dynamic_bitset <ull> > interData(maxHopId + 1, dynamic_bitset <ull> (nFrame, 0));
    unordered_map <int, dynamic_bitset <ull> *> hop2Data;
    Abc_Obj_t * pFanin = nullptr;
    Abc_ObjForEachFanin(pObj, pFanin, i)
        hop2Data[Hop_ManPi(pMan, i)->Id] = &dat[pFanin->Id];

    // special case for inverter or buffer
    if (pRootR->Type == AIG_PI) {
        pFanin = Abc_ObjFanin0(pObj);
        dat[pObj->Id] = dat[pFanin->Id];
    }

    // simulate
    Vec_PtrForEachEntry(Hop_Obj_t *, vHopNodes, pHopObj, i) {
        assert(Hop_ObjIsAnd(pHopObj));
        auto pHopFanin0 = Hop_ObjFanin0(pHopObj);
        auto pHopFanin1 = Hop_ObjFanin1(pHopObj);
        #ifdef DEBUG
        assert(!Hop_ObjIsConst1(pHopFanin0));
        assert(!Hop_ObjIsConst1(pHopFanin1));
        #endif
        dynamic_bitset <ull> & data0 = Hop_ObjIsPi(pHopFanin0) ? *hop2Data[pHopFanin0->Id] : interData[pHopFanin0->Id];
        dynamic_bitset <ull> & data1 = Hop_ObjIsPi(pHopFanin1) ? *hop2Data[pHopFanin1->Id] : interData[pHopFanin1->Id];
        dynamic_bitset <ull> & out = (pHopObj == pRootR) ? dat[pObj->Id] : interData[pHopObj->Id];
        bool isFanin0C = Hop_ObjFaninC0(pHopObj);
        bool isFanin1C = Hop_ObjFaninC1(pHopObj);
        if (!isFanin0C && !isFanin1C)
            out = data0 & data1;
        else if (!isFanin0C && isFanin1C)
            out = data0 & ~data1;
        else if (isFanin0C && !isFanin1C)
            out = ~data0 & data1;
        else if (isFanin0C && isFanin1C)
            out = ~(data0 | data1);
    }

    // complement
    if (Hop_IsComplement(pRoot))
        dat[pObj->Id].flip();

    // recycle memory
    Vec_PtrFree(vHopNodes); 
}


void Simulator::UpdSopNode(Abc_Obj_t * pObj) {
    #ifdef DEBUG
    assert(Abc_ObjIsNode(pObj));
    #endif
    // skip constant node
    if (Abc_NodeIsConst(pObj))
        return;
    // update sop
    char * pSop = static_cast <char *> (pObj->pData);
    UpdSop(pObj, pSop);
}


void Simulator::UpdGateNode(Abc_Obj_t * pObj) {
    #ifdef DEBUG
    assert(Abc_ObjIsNode(pObj));
    #endif
    // skip constant node
    if (Abc_NodeIsConst(pObj))
        return;
    // update sop
    char * pSop = static_cast <char *> ((static_cast <Mio_Gate_t *> (pObj->pData))->pSop);
    UpdSop(pObj, pSop);
}


void Simulator::UpdStrashNode(Abc_Obj_t * pObj) {
    #ifdef DEBUG
    assert(Abc_ObjIsNode(pObj));
    #endif
    // skip constant node
    auto pConst1 = Abc_AigConst1(NetMan::GetNet());
    if (Abc_ObjRegular(pObj) == pConst1) {
        assert(0);
        return;
    }
    // update node
    assert(!Abc_ObjIsComplement(pObj));
    auto pFanin0 = Abc_ObjFanin0(pObj), pFanin1 = Abc_ObjFanin1(pObj);
    int compl0 = Abc_ObjFaninC0(pObj), compl1 = Abc_ObjFaninC1(pObj);
    dynamic_bitset<ull> c0(nFrame, 0); if (compl0) c0.set();
    dynamic_bitset<ull> c1(nFrame, 0); if (compl1) c1.set();
    dat[pObj->Id] = (dat[pFanin0->Id] ^ c0) & (dat[pFanin1->Id] ^ c1);
}


void Simulator::UpdSop(Abc_Obj_t * pObj, char * pSop) {
    int nVars = Abc_SopGetVarNum(pSop);
    dynamic_bitset <ull> product(nFrame, 0);
    for (char * pCube = pSop; *pCube; pCube += nVars + 3) {
        bool isFirst = true;
        for (int i = 0; pCube[i] != ' '; i++) {
            Abc_Obj_t * pFanin = Abc_ObjFanin(pObj, i);
            switch (pCube[i]) {
                case '-':
                    continue;
                    break;
                case '0':
                    if (isFirst) {
                        isFirst = false;
                        product = ~dat[pFanin->Id];
                    }
                    else
                        product &= ~dat[pFanin->Id];
                    break;
                case '1':
                    if (isFirst) {
                        isFirst = false;
                        product = dat[pFanin->Id];
                    }
                    else
                        product &= dat[pFanin->Id];
                    break;
                default:
                    assert(0);
            }
        }
        if (isFirst) {
            isFirst = false;
            product.set();
        }
        #ifdef DEBUG
        assert(!isFirst);
        #endif
        if (pCube == pSop)
            dat[pObj->Id] = product;
        else
            dat[pObj->Id] |= product;
    }

    // complement
    if (Abc_SopIsComplement(pSop))
        dat[pObj->Id].flip();
}


BigInt Simulator::GetInput(int iPatt, int lsb, int msb) const {
    #ifdef DEBUG
    assert(lsb >= 0 && msb < NetMan::GetPiNum());
    assert(iPatt < nFrame);
    assert(lsb <= msb && msb - lsb < 512);
    #endif
    BigInt ret(0);
    for (int k = msb; k >= lsb; --k) {
        ret <<= 1;
        if (dat[NetMan::GetPiId(k)][iPatt])
            ++ret;
    }
    return ret;
}


void Simulator::PrintInpStream(int iPatt) const {
    #ifdef DEBUG
    assert(iPatt < nFrame);
    #endif
    for (int k = GetPiNum() - 1; k >= 0; --k)
        cout << dat[GetPiId(k)][iPatt];
    cout << endl;
}


BigInt Simulator::GetOutput(int iPatt) const {
    int lsb = 0;
    int msb = NetMan::GetPoNum() - 1;
    #ifdef DEBUG
    assert(iPatt < nFrame);
    assert(msb < 512);
    #endif
    BigInt ret(0);
    for (int k = msb; k >= lsb; --k) {
        ret <<= 1;
        if (dat[NetMan::GetPoId(k)][iPatt])
            ++ret;
    }
    return ret;
}


int Simulator::GetOutputFast(int iPatt) const {
    int lsb = 0;
    int msb = NetMan::GetPoNum() - 1;
    #ifdef DEBUG
    assert(iPatt < nFrame);
    assert(msb < 64);
    #endif
    int ret = 0;
    for (int k = msb; k >= lsb; --k) {
        ret <<= 1;
        if (dat[NetMan::GetPoId(k)][iPatt])
            ++ret;
    }
    return ret;
}


int Simulator::GetTempOutputFast(int iPatt) const {
    int lsb = 0;
    int msb = NetMan::GetPoNum() - 1;
    #ifdef DEBUG
    assert(iPatt < nFrame);
    assert(msb < 64);
    #endif
    int ret = 0;
    for (int k = msb; k >= lsb; --k) {
        ret <<= 1;
        if (tempDat[NetMan::GetPoId(k)][iPatt])
            ++ret;
    }
    return ret;
}


void Simulator::PrintOutpStream(int iPatt) const {
    #ifdef DEBUG
    assert(iPatt < nFrame);
    #endif
    for (int k = GetPoNum() - 1; k >= 0; --k)
        cout << dat[GetPoId(k)][iPatt];
    cout << endl;
}


double Simulator::GetSignalProb(int objId) const {
    #ifdef DEBUG
    assert(objId < NetMan::GetIdMaxPlus1());
    #endif
    return dat[objId].count() / static_cast <double> (nFrame);
}


void Simulator::PrintSignalProb() const {
    for (int i = 0; i < NetMan::GetPoNum(); ++i) {
        cout << NetMan::GetName(NetMan::GetPo(i)) << " " << GetSignalProb(NetMan::GetPoId(i)) << endl;
    }
}


bool Simulator::IsPIOSame(const Simulator & oth_smlt) const {
    if (this->GetPiNum() != oth_smlt.GetPiNum())
        return false;
    for (int i = 0; i < this->GetPiNum(); ++i) {
        // if (strcmp(Abc_ObjName(Abc_NtkPo(pNtk1, i)), Abc_ObjName(Abc_NtkPo(pNtk2, i))) != 0)
        if (this->GetPiName(i) != oth_smlt.GetPiName(i))
            return false;
    }
    if (this->GetPoNum() != oth_smlt.GetPoNum())
        return false;
    for (int i = 0; i < this->GetPoNum(); ++i) {
        // if (strcmp(Abc_ObjName(Abc_NtkPo(pNtk1, i)), Abc_ObjName(Abc_NtkPo(pNtk2, i))) != 0)
        if (this->GetPoName(i) != oth_smlt.GetPoName(i))
            return false;
    }
    return true;
}


double Simulator::GetErrRate(const Simulator & oth_smlt, bool isCheck) const {
    if (isCheck)
        assert(IsPIOSame(oth_smlt));
    dynamic_bitset <ull> temp(nFrame, 0);
    for (int i = 0; i < NetMan::GetPoNum(); ++i)
        temp |= (this->dat[NetMan::GetPoId(i)] ^ oth_smlt.dat[oth_smlt.GetPoId(i)]);
    return temp.count() / static_cast <double> (nFrame);
}


double Simulator::GetMeanErrDist(const Simulator & oth_smlt, bool isCheck) const {
    if (isCheck) {
        assert(IsPIOSame(oth_smlt));
        assert(GetPoNum() < 64);
    }
    const int warnPoint = numeric_limits <int>::max() * 0.8;
    int sed = 0;
    for (int i = 0; i < nFrame; ++i) {
        int accOut = GetOutputFast(i);
        int appOut = oth_smlt.GetOutputFast(i);
        sed += abs(accOut - appOut);
        #ifdef DEBUG
        if (sed >= warnPoint)
            assert(0);
        #endif
    }
    // cout << "sed = " << sed << ", nFrame = " << nFrame << ", med = " << sed / static_cast <double> (nFrame) << endl;
    return sed / static_cast <double> (nFrame);
}


void Simulator::CalcLocBoolDiff(Abc_Obj_t * pObj, list <Abc_Obj_t *> & disjCut, vector <Abc_Obj_t *> & cutNtk, vector < dynamic_bitset <ull> > & bdCut2Node) {
    #ifdef DEBUG
    assert(pObj->pNtk == GetNet());
    #endif
    if (tempDat.size() != dat.size())
        tempDat.resize(dat.size(), dynamic_bitset <ull>(nFrame, 0));
    // flip the node
    tempDat[pObj->Id] = ~dat[pObj->Id];

    // simulate
    Abc_NtkIncrementTravId(GetNet());
    Abc_NodeSetTravIdCurrent(pObj);
    for (auto & pInner: cutNtk)
        Abc_NodeSetTravIdCurrent(pInner);
    auto type = NetMan::GetNetType();
    if (type == NET_TYPE::AIG)
        assert(0);
        // UpdAigNodeForBoolDiff(pObj);
    else if (type == NET_TYPE::SOP) {
        for (auto & pInner: cutNtk)
            UpdSopNodeForBoolDiff(pInner);
    }
    else if (type == NET_TYPE::GATE) {
        for (auto & pInner: cutNtk)
            UpdGateNodeForBoolDiff(pInner);
    }
    else
        assert(0);
    // get boolean difference from the node to its disjoint cuts
    bdCut2Node.resize(disjCut.size(), dynamic_bitset <ull>(nFrame, 0));
    int i = 0;
    for (auto & pCut: disjCut) {
        bdCut2Node[i] = dat[pCut->Id] ^ tempDat[pCut->Id];
        ++i;
    }
}


void Simulator::UpdSopNodeForBoolDiff(Abc_Obj_t * pObj) {
    #ifdef DEBUG
    assert(!Abc_ObjIsPi(pObj));
    assert(!Abc_NodeIsConst(pObj));
    #endif
    if (Abc_ObjIsPo(pObj)) {
        #ifdef DEBUG
        assert(!Abc_ObjIsComplement(pObj));
        #endif
        Abc_Obj_t * pDriver = Abc_ObjFanin0(pObj);
        if (Abc_NodeIsTravIdCurrent(pDriver))
            tempDat[pObj->Id] = tempDat[pDriver->Id];
        else
            tempDat[pObj->Id] = dat[pDriver->Id];
        return;
    }
    // update sop
    char * pSop = static_cast <char *> (pObj->pData);
    UpdSopForBoolDiff(pObj, pSop);
}


void Simulator::UpdGateNodeForBoolDiff(Abc_Obj_t * pObj) {
    #ifdef DEBUG
    assert(!Abc_ObjIsPi(pObj));
    assert(!Abc_NodeIsConst(pObj));
    #endif
    if (Abc_ObjIsPo(pObj)) {
        Abc_Obj_t * pDriver = Abc_ObjFanin0(pObj);
        if (Abc_NodeIsTravIdCurrent(pDriver))
            tempDat[pObj->Id] = tempDat[pDriver->Id];
        else
            tempDat[pObj->Id] = dat[pDriver->Id];
        return;
    }
    // update sop
    char * pSop = static_cast <char *> ((static_cast <Mio_Gate_t *> (pObj->pData))->pSop);
    UpdSopForBoolDiff(pObj, pSop);
}


void Simulator::UpdSopForBoolDiff(Abc_Obj_t * pObj, char * pSop) {
    int nVars = Abc_SopGetVarNum(pSop);
    dynamic_bitset <ull> product(nFrame, 0);
    for (char * pCube = pSop; *pCube; pCube += nVars + 3) {
        bool isFirst = true;
        for (int i = 0; pCube[i] != ' '; i++) {
            Abc_Obj_t * pFanin = Abc_ObjFanin(pObj, i);
            dynamic_bitset <ull> & datFi = Abc_NodeIsTravIdCurrent(pFanin)? tempDat[pFanin->Id]: dat[pFanin->Id];
            switch (pCube[i]) {
                case '-':
                    continue;
                    break;
                case '0':
                    if (isFirst) {
                        isFirst = false;
                        product = ~datFi;
                    }
                    else
                        product &= ~datFi;
                    break;
                case '1':
                    if (isFirst) {
                        isFirst = false;
                        product = datFi;
                    }
                    else
                        product &= datFi;
                    break;
                default:
                    assert(0);
            }
        }
        if (isFirst) {
            isFirst = false;
            product.set();
        }
        #ifdef DEBUG
        assert(!isFirst);
        #endif
        if (pCube == pSop) {
            tempDat[pObj->Id] = product;
        }
        else
            tempDat[pObj->Id] |= product;
    }

    // complement
    if (Abc_SopIsComplement(pSop))
        tempDat[pObj->Id].flip();
}


bool IsPIOSame(Simulator & smlt0, Simulator & smlt1) {
    if (smlt0.GetPiNum() != smlt1.GetPiNum())
        return false;
    for (int i = 0; i < smlt0.GetPiNum(); ++i) {
        // if (strcmp(Abc_ObjName(Abc_NtkPo(pNtk1, i)), Abc_ObjName(Abc_NtkPo(pNtk2, i))) != 0)
        if (smlt0.GetPiName(i) != smlt1.GetPiName(i))
            return false;
    }
    if (smlt0.GetPoNum() != smlt1.GetPoNum())
        return false;
    for (int i = 0; i < smlt0.GetPoNum(); ++i) {
        // if (strcmp(Abc_ObjName(Abc_NtkPo(pNtk1, i)), Abc_ObjName(Abc_NtkPo(pNtk2, i))) != 0)
        if (smlt0.GetPoName(i) != smlt1.GetPoName(i))
            return false;
    }
    return true;
}


bool IsPIOSame(NetMan & net0, NetMan & net1) {
    if (net0.GetPiNum() != net1.GetPiNum())
        return false;
    for (int i = 0; i < net0.GetPiNum(); ++i) {
        // if (strcmp(Abc_ObjName(Abc_NtkPo(pNtk1, i)), Abc_ObjName(Abc_NtkPo(pNtk2, i))) != 0)
        if (net0.GetPiName(i) != net1.GetPiName(i))
            return false;
    }
    if (net0.GetPoNum() != net1.GetPoNum())
        return false;
    for (int i = 0; i < net0.GetPoNum(); ++i) {
        // if (strcmp(Abc_ObjName(Abc_NtkPo(pNtk1, i)), Abc_ObjName(Abc_NtkPo(pNtk2, i))) != 0)
        if (net0.GetPoName(i) != net1.GetPoName(i))
            return false;
    }
    return true;
}