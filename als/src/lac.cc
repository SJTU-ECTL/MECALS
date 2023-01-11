#include "lac.h"


using namespace std;
using namespace abc;


void ResubLAC::Apply(NetMan& net) {
    int targId = GetTargId();
    int fi0 = net.GetFaninId(targId, 0), fi1 = net.GetFaninId(targId, 1);
    int div0 = divs.first, div1 = divs.second;
    bool compl0 = compls.first, compl1 = compls.second;
    int term0 = compl0? net.CreateInv(div0): div0;
    int term1 = compl1? net.CreateInv(div1): div1; 
    int subId = net.CreateAnd(term0, term1);

    net.GetLev();
    cout << "apply lac: ";
    cout << "targ=" << targId << ", ";
    cout << "fi0=" << fi0 << "(l=" << net.GetObjLev(fi0) << ")" << ", ";
    cout << "fi1=" << fi1 << "(l=" << net.GetObjLev(fi1) << ")" << ", ";
    cout << "div0=" << div0 << "(l=" << net.GetObjLev(div0) << ")" << ", ";
    cout << "div1=" << div1 << "(l=" << net.GetObjLev(div1) << ")" << endl;

    net.Replace(targId, subId);
}


void LACMan::GenLACs(NetMan & net, IntVec & targIds, unsigned seed) {
    if (lacType == LAC_TYPE::CONS)
        GenConstLACs(net, targIds);
    else if (lacType == LAC_TYPE::RAC)
        GenRacLACs(net, targIds, seed);
    else if (lacType == LAC_TYPE::RESUB)
        GenResubLACs(net, targIds);
    else
        assert(0);
    cout << "#lacs = " << pLacs.size() << endl;
}


void LACMan::GenConstLACs(NetMan & net, IntVec & targIds) {
    pLacs.reserve(targIds.size() * 2);
    for (auto targId: targIds) {
        if (!net.IsNode(targId))
            continue;
        if (net.IsConst(targId))
            continue;
        pLacs.emplace_back(make_shared <ConstLAC> (numeric_limits <double>::max(), targId, true));
        pLacs.emplace_back(make_shared <ConstLAC> (numeric_limits <double>::max(), targId, false));
    }
}


void LACMan::GenSasimiLACs(NetMan & net, IntVec & targIds) {
    pLacs.clear();
    if (net.GetNetType() == NET_TYPE::SOP) {
        net.GetLev();
        for (auto const & targId: targIds) {
            if (!net.IsNode(targId))
                continue;
            if (net.IsConst(targId))
                continue;
            for (int subId = 0; subId < net.GetIdMaxPlus1(); ++subId) {
                if (!net.IsObj(subId))
                    continue;
                if (net.IsObjPo(subId))
                    continue;
                if (net.IsConst(subId))
                    pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, false));
                else if (targId != subId && net.GetObjLev(subId) < net.GetObjLev(targId)) {
                    if (net.GetObjLev(subId) < net.GetObjLev(targId) - 1) {
                        pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, false));
                        pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, true));
                    }
                    else
                        pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, false));
                }
            }
        }
    }
    else if (net.GetNetType() == NET_TYPE::GATE) {
        double invDelay = net.GetInvDelay();
        net.GetDelay();
        for (auto const & targId: targIds) {
            if (!net.IsNode(targId))
                continue;
            if (net.IsConst(targId))
                continue;
            for (int subId = 0; subId < net.GetIdMaxPlus1(); ++subId) {
                if (!net.IsObj(subId))
                    continue;
                if (net.IsObjPo(subId))
                    continue;
                if (net.IsConst(subId))
                    pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, false));
                else if (targId != subId && DoubleLess(net.GetArrTime(subId), net.GetArrTime(targId), DELAY_TOL)) {
                    if (DoubleLess(net.GetArrTime(subId), net.GetArrTime(targId) - invDelay, DELAY_TOL)) {
                        pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, false));
                        pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, true));
                    }
                    else
                        pLacs.emplace_back(make_shared <SasimiLAC> (numeric_limits <double>::max(), targId, subId, false));
                }
            }
        }
    }
    else
        assert(0);
    // for (auto const & lac: pLacs) {
    //     auto sasimi = dynamic_pointer_cast <SasimiLAC> (lac);
    //     cout << sasimi->GetTargId() << "\t" << sasimi->GetSubstNodeId() << "\t" << sasimi->GetErr() << endl;
    // }
}


void LACMan::GenRacLACs(NetMan & net, IntVec & targIds, unsigned seed) {
    const int nGrowthLevel = 0;
    const int nCandLimit = 1;
    const int appFrame = 32;

    // simulate
    Simulator smlt(net, seed, appFrame);
    smlt.InpUnif();
    smlt.Sim();

    // initialize
    pLacs.clear();
    net.GetLev();
    Abc_NtkStartReverseLevels(net.GetNet(), nGrowthLevel);
    
    // generate LACs
    // cout << "generating LACs" << endl;
    // boost::timer::progress_display pd(targIds.size());
    for (auto targId: targIds) {
        if (!net.IsNode(targId) || net.IsConst(targId)) {
            // ++pd;
            continue;
        }
        auto pPivot = net.GetObj(targId);
        // compute divisors
        auto divs = GetDivs(pPivot, Abc_ObjRequiredLevel(pPivot) - 1);
        // cout << pPivot << ":";
        // PrintVect(divs, "\n");
        // enumerate resubstitution
        int nFanin = net.GetFaninNum(pPivot);
        int targLev = net.GetObjLev(targId);
        int countFocus = 0;
        for (int i = 0; i < nFanin; ++i) {
            int ithFanin = net.GetFaninId(pPivot, i);
            if (net.GetObjLev(ithFanin) == targLev - 1)
                ++countFocus;
        }
        if (countFocus == 1) {
            int cnt = 0;
            for (int i = 0; i < nFanin; ++i) {
                // skip small levels
                int ithFanin = net.GetFaninId(pPivot, i);
                if (net.GetObjLev(ithFanin) != targLev - 1)
                    continue;
                // init temp divisors
                IntVec faninIds;
                faninIds.reserve(10);
                for (int j = 0; j < nFanin; ++j) {
                    if (i != j)
                        faninIds.emplace_back(net.GetFaninId(pPivot, j));
                }
                // try removing the i-th fanin
                if (nFanin > 1) {
                    string func = BuildFuncWithEspresso(smlt, pPivot, faninIds);
                    if (func != string("")) {
                        // cout << pPivot << " approx \n"; PrintVect(faninIds, "\n"); cout << func << endl;
                        if (!(func.length() == 4 && func[0] == '-'))
                            pLacs.emplace_back(make_shared <RacLAC> (numeric_limits <double>::max(), targId, faninIds, func));
                    }
                }
                // try replacing the i-th fanin with another divisor
                if (nFanin >= 1) {
                    set <int> faninIdSet(faninIds.begin(), faninIds.end());
                    faninIds.emplace_back(-1);
                    for (const auto & div: divs) {
                        if (div == ithFanin || faninIdSet.count(div) || net.GetObjLev(div) >= targLev - 1)
                            continue;
                        faninIds.back() = div;
                        string func = BuildFuncWithEspresso(smlt, pPivot, faninIds);
                        // cout << "try " << pPivot << " with ";
                        // PrintVect(faninIds, "\n");
                        if (func != string("")) {
                            // cout << pPivot << " approx \n"; PrintVect(faninIds, "\n"); cout << func << endl;
                            if (func.length() == 5) {
                                assert(faninIds.size() == 2);
                                string newFunc("");
                                IntVec newFaninIds;
                                for (int pos = 0; pos < func.length(); ++pos) {
                                    if (func[pos] != '-') {
                                        newFunc += func[pos];
                                        if (pos < faninIds.size())
                                            newFaninIds.emplace_back(faninIds[pos]);
                                    }
                                }
                                if (newFaninIds.size())
                                    pLacs.emplace_back(make_shared <RacLAC> (numeric_limits <double>::max(), targId, newFaninIds, newFunc));
                            }
                            else
                                pLacs.emplace_back(make_shared <RacLAC> (numeric_limits <double>::max(), targId, faninIds, func));
                            if (++cnt > nCandLimit)
                                break;
                        }
                    }
                }
            }
        }
        // add constant
        pLacs.emplace_back(make_shared <RacLAC> (numeric_limits <double>::max(), targId, IntVec (), string(" 0\n")));
        pLacs.emplace_back(make_shared <RacLAC> (numeric_limits <double>::max(), targId, IntVec (), string(" 1\n")));
        // ++pd;
    }

    Abc_NtkStopReverseLevels(net.GetNet());
}


void LACMan::GenResubLACs(NetMan & net, IntVec & targIds) {
    const int nGrowthLevel = 0;
    // initialize
    pLacs.clear();
    net.GetLev();
    Abc_NtkStartReverseLevels(net.GetNet(), nGrowthLevel);
    boost::mt19937 randGen; randGen.seed(19960822);
    boost::uniform_real<> unDistr(0.0, 1.0);
    
    // generate LACs
    for (auto targId: targIds) {
        if (!net.IsNode(targId) || net.IsConst(targId) || net.GetFaninNum(targId) == 1)
            continue;
        assert(net.GetFaninNum(targId) == 2);
        // compute divisors
        auto divs = GetDivs(net.GetObj(targId), Abc_ObjRequiredLevel(net.GetObj(targId)) - 1);
        // enumerate resubstitution
        int fi0 = net.GetFaninId(targId, 0), fi1 = net.GetFaninId(targId, 1);
        for (int div: divs) {
            if (div != fi0) {
                bool accept = false;
                assert(net.GetFanoutNum(fi0));
                if (net.GetFanoutNum(fi0) == 1) accept = true;
                else {
                    if (net.GetObjLev(fi0) > net.GetObjLev(div)) accept = true;
                    // else if (net.GetObjLev(fi0) == net.GetObjLev(div)) {
                    //     if (unDistr(randGen) <= 0.2) accept = true;
                    // }
                    // else {
                    //     if (unDistr(randGen) <= 0.1) accept = true;
                    // }
                }
                if (accept) {
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, div, fi1, false, false));
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, div, fi1, false, true));
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, div, fi1, true, false));
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, div, fi1, true, true));
                }
            }
            if (div != fi1) {
                bool accept = false;
                assert(net.GetFanoutNum(fi1));
                if (net.GetFanoutNum(fi1) == 1) accept = true;
                else {
                    if (net.GetObjLev(fi1) > net.GetObjLev(div)) accept = true;
                    // else if (net.GetObjLev(fi1) == net.GetObjLev(div)) {
                    //     if (unDistr(randGen) <= 0.2) accept = true;
                    // }
                    // else {
                    //     if (unDistr(randGen) <= 0.1) accept = true;
                    // }
                }
                if (accept) {
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, fi0, div, false, false));
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, fi0, div, false, true));
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, fi0, div, true, false));
                    pLacs.emplace_back(make_shared<ResubLAC>(numeric_limits<double>::max(), targId, fi0, div, true, true));
                }
            }
        }
    }

    // recycle
    Abc_NtkStopReverseLevels(net.GetNet());
}


void LACMan::Print() const {
    for (const auto& pLac: pLacs) {
        if (lacType == LAC_TYPE::CONS)
            dynamic_pointer_cast<ConstLAC>(pLac)->Print();
        else if (lacType == LAC_TYPE::RAC)
            dynamic_pointer_cast<RacLAC>(pLac)->Print();
        else
            assert(0);
    }
    cout << "#lacs = " << pLacs.size() << endl;
}


void LACMan::SortAndFilt(int errUppBound) {
    sort(pLacs.begin(), pLacs.end(),
        [](const auto & A, const auto & B) {return A->GetErr() < B->GetErr();}
    );
    int i = 0;
    for (i = 0; i < pLacs.size(); ++i) {
        if (pLacs[i]->GetErr() > errUppBound)
            break;
    }
    pLacs.resize(i);
}


shared_ptr <LAC> LACMan::GetBestLac() const {
    #ifdef DEBUG
    assert(pLacs.size());
    #endif
    double bestErr = numeric_limits <double>::max();
    shared_ptr <LAC> pBestLac = pLacs[0];
    for (const auto & pLac: pLacs) {
        if (bestErr > pLac->GetErr()) {
            bestErr = pLac->GetErr();
            pBestLac = pLac;
        }
    }
    return pBestLac;
}


extern "C" {
    Vec_Ptr_t * Abc_MfsWinMarkTfi(Abc_Obj_t * pNode);
    void Abc_MfsWinSweepLeafTfo_rec(Abc_Obj_t * pObj, int nLevelLimit);
}
IntVec LACMan::GetDivs(Abc_Obj_t * pNode, int nLevDivMax) {
    const int nWinMax = 300;
    const int nFanoutsMax = 30;
    IntVec divs;
    Vec_Ptr_t * vCone, * vDivs;
    Abc_Obj_t * pObj, * pFanout, * pFanin;
    int k, f, m;
    int nDivsPlus = 0, nTrueSupp;

    // mark the TFI with the current trav ID
    Abc_NtkIncrementTravId( pNode->pNtk );
    vCone = Abc_MfsWinMarkTfi( pNode );

    // count the number of PIs
    nTrueSupp = 0;
    Vec_PtrForEachEntry( Abc_Obj_t *, vCone, pObj, k )
        nTrueSupp += Abc_ObjIsCi(pObj);
//    printf( "%d(%d) ", Vec_PtrSize(p->vSupp), m );

    // mark with the current trav ID those nodes that should not be divisors:
    // (1) the node and its TFO
    // (2) the MFFC of the node
    // (3) the node's fanins (these are treated as a special case)
    Abc_NtkIncrementTravId( pNode->pNtk );
    Abc_MfsWinSweepLeafTfo_rec( pNode, nLevDivMax );
//    Abc_MfsWinVisitMffc( pNode );
    Abc_ObjForEachFanin( pNode, pObj, k )
        Abc_NodeSetTravIdCurrent( pObj );

    // at this point the nodes are marked with two trav IDs:
    // nodes to be collected as divisors are marked with previous trav ID
    // nodes to be avoided as divisors are marked with current trav ID

    // start collecting the divisors
    vDivs = Vec_PtrAlloc( nWinMax );
    Vec_PtrForEachEntry( Abc_Obj_t *, vCone, pObj, k )
    {
        if ( !Abc_NodeIsTravIdPrevious(pObj) )
            continue;
        if ( (int)pObj->Level > nLevDivMax )
            continue;
        Vec_PtrPush( vDivs, pObj );
        if ( Vec_PtrSize(vDivs) >= nWinMax )
            break;
    }
    Vec_PtrFree( vCone );

    // explore the fanouts of already collected divisors
    if ( Vec_PtrSize(vDivs) < nWinMax )
    Vec_PtrForEachEntry( Abc_Obj_t *, vDivs, pObj, k )
    {
        // consider fanouts of this node
        Abc_ObjForEachFanout( pObj, pFanout, f )
        {
            // stop if there are too many fanouts
            if ( nFanoutsMax && f > nFanoutsMax )
                break;
            // skip nodes that are already added
            if ( Abc_NodeIsTravIdPrevious(pFanout) )
                continue;
            // skip nodes in the TFO or in the MFFC of node
            if ( Abc_NodeIsTravIdCurrent(pFanout) )
                continue;
            // skip COs
            if ( !Abc_ObjIsNode(pFanout) )
                continue;
            // skip nodes with large level
            if ( (int)pFanout->Level > nLevDivMax )
                continue;
            // skip nodes whose fanins are not divisors  -- here we skip more than we need to skip!!! (revise later)  August 7, 2009
            Abc_ObjForEachFanin( pFanout, pFanin, m )
                if ( !Abc_NodeIsTravIdPrevious(pFanin) )
                    break;
            if ( m < Abc_ObjFaninNum(pFanout) )
                continue;
            // make sure this divisor in not among the nodes
//            Vec_PtrForEachEntry( Abc_Obj_t *, p->vNodes, pFanin, m )
//                assert( pFanout != pFanin );
            // add the node to the divisors
            Vec_PtrPush( vDivs, pFanout );
            // Vec_PtrPushUnique( p->vNodes, pFanout );
            Abc_NodeSetTravIdPrevious( pFanout );
            nDivsPlus++;
            if ( Vec_PtrSize(vDivs) >= nWinMax )
                break;
        }
        if ( Vec_PtrSize(vDivs) >= nWinMax )
            break;
    }

    // sort the divisors by level in the increasing order
    Vec_PtrSort( vDivs, (int (*)(const void *, const void *))Abc_NodeCompareLevelsIncrease );

    // add the fanins of the node
    Abc_ObjForEachFanin( pNode, pFanin, k )
        Vec_PtrPush( vDivs, pFanin );
    
    divs.reserve(Vec_PtrSize(vDivs));
    Vec_PtrForEachEntry(Abc_Obj_t *, vDivs, pObj, k)
        divs.emplace_back(pObj->Id);

    return divs;
}


string LACMan::BuildFuncWithEspresso(Simulator & smlt, Abc_Obj_t * pPivot, const IntVec & faninIds) {
    Abc_Ntk_t * pAppNtk = smlt.GetNet();
    int nFrame = smlt.GetFrameNumb();

    assert(pAppNtk == pPivot->pNtk);
    assert(faninIds.size() >= 1);

    // check the existence of resubstitution and build truth table
    typedef unordered_map <string, bool> table_t;
    table_t truthTable;
    for (int i = 0; i < nFrame; ++i) {
        string minterm("");
        // Vec_PtrForEachEntry(Abc_Obj_t *, vFanins, pFanin, k)
        for (const auto & faninId: faninIds)
            minterm += (*smlt.GetDat(faninId))[i]? '1': '0';
        bool val = (*smlt.GetDat(pPivot->Id))[i];
        table_t::const_iterator got = truthTable.find(minterm);
        if (got == truthTable.end())
            truthTable[minterm] = val;
        else {
            if (got->second != val)
                return string("");
        }
    }
    // for (const auto & [key, value]: truthTable)
    //     cout << key << "," << value << endl;

    // construct function with espresso
    int nVars = faninIds.size();
    auto PLA = ESPRESSO::new_PLA();
    PLA->pla_type = FR_type;

    extern struct ESPRESSO::cube_struct cube;
    assert(cube.fullset == nullptr);
    cube.num_binary_vars = nVars;
    cube.num_vars = cube.num_binary_vars + 1;
    cube.part_size = ALLOC(int, cube.num_vars);

    assert(cube.fullset == nullptr);
    assert(cube.part_size != nullptr);
    cube.part_size[cube.num_vars-1] = 1;
    ESPRESSO::cube_setup();
    ESPRESSO::PLA_labels(PLA);

    if (PLA->F == nullptr) {
        PLA->F = ESPRESSO::sf_new(10, cube.size);
        PLA->D = ESPRESSO::sf_new(10, cube.size);
        PLA->R = ESPRESSO::sf_new(10, cube.size);
    }

    ESPRESSO::pcube cf = cube.temp[0];
    for (table_t::const_iterator iter = truthTable.begin(); iter != truthTable.end(); ++iter) {
        ESPRESSO::set_clear(cf, cube.size);
        const string & minterm = iter->first;
        for (int i = 0; i < nVars; ++i) {
            if (minterm[i] == '0')
                set_insert(cf, 2*i);
            else if (minterm[i] == '1')
                set_insert(cf, 2*i+1);
            else
                assert(0);
        }
        set_insert(cf,2*nVars);
        if (iter->second)
            PLA->F = ESPRESSO::sf_addset(PLA->F, cf);
        else
            PLA->R = ESPRESSO::sf_addset(PLA->R, cf);
    }

    ESPRESSO::pcover X;
    ESPRESSO::free_cover(PLA->D);
    X = ESPRESSO::d1merge(ESPRESSO::sf_join(PLA->F, PLA->R), cube.num_vars - 1);
    PLA->D = ESPRESSO::complement(cube1list(X));
    free_cover(X);

    PLA->F = ESPRESSO::espresso(PLA->F, PLA->D, PLA->R);

    // convert to sop
    string func("");
    ESPRESSO::pcube last, c;
    foreach_set(PLA->F, last, c) {
        for (int var = 0; var < cube.num_binary_vars; var++) {
            char item = ("?01-" [GETINPUT(c, var)]);
            assert(item != '?');
            func += item;
        }
        assert(cube.num_binary_vars == cube.num_vars - 1);
        assert(cube.output != -1);
        int llast = cube.last_part[cube.output];
        assert(cube.first_part[cube.output] == llast);
        assert("01" [is_in_set(c, llast) != 0] == '1');
        func += " 1\n";
    }

    // clean up
    ESPRESSO::free_PLA(PLA);
    FREE(cube.part_size);
    ESPRESSO::setdown_cube();
    ESPRESSO::sf_cleanup();
    ESPRESSO::sm_cleanup();

    return func;
}


void LACMan::GenCandLacs() {
    int maxId = -1;
    for (int i = 0; i < GetLacNum(); ++i)
        maxId = max(maxId, GetLac(i)->GetTargId());
    #ifdef DEBUG
    assert(maxId != -1);
    #endif
    candLacs.resize(maxId + 1, nullptr);

    vector <double> minErrs(maxId + 1, numeric_limits <double>::max());
    for (int i = 0; i < GetLacNum(); ++i) {
        auto pLac =  GetLac(i);
        auto targId = pLac->GetTargId();
        if (pLac->GetErr() < minErrs[targId]) {
            candLacs[targId] = pLac;
            minErrs[targId] = pLac->GetErr();
        }
    }
}


int LACMan::GenCandLacs(const IntVec & critGraph) {
    int maxId = -1;
    for (int i = 0; i < GetLacNum(); ++i)
        maxId = max(maxId, GetLac(i)->GetTargId());
    #ifdef DEBUG
    assert(maxId != -1);
    #endif
    candLacs.resize(maxId + 1, nullptr);

    vector <double> minErrs(maxId + 1, numeric_limits <double>::max());
    set <int> S(critGraph.begin(), critGraph.end());
    double bestErr = numeric_limits <double>::max();
    int bestId = -1;
    for (int i = 0; i < GetLacNum(); ++i) {
        auto pLac =  GetLac(i);
        auto targId = pLac->GetTargId();
        if (S.count(targId) == 0)
            continue;
        if (pLac->GetErr() < minErrs[targId]) {
            candLacs[targId] = pLac;
            minErrs[targId] = pLac->GetErr();
            if (pLac->GetErr() < bestErr) {
                bestErr = pLac->GetErr();
                bestId = targId;
            }
        }
    }
    return bestId;
}