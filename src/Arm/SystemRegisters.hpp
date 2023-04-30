#ifndef ARM_SystemRegisters
#define ARM_SystemRegisters

#include <map>
#include <vector>
#include <bitset>
#include <iostream>

using namespace std;

// op0 	 op1 	 CRn 	 CRm 	op2
// 0b11 	 0b011 	 0b0100 	 0b0010 	0b000


/*
<Xt> 	Is the 64-bit name of the general-purpose destination register, encoded in the "Rt" field.
<systemreg> 	Is a System register name, encoded in the "o0:op1:CRn:CRm:op2".
	The System register names are defined in 'AArch64 System Registers' in the System Register XML.
<op0> 	Is an unsigned immediate, encoded in “o0”:
                                                      	o0  <op0>
                                                      	0  2
                                                      	1  3
<op1> 	Is a 3-bit unsigned immediate, in the range 0 to 7, encoded in the "op1" field.
<Cn> 	Is a name 'Cn', with 'n' in the range 0 to 15, encoded in the "CRn" field.
<Cm> 	Is a name 'Cm', with 'm' in the range 0 to 15, encoded in the "CRm" field.
<op2> 	Is a 3-bit unsigned immediate, in the range 0 to 7, encoded in the "op2" field.
*/

int hexCharToInt(char c){
  static map<char, int> conversionMap = {
    {'0', 0},
    {'1', 1},
    {'2', 2},
    {'3', 3},
    {'4', 4},
    {'5', 5},
    {'6', 6},
    {'7', 7},
    {'8', 8},
    {'9', 9},

    {'A', 10},
    {'B', 11},
    {'C', 12},
    {'D', 13},
    {'E', 14},
    {'F', 15},

    {'a', 10},
    {'b', 11},
    {'c', 12},
    {'d', 13},
    {'e', 14},
    {'f', 15},
  };
  return conversionMap[c];
}

bool isHexChar(char c){
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

//accepts formatted string of format S<op0><op1><Cn><Cm><op2>
//everything must be in hex
//returns {o0,op1,CRn,CRm,op2} or {} if not valid
vector<int> getSystemRegisterBasicHexNames(string name){
  // S31772
  if(name.size() != 6 || (name[0] != 'S' && name[0] != 's')) return {};
  vector<int> result = {};
  name = name.substr(1, name.size()-1);
  for(char c: name){
    if(!isHexChar(c)) return {};
    result.push_back(hexCharToInt(c));
  }
  return result;
}

//accepts formatted string of format S<op0><op1><Cn><Cm><op2>
//everything must be in hex
//returns {o0,op1,CRn,CRm,op2} or {} if not valid
vector<int> getSystemRegisterDescriptiveNames(string name){
  if(name == "nzcv") return {3, 3, 4, 2, 0};
  if(name == "mpidr_el1") return {3, 0, 0, 0, 5};
  return {};
}

//returns {o0,op1,CRn,CRm,op2} or {} if not valid
vector<int> getSystemRegisterEncoding(string name){
  auto val = getSystemRegisterBasicHexNames(name);
  if(val.size() != 5) val = getSystemRegisterDescriptiveNames(name);
  if(val.size() != 5) return {};
  if(val[0] < 4 && val[1] < 8 && val[2] < 16 && val[3] < 16 && val[4] < 8) return val;
  return {};
}

//returns "o0:op1:CRn:CRm:op2" string in binary
string getSystemRegisterEncodedString(string name){
  auto val = getSystemRegisterEncoding(name);
  if(val.size() != 5) return {};
  return
    bitset<2>(val[0]).to_string() +
    bitset<3>(val[1]).to_string() +
    bitset<4>(val[2]).to_string() +
    bitset<4>(val[3]).to_string() +
    bitset<3>(val[4]).to_string();
}


#endif
