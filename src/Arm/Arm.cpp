#include "Arm.hpp"
#include <map>
#include <iostream>
#include <iomanip>
#include <bitset>

using namespace std;

#include "Constants.hpp"
#include "SystemRegisters.hpp"
#include "Helpers.hpp"
#include "Instructions.hpp"

unsigned long long Arm64::getOffset(){return baseOffset;}

void Arm64::assembleStatement(Element& s){
  if(s[0].isInt() && s.size() == 1){
    return; // this constant was stored by the preprocessosr
  }
  if(!s[0].isName()){
    log << s.location <<" [error] missing assembly command, blank statment\n"; // could be a warning
    return;
  }
  auto command = s[0].getName();

  if(!assembleData(s) && !assembleInstruction(s)){
    log << s.location <<" [error] command " << command << " is not recognized\n";
    bs << nopStatement;
  }
}

bool Arm64::assembleData(Element& s){
  auto command = s[0].getName();
  if(!isData(s)){
    return false;
  }

  auto str = s[1].getString();
  auto size = s[1].size();
  auto values = s[1];

  if(command == "align"){
    if(!s[1]["to"].isInt()) {
      log << s[1].location << " [error] value is not an int\n";
      return true;
    }
    long long alignTo = s[1]["to"].getInt();
    long long alignOffset = s[1]["offset"].getInt();
    unsigned long long alignAmount = (alignTo-(currentOffset%alignTo)+alignOffset)%alignTo;
    for(int i = 0; i < alignAmount; i++){
      bs << "00000000";
    }
  }

  if(command == "utf8"){
    if(!s[1].isString()) {
      log << s[1].location << " [error] value is not a string for utf8 encoding\n";
      return true;
    }
    bs.endian = Endian::Big;
    auto alignmentError = str.size()%4;
    for(int i = alignmentError; i%4 != 0; i++)
      str += '\0';
    for(int i = 0; i < str.size(); i++){
      bs << bitset<8>(str[i]).to_string();
    }
    bs.endian = Endian::Little;
    return true;
  }

  if(s[1].isInt() || s[1].isName()){
    values = Element(
      Tuple(
        {{
          "",
          Element(
            Statement({Element(*s[1].value)})
          )
        }}
      )
    );
    size = 1;
  }else if(!s[1].isTuple()){
    log << s[1].location << " [error] value is not a tuple\n";
    return true;
  }

  for(int i = 0; i < values.size(); i++){
    if(!values[i].isInt() && !values[i].isName()){
      log << values[i].location << " [error] value is not an integer or label\n";
    }
  }
  // TODO - add size checks
  if(command == "byte"){
    for(int i = 0; i < size; i++){
      if(values[i].isName() && labels.count(values[i].getName()))
        bs << bitset<32>(labels[values[i].getName()]).to_string();
      else
        bs << bitset<32>(values[i].getInt()).to_string();
    }
  }
  if(command == "word"){
    for(int i = 0; i < size; i++){
      if(values[i].isName() && labels.count(values[i].getName()))
        bs << bitset<32>(labels[values[i].getName()]).to_string();
      else
        bs << bitset<32>(values[i].getInt()).to_string();
    }
  }
  if(command == "dword"){
    for(int i = 0; i < size; i++){
      long long val = 0;
      if(values[i].isName() && labels.count(values[i].getName()))
        val = labels[values[i].getName()];
      else
        val = values[i].getInt();
      //flipped for endianness we have to flip this. (not taken care of by bitstream since it is 2 words)
      auto str = bitset<64>(val).to_string();
      bs << str.substr(32, 32);
      bs << str.substr(0, 32);
    }
  }
  return true;
}

bool Arm64::assembleInstruction(Element& s){
  return
      assembleAdd(s) || assembleAdds(s) || assembleSub(s) || assembleSubs(s) || assembleCmp(s) || assembleCmn(s) ||
      assembleAdc(s) || assembleAdcs(s) || assembleSbc(s) || assembleSbcs(s) ||
      assembleMov(s) || assembleMovz(s) || assembleMovn(s) || assembleMovk(s) || assembleSvc(s) ||
      assembleAdr(s) || assembleAdrp(s) ||
      assembleAnd(s) || assembleAnds(s) || assembleOrr(s) || assembleEor(s) || assembleTst(s) ||
      assembleBic(s) || assembleBics(s) || assembleEon(s) || assembleOrn(s) ||
      assembleAsr(s) || assembleRor(s) || assembleLsr(s) || assembleLsl(s) ||
      assembleWfe(s) || assembleNop(s) ||
      assembleMsr(s) || assembleMrs(s) ||
      assembleMadd(s) || assembleMneg(s) || assembleMul(s) || assembleMsub(s) ||
      assembleNeg(s) || assembleNegs(s) ||
      assembleNgc(s) || assembleNgcs(s) ||
      assembleUdiv(s) || assembleSdiv(s) ||
      assembleUmaddl(s) || assembleSmaddl(s) ||
      assembleUmsubl(s) || assembleSmsubl(s) ||
      assembleUmnegl(s) || assembleSmnegl(s) ||
      assembleUmulh(s) || assembleSmulh(s) ||
      assembleUmull(s) || assembleSmull(s) ||
      assembleMvn(s) ||
      assembleLdr(s) || assembleLdrsw(s) || assembleLdrsh(s) || assembleLdrsb(s) || assembleLdrh(s) || assembleLdrb(s) ||
      assembleStr(s) || assembleStrh(s)|| assembleStrb(s) ||
      assembleLdp(s) || assembleLdpsw(s) ||
      assembleStp(s) ||
      assembleCcmn(s) || assembleCcmp(s) ||
      assembleCsel(s) || assembleCsinv(s) || assembleCsinc(s) || assembleCsneg(s) ||
      assembleCset(s) || assembleCsetm(s) || assembleCinc(s)|| assembleCinv(s)|| assembleCneg(s) ||
      assembleB(s) || assembleBl(s) || assembleBr(s) || assembleBlr(s) || assembleCbnz(s) || assembleCbz(s) || assembleRet(s) ||
      assembleTbnz(s) || assembleTbz(s);
}

Bitstream Arm64::assemble(Element& t){
    baseOffset = t["offset"].getInt();
    generateLabels(t);
    currentOffset = baseOffset;
    for(auto e : t.tuple->elements){
      assembleStatement(e.second);
      if(isData(e.second))
        currentOffset += getDataSize(e.second);
      else if(!e.second.isInt())
        currentOffset += 4;
    }
    return bs;
}
