//
//  main.cpp
//  scat
//
//  Created by Alex Tarasov on 2/5/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <bitset>
#include <math.h>
#include "n_imms_immr.hpp"

#include "util.hpp"

using namespace std;

// returns return {successful, immr, imms};
template<int bits>
tuple<bool, long, long> immr_imms_Encode(unsigned long long targetValue, int lenImmr, int lenImms){
  unsigned long long shiftedValue = targetValue;
  unsigned long long bitmask = ones(bits);//this will never change so it can be static
  if(bits == 32){
    if(
      (targetValue | bitmask) != bitmask &&
      (targetValue | bitmask) != -1
    ) return {false, 0, 0}; // not sign extended and is too big, abort
  }
  targetValue&=bitmask;
  bool foundShift = false;

  if(targetValue == bitmask)//all ones
    return {false, 0, 0};
  else if(targetValue == 0)//all zeros
    return {false, 0, 0};

  //find the immediate size
  int immsSize;
  for(immsSize = 1; immsSize <= bits; immsSize++){
    shiftedValue = rol<bits>(targetValue, immsSize);
    if((shiftedValue&bitmask) == (targetValue&bitmask)) {
      foundShift = true;
      break;
    }
  }

  shiftedValue = targetValue;
  int numberOfOnes=0;
  int numberOfZeros=0;
  int offset=0;
  int lastBit = 1&targetValue;
  //rotate to the first one in the right and zero that just got cut off by the shift and moved to start.
  for(int i = 0; i <= immsSize+4; i++){//immsSize + 4 just to be safe, could even be a while loop until it finds the offset.
    shiftedValue = rol<bits>(shiftedValue);
    int currentBit = 1&shiftedValue;
    if(currentBit == 1 && lastBit == 0){
      offset = i;
      lastBit = currentBit;
      break;
    }
    lastBit = currentBit;
  }
  //rotate to the first zero in the right and one that just got cut off.
  for(int i = 1; i <= immsSize+4; i++){//immsSize + 4 just to be safe, could even be a while loop until it finds the offset.
    shiftedValue = rol<bits>(shiftedValue);
    int currentBit = 1&shiftedValue;
    if(currentBit == 0 && lastBit == 1){
      numberOfOnes = i;
      lastBit = currentBit;
      break;
    }
    lastBit = currentBit;
  }

  for(int i = 1; i <= immsSize+4; i++){//immsSize + 4 just to be safe, could even be a while loop until it finds the offset.
    shiftedValue = rol<bits>(shiftedValue);
    int currentBit = 1&shiftedValue;
    if(currentBit == 1 && lastBit == 0){
      numberOfZeros = i;
      lastBit = currentBit;
      break;
    }
    lastBit = currentBit;
  }

  offset+=numberOfOnes;
  offset%=immsSize;

  // cout << "offset: " << offset << endl;
  // cout << "immsSize: " << immsSize << endl;
  // cout << "numberOfOnes: " << numberOfOnes << endl;
  // cout << "numberOfZeros: " << numberOfZeros << endl;

  if(!isExactSqrt(immsSize)){ // check if it is a power of two
    return {false, 0, 0};
  }

  if(numberOfOnes+numberOfZeros!=immsSize){
    return {false, 0, 0};
  }

  unsigned long long imms = (bitmask << (exactSqrt(immsSize)+1))&bitmask;
  imms |= numberOfOnes-1;

  return {true, offset, imms};
}

template<int bits>
tuple<bool, long, long, long> n_immr_imms_Encode(unsigned long long targetValue, int lenImmr, int lenImms){
  lenImms++;
  long n = 0;
  auto tmp = immr_imms_Encode<bits>(targetValue, lenImmr, lenImms);
  auto imms = get<2>(tmp);
  unsigned long bitmask = 1<<(lenImms-1);
  if((imms&bitmask)==0) n=1;//n is opposite;
  imms&=~bitmask;
  return tuple<bool, long, long, long>(get<0>(tmp), n, get<1>(tmp), imms);
}

string n_immr_imms_Encode(unsigned long long targetValue, bool is64){
  tuple<bool, long, long, long> tmp;
  if(is64)
    tmp = n_immr_imms_Encode<64>(targetValue, 6, 6);
  else
    tmp = n_immr_imms_Encode<32>(targetValue, 6, 6);
  if(!get<0>(tmp)){
    return "";
  }
  auto n = get<1>(tmp);
  auto imms = get<3>(tmp);
  auto immr = get<2>(tmp);
  return bitset<1>(n).to_string() + bitset<6>(immr).to_string() + bitset<6>(imms).to_string();
}
