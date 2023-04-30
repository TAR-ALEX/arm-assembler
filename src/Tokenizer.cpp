//
//  tokenizer.cpp
//  scat
//
//  Created by Alex Tarasov on 2/5/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.
//

#include "Tokenizer.hpp"
#include <string>
#include <sstream>

bool oddBackslashes(string s) {
    bool result = false;
    for(auto i = s.size()-1; i>=0; i--){
        if(s[i] == '\\'){
            result = !result;
        } else {
            return result;
        }
    }
    return result;
}

vector<Token> Tokenizer::tokenize(ifstream& infile) {
    char c;
    bool inString = false;
    vector<Token> tokens;
    Token token;
    int lineNumber = 1;
    int charachterNumber = 0;
    while (infile >> noskipws >> c) {
        if (c == '\n'){
          charachterNumber = 0;
          lineNumber++;
        }else{
          charachterNumber++;
        }
        if (c != ' ' && c != '\n' && token.location == ""){
          stringstream ss;
          ss << lineNumber << "{" << charachterNumber << "}";
          token.location = ss.str();
        }
        if (c == ' ' || c == '\n') {
            if(inString){
                token.value+=c;
            }else{
                if (token.value != "") tokens.push_back(token);
                token.location = "";
                token.value = "";
            }
        } else if (c == '[' || c == ']' || c == ':' || c == ',') {
            if (token.value != "") tokens.push_back(token);
            token.value = c;
            tokens.push_back(token);
            token.location = "";
            token.value = "";
        } else if (c == '"'){
            if(!inString) {
                if (token.value != "") tokens.push_back(token);
                stringstream ss;
                ss << lineNumber << "{" << charachterNumber << "}";
                token.location = ss.str();
                token.value = "";
                token.value += c;
                inString = true;
            }else if(token.value[token.value.size()-1] != '\\' && !oddBackslashes(token.value)) {
                token.value += c;
                if (token.value != "") tokens.push_back(token);
                token.location = "";
                token.value = "";
                inString = false;
            }else{
               token.value += c;
            }
        } else {
            token.value+=c;
        }
    }
    if (token.value != "") tokens.push_back(token);
    return tokens;
}
