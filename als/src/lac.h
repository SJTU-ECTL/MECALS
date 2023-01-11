#pragma once


#include "my_abc.h"
#include "header.h"
#include "simulator.h"
#include "espressoApi.h"


enum class LAC_TYPE {
    CONS, SASIMI, RAC, RESUB
};


class LAC{
private:
    double err;
    int targId;

public:
    explicit LAC() = default;
    explicit LAC(double _err, int targ_node_id): err(_err), targId(targ_node_id) {}
    virtual ~LAC() = default;
    LAC(const LAC & oth_lac) = default;
    LAC(LAC &&) = default;
    LAC & operator = (const LAC & oth_lac) = default;
    LAC & operator = (LAC &&) = default;

    inline double GetErr() const {return err;}
    inline void SetErr(double _err) {err = _err;}
    inline int GetTargId() const {return targId;}
    inline void SetTargId(int targ_node_id) {targId = targ_node_id;}
    inline void Print(bool isNewLine = true) const {std::cout << "target node id = " << targId << ", " << "error = " << err; if (isNewLine) std::cout << std::endl;}
};


class ConstLAC: public LAC {
private:
    bool isConst0;

public:
    explicit ConstLAC() = default;
    explicit ConstLAC(double _err, int targ_node_id, bool is_const0): LAC(_err, targ_node_id), isConst0(is_const0) {}
    ~ConstLAC() = default;
    ConstLAC(const ConstLAC & oth_lac) = default;
    ConstLAC(ConstLAC && oth_lac) = default;
    ConstLAC & operator = (const ConstLAC & oth_lac) = default;
    ConstLAC & operator = (ConstLAC && oth_lac) = default;

    inline bool IsConst0() const {return isConst0;}
    inline void SetConst0(bool is_const0) {isConst0 = is_const0;}
    inline void Print(bool isNewLine = true) const {LAC::Print(false); std::cout << ", " << (isConst0? "const0": "const1"); if (isNewLine) std::cout << std::endl;}
};


class SasimiLAC: public LAC {
private:
    int subId;
    bool isInv;

public:
    explicit SasimiLAC() = default;
    explicit SasimiLAC(double _err, int targ_node_id, int sub_node_id, bool is_inv): LAC(_err, targ_node_id), subId(sub_node_id), isInv(is_inv) {}
    ~SasimiLAC() = default;
    SasimiLAC(const SasimiLAC & oth_lac) = default;
    SasimiLAC(SasimiLAC && oth_lac) = default;
    SasimiLAC & operator = (const SasimiLAC & oth_lac) = default;
    SasimiLAC & operator = (SasimiLAC && oth_lac) = default;

    inline int GetSubId() const {return subId;}
    inline bool GetIsInv() const {return isInv;}
    inline void Print(bool isNewLine = true) const {LAC::Print(false); std::cout << ", substitute node id = " << subId << ", " << (isInv? "inv": "buf"); if (isNewLine) std::cout << std::endl;}
};


class RacLAC: public LAC {
private:
    IntVec divs;
    std::string sop;

public:
    explicit RacLAC() = default;
    explicit RacLAC(double _err, int targ_node_id, const IntVec & _divs, const std::string & _sop): LAC(_err, targ_node_id), divs(_divs), sop(_sop) {}
    ~RacLAC() = default;
    RacLAC(const RacLAC & oth_lac) = default;
    RacLAC(RacLAC && oth_lac) = default;
    RacLAC & operator = (const RacLAC & oth_lac) = default;
    RacLAC & operator = (RacLAC && oth_lac) = default;

    inline IntVec GetDivIds() const {return divs;}
    inline std::string GetSop() const {return sop;}
    inline void Print(bool isNewLine = true) const {LAC::Print(false); std::cout << ", divisors = "; PrintVect(divs, ","); std::cout << sop; if (isNewLine) std::cout << std::endl;}
};


class ResubLAC: public LAC {
private:
    std::pair<int, int> divs;
    std::pair<bool, bool> compls;

public:
    explicit ResubLAC() = default;
    explicit ResubLAC(double _err, int targ_node_id, int _div0, int _div1, bool _compl0, bool _compl1): LAC(_err, targ_node_id), divs(_div0, _div1), compls(_compl0, _compl1) {}
    ~ResubLAC() = default;
    ResubLAC(const ResubLAC & oth_lac) = default;
    ResubLAC(ResubLAC && oth_lac) = default;
    ResubLAC & operator = (const ResubLAC & oth_lac) = default;
    ResubLAC & operator = (ResubLAC && oth_lac) = default;

    void Apply(NetMan& net);

    inline int GetDiv(int i) const {return i? divs.second: divs.first;}
    inline bool GetCompl(int i) const {return i? compls.second: compls.first;}
    inline void Print(bool isNewLine = true) const {LAC::Print(false); std::cout << ", divisors = " << divs.first << "," << divs.second << ", compl = " << compls.first << "," << compls.second; if (isNewLine) std::cout << std::endl;}
};


class LACMan {
private:
    LAC_TYPE lacType;
    std::vector < std::shared_ptr <LAC> > pLacs;
    std::vector < std::shared_ptr <LAC> > candLacs;

public:
    explicit LACMan(LAC_TYPE lac_type): lacType(lac_type) {}
    ~LACMan() = default;
    LACMan(const LACMan &) = delete;
    LACMan(LACMan &&) = delete;
    LACMan & operator = (const LACMan &) = delete;
    LACMan & operator = (LACMan &&) = delete;

    void GenLACs(NetMan & net, IntVec & targIds, unsigned seed = 0);
    void GenConstLACs(NetMan & net, IntVec & targIds);
    void GenSasimiLACs(NetMan & net, IntVec & targIds);
    void GenRacLACs(NetMan & net, IntVec & targIds, unsigned seed);
    void GenResubLACs(NetMan & net, IntVec & targIds);
    void Print() const;
    void SortAndFilt(int errUppBound);
    std::shared_ptr <LAC> GetBestLac() const;
    IntVec GetDivs(abc::Abc_Obj_t * pNode, int nLevDivMax);
    std::string BuildFuncWithEspresso(Simulator & smlt, abc::Abc_Obj_t * pPivot, const IntVec & faninIds);
    void GenCandLacs();
    int GenCandLacs(const IntVec & critGraph);

    inline LAC_TYPE GetLacType() const {return lacType;}
    inline int GetLacNum() const {return static_cast <int> (pLacs.size());}
    inline std::shared_ptr <LAC> GetLac(int i) const {return pLacs[i];}
    inline int GetCandLacSize() const {return candLacs.size();}
    inline std::shared_ptr <LAC> GetCandLac(int id) const {return candLacs[id];}
};