#include "Mifare.h"

static byte packetbuffer[PN532_PACKBUFFSIZE];

boolean Mifare::SAMConfig(void) {
//    packetbuffer[0] = PN532_COMMAND_SAMCONFIGURATION;
//    packetbuffer[1] = 0x01; // normal mode;
//    packetbuffer[2] = 0x14; // timeout 50ms * 20 = 1 second
//    packetbuffer[3] = 0x01; // use IRQ pin!
//    
//    if (! board.sendCommandCheckAck(packetbuffer, 4))
//        return false;
//    
//    // read data packet
//    board.readspidata(packetbuffer, 8);
//    
//    return  (packetbuffer[5] == 0x15);
}