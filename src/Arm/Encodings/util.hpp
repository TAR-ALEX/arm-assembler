#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <bitset>
#include <math.h>
#include "block_shift_immeditate.hpp"

using namespace std;

template <size_t bits>
unsigned long long rol(unsigned long long val, size_t len = 1) {
  return (val << len) | (val >> (-len & (bits - 1)));
}

template <size_t bits>
unsigned long long ror(unsigned long long val, size_t len = 1) {
  return (rol<bits>(val, bits-len));
}

unsigned long long ones(int n){
  unsigned long long result = 1;
  if(n == 64) return -1;
  return (result<<(n))-1;
}

bool isExactSqrt(unsigned long long num){
  return num != 0 && (num & (num-1)) == 0;
}

//maybe slow
int exactSqrt(unsigned long long num){
  for(int i = 0; i < 65; i++){
    num>>=1;
    if(num == 0){
      return i;
    }
  }
  return 0;
}

//may be slow
int getHighestSetBitLocation(long long value){
  int i = 0;
  for(i = 0; i <= sizeof(value)*8; i++){
    if(value == 0) return i;
    value >>= 1;
  }
  return i;
}
