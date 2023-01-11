#include "cmdline.hpp"
#include "als.h"

using cmdline::parser;
using std::cout;
using std::endl;
using std::string;
using std::vector;


parser CommPars(int argc, char * argv[]) {
    parser option;
    option.add <string> ("accCirc", 'i', "path to accurate circuit", true);
    option.add <string> ("mitCirc", 'm', "path to miter circuit", true);
    option.add <string> ("appCirc", 'a', "path to approximate circuit; if this option is not empty, then error checking is performed", false, "");
    option.add <string> ("standCell", 'l', "path to standard cell library", false, "input/standard-cell/nangate_45nm_typ.lib");
    option.add <string> ("outpPath", 'o', "path to approximate circuits", false, "tmp/");
    option.add <unsigned> ("seed", 's', "seed", false, 0);
    option.add <int> ("nFrame", 'f', "#simulation patterns", false, 8192);
    option.add <int> ("fSASIMI", '\0', "flag of using SASIMI", false, 1);
    option.add <double> ("exactPBDPerc", 'p', "proportion of exact PBD", false, 1.0);
    option.parse_check(argc, argv);
    return option;
}


void ALS(parser& option) {
    string accCirc = option.get <string> ("accCirc");
    string mitCirc = option.get <string> ("mitCirc");
    string standCell = option.get <string> ("standCell");
    string outpPath = option.get <string> ("outpPath");
    unsigned seed = option.get <unsigned> ("seed");
    int nFrame = option.get <int> ("nFrame");
    int fSASIMI = option.get <int> ("fSASIMI");
    double exactPBDPerc = option.get <double> ("exactPBDPerc");

    AbcMan abc;
    abc.ReadStandCell(standCell);
    ALSOpt alsOpt;
    alsOpt.pAccNet = std::make_shared<NetMan>(accCirc);
    // alsOpt.pAppNet = std::make_shared<NetMan>(appCirc);
    alsOpt.pMitNet = std::make_shared<NetMan>(mitCirc); 
    FixPath(outpPath);
    CreatePath(outpPath);
    alsOpt.outpPath = outpPath;
    alsOpt.seed = seed;
    if (alsOpt.seed == 0) {
        boost::random::mt19937 rng(time(0));
        boost::uniform_int <> unif(INT_MIN, INT_MAX);
        alsOpt.seed = static_cast <unsigned> (unif(rng));
    }
    alsOpt.nFrame = nFrame;
    alsOpt.fSASIMI = fSASIMI;
    alsOpt.exactPBDPerc = exactPBDPerc;
    alsOpt.Print();
    ALSMan als(alsOpt);
    // als.RunCompWithSAT();
    als.Run();
}


void MaxErrCheck(parser& option) {
    string accCirc = option.get <string> ("accCirc");
    string mitCirc = option.get <string> ("mitCirc");
    string appCirc = option.get <string> ("appCirc");
    string standCell = option.get <string> ("standCell");

    AbcMan abc;
    abc.ReadStandCell(standCell);

    auto pAccNet = std::make_shared<NetMan>(accCirc);
    auto pMitNet = std::make_shared<NetMan>(mitCirc); 
    auto pAppNet = std::make_shared<NetMan>(appCirc);

    PBDMan pbdMan;
    auto net = BuildMit(*pAccNet, *pAppNet, *pMitNet);
    net.Comm("st; resyn2; resyn2; resyn2; dsat;");
}


int main(int argc, char *argv[]) {
    parser option = CommPars(argc, argv);
    string appCirc = option.get <string> ("appCirc");
    GlobStartAbc();
    if (appCirc == "")
        ALS(option);
    else
        MaxErrCheck(option);
    GlobStopAbc();
    return 0;
}