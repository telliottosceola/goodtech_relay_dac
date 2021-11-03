#include "arduino.h"
#include "Wire.h"
class MCP4725{
public:

bool begin(bool addrJumper);

bool setOutputRaw(int point);

int setPoint = 2048;

private:

int address = 0x60;

int writeDacReg = 0x40;

};
