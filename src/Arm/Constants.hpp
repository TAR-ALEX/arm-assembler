#ifndef ARM_CONSTANTS
#define ARM_CONSTANTS

#include <map>
#include <string>

using namespace std;

string nopStatement = "11010101000000110010000000011111";

//for add
map<string, string> shiftTypeMap2 =
{
  {"lsl",  "00"},
  {"lsr",  "01"},
  {"asr",  "10"},
  //{"reserved",  "00011"}
};

//for and
map<string, string> shiftTypeMap2And =
{
  {"lsl",  "00"},
  {"lsr",  "01"},
  {"asr",  "10"},
  {"ror",  "11"}
};

map<string, string> registerNameToBits =
{
  {"x0",  "00000"},
  {"x1",  "00001"},
  {"x2",  "00010"},
  {"x3",  "00011"},
  {"x4",  "00100"},
  {"x5",  "00101"},
  {"x6",  "00110"},
  {"x7",  "00111"},
  {"x8",  "01000"},
  {"x9",  "01001"},
  {"x10", "01010"},
  {"x11", "01011"},
  {"x12", "01100"},
  {"x13", "01101"},
  {"x14", "01110"},
  {"x15", "01111"},
  {"x16", "10000"},
  {"x17", "10001"},
  {"x18", "10010"},
  {"x19", "10011"},
  {"x20", "10100"},
  {"x21", "10101"},
  {"x22", "10110"},
  {"x23", "10111"},
  {"x24", "11000"},
  {"x25", "11001"},
  {"x26", "11010"},
  {"x27", "11011"},
  {"x28", "11100"},
  {"x29", "11101"},
  {"x30", "11110"},

  {"w0",  "00000"},
  {"w1",  "00001"},
  {"w2",  "00010"},
  {"w3",  "00011"},
  {"w4",  "00100"},
  {"w5",  "00101"},
  {"w6",  "00110"},
  {"w7",  "00111"},
  {"w8",  "01000"},
  {"w9",  "01001"},
  {"w10", "01010"},
  {"w11", "01011"},
  {"w12", "01100"},
  {"w13", "01101"},
  {"w14", "01110"},
  {"w15", "01111"},
  {"w16", "10000"},
  {"w17", "10001"},
  {"w18", "10010"},
  {"w19", "10011"},
  {"w20", "10100"},
  {"w21", "10101"},
  {"w22", "10110"},
  {"w23", "10111"},
  {"w24", "11000"},
  {"w25", "11001"},
  {"w26", "11010"},
  {"w27", "11011"},
  {"w28", "11100"},
  {"w29", "11101"},
  {"w30", "11110"},
};

map<string, string> registerNameToBitsWithSP = [](){
  map<string, string> combinedMap(registerNameToBits);
  combinedMap.insert({"wsp", "11111"});
  combinedMap.insert({"xsp", "11111"});
  combinedMap.insert({"sp", "11111"});
  combinedMap.insert({"w31", "11111"});
  combinedMap.insert({"x31", "11111"});
  return combinedMap;
}();

map<string, string> registerNameToBitsWithZR = [](){
  map<string, string> combinedMap(registerNameToBits);
  combinedMap.insert({"wzr", "11111"});
  combinedMap.insert({"xzr", "11111"});
  combinedMap.insert({"zr", "11111"});
  combinedMap.insert({"w31", "11111"});
  combinedMap.insert({"x31", "11111"});
  return combinedMap;
}();

map<string, string> registerNameToBitsSignExtendable = [](){
  map<string, string> newMap(registerNameToBits);
  for(auto p: registerNameToBits){
    if(p.first.size() && p.first[0] == 'x') {
      auto newStr = p.first;
      newStr[0] = 'b';
      newMap.insert({newStr, p.second});
      newStr[0] = 'h';
      newMap.insert({newStr, p.second});
    }
  }
  return newMap;
}();

map<string, string> registerNameToBitsSignExtendableWithZR = [](){
  map<string, string> newMap(registerNameToBitsWithZR);
  for(auto p: registerNameToBitsWithZR){
    if(p.first.size() && p.first[0] == 'x') {
      auto newStr = p.first;
      newStr[0] = 'b';
      newMap.insert({newStr, p.second});
      newStr[0] = 'h';
      newMap.insert({newStr, p.second});
    }
  }
  return newMap;
}();

map<string, string> registerNameToBitsSignExtendableWithSP = [](){
  map<string, string> newMap(registerNameToBitsWithSP);
  for(auto p: registerNameToBitsWithSP){
    if(p.first.size() && p.first[0] == 'x') {
      auto newStr = p.first;
      newStr[0] = 'b';
      newMap.insert({newStr, p.second});
      newStr[0] = 'h';
      newMap.insert({newStr, p.second});
    }
  }
  return newMap;
}();

map<string, string> registerNameToBitsAll = [](){
  map<string, string> combinedMap(registerNameToBitsSignExtendableWithZR);
  combinedMap.insert(registerNameToBitsSignExtendableWithSP.begin(), registerNameToBitsSignExtendableWithSP.end());
  return combinedMap;
}();


map<string, string> conditionalToBits = {
  {"eq", "0000"},
  {"ne", "0001"},
  {"hs", "0010"},
  {"lo", "0011"},
  {"mi", "0100"},
  {"pl", "0101"},
  {"vs", "0110"},
  {"vc", "0111"},
  {"hi", "1000"},
  {"ls", "1001"},
  {"ge", "1010"},
  {"lt", "1011"},
  {"gt", "1100"},
  {"le", "1101"},
  {"al", "1110"},
  {"nv", "1111"}
};

#endif
