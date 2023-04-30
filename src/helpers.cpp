//
//  helpers.cpp
//  scat
//
//  Created by Alex Tarasov on 2/11/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.
//

#include "helpers.hpp"
#include <cstring>
#include <string>

using namespace std;

string indent(string input, string indentation) {
  string output = "";
  for (int i = 0; i < input.length(); i++) {
    if (i == 0) {
      output += indentation;
    }
    output += input[i];
    if (input[i] == '\n' && i < input.length() - 1) {
      output += indentation;
    }
  }
  return output;
}
