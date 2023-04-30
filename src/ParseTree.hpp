//
//  ParseTree.hpp
//  scat
//
//  Created by Alex Tarasov on 2/5/20.
//  Copyright © 2020 Alex Tarasov. All rights reserved.
//

#ifndef ParseTree_hpp
#define ParseTree_hpp

#include "Tokenizer.hpp"
#include "Logging.tpp"
#include <map>
#include <string>
#include <vector>
#include <set>

using namespace std;

class Element;
class Statement;

class Tuple {
public:
  Tuple();
  Tuple(vector<pair<string, Element>> e);
  vector<pair<string, Element>> elements;
};

class Statement {
public:
  Statement();
  Statement(vector<Element> e);
  vector<Element> elements;
};

class Element {
public:
  Element();
  Element(string v);
  Element(Tuple t);
  Element(Statement s);

  bool isNull = false;
  string location = "";
  Tuple *tuple = nullptr;
  Statement *statement = nullptr;
  string *value = nullptr;
  string toString();

  int size();
  bool onlyContains(set<string> names);
  bool contains(string name);
  bool contains(int id);
  Element operator[](string name);
  Element operator[](int id);

  bool isTuple();
  bool isStatement();
  bool isString();
  bool isInt();
  bool isName();
  bool isSingle();

  string getString();
  long long getInt();
  string getName();
  Element getSingle();

  Element overwriteLocation(string l);
};

class ParseTreeBuilder{
  long findMatchingBracket(vector<Token> &tokens, long i);
  bool isTuple(vector<Token> &tokens, long i);
  Element parseStatement(vector<Token> &tokens, long &i);
  Element parseTuple(vector<Token> &tokens, long &i);
  // Tuple normalizeTuple(Tuple t);
  // Statement normalizeStatement(Statement s);

public:
  Logging log;
  Element buildParseTree(vector<Token> &vector);
  Element normalize(Element t);
};

extern Element nullElement;
extern Statement nullStatement;
extern Tuple nullTuple;

#endif /* ParseTree_hpp */
