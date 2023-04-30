#ifndef arm_hpp
#define arm_hpp

#include "../ParseTree.hpp"
#include "../Bitstream.hpp"
#include "../Logging.tpp"
#include <utility>
#include <map>

extern map<string, string> shiftTypeMap2;

struct RotatedConstant{
  string rotationType = "";
  string roationAmount = "";
  string value = "";
};

class Arm64{
private:
  Bitstream bs;
  map<string, unsigned long long> labels;
  long long currentOffset = 0;
  long long baseOffset = 0;
  unsigned long long getDataSize(Element& arg);
  bool isData(Element& arg);
  void generateLabels(Element& t);

  void assembleStatement(Element& s);

  bool containsRegisterExtenstion(Element& arguments, string key);
  int countRegisterExtenstion(Element& arguments, vector<string> keys);
  //returns -1 for signed, 0 for unsigned (or true = signed, false = unsigned)
  int assertRegisterExtenstion(Element& arguments, string key);

  // method will log errors of it detects a shift, but shift is not of correct format.
  bool containsRegisterShift(Element& arguments, string key);
  // method will log errors of it detects a shift, but shift is not of correct format.
  int countRegisterShift(Element& arguments, vector<string> keys);
  // method will log errors of it detects a shift, but shift is not of correct format.
  // 1st arg is shift type, 2nd arg is shift amount.
  pair<string, string> assertRegisterShift(Element& arguments, string key, int registerSize, map<string, string>& shiftTypeMap);
  pair<string, int> assertRegisterShift(Element& arguments, string key);
  string assertRegisterShift4(Element& arguments, string key);

  string assertConditional(Element& arguments, string key);
  string assertInverseConditional(Element& arguments, string key);
  bool containsConditional(Element& arguments, string key);

  // gets a register. assers and logs error is missing, check with contains register first.
  string assertRegister(Element& arguments, string key, bool allowSubTuples, int size, map<string,string>& registerNameToBits);
  vector<string> assertRegisterList(Element& arguments, string key, int size, map<string,string>& registerNameToBits, int requiredLength);
  string assertSystemRegister(Element& arguments, string key);

  // returns size of found register or 0 is no register is found. (will work as a boolean)
  int containsRegister(Element& arguments, string key, bool allowSubTuple = false, int size = -1);
  // returns size of found register or 0 is no register is found. (will work as a boolean)
  int getRegisterSize(Element& arguments, string key, bool allowShifts = false); // todo - delete shift param
  int getMaxRegisterSize(Element& arguments, vector<string> keys, bool allowShifts = false); // todo - delete shift param
  bool containsDifferentSizedRegisters(Element& arguments, vector<string> keys);


  bool containsImmediate(Element& arguments, string key);
  int countImmediates(Element& arguments, vector<string> keys);

  unsigned long long assertImmediate(Element& arguments, string key);
  pair<string,string> assertImmediate12(Element& arguments, string key);
  string assertImmediateImmrImms(Element& arguments, string key, bool is64);

  string assertMultipleOfEncode(unsigned long long value, int zeroBits, int bits, string location);

  bool assertCmdArg(Element& s, string name);
  bool assertCommandName(Element& s, string name);
  bool assertTupleArguments(Element& s, string cmd);
  bool containsStackPointer(Element& args, vector<string> keys);

  void assertValueInBounds(unsigned long long val, bool hasSign, int numBits, string location);


  bool assembleData(Element& s);
  bool assembleInstruction(Element& s);

  inline bool assembleAddFamily(Element& s, string cmd);
  bool assembleAdd(Element& s);
  bool assembleSub(Element& s);
  bool assembleAdds(Element& s);
  bool assembleSubs(Element& s);

  inline bool assembleAdcFamily(Element& s, string cmd);
  bool assembleAdc(Element& s);
  bool assembleSbc(Element& s);
  bool assembleSbcs(Element& s);
  bool assembleAdcs(Element& s);
  bool assembleCmp(Element& s);
  bool assembleCmn(Element& s);

  bool assembleMov(Element& s);

  bool assembleSvc(Element& s);

  bool assembleAdr(Element& s);
  bool assembleAdrp(Element& s);

  inline bool assembleAndFamily(Element& s, string cmd);
  bool assembleAnd(Element& s);
  bool assembleAnds(Element& s);
  bool assembleOrr(Element& s);
  bool assembleEor(Element& s);
  bool assembleTst(Element& s);

  inline bool assembleAsrFamily(Element& s, string cmd);
  bool assembleAsr(Element& s);
  bool assembleLsl(Element& s);
  bool assembleLsr(Element& s);
  bool assembleRor(Element& s);

  bool assembleAsrv(Element& s);

  bool assembleWfe(Element& s);
  bool assembleNop(Element& s);

  bool assembleMsr(Element& s);
  bool assembleMrs(Element& s);

  bool assembleMadd(Element& s);
  bool assembleMneg(Element& s);
  bool assembleMsub(Element& s);
  bool assembleMul(Element& s);


  inline bool assembleNegFamily(Element& s, string cmd);
  bool assembleNeg(Element& s);
  bool assembleNegs(Element& s);

  inline bool assembleNgcFamily(Element& s, string cmd);
  bool assembleNgc(Element& s);
  bool assembleNgcs(Element& s);

  inline bool assembleDivFamily(Element& s, string cmd);
  bool assembleSdiv(Element& s);
  bool assembleUdiv(Element& s);

  inline bool assembleSmaddlFamily(Element& s, string cmd);
  bool assembleSmaddl(Element& s);
  bool assembleUmaddl(Element& s);

  inline bool assembleUmsublFamily(Element& s, string cmd);
  bool assembleUmsubl(Element& s);
  bool assembleSmsubl(Element& s);


  inline bool assembleUmneglFamily(Element& s, string cmd);
  bool assembleUmnegl(Element& s);
  bool assembleSmnegl(Element& s);

  inline bool assembleUmulhFamily(Element& s, string cmd);
  bool assembleUmulh(Element& s);
  bool assembleSmulh(Element& s);

  inline bool assembleUmullFamily(Element& s, string cmd);
  bool assembleUmull(Element& s);
  bool assembleSmull(Element& s);

  inline bool assembleBicFamily(Element& s, string cmd);
  bool assembleBic(Element& s);
  bool assembleBics(Element& s);
  bool assembleOrn(Element& s);
  bool assembleEon(Element& s);

  bool assembleMvn(Element& s);

  inline bool assembleMovkFamily(Element& s, string cmd);
  bool assembleMovz(Element& s);
  bool assembleMovn(Element& s);
  bool assembleMovk(Element& s);

  bool assembleLdrFamily(Element& s, string cmd);
  bool assembleLdr(Element& s);
  bool assembleLdrsw(Element& s);
  bool assembleLdrsh(Element& s);
  bool assembleLdrsb(Element& s);
  bool assembleLdrh(Element& s);
  bool assembleLdrb(Element& s);

  bool assembleStrFamily(Element& s, string cmd);
  bool assembleStr(Element& s);
  bool assembleStrb(Element& s);
  bool assembleStrh(Element& s);

  inline bool assembleLdpFamily(Element& s, string cmd);
  bool assembleLdp(Element& s);
  bool assembleLdpsw(Element& s);

  bool assembleStp(Element& s);

  inline bool assembleCcmpFamily(Element& s, string cmd);
  bool assembleCcmn(Element& s);
  bool assembleCcmp(Element& s);

  inline bool assembleCselFamily(Element& s, string cmd);
  bool assembleCsel(Element& s);
  bool assembleCsinv(Element& s);
  bool assembleCsinc(Element& s);
  bool assembleCsneg(Element& s);
  inline bool assembleCsetFamily(Element& s, string cmd);
  bool assembleCset(Element& s);
  bool assembleCsetm(Element& s);
  inline bool assembleCincFamily(Element& s, string cmd);
  bool assembleCinc(Element& s);
  bool assembleCinv(Element& s);
  bool assembleCneg(Element& s);

  bool assembleB(Element& s);
  bool assembleBl(Element& s);
  bool assembleBr(Element& s);
  bool assembleBlr(Element& s);

  inline bool assembleCbzFamily(Element& s, string cmd);
  bool assembleCbz(Element& s);
  bool assembleCbnz(Element& s);

  bool assembleRet(Element& s);

  inline bool assembleTbzFamily(Element& s, string cmd);
  bool assembleTbz(Element& s);
  bool assembleTbnz(Element& s);
public:
  Arm64():bs(Endian::Little){}
  Bitstream assemble(Element& t);
  Logging log;
  unsigned long long getOffset();
};

#endif
