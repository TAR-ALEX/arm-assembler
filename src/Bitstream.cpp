#include <iomanip>
#include <sstream>
#include <iostream>

#include "Bitstream.hpp"

using namespace std;

Bitstream::Bitstream(int e){
  nextByte = 0;
  nextByteIndex = 0;
  endian = e;
}
// Bitstream& Bitstream::operator<<(bool v){
//   cout << "Bitstream& Bitstream::operator<<(bool v)" << endl;
//   if(v == true){
//     (*this) << 1;
//   }else{
//     (*this) << 0;
//   }
//   return *this;
// }
Bitstream& Bitstream::operator<<(int v){
  int shift = 7-nextByteIndex;
  v = v << shift;
  nextByte |= v;
  nextByteIndex++;
  if(nextByteIndex > 7){
    word.push_back(nextByte);
    nextByte = 0;
    nextByteIndex = 0;
    if(word.size() >= 4){
      if(endian == Endian::Big){
        bytes.insert(bytes.end(), word.begin(), word.end());
      }else{
        bytes.insert(bytes.end(), word.rbegin(), word.rend());
      }
      word.clear();
    }
  }
  return *this;
}
Bitstream& Bitstream::operator<<(string vs){
  for(int i = 0; i < vs.size(); i++){
    if(vs[i] == '0'){
      (*this) << 0;
    }else if(vs[i] == '1'){
      (*this) << 1;
    }
  }
  return *this;
}
// Bitstream& Bitstream::operator<<(vector<bool> vs){
//   for(int i = 0; i < vs.size(); i++){
//     (*this) << vs[i];
//   }
//   return *this;
// }
Bitstream& Bitstream::operator<<(vector<int> vs){
  for(int i = 0; i < vs.size(); i++){
    (*this) << vs[i];
  }
  return *this;
}
vector<unsigned char> Bitstream::getBits(){
  return bytes;
}
string Bitstream::getHexDWords(){
  stringstream ss;
  for(int i = 0; i < bytes.size(); i++){
    ss << std::setfill('0') << std::setw(2) ;
    ss << std::hex << (int)bytes[i];
    if(i%4 == 3){
      ss << endl;
     }
  }
  if(nextByteIndex != 0 || word.size() != 0){
    ss << "[error: unfinished word]\n";
  }
  return ss.str();
}
