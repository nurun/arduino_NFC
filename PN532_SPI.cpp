#include "PN532_SPI.h"

static const byte PN532_ACK[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
static const byte PN532_RESPONSE_FIRMWARE_VERSION[6] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
static byte packetbuffer[PN532_PACKBUFFSIZE];

/**************************************************************************/
/*!
 @brief  Instantiates a new PN532 SPI class
 
 @param  CLK    
 @param  MISO    
 @param  MOSI     
 @param  SS       
*/
/**************************************************************************/

PN532_SPI::PN532_SPI(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t ss) {
    _clk = clk;
    _miso = miso;
    _mosi = mosi;
    _ss = ss;
    
    pinMode(_clk, OUTPUT);
    pinMode(_miso, INPUT);
    pinMode(_ss, OUTPUT);
    pinMode(_mosi, OUTPUT);
}

/**************************************************************************/
/*!
 @brief  Setups the HW
 */
/**************************************************************************/

void PN532_SPI::begin() {
    digitalWrite(_ss, LOW);
    
    delay(1000);
    
    // not exactly sure why but we have to send a dummy command to get synced up
    packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    sendCommandCheckAck(packetbuffer, 1);
}


/**************************************************************************/
/*!
 @brief  Checks the firmware version of the PN5xx chip
 
 @returns  The chip's firmware version and ID
 */
/**************************************************************************/

uint32_t PN532_SPI::getFirmwareVersion(void) {
    uint32_t response;
    
    packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    
    if (! sendCommandCheckAck(packetbuffer, 1))
        return 0;
    
    // read data packet
    readdata(packetbuffer, 12);
    // check some basic stuff
    if (0 != strncmp((char *)packetbuffer, (char *)PN532_RESPONSE_FIRMWARE_VERSION, 6)) {
        return 0;
    }
    
    response = packetbuffer[6];
    response <<= 8;
    response |= packetbuffer[7];
    response <<= 8;
    response |= packetbuffer[8];
    response <<= 8;
    response |= packetbuffer[9];
    
    return response;
}

/**************************************************************************/
/*!
 @brief  Tries to read the PN532 ACK frame 
 (not to be confused with the ACK signal)
 */
/**************************************************************************/

boolean PN532_SPI::readack() {
    uint8_t ackbuff[6];
    
    readdata(ackbuff, 6);
    
    return (0 == strncmp((char *)ackbuff, (char *)PN532_ACK, 6));
}

/**************************************************************************/
/*!
 @brief  Sends a command and waits a specified period for the ACK
 
 @param  cmd       Pointer to the command buffer
 @param  cmdlen    The size of the command in bytes
 @param  timeout   timeout before giving up
 
 @returns  1 if everything is OK, 0 if timeout occured before an
 ACK was recieved
 */
/**************************************************************************/

// default timeout of one second
boolean PN532_SPI::sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {
    uint16_t timer = 0;
    
    // write the command
    sendcommand(cmd, cmdlen);
    
    // Wait for chip to say its ready!
    while (readstatus() != PN532_SPI_READY) {
        if (timeout != 0) {
            timer+=10;
            if (timer > timeout)
                return false;
        }
        delay(10);
    }
    
    // read acknowledgement
    if (!readack()) {
        return false;
    }
    
    timer = 0;
    // Wait for chip to say its ready!
    while (readstatus() != PN532_SPI_READY) {
        if (timeout != 0) {
            timer+=10;
            if (timer > timeout)
                return false;
        }
        delay(10);
    }
    
    return true;
}



/**************************************************************************/
/*!
 @brief  Checks if the PN532 is ready
 
 @returns 0 if the PN532 is busy, 1 if it is free
 */
/**************************************************************************/

uint8_t PN532_SPI::readstatus(void) {
    digitalWrite(_ss, LOW);
    delay(2);
    spiwrite(PN532_SPI_STATREAD);
    // read byte
    uint8_t x = spiread();
    
    digitalWrite(_ss, HIGH);
    return x;
}

/**************************************************************************/
/*!
 @brief  Reads n bytes of data from the PN532 via SPI
 
 @param  buff      Pointer to the buffer where data will be written
 @param  n         Number of bytes to be read
 */
/**************************************************************************/

void PN532_SPI::readdata(uint8_t* buff, uint8_t n) {
    digitalWrite(_ss, LOW);
    delay(2);
    spiwrite(PN532_SPI_DATAREAD);
    
#ifdef PN532DEBUG
    Serial.print("Reading: ");
#endif
    for (uint8_t i=0; i<n; i++) {
        delay(1);
        buff[i] = spiread();
#ifdef PN532DEBUG
        Serial.print(" 0x");
        Serial.print(buff[i], HEX);
#endif
    }
    
#ifdef PN532DEBUG
    Serial.println();
#endif
    
    digitalWrite(_ss, HIGH);
}

/**************************************************************************/
/*!
 @brief  Writes a command to the PN532, automatically inserting the
 preamble and required frame details (checksum, len, etc.)
 
 @param  cmd       Pointer to the command buffer
 @param  cmdlen    Command length in bytes
 */
/**************************************************************************/

void PN532_SPI::sendcommand(uint8_t* cmd, uint8_t cmdlen) {
    uint8_t checksum;
    
    cmdlen++;
    
#ifdef PN532DEBUG
    Serial.print("\nSending: ");
#endif
    
    digitalWrite(_ss, LOW);
    delay(2);     // or whatever the delay is for waking up the board
    spiwrite(PN532_SPI_DATAWRITE);
    
    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    spiwrite(PN532_PREAMBLE);
    spiwrite(PN532_PREAMBLE);
    spiwrite(PN532_STARTCODE2);
    
    spiwrite(cmdlen);
    uint8_t cmdlen_1=~cmdlen + 1;
    spiwrite(cmdlen_1);
    
    spiwrite(PN532_HOSTTOPN532);
    checksum += PN532_HOSTTOPN532;
    
#ifdef PN532DEBUG
    Serial.print(" 0x"); Serial.print(PN532_PREAMBLE, HEX);
    Serial.print(" 0x"); Serial.print(PN532_PREAMBLE, HEX);
    Serial.print(" 0x"); Serial.print(PN532_STARTCODE2, HEX);
    Serial.print(" 0x"); Serial.print(cmdlen, HEX);
    Serial.print(" 0x"); Serial.print(cmdlen_1, HEX);
    Serial.print(" 0x"); Serial.print(PN532_HOSTTOPN532, HEX);
#endif
    
    for (uint8_t i=0; i<cmdlen-1; i++) {
        spiwrite(cmd[i]);
        checksum += cmd[i];
#ifdef PN532DEBUG
        Serial.print(" 0x"); Serial.print(cmd[i], HEX);
#endif
    }
    uint8_t checksum_1=~checksum;
    spiwrite(checksum_1);
    spiwrite(PN532_POSTAMBLE);
    digitalWrite(_ss, HIGH);
    
#ifdef PN532DEBUG
    Serial.print(" 0x"); Serial.print(checksum_1, HEX);
    Serial.print(" 0x"); Serial.print(PN532_POSTAMBLE, HEX);
    Serial.println();
#endif
}

/**************************************************************************/
/*!
 @brief  Sends a single byte via SPI
 
 @param  x    The byte to send
 */
/**************************************************************************/


void PN532_SPI::spiwrite(uint8_t x) {
    int8_t i;
    digitalWrite(_clk, HIGH);
    
    for (i=0; i<8; i++) {
        digitalWrite(_clk, LOW);
        if (x & _BV(i)) {
            digitalWrite(_mosi, HIGH);
        } else {
            digitalWrite(_mosi, LOW);
        }
        digitalWrite(_clk, HIGH);
    }
}

/**************************************************************************/
/*!
 @brief  Reads a single byte via SPI
 */
/**************************************************************************/

uint8_t PN532_SPI::spiread(void) {
    int8_t i, x;
    x = 0;
    digitalWrite(_clk, HIGH);
    
    for (i=0; i<8; i++) {
        if (digitalRead(_miso)) {
            x |= _BV(i);
        }
        digitalWrite(_clk, LOW);
        digitalWrite(_clk, HIGH);
    }
    return x;
}
