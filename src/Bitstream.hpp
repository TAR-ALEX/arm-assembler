#ifndef bitstream_hpp
#define bitstream_hpp

#include <vector>
#include <string>

using namespace std;

class Endian{
public:
  static const int Little = 0;
  static const int Big = 1;
};

class Bitstream{
private:
  vector<unsigned char> word;
  vector<unsigned char> bytes;
  unsigned char nextByte = 0;
  unsigned char nextByteIndex = 0;
public:
  int endian;
  Bitstream(int endian = Endian::Little);
  Bitstream& operator<<(string vs);

  //Bitstream& operator<<(bool v);
  Bitstream& operator<<(int v);

  //Bitstream& operator<<(vector<bool> vs);
  Bitstream& operator<<(vector<int> vs);
  vector<unsigned char> getBits();
  string getHexDWords();
};

#endif
