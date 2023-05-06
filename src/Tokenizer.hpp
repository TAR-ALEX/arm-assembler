//
//  Tokenizer.hpp
//  scat
//
//  Created by Alex Tarasov on 2/5/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.
//

#ifndef tokenizer_hpp
#define tokenizer_hpp

#include "Logging.tpp"
#include <fstream>
#include <vector>

using namespace std;

struct Token{
  string location;
  string value;
  Token(){
    location = "";
    value = "";
  }
  Token(string s){
    location = "";
    value = s;
  }
  Token(string s, string c){
    location = c;
    value = s;
  }
  string toString(){
    return value + " //"+ location;
  }
};

struct Tokenizer{
  Logging log;
  vector<Token> tokenize(istream &infile);
};

#endif /* tokenizer_hpp */
