#include "MCP4725.h"

bool MCP4725::begin(bool addrJumper){
  if(addrJumper){
    address++;
  }
  Wire.begin();
  Wire.beginTransmission(address);
  byte status = Wire.endTransmission();
  if(status == 0){
    return true;
  }else{
    return false;
  }
}

bool MCP4725::setOutputRaw(int point){

  Wire.beginTransmission(address);
  Wire.write(writeDacReg);
  Wire.write(point >> 4);
  Wire.write((point&15)<<4);
  byte status = Wire.endTransmission();
  if(status == 0){
    setPoint = point;
    return true;
  }else{
    return false;
  }
  
}
