#ifndef ARM_HELPERS
#define ARM_HELPERS

#include "Arm.hpp"
#include <map>
#include <iostream>
#include <iomanip>
#include <bitset>

using namespace std;

#include "Constants.hpp"
#include "SystemRegisters.hpp"
#include "Encodings/n_imms_immr.hpp"
#include "Encodings/block_shift_immeditate.hpp"
#include "Encodings/multiple_of.hpp"

unsigned long long Arm64::getDataSize(Element& arg){
  auto name = arg[0].getName();
  auto str = arg[1].getString();
  auto size = arg[1].size();

  if(arg[0].isName() && name == "align"){
    if(!arg[1]["to"].isInt()){
      return 0;
    }else{
      int alignTo = arg[1]["to"].getInt();
      long long alignOffset = arg[1]["offset"].getInt();
      unsigned long long alignAmount = (alignTo-(currentOffset%alignTo)+alignOffset)%alignTo;
      return alignAmount;
    }
  }

  if(arg[0].isName() && arg[1].isString() && name == "utf8"){
    if(str.size()%4 == 0){
      //log << "[utf8] " << str.size() << " " << str << endl;
      return str.size();
    }
    //log << "[utf8] " << (str.size()/4+1)*4 << " " << str << endl;
    return (str.size()/4+1)*4; // has remainder round up to multiple of 4
  }
  if(arg[0].isName() && (arg[1].isInt() || arg[1].isName())) size = 1;
  else if(!arg[0].isName() || !arg[1].isTuple()) return 0; // errors will be caught on the second pass

  if(name == "byte"){
    return 4 * size; // min register size is 4 bytes
  }
  if(name == "word"){
    return 4 * size;
  }
  if(name == "dword"){
    return 8 * size;
  }
  return 0;
}

bool Arm64::isData(Element& arg){
  auto name = arg[0].getName();
  return name == "byte" || name == "word" || name == "dword" || name == "utf8" || name == "align";
}

void Arm64::generateLabels(Element& t){
  currentOffset = baseOffset;
  for(auto e : t.tuple->elements){
    if(isData(e.second)){ // not efficient but readable
      if(e.first != "") labels.insert({e.first, currentOffset});
      currentOffset += getDataSize(e.second);
    }else if(e.second.isInt()) {
      if(e.first != "") labels.insert({e.first, e.second.getInt()});
    }else{
      if(e.first != "") labels.insert({e.first, currentOffset});
      currentOffset += 4;
    }
  }

  // log << "{\n";
  // for(auto lp : labels){
  //   log << "\tlabel[" << lp.first << "] = " << lp.second << endl;
  // }
  // log << "}\n";

}

// bool Arm64::containsRegisterShift(Element& arguments, string key){
//   if(!arguments[key].isTuple()){
//     return false;
//   }
//   if(!arguments[key]["shift"].isElement()) {
//     log << arguments[key].location;
//     log << " [error] argument does not contain shift\n";
//     return false;
//   }
//   auto shiftTuple = arguments[key]["shift"];
//   if(shiftTuple.isNull) {
//     log << arguments[key]["shift"].getElement().location;
//     log << " [error] shift is not a tuple\n";
//     return false;
//   }
//
//   if(shiftTuple.size() != 2) {
//     log << shiftTuple.location;
//     log << " [error] shift does not contain 2 arguments, it contains "<<shiftTuple.size()<<"\n";
//     return false;
//   }
//
//   if(!shiftTuple[0].isName()) {
//     log << shiftTuple[0].location;
//     log << " [error] wrong type used for 1st argument - should be shiftType (a name)\n";
//     return false;
//   }
//
//   if(!shiftTuple[1].isInt()) {
//     log << shiftTuple[1].location;
//     log << " [error] wrong type used for 1st argument - should be shiftAmount (a integer constant)\n";
//     return false;
//   }
//   return true;
// }

bool Arm64::containsRegisterShift(Element& arguments, string key){
  if(!arguments[key].isTuple()){
    return false;
  }
  if(!arguments[key]["shift"].isTuple()) {
    return false;
  }
  auto shiftTuple = arguments[key]["shift"];
  if(shiftTuple.isNull) {
    return false;
  }

  if(shiftTuple.size() != 2) {
    return false;
  }

  if(!shiftTuple[0].isName()) {
    return false;
  }

  if(!shiftTuple[1].isInt()) {
    return false;
  }
  return true;
}

int Arm64::countRegisterShift(Element& arguments, vector<string> keys){
  int count = 0;
  for(auto key: keys) if(containsRegisterShift(arguments, key)) count++;
  return count;
}

pair<string, int> Arm64::assertRegisterShift(Element& arguments, string key){
  if(!arguments.contains(key)){
    log << arguments.location;
    log << " [error] does not contain key " << key << "\n";
    return {"",0};
  }
  if(!arguments[key].isTuple()) {
    log << arguments[key].location;
    log << " [error] argument does not contain shift\n";
    return {"",0};
  }
  if(!arguments[key]["shift"].isTuple()) {
    log << arguments[key]["shift"].location;
    log << " [error] shift is not a tuple\n";
    return {"",0};
  }
  auto shiftTuple = arguments[key]["shift"];
  if(shiftTuple.size() != 2) {
    log << shiftTuple.location;
    log << " [error] shift does not contain 2 arguments, it contains "<<shiftTuple.size()<<"\n";
    return {"",0};
  }

  if(!shiftTuple[0].isName()) {
    log << shiftTuple[0].location;
    log << " [error] wrong type used for 1st argument - should be shiftType (a name)\n";
    return {"",0};
  }

  if(!shiftTuple[1].isInt()) {
    log << shiftTuple[1].location;
    log << " [error] wrong type used for 1st argument - should be shiftAmount (a integer constant)\n";
    return {"",0};
  }

  string shiftTypeName = shiftTuple[0].getName();
  int shiftAmountInt = shiftTuple[1].getInt();
  return {shiftTypeName, shiftAmountInt};
}

string Arm64::assertRegisterShift4(Element& arguments, string key){
  string noShift = "000";

  auto p = assertRegisterShift(arguments, key);

  auto shiftTypeName = p.first;
  auto shiftAmountInt = p.second;

  auto shiftAmountBits = bitset<3>(shiftAmountInt).to_string();

  if(shiftTypeName != "lsl") {
    log << arguments[key][0]["shift"][0].location;
    log << " [error] shift type not recognized - "<<shiftTypeName<<", try lsl\n";
    return noShift;
  }

  if(shiftAmountInt > 4 || shiftAmountInt < 0) {
    log << arguments[key][0]["shift"][1].location;
    log << " [error] shift is out of bounds, needs to be in range [0-"<<4<<"]\n";
    return noShift;
  }

  return shiftAmountBits;
}

pair<string, string> Arm64::assertRegisterShift(Element& arguments, string key, int registerSize, map<string, string>& shiftTypeMap){
  pair<string, string> noShift = {"00", "000000"};

  auto p = assertRegisterShift(arguments, key);

  auto shiftTypeName = p.first;
  auto shiftAmountInt = p.second;

  auto shiftTypeBits = shiftTypeMap[shiftTypeName];
  auto shiftAmountBits = bitset<6>(shiftAmountInt).to_string();

  if(shiftTypeBits == "") {
    log << arguments[key][0]["shift"][0].location;
    log << " [error] shift type not recognized - "<<shiftTypeName<<", try [lsl, lsr, asr, ror]\n";
    return noShift;
  }

  if(shiftAmountInt >= registerSize || shiftAmountInt < 0) {
    log << arguments[key][0]["shift"][1].location;
    log << " [error] shift is out of bounds, needs to be in range [0-"<<registerSize-1<<"]\n";
    return noShift;
  }

  return {shiftTypeBits,shiftAmountBits};
  // log << shiftTuple[1].getElement().location;
  // log << " [error] unsupported shift type\n";
  // return noShift;
}

inline int Arm64::getRegisterSize(Element& arguments, string key, bool allowShifts){
  return containsRegister(arguments, key, allowShifts, -1);
}

int Arm64::getMaxRegisterSize(Element& arguments, vector<string> keys, bool allowShifts){
  int max = 0;
  for(auto key: keys){
    int val = getRegisterSize(arguments, key, allowShifts);
    if(val > max) max = val;
  }
  return max;
}

int Arm64::containsRegister(Element& arguments, string key, bool allowShifts, int size){
  string name = "";
  if(!arguments[key].isNull) {
    if(arguments[key].isName())
      name = arguments[key].getName();
    else if(
      allowShifts &&
      arguments[key][0].isName()
    )
      name = arguments[key][0].getName();
  } else return 0;
  string bits = registerNameToBitsAll[name];
  if(bits == "") {
    return 0;
  }
  if(name.size() <= 0) return 0;
  if(size <= 0){
    if(name[0] == 'b') return 8;
    if(name[0] == 'h') return 16;
    if(name[0] == 'w') return 32;
    if(name[0] == 'x' || name[0] == 'z' ||  name[0] == 's') return 64;
  }
  if(size == 8){
    if(name[0] == 'b') return 16;
  }
  if(size == 16){
    if(name[0] == 'h') return 16;
  }
  if(size == 32){
    if(name[0] == 'w') return 32;
  }
  if(size == 64){
    if(name[0] == 'x'|| name[0] == 'z' ||  name[0] == 's') return 64;
  }
  return 0;
}

bool Arm64::containsImmediate(Element& arguments, string key){
  return arguments[key].isInt() || (arguments[key].isName() && labels.count(arguments[key].getName()));
}

int Arm64::countImmediates(Element& arguments, vector<string> keys){
  int count = 0;
  for(auto key : keys) if(containsImmediate(arguments, key)) count++;
  return count;
}

unsigned long long Arm64::assertImmediate(Element& arguments, string key){
  unsigned long long constantValue = arguments[key].getInt();

  if(arguments[key].isName() && labels.count(arguments[key].getName())){
    constantValue = labels[arguments[key].getName()];
  }else if(!arguments[key].isInt()){
    log << arguments.location << " [error] immediate "<<key<<" is missing\n";
    return 0;
  }

  return constantValue;
}

pair<string,string> Arm64::assertImmediate12(Element& arguments, string key) {
  auto val = assertImmediate(arguments, key);
  auto r = block_shift_immeditate_Encode(val, 2, 12);
  if(r.first == "" || r.second == "") {
    log << arguments[key].location << " [error] immediate for constant "<<key<<" cannot be encoded by block_shift_immeditate_Encode\n";
    return {"00", "000000000000"};
  }
  return {"0"+r.first, r.second};
}

string Arm64::assertImmediateImmrImms(Element& arguments, string key, bool is64) {
  auto constantValue = assertImmediate(arguments, key);
  auto rslt = n_immr_imms_Encode(constantValue, is64);
  if(rslt == "") {
    log << arguments.location << " [error] immediate "<<key<<" cannot be encoded with n_immr_imms encoding\n";
    return "0 000000 000000";
  }
  return rslt;
}

string Arm64::assertSystemRegister(Element& arguments, string key){
  if(!arguments.contains(key)){
    log << arguments.location << " [error] system register "<<key<<" is not in the arguments tuple\n";
    return "1000000000000000";
  }

  if(!arguments[key].isName()){
    log << arguments[key].location << " [error] statement is too complex, expected 1 element with name\n";
    return "1000000000000000";
  }
  auto regBits = getSystemRegisterEncodedString(arguments[key].getName());
  if(regBits == ""){
    log << arguments[key].location << " [error] system register not recognized\n";
    return "1000000000000000";
  }
  return regBits;
}

string Arm64::assertRegister(Element& arguments, string key, bool allowSubTuple, int size, map<string,string>& rgMap){
  string bits = "";
  string name = "";
  if(!arguments.contains(key)){
    log << arguments.location << " [error] register "<<key<<" is not in the arguments tuple\n";
    return "00000";
  }

  if(arguments[key].isName()){
    name = arguments[key].getName();
    bits = rgMap[name];
  }else if(allowSubTuple){
    if(!arguments[key][0].isName()){
      log << arguments[key][0].location << " [error] register is not a tuple or name\n";
      return "00000";
    }
    name = arguments[key][0].getName();
    bits = rgMap[name];
  }else{
    log << arguments[key].location << " [error] argument in "<<key<<" needs to be a register name\n";
    return "00000";
  }

  if(name.size() <= 0) {
    log << arguments[key].location << " [error] this should never happen, register name is empty\n";
    return "00000";
  }

  if(bits == "") {
    log << arguments[key].location << " [error] "<<name<<" is not a valid register name\n";
    return "00000";
  }
  if(size <= 0){
    return bits;
  }else if(size == 8){
    if(name[0] == 'b') return bits;
    else log << arguments[key].location << " [error] "<<name<<" is not a 8 bit register\n";
  }else if(size == 16){
    if(name[0] == 'h') return bits;
    else log << arguments[key].location << " [error] "<<name<<" is not a 16 bit register\n";
  }else if(size == 32){
    if(name[0] == 'w') return bits;
    else log << arguments[key].location << " [error] "<<name<<" is not a 32 bit register\n";
  }else if(size == 64){
    if(name[0] == 'x' || name[0] == 's' || name[0] == 'z' ) return bits;
    else log << arguments[key].location << " [error] "<<name<<" is not a 64 bit register\n";
  }else
    log << arguments[key].location << " [error] this should never happen\n";
  return bits;
}

// made this function to toggle case insensetive command names
bool Arm64::assertCommandName(Element& s, string name){
  auto command = s[0].getName();
  if(command != name) return false;
  return true;
}

bool Arm64::assertTupleArguments(Element& s, string cmd){
  if(!s[1].isTuple()){
    log << s.location << " [error] command " << cmd << " is missing arguments\n";
    bs << nopStatement;
    return false;
  }
  return true;
}

bool Arm64::containsDifferentSizedRegisters(Element& arguments, vector<string> keys){
  if(keys.size() <= 1) return false;
  int reference = -1;
  for(auto k: keys){
    int compareWith = getRegisterSize(arguments, k, true);
    if(compareWith <= 0) continue;
    if(reference <= 0) reference = compareWith;
    if(compareWith != reference) return true;
  }
  return false;
}

bool Arm64::containsStackPointer(Element& args, vector<string> keys){
  for(auto k: keys){
    string name = "";
    if(args[k].isName()){
      name = args[k].getName();
    }else if(args[k].isTuple() && args[k][0].isName()){
      name = args[k][0].getName();
    }
    if(name == "sp" || name == "wsp" || name == "xsp" || name == "hsp" || name == "bsp") return true;
  }
  return false;
}

bool Arm64::containsRegisterExtenstion(Element& arguments, string key){
  return arguments[key][0].contains("extend");
}
int Arm64::countRegisterExtenstion(Element& arguments, vector<string> keys){
  int count = 0;
  for(auto key: keys) if(containsRegisterExtenstion(arguments, key)) count++;
  return count;
}
//returns -1 for signed, 0 for unsigned (or true = signed, false = unsigned)
int Arm64::assertRegisterExtenstion(Element& arguments, string key){
  auto elem = arguments[key][0]["extend"];
  if(elem.isNull) return 0;
  if(elem.getName() == "uxt") return 0;
  if(elem.getName() == "sxt") return 1;
  log << elem.location << " [error] extension is of wrong type try [uxt, sxt]\n";
  return 0;
}

vector<string> Arm64::assertRegisterList(Element& arguments, string key, int size, map<string,string>& rgMap, int requiredLength){
  vector<string> bits = {};
  vector<string> name = {};
  if(arguments[key].isNull){
    log << arguments[key].location << " [error] registers for "<<key<<" are not in the arguments\n";
    while(bits.size() < requiredLength) bits.push_back("00000");
    return bits;
  }

  for(int i = 0; i < arguments[key].size(); i++){
    if(!arguments[key][0][i].isName()){
      log << arguments[key][0][i].location << " [error] argument in "<<key<<" needs to be a register name\n";
      continue;
    }
    name.push_back(arguments[key][0][i].getName());
    bits.push_back(rgMap[name[i]]);
    if(name[i].size() <= 0) {
      log << arguments[key][0][i].location << " [error] this should never happen, register name is empty\n";
      name.pop_back();
      bits.pop_back();
    }else if(bits[i] == "") {
      log << arguments[key][0][i].location << " [error] "<<name[i]<<" is not a valid register name\n";
      name.pop_back();
      bits.pop_back();
    }else if(size > 0){
      if(size == 8){
        if(name[i][0] == 'b') continue;
        else log << arguments[key][0][i].location << " [error] "<<name[i]<<" is not a 8 bit register\n";
      }else if(size == 16){
        if(name[i][0] == 'h') continue;
        else log << arguments[key][0][i].location << " [error] "<<name[i]<<" is not a 16 bit register\n";
      }else if(size == 32){
        if(name[i][0] == 'w') continue;
        else log << arguments[key][0][i].location << " [error] "<<name[i]<<" is not a 32 bit register\n";
      }else if(size == 64){
        if(name[i][0] == 'x' || name[i][0] == 's' || name[i][0] == 'z' ) continue;
        else log << arguments[key][0][i].location << " [error] "<<name[i]<<" is not a 64 bit register\n";
      }else
        log << arguments[key][0][i].location << " [error] this should never happen\n";
    }
  }

  if(requiredLength < 0){
    return bits;
  }

  if(requiredLength > bits.size()){
    log << arguments[key].location<< " [error] array of registers is not long enough, should have a size of: "<<requiredLength<<"\n";
    while(bits.size() < requiredLength) bits.push_back("00000");
  }

  return bits;
}

bool Arm64::containsConditional(Element& arguments, string key){
  return !arguments[key][0].isNull;
}

string Arm64::assertConditional(Element& arguments, string key){
  auto elem = arguments[key][0];
  if(!elem.isName()) {
    log << arguments[key].location<< " [error] missing conditional in key "<<key<<"\n";
    return "0000";
  }
  string name = elem.getName();
  string bits = conditionalToBits[name];
  if(bits == ""){
    log << elem.location << " [error] conditional "<<name<<" is not recognized in key "<<key<<" should be something like EQ, NE...\n";
    return "0000";
  }
  return bits;
}

string Arm64::assertInverseConditional(Element& arguments, string key){
  string cnd = assertConditional(arguments, key);
  if(cnd == "1110" || cnd == "1111"){
    log << arguments[key].location<< " [error] conditional in key "<<key<<" cannot be al or nv\n";
  }
  if(cnd[3] == '0') cnd[3] = '1';
  else cnd[3] = '0';
  return cnd;
}

string Arm64::assertMultipleOfEncode(unsigned long long value, int zeroBits, int bits, string location){
  auto str = multiple_of_Encode(value, zeroBits, bits);
  if(str == ""){
    log << location << " [error] could not encode value "<<value<<" with multiple_of_Encode\n";
    return bitset<64>(0).to_string().substr(0,bits);
  }
  return str;
}

void Arm64::assertValueInBounds(unsigned long long val, bool hasSign, int numBits, string location){
  if(hasSign) {
    if(val >= (1<<(numBits-1)) && ~val >= (1<<(numBits-1))){
      log << location << " [error] immediate value ("<<val<<") is out of range, should be "<<numBits<<" bits\n";
    }
  }else {
    if(val >= (1<<(numBits))){
      log << location << " [error] immediate value ("<<val<<") is out of range, should be "<<numBits<<" bits\n";
    }
  }
}



#endif
