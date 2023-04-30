//
//  ParseTree.cpp
//  scat
//
//  Created by Alex Tarasov on 2/5/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.
//
#include "ParseTree.hpp"
#include "helpers.hpp"
#include <iostream>

using namespace std;

string indent(string input, string indentation);

string ReplaceAll(string str, const string& from, const string& to) {
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
    }
    return str;
}

Element nullElement = []() { Element n; n.isNull = true; return n; }();

long ParseTreeBuilder::findMatchingBracket(vector<Token> &tokens, long i){
  for (; i < tokens.size(); i++) {
    if (tokens[i].value == "[")
      break;
  }
  i++;
  for (;;) {
    if (i >= tokens.size()) {
      return tokens.size();
    }else if(tokens[i].value == "["){
      i = findMatchingBracket(tokens, i)+1;
    }else if(tokens[i].value == "]"){
      return i;
    }else{
      i++;
    }
  }
}

bool ParseTreeBuilder::isTuple(vector<Token> &tokens, long i){
  for (; i < tokens.size(); i++) {
    if (tokens[i].value == "[")
      break;
  }
  i++;
  for (;;) {
    if (i >= tokens.size()) {
      return false;
    }else if(tokens[i].value == "]"){
      return false;
    }else if(tokens[i].value == ":"){
      return true;
    }else if(tokens[i].value == ","){
      return true;
    }else if(tokens[i].value == "["){
      i = findMatchingBracket(tokens,i)+1;
    }else{
      i++;
    }
  }
}

Element ParseTreeBuilder::parseStatement(vector<Token> &tokens, long &i) {
  Element result;
  result.statement = new Statement();
  if(tokens.size() > i)
    result.location = tokens[i].location;
  for (;;) {
    if (i >= tokens.size()) {
      log << "error parsing statement starting at: " + result.location +"\n";
      break;
    } // error
    if (tokens[i].value == ",")
      break; // done
    if (tokens[i].value == "]")
      break; // done
    if (tokens[i].value == "[") {
      if(isTuple(tokens, i)){
        Element elem = parseTuple(tokens, i);
        result.statement->elements.push_back(elem);
      }else{
        Element elem;
        i++;
        elem = parseStatement(tokens, i);
        i++;
        result.statement->elements.push_back(elem);
      }
    } else {
      Element elem;
      elem.value = new string;
      *elem.value = tokens[i].value;
      elem.location = tokens[i].location;
      result.statement->elements.push_back(elem);
      i++;
    }
  }
  return result;
}

/*
 parses a tuple with the parentheses example:

 ```
 [a: 1, b: 2, c: 3, d: [e: 4, f: 5 + 4],]
 ```
 */
Element ParseTreeBuilder::parseTuple(vector<Token> &tokens, long &i) {
  Element result;
  result.tuple = new Tuple();
  for (; i < tokens.size(); i++) {
    if (tokens[i].value == "["){
      result.location = tokens[i].location;
      break;
    }
  }
  i++;
  for (;;) {
    if (i >= tokens.size()) {
      log << "error parsing tuple starting at: " + result.location +"\n";
      break;
    } // error
    if (tokens[i].value == "]") {
      i++;
      break;
    }
    if (tokens[i].value == ":") {
      log << "error unexpected colon in tuple at: " + tokens[i].location +"\n";
      break;
    } // error
    if (tokens[i].value == ",") {
      i++;
      continue;
    } // skip over
    string name = "";
    Element statement;
    if (i + 1 < tokens.size() && tokens[i + 1].value == ":") {
      name = tokens[i].value;
      i += 2;
      statement = parseStatement(tokens, i);
    } else {
      statement = parseStatement(tokens, i);
    }
    result.tuple->elements.push_back(pair<string, Element>(name, statement));
  }

  return result;
}

Element ParseTreeBuilder::buildParseTree(vector<Token> &tokens) {
  long startIndex = 0;
  return parseTuple(tokens, startIndex);
}

Tuple::Tuple(){}

Tuple::Tuple(vector<pair<string, Element>> e){
  elements = e;
}

Statement::Statement(){}

Statement::Statement(vector<Element> e){
  elements = e;
}

Element::Element(){}

Element::Element(string v){
  value = new string;
  *value = v;
}

Element::Element(Tuple t){
  tuple = new Tuple();
  *tuple = t;
}

Element::Element(Statement s){
  statement = new Statement();
  *statement = s;
}

string Element::toString() {
  // log << "Element::toString()\n";
  if (this->tuple != nullptr) {
    string result = "";
    for (pair<string, Element> namedStatement : this->tuple->elements) {
      if (namedStatement.first != "") {
        result += namedStatement.first;
        result += ": ";
      }
      // else{
      //    result += "<nil>";
      //    result += ": ";
      // }

      result += namedStatement.second.toString();
      result += ", \n";
    }
    if (result == "")
      result = "[]";
    else
      result = "[\n" + indent(result, "   ") + "]";
    return result;
  }else if (this->statement != nullptr) {
    string result = "";
    bool notFirst = false;
    for (Element e : this->statement->elements) {
      if (notFirst) {
        result += " ";
      }
      notFirst = true;
      result += e.toString();
    }
    return "(\n"+indent(result, "   ")+"\n)";
  } else if (this->value != nullptr) {
    return *this->value;
  }
  return "unknown type";
}

// bool Statement::isElement() {
//   return this->elements.size() == 1;
// }

// Element Statement::operator[](int id){
//   if(id >= elements.size()) return nullElement;
//   return elements[id];
// }

// int Statement::size(){
//   return elements.size();
// }
//
// bool Statement::isTuple() {
//   if (!isElement()) return false;
//   return this->elements[0].isTuple();
// }
//
// bool Statement::isString() {
//   if (!isElement()) return false;
//   return this->elements[0].isString();
// }
// bool Statement::isInt() {
//   if (!isElement()) return false;
//   return this->elements[0].isInt();
// }
// bool Statement::isName() {
//   if (!isElement()) return false;
//   return this->elements[0].isName();
// }
//
// Element Statement::getElement() {
//   if (!isElement()) return nullElement;
//   return this->elements[0];
// }
//
// string Statement::getString() {
//   if (!isElement()) return "";
//   return getElement().getString();
// }
//
// string Statement::getName() {
//   if (!isElement()) return "";
//   return getElement().getName();
// }
//
// long long Statement::getInt() {
//   if (!isElement()) return 0;
//   return getElement().getInt();
// }

Element ParseTreeBuilder::normalize(Element t) { return t; }

Element Element::operator[](string name){
  if(tuple != nullptr){
    for (int i = 0; i < this->tuple->elements.size(); i++) {
      if (this->tuple->elements[i].first == name) {
        return this->tuple->elements[i].second;
      }
    }
  }
  return nullElement.overwriteLocation(location);
}

Element Element::operator[](int id){
  if(tuple != nullptr){
    int j = 0;
    for (int i = 0; i < tuple->elements.size(); i++) {
      if(tuple->elements[i].first == "") {
        if(j == id) return tuple->elements[i].second;
        j++;
      }
    }
  } else if(statement != nullptr){
      if(id >= statement->elements.size()) return nullElement;
      return statement->elements[id];
  }
  return nullElement.overwriteLocation(location);
}

int Element::size(){
  if(tuple != nullptr){
    int j = 0;
    for (int i = 0; i < this->tuple->elements.size(); i++) if(this->tuple->elements[i].first == "") j++;
    return j;
  }else if(statement != nullptr){
    return this->statement->elements.size();
  }
  return 0;
}

bool Element::onlyContains(set<string> names){
  if(tuple == nullptr) return false;
  for (int i = 0; i < this->tuple->elements.size(); i++) {
    if (names.find(this->tuple->elements[i].first) == names.end()) {
      return false;
    }
  }
  return true;
}

bool Element::contains(string name){
  if(tuple != nullptr){
    for (int i = 0; i < this->tuple->elements.size(); i++) {
      if (this->tuple->elements[i].first == name) {
        return true;
      }
    }
  }
  return false;
}

bool Element::contains(int id){
  if(tuple != nullptr){
    int j = 0;
    for (int i = 0; i < this->tuple->elements.size(); i++) {
      if(this->tuple->elements[i].first == "") {
        if(j >= id) return true;
        j++;
      }
    }
  }
  return false;
}

bool Element::isSingle(){
  if(statement != nullptr)
    return statement->elements.size() == 1;
  return false;
}

Element Element::getSingle(){
  if(statement != nullptr){
    if(statement->elements.size() != 1){
      return nullElement;
    }else{
      return statement->elements[0];
    }
  }
  return nullElement;
}

bool Element::isString(){
  if(isStatement() && statement->elements.size() == 1) return statement->elements[0].isString();
  return value != nullptr && value->size() != 0 && (*value)[0] == '\"';
}

string Element::getString(){
  string result = "";

  if(isStatement() && statement->elements.size() == 1)
    result = statement->elements[0].getString();

  if(value != nullptr && value->size() != 0 && (*value)[0] == '\"')
    result = *value;

  result = ReplaceAll(result, "\\n", "\n");
  if(result.size() < 2 || result[0] != '"' || result[result.size()-1] != '"') return "";
  return result.substr(1,result.size()-2);
}

bool Element::isInt(){
  if(isStatement() && statement->elements.size() == 1) return statement->elements[0].isInt();
  try{
    if(value != nullptr){
      try{
        stoll(*value);
      }catch(...){
        stoull(*value);
      }
      return true;
    }
    return false;
  }catch(...){
    return false;
  }
}

long long Element::getInt(){
  if(isStatement() && statement->elements.size() == 1)
    return statement->elements[0].getInt();
  try{
    if(value != nullptr){
      try{
        return stoll(*value);
      }catch(...){
        return stoull(*value);
      }
    }
    return 0;
  }catch(...){
    return 0;
  }
}

bool Element::isName(){
    if(isStatement() && statement->elements.size() == 1) return statement->elements[0].isName();
    return value != nullptr && value->size() != 0 && ((*value)[0] < '0' || (*value)[0] > '9');
}

string Element::getName(){
  if(isStatement() && statement->elements.size() == 1)
    return statement->elements[0].getName();
  if(value != nullptr && value->size() != 0 && ((*value)[0] < '0' || (*value)[0] > '9'))
    return *value;
  return "";
}

bool Element::isTuple(){
  return tuple != nullptr;
}

bool Element::isStatement(){
  return statement != nullptr;
}

//To propagate location to null objects
Element Element::overwriteLocation(string l){
  auto clone = *this;
  clone.location = l;
  return clone;
}
