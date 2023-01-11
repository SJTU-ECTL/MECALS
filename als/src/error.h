#pragma once


#include "my_abc.h"
#include "simulator.h"
#include "lac.h"
#include "cut.h"


class PBDMan {
private:
    NetMan net;
    std::vector<ObjVec> oldFos;
    std::vector<ObjVec> appFos;
    ObjVec oneCuts;
    std::vector<ObjVec> cutNtks;
    IntVec topoIds;
    DblVec flows;
public:
    explicit PBDMan() = default;
    ~PBDMan() = default;
    PBDMan(const PBDMan &) = delete;
    PBDMan(PBDMan &&) = delete;
    PBDMan & operator = (const PBDMan &) = delete;
    PBDMan & operator = (PBDMan &&)  = delete;

    void BuildMit(NetMan& accNet, NetMan& appNet, NetMan& mitNet);
    void BuildPBD12(double exactPBDPerc);
    int Synth10(int fSASIMI);
    int CheckWithSweepSASIMI(std::vector<std::pair<std::string, std::string>> & lacNames);
    std::vector<std::pair<std::string, std::string>> CheckWithSATSASIMI();
    NetMan PostProc();
    ObjVec GetTFO(AbcObj* pObj) const;
    void GetTFORec(AbcObj* pObj, ObjVec& nodes) const;
    AbcObj* GetLocPBD(AbcObj* pV, AbcObj* pU);
};


NetMan BuildMit(NetMan& accNet, NetMan& appNet, NetMan& mitNet);