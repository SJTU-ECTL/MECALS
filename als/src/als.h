#pragma once


#include "header.h"
#include "my_abc.h"
#include "error.h"
#include "lac.h"
#include "my_util.h"


struct ALSOpt {
    std::shared_ptr<NetMan> pAccNet;
    std::shared_ptr<NetMan> pMitNet;
    // std::shared_ptr<NetMan> pAppNet;
    std::string outpPath;
    unsigned seed;
    int nFrame;
    int fSASIMI;
    // int errUppBound;
    double exactPBDPerc;

    void Print() {
        pAccNet->PrintStat();
        pMitNet->PrintStat();
        // pAppNet->PrintStat();
        std::cout << "outpPath = " << outpPath << std::endl;
        std::cout << "seed = " << seed << std::endl;
        std::cout << "nFrame = " << nFrame << std::endl;
        std::cout << "fSASIMI = " << fSASIMI << std::endl;
        // std::cout << "errUppBound = " << errUppBound << std::endl;
        std::cout << "exactPBDPerc = " << exactPBDPerc << std::endl;
    }
};


class ALSMan {
private:
    NetMan& accNet;
    NetMan& mitNet;
    std::string outpPath;
    unsigned seed;
    int nFrame;
    int fSASIMI;
    // int errUppBound;
    double exactPBDPerc;
    LAC_TYPE lacType;
    double maxDelay;

public:
    explicit ALSMan(ALSOpt& opt): accNet(*opt.pAccNet), mitNet(*opt.pMitNet), outpPath(opt.outpPath), seed(opt.seed), nFrame(opt.nFrame), fSASIMI(opt.fSASIMI), exactPBDPerc(opt.exactPBDPerc), lacType(LAC_TYPE::RAC) {}
    ~ALSMan() = default;
    ALSMan(const ALSMan &) = delete;
    ALSMan(ALSMan &&) = delete;
    ALSMan & operator = (const ALSMan &) = delete;
    ALSMan & operator = (ALSMan &&) = delete;

    void Run();
    void RunCompWithSAT();
    void Map(NetMan & net);
};