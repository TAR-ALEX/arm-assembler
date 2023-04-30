#include <string>
#include <bitset>
#include "util.hpp"

using namespace std;

string multiple_of_Encode(unsigned long long targetValue, int zeroBits, int totalBits){
  if(targetValue >> zeroBits << zeroBits != targetValue) return ""; // does not have trailing zeros
  targetValue >>= zeroBits;
  if(
    (targetValue | ones(totalBits)) != ones(totalBits) &&
    (targetValue | ones(totalBits)) != ones(64-zeroBits)
  ) return ""; // value too big

  return bitset<64>(targetValue).to_string().substr(64-totalBits, totalBits);
}
