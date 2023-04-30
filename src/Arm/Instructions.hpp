#ifndef ARM_INSTRUCTIONS
#define ARM_INSTRUCTIONS

#include <map>
#include <iostream>
#include <iomanip>
#include <bitset>

using namespace std;

#include "Arm.hpp"
#include "Constants.hpp"
#include "SystemRegisters.hpp"
#include "Helpers.hpp"
#include "Encodings/n_imms_immr.hpp"
#include "Encodings/block_shift_immeditate.hpp"
#include "Encodings/multiple_of.hpp"

bool Arm64::assembleAddFamily(Element& s, string subname){
  if(!assertCommandName(s, subname)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  string destinationRegisterBits = "00000";
  string leftRegisterBits = "00000";
  string rightRegisterBits = "00000";

  string opType = "0";
  string includeS = "0";

  auto operandRegisterMap = registerNameToBitsWithZR;
  auto destinationRegisterMap = registerNameToBitsWithZR;


  if(command == "subs" || command == "adds" || command == "cmn" || command == "cmp") includeS = "1";
  if(command == "subs" || command == "sub"  || command == "cmp") opType = "1";

  if(containsDifferentSizedRegisters(arguments, {"left", "right", "destination"}) || countRegisterExtenstion(arguments, {"left", "right", "destination"})){
    operandRegisterMap = registerNameToBitsWithSP;
    if(command == "sub" || command == "add")
      destinationRegisterMap = registerNameToBitsWithSP;

    if(command == "cmn" || command == "cmp")
      destinationRegisterBits = "11111";
    else
      destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, destinationRegisterMap);

    int regSize = -1;
    string shiftBits = "000";
    int sygned = 0;
    if(getRegisterSize(arguments, "left", true) != commandSize || containsRegisterExtenstion(arguments, "left")){
      regSize = getRegisterSize(arguments, "left", true);
      rightRegisterBits = assertRegister(arguments, "left", true, -1, registerNameToBitsSignExtendableWithZR);
      leftRegisterBits = assertRegister(arguments, "right", false, commandSize, operandRegisterMap);
      if(containsRegisterShift(arguments, "left"))
        shiftBits = assertRegisterShift4(arguments, "left");
      if(containsRegisterExtenstion(arguments, "left"))
        sygned = assertRegisterExtenstion(arguments, "left");
    }else{
      if(command == "sub" || command == "subs" || command == "cmp")
        log << arguments["left"].location <<" [error] command " << command << " cannot contain immediates in key left\n";
      regSize = getRegisterSize(arguments, "right", true);
      rightRegisterBits = assertRegister(arguments, "right", true, -1, registerNameToBitsSignExtendableWithZR);
      leftRegisterBits = assertRegister(arguments, "left", false, commandSize, operandRegisterMap);
      if(containsRegisterShift(arguments, "right"))
        shiftBits = assertRegisterShift4(arguments, "right");
      if(containsRegisterExtenstion(arguments, "right"))
        sygned = assertRegisterExtenstion(arguments, "right");
    }

    string option = "00";

    if(regSize == 8) option = "00";
    if(regSize == 16) option = "01";
    if(regSize == 32) option = "10";
    if(regSize == 64) option = "11";

    bs << commandSizeBit << opType << includeS << "01011 00 1" << rightRegisterBits;
    bs << sygned << option << shiftBits << leftRegisterBits << destinationRegisterBits;
  }else if(countImmediates(arguments, {"left", "right"})){
    operandRegisterMap = registerNameToBitsWithSP;
    if(command == "sub" || command == "add")
      destinationRegisterMap = registerNameToBitsWithSP;

    if(command == "cmn" || command == "cmp")
      destinationRegisterBits = "11111";
    else
      destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, destinationRegisterMap);

    pair<string, string> rightImmediateBits;
    if(containsImmediate(arguments, "right")){
      rightImmediateBits = assertImmediate12(arguments, "right");
      leftRegisterBits = assertRegister(arguments, "left", false, commandSize, operandRegisterMap);
    }else{
      if(command == "sub" || command == "subs" || command == "cmp")
        log << arguments["left"].location <<" [error] command " << command << " cannot contain immediates in key left\n";
      rightImmediateBits = assertImmediate12(arguments, "left");
      leftRegisterBits = assertRegister(arguments, "right", false, commandSize, operandRegisterMap);
    }
    bs << commandSizeBit << opType << includeS << "10001" << rightImmediateBits.first;
    bs << rightImmediateBits.second << leftRegisterBits;
    bs << destinationRegisterBits;
  }else{
    pair<string, string> rightRegisterShiftBits = {"00", "000000"};

    if(command == "cmn" || command == "cmp")
      destinationRegisterBits = "11111";
    else
      destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, destinationRegisterMap);

    if(containsRegisterShift(arguments, "right")){
      rightRegisterShiftBits = assertRegisterShift(arguments, "right", commandSize, shiftTypeMap2);
      rightRegisterBits = assertRegister(arguments, "right", true, commandSize, operandRegisterMap);
      leftRegisterBits = assertRegister(arguments, "left", false, commandSize, operandRegisterMap);
    }else if(containsRegisterShift(arguments, "left")){
      if(command == "sub" || command == "subs" || command == "cmp")
        log << arguments["left"].location <<" [error] command " << command << " cannot contain register shift in key left\n";
      rightRegisterShiftBits = assertRegisterShift(arguments, "left", commandSize, shiftTypeMap2);
      rightRegisterBits = assertRegister(arguments, "left", true, commandSize, operandRegisterMap);
      leftRegisterBits = assertRegister(arguments, "right", false, commandSize, operandRegisterMap);
    }else{
      rightRegisterBits = assertRegister(arguments, "right", false, commandSize, operandRegisterMap);
      leftRegisterBits = assertRegister(arguments, "left", false, commandSize, operandRegisterMap);
    }
    bs << commandSizeBit << opType << includeS << "01011" << rightRegisterShiftBits.first;
    bs << "0" << rightRegisterBits << rightRegisterShiftBits.second;
    bs << leftRegisterBits << destinationRegisterBits;
  }
  return true;
}

bool Arm64::assembleAdd(Element& s){
  return assembleAddFamily(s, "add");
}

bool Arm64::assembleSub(Element& s){
  return assembleAddFamily(s, "sub");
}

bool Arm64::assembleAdds(Element& s){
  return assembleAddFamily(s, "adds");
}

bool Arm64::assembleSubs(Element& s){
  return assembleAddFamily(s, "subs");
}

bool Arm64::assembleCmn(Element& s){
  return assembleAddFamily(s, "cmn");
}

bool Arm64::assembleCmp(Element& s){
  return assembleAddFamily(s, "cmp");
}

bool Arm64::assembleAdcFamily(Element& s, string subname){
  if(!assertCommandName(s, subname)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);

  string opType = "0";
  string includeS = "0";

  if(command == "sbcs" || command == "adcs") includeS = "1";
  if(command == "sbcs" || command == "sbc") opType = "1";

  bs << commandSizeBit << opType << includeS << "11010000" << rightRegisterBits;
  bs << "000000" << leftRegisterBits << destinationRegisterBits;

  return true;
}

bool Arm64::assembleAdc(Element& s){
  return assembleAdcFamily(s, "adc");
}

bool Arm64::assembleSbc(Element& s){
  return assembleAdcFamily(s, "sbc");
}

bool Arm64::assembleAdcs(Element& s){
  return assembleAdcFamily(s, "adcs");
}

bool Arm64::assembleSbcs(Element& s){
  return assembleAdcFamily(s, "sbcs");
}

bool Arm64::assembleMov(Element& s){
  if(!assertCommandName(s, "mov")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"value", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  if(containsStackPointer(arguments, {"value", "destination"})){
    auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithSP);
    auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithSP);
    bs << commandSizeBit << "00 10001 00 000000000000" << valueRegisterBits << destinationRegisterBits;
    return true;
  }
  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);

  if(containsImmediate(arguments, "value")){
    auto immediateValue = assertImmediate(arguments, "value");
    auto blockEncoding = block_shift_flip_immeditate_Encode(immediateValue, 4, 16);
    if(get<1>(blockEncoding) != ""){
      bs << commandSizeBit << !get<0>(blockEncoding) << "0 100101" << get<1>(blockEncoding) << get<2>(blockEncoding) << destinationRegisterBits;
      return true;
    }
    auto immsEncoding = n_immr_imms_Encode(immediateValue, commandSize == 64);
    if(immsEncoding != ""){
      bs << commandSizeBit << "01 100100" << immsEncoding << "11111" << destinationRegisterBits;
      return true;
    }
    log << arguments.location <<" [error] could not encode immediate value with block_shift_flip_immeditate_Encode or n_immr_imms_Encode\n";
    bs << nopStatement;
  } else{
    auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
    bs << commandSizeBit << "01 01010 00 0" << valueRegisterBits << "000000 11111" << destinationRegisterBits;
  }
  return true;
}

bool Arm64::assembleSvc(Element& s){
  if(!assertCommandName(s, "svc")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto valueImm = assertImmediate(arguments, "value");

  auto encodedVal = multiple_of_Encode(valueImm, 0, 16);
  if(encodedVal == ""){
    log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
    bs << nopStatement;
    return true;
  }

  bs << "11010100000" << encodedVal << "000 01";

  return true;
}

bool Arm64::assembleAdr(Element& s){
  if(!assertCommandName(s, "adr")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto valueImm = assertImmediate(arguments, "value");
  valueImm = valueImm - currentOffset;
  auto destinationRegisterBits = assertRegister(arguments, "destination", false, 64, registerNameToBitsWithZR);

  auto encodedVal = multiple_of_Encode(valueImm, 0, 21);
  if(encodedVal == ""){
    log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
    bs << nopStatement;
    return true;
  }

  bs << "0" << encodedVal.substr(21-2, 2) << "10000" << encodedVal.substr(0, 19) << destinationRegisterBits;

  return true;
}

bool Arm64::assembleAdrp(Element& s){
  if(!assertCommandName(s, "adrp")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto valueImm = assertImmediate(arguments, "value");
  valueImm = valueImm - currentOffset;
  auto destinationRegisterBits = assertRegister(arguments, "destination", false, 64, registerNameToBitsWithZR);

  auto encodedVal = multiple_of_Encode(valueImm, 12, 21);
  if(encodedVal == ""){
    log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
    bs << nopStatement;
    return true;
  }

  bs << "1" << encodedVal.substr(21-2, 2) << "10000" << encodedVal.substr(0, 19) << destinationRegisterBits;

  return true;
}

bool Arm64::assembleAndFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  if(cmd != "tst" && !arguments.onlyContains({"left", "right", "destination"}))
    log << arguments.location <<  "[error] arguments should only contain [left, right, destination] for command " << cmd << "\n";
  if(cmd == "tst" && !arguments.onlyContains({"left", "right"}))
    log << arguments.location <<  "[error] arguments should only contain [left, right] for command " << cmd << "\n";

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  string opc = "00";
  if(cmd == "ands" || cmd == "tst") opc = "11";
  if(cmd == "eor") opc = "10";
  if(cmd == "orr") opc = "01";
  //if(cmd == "ands") opc = "11";

  auto regShfMap = registerNameToBitsWithSP;
  if(cmd == "ands" || cmd == "tst" || !countImmediates(arguments, {"left", "right", "destination"})) regShfMap = registerNameToBitsWithZR;

  string destinationRegisterBits = "11111";
  if(cmd != "tst")
    destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, regShfMap);

  if(containsImmediate(arguments, "right")){
    auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
    auto rightImmediate = assertImmediate(arguments, "right");
    auto rightImmediateEncoded = n_immr_imms_Encode(rightImmediate, commandSize == 64);
    if(rightImmediateEncoded == ""){
      log << arguments.location <<  "[error] could not encode immediate value with n_immr_imms_Encode\n";
      bs << nopStatement;
      return true;
    }
    bs << commandSizeBit << opc << "100100" << rightImmediateEncoded << leftRegisterBits << destinationRegisterBits;
  }else if(containsImmediate(arguments, "left")){
    auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
    auto leftImmediate = assertImmediate(arguments, "left");
    auto leftImmediateEncoded = n_immr_imms_Encode(leftImmediate, commandSize == 64);
    if(leftImmediateEncoded == ""){
      log << arguments.location <<  "[error] could not encode immediate value with n_immr_imms_Encode\n";
      bs << nopStatement;
      return true;
    }
    bs << commandSizeBit << opc << "100100" << leftImmediateEncoded << rightRegisterBits << destinationRegisterBits;
  }else if(containsRegisterShift(arguments, "right")){
    auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
    auto rightRegisterBits = assertRegister(arguments, "right", true, commandSize, registerNameToBitsWithZR);
    auto rightRegisterShiftBits = assertRegisterShift(arguments, "right", commandSize, shiftTypeMap2And);
    bs << commandSizeBit << opc << "01010" << rightRegisterShiftBits.first << "0" << rightRegisterBits;
    bs << rightRegisterShiftBits.second << leftRegisterBits << destinationRegisterBits;
  }else if(containsRegisterShift(arguments, "left")){
    auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
    auto leftRegisterBits = assertRegister(arguments, "left", true, commandSize, registerNameToBitsWithZR);
    auto leftRegisterShiftBits = assertRegisterShift(arguments, "left", commandSize, shiftTypeMap2And);
    bs << commandSizeBit << opc << "01010" << leftRegisterShiftBits.first << "0" << leftRegisterBits;
    bs << leftRegisterShiftBits.second << rightRegisterBits << destinationRegisterBits;
  }else{
    auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
    auto leftRegisterBits = assertRegister(arguments, "left", true, commandSize, registerNameToBitsWithZR);
    bs << commandSizeBit << opc << "01010" << "00" << "0" << rightRegisterBits;
    bs << "000000" << leftRegisterBits << destinationRegisterBits;
  }

  return true;
}

bool Arm64::assembleAnd(Element& s){
  return assembleAndFamily(s, "and");
}

bool Arm64::assembleAnds(Element& s){
  return assembleAndFamily(s, "ands");
}

bool Arm64::assembleOrr(Element& s){
  return assembleAndFamily(s, "orr");
}

bool Arm64::assembleEor(Element& s){
  return assembleAndFamily(s, "eor");
}

bool Arm64::assembleTst(Element& s){
  return assembleAndFamily(s, "tst");
}

bool Arm64::assembleAsrFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"value", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  string opCode2 = "00";
  if(cmd == "lsr") opCode2 = "01";
  if(cmd == "asr") opCode2 = "10";
  if(cmd == "ror") opCode2 = "11";

  string opCode0 = "10";
  if(cmd == "lsr") opCode0 = "10";
  if(cmd == "asr") opCode0 = "00";

  if(containsImmediate(arguments, "shift")){
    auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
    auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
    auto shiftValue = assertImmediate(arguments, "shift");

    string imms = commandSizeBit+"11111";
    string immr = "000000";

    if(shiftValue < 0 || shiftValue >= commandSize){
      cout << commandSize;
      shiftValue = commandSize-1;
      log << arguments.location <<  "[error] shift is out of bounds\n";
    }

    if(cmd == "ror"){
      imms = multiple_of_Encode(shiftValue, 0, 6);
      if(imms == ""){
        imms = "000000";
        log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
      }
    }else if(cmd == "lsl"){
      imms=bitset<6>((commandSize -1 - shiftValue)%commandSize).to_string();
      if(imms == "011111" || imms == "111111") log << arguments.location <<  "[error] imms cannot contain x11111\n";
      immr = bitset<6>((commandSize-shiftValue)%commandSize).to_string();
    }else{
      immr = multiple_of_Encode(shiftValue, 0, 6);
      if(immr == ""){
        immr = "000000";
        log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
      }
    }

    if(cmd != "ror")
      bs << commandSizeBit << opCode0 << "100110" << commandSizeBit << immr << imms << valueRegisterBits << destinationRegisterBits;
    else
      bs << commandSizeBit << "00" << "100111" << commandSizeBit << "0" << valueRegisterBits << imms << valueRegisterBits << destinationRegisterBits;
    return true;
  }
  auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
  auto shiftRegisterBits = assertRegister(arguments, "shift", false, commandSize, registerNameToBitsWithZR);
  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);

  bs << commandSizeBit << "00" << "11010110" << shiftRegisterBits << "0010" << opCode2 << valueRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleAsr(Element& s){
  return assembleAsrFamily(s, "asr");
}

bool Arm64::assembleLsl(Element& s){
  return assembleAsrFamily(s, "lsl");
}

bool Arm64::assembleLsr(Element& s){
  return assembleAsrFamily(s, "lsr");
}

bool Arm64::assembleRor(Element& s){
  return assembleAsrFamily(s, "ror");
}


bool Arm64::assembleBicFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  string opCode = "00";
  if(cmd == "bics") opCode = "11";
  if(cmd == "orn") opCode = "01";
  if(cmd == "eon") opCode = "10";

  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  string rightRegisterBits = "00000";
  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  pair<string, string> rightRegisterShift = {"00", "000000"};

  if(containsRegisterShift(arguments, "right")){
    rightRegisterBits = assertRegister(arguments, "right", true, commandSize, registerNameToBitsWithZR);
    rightRegisterShift = assertRegisterShift(arguments, "right", commandSize, shiftTypeMap2And);
  }else{
    rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
  }

  bs << commandSizeBit << opCode << "01010" << rightRegisterShift.first;
  bs << "1" << rightRegisterBits << rightRegisterShift.second;
  bs << leftRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleBic(Element& s){
  return assembleBicFamily(s, "bic");
}

bool Arm64::assembleBics(Element& s){
  return assembleBicFamily(s, "bics");
}

bool Arm64::assembleEon(Element& s){
  return assembleBicFamily(s, "eon");
}

bool Arm64::assembleOrn(Element& s){
  return assembleBicFamily(s, "orn");
}

bool Arm64::assembleAsrv(Element& s){
  if(!assertCommandName(s, "asrv")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
  auto shiftRegisterBits = assertRegister(arguments, "shift", false, commandSize, registerNameToBitsWithZR);
  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);

  bs << commandSizeBit << "00" << "11010110" << shiftRegisterBits << "0010" << "10" << valueRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleMvn(Element& s){
  if(!assertCommandName(s, "mvn")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"value", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  string valueRegisterBits = "00000";
  pair<string, string> valueRegisterShiftBits = {"00","000000"};
  if(containsRegisterShift(arguments, "value")){
    valueRegisterBits = assertRegister(arguments, "value", true, commandSize, registerNameToBitsWithZR);
    valueRegisterShiftBits = assertRegisterShift(arguments, "value", commandSize, shiftTypeMap2And);
  }else{
    valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
  }

  bs << commandSizeBit << "01" << "01010" << valueRegisterShiftBits.first;
  bs << "1" << valueRegisterBits << valueRegisterShiftBits.second << "11111";
  bs << destinationRegisterBits;
  return true;
}

bool Arm64::assembleWfe(Element& s){
  if(!assertCommandName(s, "wfe")) return false;
  auto command = s[0].getName();
  if(s.size() != 1){
    log << s.location <<  "[error] "<<command<<" cannot have any arguments"<<"\n";
  }
  bs << "1101010100 0 00 011 0010 0000 010 11111";
  return true;
}

bool Arm64::assembleNop(Element& s){
  if(!assertCommandName(s, "nop")) return false;
  auto command = s[0].getName();
  if(s.size() != 1){
    log << s.location <<  "[error] "<<command<<" cannot have any arguments"<<"\n";
  }
  bs << nopStatement;
  return true;
}

bool Arm64::assembleMsr(Element& s){
  if(!assertCommandName(s, "msr")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = 64;
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  if(containsImmediate(arguments, "value")){
    log << arguments.location <<  "[error] immediates are currently not supported\n";
    bs<<nopStatement;
    return 1;
  }

  auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
  auto destinationRegisterBits = assertSystemRegister(arguments, "destination");

  bs << "11010101000" << destinationRegisterBits << valueRegisterBits;
  return true;
}

bool Arm64::assembleMrs(Element& s){
  if(!assertCommandName(s, "mrs")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = 64;
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto valueRegisterBits = assertSystemRegister(arguments, "value");

  bs << "11010101001" << valueRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleMadd(Element& s){
  if(!assertCommandName(s, "madd")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "addTo", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
  auto postRegisterBits = assertRegister(arguments, "addTo", false, commandSize, registerNameToBitsWithZR);

  bs << commandSizeBit << "00 11011 000 " << rightRegisterBits << "0" << postRegisterBits << leftRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleMul(Element& s){
  if(!assertCommandName(s, "mul")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);

  bs << commandSizeBit << "00 11011 000 " << rightRegisterBits << "0" << "11111" << leftRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleMsub(Element& s){
  if(!assertCommandName(s, "msub")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "subFrom", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
  auto postRegisterBits = assertRegister(arguments, "subFrom", false, commandSize, registerNameToBitsWithZR);

  bs << commandSizeBit << "00 11011 000 " << rightRegisterBits << "1" << postRegisterBits << leftRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleMneg(Element& s){
  if(!assertCommandName(s, "mneg")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);

  bs << commandSizeBit << "00 11011 000 " << rightRegisterBits << "1" << "11111" << leftRegisterBits << destinationRegisterBits;
  return true;
}

bool Arm64::assembleNeg(Element& s){
  return assembleNegFamily(s, "neg");
}

bool Arm64::assembleNegs(Element& s){
  return assembleNegFamily(s, "negs");
}

bool Arm64::assembleNegFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"value", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
  pair<string, string> valueRegisterShiftBits = {"00", "000000"};

  if(containsRegisterShift(arguments, "value"))
    valueRegisterShiftBits = assertRegisterShift(arguments, "value", commandSize, shiftTypeMap2);

  string includeS = "0";
  if(cmd == "negs") includeS = "1";


  bs << commandSizeBit << "1" << includeS << "01011";
  bs << valueRegisterShiftBits.first << "0" << valueRegisterBits;
  bs << valueRegisterShiftBits.second << "11111" << destinationRegisterBits;
  return true;
}

bool Arm64::assembleNgcFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"value", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);

  string includeS = "0";
  if(cmd == "ngcs") includeS = "1";

  bs << commandSizeBit << "1" << includeS << "11010000";
  bs << valueRegisterBits << "000000 11111" << destinationRegisterBits;

  return true;
}

bool Arm64::assembleNgc(Element& s){
  return assembleNgcFamily(s, "ngc");
}

bool Arm64::assembleNgcs(Element& s){
  return assembleNgcFamily(s, "ngcs");
}

bool Arm64::assembleDivFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);

  string signedBit = "0";
  if(command == "sdiv") signedBit = "1";

  bs << commandSizeBit << "00 11010110" << rightRegisterBits << "00001" << signedBit << leftRegisterBits << destinationRegisterBits;
  return true;
}
bool Arm64::assembleSdiv(Element& s){
  return assembleDivFamily(s, "sdiv");
}
bool Arm64::assembleUdiv(Element& s){
  return assembleDivFamily(s, "udiv");
}

bool Arm64::assembleSmaddlFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, 64, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, 32, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, 32, registerNameToBitsWithZR);
  auto postRegisterBits = assertRegister(arguments, "addTo", false, 64, registerNameToBitsWithZR);

  string unsignedBit = "0";
  if(command == "umaddl") unsignedBit = "1";

  bs << "1 00 11011" << unsignedBit << "01" << rightRegisterBits << "0";
  bs << postRegisterBits << leftRegisterBits << destinationRegisterBits;
  return true;
}
bool Arm64::assembleSmaddl(Element& s){
  return assembleSmaddlFamily(s, "smaddl");
}
bool Arm64::assembleUmaddl(Element& s){
  return assembleSmaddlFamily(s, "umaddl");
}

bool Arm64::assembleUmsublFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, 64, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, 32, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, 32, registerNameToBitsWithZR);
  auto postRegisterBits = assertRegister(arguments, "subFrom", false, 64, registerNameToBitsWithZR);

  string unsignedBit = "0";
  if(command == "umsubl") unsignedBit = "1";

  bs << "1 00 11011" << unsignedBit << "01" << rightRegisterBits << "1";
  bs << postRegisterBits << leftRegisterBits << destinationRegisterBits;
  return true;
}
bool Arm64::assembleUmsubl(Element& s){
  return assembleUmsublFamily(s, "umsubl");
}
bool Arm64::assembleSmsubl(Element& s){
  return assembleUmsublFamily(s, "smsubl");
}

bool Arm64::assembleUmneglFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, 64, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, 32, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, 32, registerNameToBitsWithZR);

  string unsignedBit = "0";
  if(command == "umnegl") unsignedBit = "1";

  bs << "1 00 11011" << unsignedBit << "01" << rightRegisterBits << "1";
  bs << "11111" << leftRegisterBits << destinationRegisterBits;
  return true;
}
bool Arm64::assembleUmnegl(Element& s){
  return assembleUmneglFamily(s, "umnegl");
}
bool Arm64::assembleSmnegl(Element& s){
  return assembleUmneglFamily(s, "smnegl");
}

bool Arm64::assembleUmulhFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, 64, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, 32, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, 32, registerNameToBitsWithZR);

  string unsignedBit = "0";
  if(command == "umulh") unsignedBit = "1";

  bs << "1 00 11011" << unsignedBit << "10" << rightRegisterBits << "0";
  bs << "11111" << leftRegisterBits << destinationRegisterBits;
  return true;
}
bool Arm64::assembleUmulh(Element& s){
  return assembleUmulhFamily(s, "umulh");
}
bool Arm64::assembleSmulh(Element& s){
  return assembleUmulhFamily(s, "smulh");
}

bool Arm64::assembleUmullFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, 64, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, 32, registerNameToBitsWithZR);
  auto leftRegisterBits = assertRegister(arguments, "left", false, 32, registerNameToBitsWithZR);

  string unsignedBit = "0";
  if(command == "umull") unsignedBit = "1";

  bs << "1 00 11011" << unsignedBit << "01" << rightRegisterBits << "0";
  bs << "11111" << leftRegisterBits << destinationRegisterBits;
  return true;
}
bool Arm64::assembleUmull(Element& s){
  return assembleUmullFamily(s, "umull");
}
bool Arm64::assembleSmull(Element& s){
  return assembleUmullFamily(s, "smull");
}

bool Arm64::assembleMovkFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  long long immediateValue = 0;
  long long shiftValue = 0;

  if(arguments["value"][0][0].isInt()){
    immediateValue = arguments["value"][0][0].getInt();
    if(arguments["value"][0].contains("shift")){
      if(
        arguments["value"][0]["shift"][0][0].getName() != "lsl" ||
        !arguments["value"][0]["shift"][0][1].isInt() ||
        arguments["value"][0]["shift"][0][1].getInt()%16
      ){
        log << arguments["value"][0]["shift"].location <<  "[error] only shift type supported is [lsl, 0/16/32/48]\n";
      }
      shiftValue = arguments["value"][0]["shift"][0][1].getInt();
    }
  }else{
    immediateValue = arguments["value"].getInt();
  }

  if(shiftValue >= commandSize){
    log << arguments["value"][0]["shift"].location <<  "[error] shift cannot exceed command size of "<<commandSize<<" bits\n";
  }

  auto encodedVal = multiple_of_Encode(immediateValue, 0, 16);
  auto encodedShift = bitset<2>(shiftValue/16).to_string();

  if(encodedVal == ""){
    encodedVal = "0000000000000000";
    log << arguments["value"].location <<  "[error] could not encode value with multiple_of_Encode()\n";
  }

  string opc = "11";
  if(cmd == "movn") opc = "00";
  if(cmd == "movz") opc = "10";

  bs << commandSizeBit<< opc << "100101" << encodedShift << encodedVal << destinationRegisterBits;
  return true;
}

bool Arm64::assembleMovk(Element& s){
  return assembleMovkFamily(s, "movk");
}

bool Arm64::assembleMovn(Element& s){
  return assembleMovkFamily(s, "movn");
}

bool Arm64::assembleMovz(Element& s){
  return assembleMovkFamily(s, "movz");
}

inline bool Arm64::assembleLdrFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"destination"});

  // register size restrictions per command
  if(cmd == "ldrsw") commandSize = 64;
  if(cmd == "ldrh") commandSize = 32;
  if(cmd == "ldrb") commandSize = 32;

  int loadSizeSqt = 4;

  string commandSizeBit = (commandSize == 64 ? "1" : "0");
  string flippedCommandSizeBit = (commandSize == 64 ? "0" : "1");

  string commandSizeBits = "10";
  if(cmd == "ldr"){
    commandSizeBits = "1"+commandSizeBit;
  }else if(cmd == "ldrsw"){ // ldrw dos not exist, but could be added for fun. (same as ldr)
    commandSizeBits = "10";
  }else if(cmd == "ldrh" || cmd == "ldrsh"){
    commandSizeBits = "01";
  }else if(cmd == "ldrb" || cmd == "ldrsb"){
    commandSizeBits = "00";
  }

  string opc1 = "01";

  if(cmd == "ldr") {
    opc1 = "01";
    loadSizeSqt = commandSize/32+1;
  }
  if(cmd == "ldrh") {
    opc1 = "01";
    loadSizeSqt = 1;
  }
  if(cmd == "ldrb") {
    opc1 = "01";
    loadSizeSqt = 0;
  }
  if(cmd == "ldrsw") {
    opc1 = "10";
    loadSizeSqt = 2;
  }
  if(cmd == "ldrsh") {
    opc1 = "1"+flippedCommandSizeBit;
    loadSizeSqt = 1;
  }
  if(cmd == "ldrsb") {
    opc1 = "1"+flippedCommandSizeBit;
    loadSizeSqt = 0;
  }

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);

  if(containsImmediate(arguments, "valueAt") && (cmd == "ldrsw" || cmd == "ldr")){
    auto valueImm = assertImmediate(arguments, "valueAt");
    valueImm = valueImm - currentOffset;

    auto encodedVal = multiple_of_Encode(valueImm, 2, 19);
    if(encodedVal == ""){
      log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
      bs << nopStatement;
      return true;
    }

    bs << (cmd=="ldr"?"0":"1") << (commandSize==64?"1":"0") << "011000" << encodedVal << destinationRegisterBits;
  }else if(containsRegister(arguments, "offset", true, -1)){
    string signExtendBit = "0";
    string shiftBit = "0";
    int extendSize = getRegisterSize(arguments, "offset", true);
    string extendSizeBits = "11";
    if(extendSize == 32) extendSizeBits = "10";
    if(containsRegisterExtenstion(arguments, "offset")){
      signExtendBit = assertRegisterExtenstion(arguments, "offset") ? "1":"0";
    }
    if(containsRegisterShift(arguments, "offset")){
      auto p = assertRegisterShift(arguments, "offset");
      if(p.first != "lsl") log << arguments.location <<  "[error] the only supported shift type is lsl\n";
      if(p.second != loadSizeSqt && p.second != 0) log << arguments.location <<  "[error] shift can only be 0 or "<<loadSizeSqt<<"\n";
      shiftBit = (p.second == 0) ? "0" : "1";
    }

    auto offsetRegisterBits = assertRegister(arguments, "offset", true, -1, registerNameToBitsWithZR);
    auto valueRegisterBits = assertRegister(arguments, "valueAt", false, 64, registerNameToBitsWithSP);

    bs << commandSizeBits << "111 0" << "00" << opc1 << "1" << offsetRegisterBits;
    bs << signExtendBit << extendSizeBits << shiftBit << "10" << valueRegisterBits ;
    bs << destinationRegisterBits;
  }
  else{
    auto valueRegisterBits = assertRegister(arguments, "valueAt", false, 64, registerNameToBitsWithSP);
    string offsetType = "offset";
    string opc0 = "01";
    string opc2 = "01";

    unsigned long long offsetImmValue = 0;
    if(containsImmediate(arguments, "offset") && arguments.onlyContains({"offset", "valueAt", "destination"})){
      offsetType = "offset";
      opc0 = "01";
      offsetImmValue = assertImmediate(arguments, offsetType);
    } else if(containsImmediate(arguments, "preAdd") && arguments.onlyContains({"preAdd", "valueAt", "destination"})){
      offsetType = "preAdd";
      opc0 = "00";
      opc2 = "11";
      offsetImmValue = assertImmediate(arguments, offsetType);
    } else if(containsImmediate(arguments, "postAdd") && arguments.onlyContains({"postAdd", "valueAt", "destination"})){
      offsetType = "postAdd";
      opc0 = "00";
      opc2 = "01";
      offsetImmValue = assertImmediate(arguments, offsetType);
    } else if(arguments.onlyContains({"valueAt", "destination"})){
      offsetType = "offset";
    }else {
      log << arguments.location <<  "[error] can only contain ONE of parameters [postAdd, preAdd, offset]\n";
    }

    if(offsetType != "offset" && (offsetImmValue >= (1<<8) && ~offsetImmValue >= (1<<8))){
      log << arguments[offsetType].location <<  "[error] offset is out of bounds.\n";
    }else if(offsetType == "offset" && offsetImmValue >= (1<<(12+loadSizeSqt))){
      log << arguments[offsetType].location <<  "[error] offset is out of bounds.\n";
    }
    string offsetImmBits = "";
    if(offsetType == "offset") offsetImmBits= multiple_of_Encode(offsetImmValue, loadSizeSqt, 12);
    else offsetImmBits= multiple_of_Encode(offsetImmValue, 0, 9);

    if(offsetType == "offset" && offsetImmBits == ""){ // try to encode as ldur
      offsetImmBits= multiple_of_Encode(offsetImmValue, 0, 9);
      if(offsetImmBits != ""){
        opc0 = "00";
        opc2 = "00";
        offsetType = "unscaled";
      }
    }

    if(offsetImmBits == ""){
      log << arguments.location <<  "[error] could not encode immediate value for "<<offsetType<<"\n";
      if(offsetType == "offset") offsetImmBits = bitset<20>(0).to_string().substr(0,12);
      else offsetImmBits = bitset<20>(0).to_string().substr(0,9);
    }


    if(offsetType == "offset"){
      bs << commandSizeBits << "111 0" << opc0 << opc1 << offsetImmBits << valueRegisterBits << destinationRegisterBits;
    }else{
      bs << commandSizeBits << "111 0" << opc0 << opc1 << "0" << offsetImmBits << opc2 << valueRegisterBits << destinationRegisterBits;
    }
  }

  return true;
}

bool Arm64::assembleLdr(Element& s){
  return assembleLdrFamily(s, "ldr");
}

bool Arm64::assembleLdrsw(Element& s){
  return assembleLdrFamily(s, "ldrsw");
}

bool Arm64::assembleLdrsh(Element& s){
  return assembleLdrFamily(s, "ldrsh");
}

bool Arm64::assembleLdrsb(Element& s){
  return assembleLdrFamily(s, "ldrsb");
}

bool Arm64::assembleLdrh(Element& s){
  return assembleLdrFamily(s, "ldrh");
}

bool Arm64::assembleLdrb(Element& s){
  return assembleLdrFamily(s, "ldrb");
}

inline bool Arm64::assembleStrFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"value"});

  int loadSizeSqt = 4;

  string commandSizeBit = (commandSize == 64 ? "1" : "0");
  string flippedCommandSizeBit = (commandSize == 64 ? "0" : "1");

  string commandSizeBits = "10";
  if(cmd == "str"){
    if(commandSize == 8) commandSizeBits = "00";
    if(commandSize == 16) commandSizeBits = "01";
    if(commandSize == 32) commandSizeBits = "10";
    if(commandSize == 64) commandSizeBits = "11";
  }else if(cmd == "strh"){
    commandSize = 16;
    commandSizeBits = "01";
  }else if(cmd == "strb"){
    commandSize = 8;
    commandSizeBits = "00";
  }


  string offsetType = "offset";
  string opc0 = "01";
  string opc1 = "00";
  string opc2 = "01";

  if(commandSize == 64) loadSizeSqt = 3;
  if(commandSize == 32) loadSizeSqt = 2;
  if(commandSize == 16) loadSizeSqt = 1;
  if(commandSize == 8) loadSizeSqt = 0;

  auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsSignExtendableWithZR);

  if(containsRegister(arguments, "offset", true, -1)){
    string signExtendBit = "0";
    string shiftBit = "0";
    int extendSize = getRegisterSize(arguments, "offset", true);
    string extendSizeBits = "11";
    if(extendSize == 32) extendSizeBits = "10";
    if(containsRegisterExtenstion(arguments, "offset")){
      signExtendBit = assertRegisterExtenstion(arguments, "offset") ? "1":"0";
    }
    if(containsRegisterShift(arguments, "offset")){
      auto p = assertRegisterShift(arguments, "offset");
      if(p.first != "lsl") log << arguments.location <<  "[error] the only supported shift type is lsl\n";
      if(p.second != loadSizeSqt && p.second != 0) log << arguments.location <<  "[error] shift can only be 0 or "<<loadSizeSqt<<"\n";
      shiftBit = (p.second == 0) ? "0" : "1";
    }

    auto offsetRegisterBits = assertRegister(arguments, "offset", true, -1, registerNameToBitsWithZR);
    auto destinationRegisterBits = assertRegister(arguments, "destinationAt", false, 64, registerNameToBitsWithSP);

    bs << commandSizeBits << "111 0" << "00" << opc1 << "1" << offsetRegisterBits;
    bs << signExtendBit << extendSizeBits << shiftBit << "10" << destinationRegisterBits;
    bs << valueRegisterBits;
  }
  else{
    auto destinationRegisterBits = assertRegister(arguments, "destinationAt", false, 64, registerNameToBitsWithSP);

    unsigned long long offsetImmValue = 0;
    if(containsImmediate(arguments, "offset") && arguments.onlyContains({"offset", "destinationAt", "value"})){
      offsetType = "offset";
      opc0 = "01";
      offsetImmValue = assertImmediate(arguments, offsetType);
    } else if(containsImmediate(arguments, "preAdd") && arguments.onlyContains({"preAdd", "destinationAt", "value"})){
      offsetType = "preAdd";
      opc0 = "00";
      opc2 = "11";
      offsetImmValue = assertImmediate(arguments, offsetType);
    } else if(containsImmediate(arguments, "postAdd") && arguments.onlyContains({"postAdd", "destinationAt", "value"})){
      offsetType = "postAdd";
      opc0 = "00";
      opc2 = "01";
      offsetImmValue = assertImmediate(arguments, offsetType);
    } else if(arguments.onlyContains({"destinationAt", "value"})){
      offsetType = "offset";
    }else {
      log << arguments.location <<  "[error] can only contain ONE of parameters [postAdd, preAdd, offset]\n";
    }

    if(offsetType != "offset" && (offsetImmValue >= (1<<8) && ~offsetImmValue >= (1<<8))){
      log << arguments[offsetType].location <<  "[error] offset is out of bounds.\n";
    }else if(offsetType == "offset" && offsetImmValue >= (1<<(12+loadSizeSqt))){
      log << arguments[offsetType].location <<  "[error] offset is out of bounds.\n";
    }
    string offsetImmBits = "";
    if(offsetType == "offset") offsetImmBits= multiple_of_Encode(offsetImmValue, loadSizeSqt, 12);
    else offsetImmBits= multiple_of_Encode(offsetImmValue, 0, 9);

    if(offsetType == "offset" && offsetImmBits == ""){ // try to encode as ldur
      offsetImmBits= multiple_of_Encode(offsetImmValue, 0, 9);
      if(offsetImmBits != ""){
        opc0 = "00";
        opc2 = "00";
        offsetType = "unscaled";
      }
    }

    if(offsetImmBits == ""){
      log << arguments.location <<  "[error] could not encode immediate value for "<<offsetType<<"\n";
      if(offsetType == "offset") offsetImmBits = bitset<20>(0).to_string().substr(0,12);
      else offsetImmBits = bitset<20>(0).to_string().substr(0,9);
    }


    if(offsetType == "offset"){
      bs << commandSizeBits << "111 0" << opc0 << opc1 << offsetImmBits << destinationRegisterBits << valueRegisterBits;
    }else{
      bs << commandSizeBits << "111 0" << opc0 << opc1 << "0" << offsetImmBits << opc2 << destinationRegisterBits << valueRegisterBits;
    }
  }

  return true;
}

bool Arm64::assembleStr(Element& s){
  return assembleStrFamily(s, "str");
}

bool Arm64::assembleStrb(Element& s){
  return assembleStrFamily(s, "strb");
}

bool Arm64::assembleStrh(Element& s){
  return assembleStrFamily(s, "strh");
}

inline bool Arm64::assembleLdpFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"destination"}, true);
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  int sizeSqrt = 2+(commandSize/64);

  string opc0 = commandSizeBit + "0";
  string opc1 = "010";

  if(cmd == "ldpsw"){
    opc0 = "01";
    commandSize = 64;
    sizeSqrt = 2;
  }

  auto destinationRegistersBits = assertRegisterList(arguments, "destination", commandSize, registerNameToBitsWithZR, 2);
  auto valueRegistersBits = assertRegister(arguments, "valueAt", false, commandSize, registerNameToBitsWithSP);

  unsigned long long offsetImmValue = 0;

  if(containsImmediate(arguments, "offset") && arguments.onlyContains({"offset", "valueAt", "destination"})){
    opc1 = "010";
    offsetImmValue = assertImmediate(arguments, "offset");
  } else if(containsImmediate(arguments, "preAdd") && arguments.onlyContains({"preAdd", "valueAt", "destination"})){
    opc1 = "011";
    offsetImmValue = assertImmediate(arguments, "preAdd");
  } else if(containsImmediate(arguments, "postAdd") && arguments.onlyContains({"postAdd", "valueAt", "destination"})){
    opc1 = "001";
    offsetImmValue = assertImmediate(arguments, "postAdd");
  } else if(arguments.onlyContains({"valueAt", "destination"})){
    opc1 = "010";
  }else {
    log << arguments.location <<  "[error] can only contain ONE of parameters [postAdd, preAdd, offset]\n";
  }

  string offsetImmBits = "0000000";
  if(offsetImmValue >= (1<<(6+sizeSqrt)) && ~offsetImmValue >= (1<<(6+sizeSqrt))){
    log << arguments.location <<  "[error] immediate is out of bounds\n";
  }else{
    offsetImmBits= multiple_of_Encode(offsetImmValue, sizeSqrt, 7);
    if(offsetImmBits == ""){
      log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
      offsetImmBits = bitset<7>(0).to_string();
    }
  }

  bs << opc0 << "101 0" << opc1 << "1" << offsetImmBits << destinationRegistersBits[1] << valueRegistersBits << destinationRegistersBits[0];

  return true;
}

bool Arm64::assembleLdp(Element& s){
  return assembleLdpFamily(s, "ldp");
}

bool Arm64::assembleLdpsw(Element& s){
  return assembleLdpFamily(s, "ldpsw");
}

bool Arm64::assembleStp(Element& s){
  if(!assertCommandName(s, "stp")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"value"}, true);
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  int sizeSqrt = 2+(commandSize/64);

  string opc0 = commandSizeBit + "0";
  string opc1 = "010";

  auto valueRegistersBits = assertRegisterList(arguments, "value", commandSize, registerNameToBitsWithZR, 2);
  auto destinationRegisterBits = assertRegister(arguments, "destinationAt", false, commandSize, registerNameToBitsWithSP);

  unsigned long long offsetImmValue = 0;

  if(containsImmediate(arguments, "offset") && arguments.onlyContains({"offset", "destinationAt", "value"})){
    opc1 = "010";
    offsetImmValue = assertImmediate(arguments, "offset");
  } else if(containsImmediate(arguments, "preAdd") && arguments.onlyContains({"preAdd", "destinationAt", "value"})){
    opc1 = "011";
    offsetImmValue = assertImmediate(arguments, "preAdd");
  } else if(containsImmediate(arguments, "postAdd") && arguments.onlyContains({"postAdd", "destinationAt", "value"})){
    opc1 = "001";
    offsetImmValue = assertImmediate(arguments, "postAdd");
  } else if(arguments.onlyContains({"destinationAt", "value"})){
    opc1 = "010";
  }else {
    log << arguments.location <<  "[error] can only contain ONE of parameters [postAdd, preAdd, offset]\n";
  }

  string offsetImmBits = "0000000";
  if(offsetImmValue >= (1<<(6+sizeSqrt)) && ~offsetImmValue >= (1<<(6+sizeSqrt))){
    log << arguments.location <<  "[error] immediate is out of bounds\n";
  }else{
    offsetImmBits= multiple_of_Encode(offsetImmValue, sizeSqrt, 7);
    if(offsetImmBits == ""){
      log << arguments.location <<  "[error] could not encode immediate value with multiple_of_Encode\n";
      offsetImmBits = bitset<7>(0).to_string();
    }
  }

  bs << opc0 << "101 0" << opc1 << "0" << offsetImmBits << valueRegistersBits[1] << destinationRegisterBits << valueRegistersBits[0];

  return true;
}

inline bool Arm64::assembleCcmpFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"left", "right"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  bool opBit = 0;
  if(cmd == "ccmp") opBit = 1;

  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  auto conditionalBits = assertConditional(arguments, "cond");
  auto nzcvValue = assertImmediate(arguments, "nzcv");
  auto nzcvEncoded = assertMultipleOfEncode(nzcvValue, 0, 4, arguments["nzcv"][0].location);

  if(containsImmediate(arguments, "right")){
    long long rightImmediateValue = assertImmediate(arguments, "right");
    if(rightImmediateValue < 0){
      opBit = !opBit;
      rightImmediateValue = -rightImmediateValue;
    }
    auto rightImmediateEncoded = assertMultipleOfEncode(rightImmediateValue, 0, 5, arguments["right"][0].location);
    bs << commandSizeBit << opBit << "1 11010010" << rightImmediateEncoded;
    bs << conditionalBits << "10" << leftRegisterBits << "0";
    bs << nzcvEncoded;
  }else{
    auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);

    bs << commandSizeBit << opBit << "1 11010010" << rightRegisterBits;
    bs << conditionalBits << "00" << leftRegisterBits << "0";
    bs << nzcvEncoded;
  }
  return true;
}

bool Arm64::assembleCcmn(Element& s){
  return assembleCcmpFamily(s, "ccmn");
}

bool Arm64::assembleCcmp(Element& s){
  return assembleCcmpFamily(s, "ccmp");
}

inline bool Arm64::assembleCselFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"destination", "left", "right", "value"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
  auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto conditionalBits = assertConditional(arguments, "cond");

  bool opBit = 0;
  bool op2Bit = 0;

  if(cmd == "csneg"){
    opBit = 1;
    op2Bit = 1;
  }
  if(cmd == "csinc"){
    opBit = 0;
    op2Bit = 1;
  }
  if(cmd == "csinv"){
    opBit = 1;
    op2Bit = 0;
  }

  bs << commandSizeBit << opBit << "0 11010100" << rightRegisterBits;
  bs << conditionalBits << "0" << op2Bit << leftRegisterBits;
  bs << destinationRegisterBits;

  return true;
}

bool Arm64::assembleCsel(Element& s){
  return assembleCselFamily(s, "csel");
}

bool Arm64::assembleCsinv(Element& s){
  return assembleCselFamily(s, "csinv");
}

bool Arm64::assembleCsinc(Element& s){
  return assembleCselFamily(s, "csinc");
}

bool Arm64::assembleCsneg(Element& s){
  return assembleCselFamily(s, "csneg");
}

inline bool Arm64::assembleCsetFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto conditionalBits = assertInverseConditional(arguments, "cond");

  bool opBit = 0;
  bool op2Bit = 1;

  if(cmd == "csetm"){
    opBit = 1;
    op2Bit = 0;
  }

  bs << commandSizeBit << opBit << "0 11010100" << "11111";
  bs << conditionalBits << "0" << op2Bit << "11111";
  bs << destinationRegisterBits;

  return true;
}
bool Arm64::assembleCset(Element& s){
  return assembleCsetFamily(s, "cset");
}
bool Arm64::assembleCsetm(Element& s){
  return assembleCsetFamily(s, "csetm");
}
inline bool Arm64::assembleCincFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto commandSize = getMaxRegisterSize(arguments, {"destination"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
  auto valueRegisterBits = assertRegister(arguments, "value", false, commandSize, registerNameToBitsWithZR);
  auto conditionalBits = assertInverseConditional(arguments, "cond");

  bool opBit = 0;
  bool op2Bit = 1;

  if(cmd == "cinv"){
    opBit = 1;
    op2Bit = 0;
  }
  if(cmd == "cneg"){
    opBit = 1;
    op2Bit = 1;
  }

  bs << commandSizeBit << opBit << "0 11010100" << valueRegisterBits;
  bs << conditionalBits << "0" << op2Bit << valueRegisterBits;
  bs << destinationRegisterBits;

  return true;
}
bool Arm64::assembleCinc(Element& s){
  return assembleCincFamily(s, "cinc");
}
bool Arm64::assembleCinv(Element& s){
  return assembleCincFamily(s, "cinv");
}
bool Arm64::assembleCneg(Element& s){
  return assembleCincFamily(s, "cneg");
}

bool Arm64::assembleB(Element& s){
  if(!assertCommandName(s, "b")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto immediateValue = assertImmediate(arguments, "to")-currentOffset;
  if(containsConditional(arguments, "cond")){
    auto conditionalBits = assertConditional(arguments, "cond");
    assertValueInBounds(immediateValue, true, 21, arguments["to"].location);
    auto encodedImmediate = assertMultipleOfEncode(immediateValue, 2, 19, arguments["to"].location);
    bs << "0101010 0" << encodedImmediate << "0" << conditionalBits;
  }else{
    assertValueInBounds(immediateValue, true, 28, arguments["to"].location);
    auto encodedImmediate = assertMultipleOfEncode(immediateValue, 2, 26, arguments["to"].location);
    bs << "0" << "00101" << encodedImmediate;
  }
  return true;
}

bool Arm64::assembleBl(Element& s){
  if(!assertCommandName(s, "bl")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto immediateValue = assertImmediate(arguments, "to")-currentOffset;
  assertValueInBounds(immediateValue, true, 28, arguments["to"].location);
  auto encodedImmediate = assertMultipleOfEncode(immediateValue, 2, 26, arguments["to"].location);
  bs << "100101" << encodedImmediate;
  return true;
}

bool Arm64::assembleBr(Element& s){
  if(!assertCommandName(s, "br")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto locationRegisterBits = assertRegister(arguments, "to", false, 64, registerNameToBitsWithZR);
  bs << "1101011 0 0 00 11111 0000 0 0" << locationRegisterBits << "00000";
  return true;
}

bool Arm64::assembleBlr(Element& s){
  if(!assertCommandName(s, "blr")) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  auto locationRegisterBits = assertRegister(arguments, "to", false, 64, registerNameToBitsWithZR);
  bs << "1101011 0 0 01 11111 0000 0 0" << locationRegisterBits << "00000";
  return true;
}

inline bool Arm64::assembleCbzFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  string op0 = (cmd=="cbnz"?"1":"0");

  auto commandSize = getMaxRegisterSize(arguments, {"test"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto testRegisterBits = assertRegister(arguments, "test", false, commandSize, registerNameToBitsWithZR);

  auto immediateValue = assertImmediate(arguments, "to")-currentOffset;
  assertValueInBounds(immediateValue, true, 21, arguments["to"].location);
  auto encodedImmediate = assertMultipleOfEncode(immediateValue, 2, 19, arguments["to"].location);
  bs << commandSizeBit << "011010" << op0 << encodedImmediate << testRegisterBits;
  return true;
}

bool Arm64::assembleCbz(Element& s){
  return assembleCbzFamily(s, "cbz");
}

bool Arm64::assembleCbnz(Element& s){
  return assembleCbzFamily(s, "cbnz");
}

bool Arm64::assembleRet(Element& s){
  if(!assertCommandName(s, "ret")) return false;
  auto command = s[0].getName();
  auto arguments = s[1];

  int commandSize = 64;

  auto locationRegisterBits = registerNameToBits["x30"];
  if(!arguments.isNull && containsRegister(arguments, "to", false, -1)){
    locationRegisterBits = assertRegister(arguments, "to", false, commandSize, registerNameToBitsWithZR);
  }

  bs << "1101011 0 0 10 11111 0000 0 0" <<locationRegisterBits<<"00000";
  return true;
}

inline bool Arm64::assembleTbzFamily(Element& s, string cmd){
  if(!assertCommandName(s, cmd)) return false;
  auto command = s[0].getName();
  if(!assertTupleArguments(s, command)) return true;
  auto arguments = s[1];

  string op0 = (cmd=="tbnz"?"1":"0");

  auto commandSize = getMaxRegisterSize(arguments, {"test"});
  string commandSizeBit = (commandSize == 64 ? "1" : "0");

  auto testRegisterBits = assertRegister(arguments, "test", false, commandSize, registerNameToBitsWithZR);

  auto offsetValue = assertImmediate(arguments, "to")-currentOffset;
  assertValueInBounds(offsetValue, true, 16, arguments["to"].location);
  auto encodedOffset = assertMultipleOfEncode(offsetValue, 2, 14, arguments["to"].location);

  auto bitSelectionValue = assertImmediate(arguments, "bit");
  if(bitSelectionValue >= commandSize){
    log << arguments["bit"].location <<  "[error] bit, it must be between 0 and "<<commandSize-1<<"\n";
  }
  auto bitSelectionEncoded = assertMultipleOfEncode(bitSelectionValue, 0, 6, arguments["bit"].location);

  bs << bitSelectionEncoded.substr(0,1) << "0110110" << bitSelectionEncoded.substr(1,5) << encodedOffset << testRegisterBits;
  return true;
}

bool Arm64::assembleTbz(Element& s){
  return assembleTbzFamily(s, "tbz");
}

bool Arm64::assembleTbnz(Element& s){
  return assembleTbzFamily(s, "tbnz");
}



// bool Arm64::assembleMneg(Element& s){
//   if(!assertCommandName(s, "mneg")) return false;
//   auto command = s[0].getName();
//   if(!assertTupleArguments(s, command)) return true;
//   auto arguments = s[1];
//
//   auto commandSize = getMaxRegisterSize(arguments, {"left", "right", "destination"});
//   string commandSizeBit = (commandSize == 64 ? "1" : "0");
//
//   auto destinationRegisterBits = assertRegister(arguments, "destination", false, commandSize, registerNameToBitsWithZR);
//   auto leftRegisterBits = assertRegister(arguments, "left", false, commandSize, registerNameToBitsWithZR);
//   auto rightRegisterBits = assertRegister(arguments, "right", false, commandSize, registerNameToBitsWithZR);
//
//   bs << commandSizeBit << "00 11011 000 " << rightRegisterBits << "1" << "11111" << leftRegisterBits << destinationRegisterBits;
//   return true;
// }





#endif
