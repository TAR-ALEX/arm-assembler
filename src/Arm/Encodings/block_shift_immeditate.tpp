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
#include "block_shift_immeditate.hpp"
#include "util.hpp"

using namespace std;

//returns {shift, immediate}
pair<string, string> block_shift_immeditate_Encode(long long targetValue, int shiftOptions, int blockSize){
  int shiftSize = getHighestSetBitLocation(shiftOptions-1);

  int maxBit = getHighestSetBitLocation(targetValue);
  if(maxBit < blockSize) maxBit = blockSize;

  // round down to get shift amount
  if(maxBit%blockSize == 0) maxBit-=1;
  int shiftAmount = (maxBit/blockSize)*blockSize;
  if(shiftAmount/blockSize >= shiftOptions) return {"",""};

  if(targetValue>>shiftAmount<<shiftAmount == targetValue){
    return pair<string, string>{
      bitset<64>(shiftAmount/blockSize).to_string().substr(64-shiftSize, shiftSize),
      bitset<64>(targetValue>>shiftAmount).to_string().substr(64-blockSize, blockSize)
    };
  }
  return {"",""};
}

//returns {flipBits, shift, immediate}
tuple<bool, string, string> block_shift_flip_immeditate_Encode(long long targetValue, int shiftOptions, int blockSize){
  bool flipBits = false;
  auto t = block_shift_immeditate_Encode(targetValue, shiftOptions, blockSize);
  if(t.first == "" || t.second == ""){
    flipBits = true;
    t = block_shift_immeditate_Encode(~targetValue, shiftOptions, blockSize);
  }
  if(t.first == "" || t.second == "") flipBits = false;
  return tuple<bool, string, string>{flipBits, t.first, t.second};
}
