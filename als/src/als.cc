#include "als.h"


using std::cout;
using std::endl;
using std::ostringstream;
using std::numeric_limits;
using std::shared_ptr;
using std::string;
using std::vector;


void ALSMan::Run() {
    auto appNet = accNet;
    int round = 0;
    ostringstream oss("");
    string netName = string(accNet.GetNet()->pName);
    oss << outpPath << round;
    appNet.WriteNet(oss.str() + ".blif", true); // appNet.WriteNet(oss.str() + ".dot", true);
    clock_t st = clock();
    for (round = 1; ; ++round) {
        cout << "---------- round " << round <<  "---------- " << endl;
        PBDMan pbdMan;
        pbdMan.BuildMit(accNet, appNet, mitNet);
        pbdMan.BuildPBD12(exactPBDPerc);
        int ret = pbdMan.Synth10(fSASIMI);
        if (ret == -1)
            break;
        appNet = pbdMan.PostProc();
        // appNet.Print(true);
        ostringstream oss("");
        oss << outpPath << round;
        appNet.WriteNet(oss.str() + ".blif", true); // appNet.WriteNet(oss.str() + ".dot", true);
        cout << "current runtime = " << (clock() - st) / 1000000.0 << "s" << endl;
    }
    oss.str("");
    oss << outpPath << "final";
    appNet.WriteNet(oss.str() + ".blif", true);
    cout << "current runtime = " << (clock() - st) / 1000000.0 << "s" << endl;
    Map(appNet);
}


void ALSMan::RunCompWithSAT() {
    auto appNet = accNet;
    int round = 0;
    ostringstream oss("");
    string netName = string(accNet.GetNet()->pName);
    oss << outpPath << round;
    appNet.WriteNet(oss.str() + ".blif", true); // appNet.WriteNet(oss.str() + ".dot", true);
    clock_t st = clock();
    ll pbdTime = 0, satTime = 0;
    for (round = 1; round <= 3; ++round) {
        cout << "---------- round " << round <<  "---------- " << endl;
        PBDMan pbdMan;
        pbdMan.BuildMit(accNet, appNet, mitNet);

        clock_t stSat = clock();
        auto lacNames = pbdMan.CheckWithSATSASIMI();
        satTime += clock() - stSat;

        clock_t stCheck = clock();
        pbdMan.BuildPBD12(exactPBDPerc);
        pbdMan.CheckWithSweepSASIMI(lacNames);
        pbdTime += clock() - stCheck;

        int ret = pbdMan.Synth10(fSASIMI);
        if (ret == -1)
            break;
        appNet = pbdMan.PostProc();
        // appNet.Print(true);
        ostringstream oss("");
        oss << outpPath << round;
        appNet.WriteNet(oss.str() + ".blif", true); // appNet.WriteNet(oss.str() + ".dot", true);
        cout << "current runtime = " << (clock() - st) / 1000000.0 << "s" << endl;
    }
    oss.str("");
    oss << outpPath << "final";
    appNet.WriteNet(oss.str() + ".blif", true);
    cout << "current runtime = " << (clock() - st) / 1000000.0 << "s" << endl;
    cout << "maximum error checking time with PBD = " << pbdTime / 1000000.0 << "s" << endl;
    cout << "maximum error checking time with SAT = " << satTime / 1000000.0 << "s" << endl;
    Map(appNet);
}


void ALSMan::Map(NetMan & net) {
    assert(net.GetNetType() == NET_TYPE::STRASH);
    auto newNet = net;
    newNet.Comm("dch; amap; stime;", false);
}