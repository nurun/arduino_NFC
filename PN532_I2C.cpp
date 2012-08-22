#include "PN532_I2C.h"

static const byte PN532_ACK[6] = {0x00, 0x00, 0xFF, 0x00, 0xFF, 0x00};
static const byte PN532_RESPONSE_FIRMWARE_VERSION[6] = {0x00, 0xFF, 0x06, 0xFA, 0xD5, 0x03};
static byte packetbuffer[PN532_PACKBUFFSIZE];

/**************************************************************************/
/*!
 @brief  Instantiates a new PN532 I2C class
 
 @param  irq       Location of the IRQ pin
 @param  reset     Location of the RSTPD_N pin
 */
/**************************************************************************/
PN532_I2C::PN532_I2C(uint8_t irq, uint8_t reset) {
    _irq = irq;
    _reset = reset;
    
    pinMode(_irq, INPUT);
    pinMode(_reset, OUTPUT);
}


/**************************************************************************/
/*!
 @brief  Setups the HW
 */
/**************************************************************************/
void PN532_I2C::begin(void) {
    Wire.begin();
    
    // Reset the PN532
    digitalWrite(_reset, HIGH);
    digitalWrite(_reset, LOW);
    delay(400);
    digitalWrite(_reset, HIGH);
}


/**************************************************************************/
/*!
 @brief  Checks the firmware version of the PN5xx chip
 
 @returns  The chip's firmware version and ID
 */
/**************************************************************************/
uint32_t PN532_I2C::getFirmwareVersion(void) {
    uint32_t response;
    
    packetbuffer[0] = PN532_COMMAND_GETFIRMWAREVERSION;
    
    if (! sendCommandCheckAck(packetbuffer, 1))
        return 0;
	
    // read data packet
    readdata(packetbuffer, 12);
    
    // check some basic stuff
    if (0 != strncmp((char *)packetbuffer, (char *)PN532_RESPONSE_FIRMWARE_VERSION, 6)) {
#ifdef PN532DEBUG
        Serial.println("Firmware doesn't match!");
#endif
        return 0;
    }
    
    response = packetbuffer[7];
    response <<= 8;
    response |= packetbuffer[8];
    response <<= 8;
    response |= packetbuffer[9];
    response <<= 8;
    response |= packetbuffer[10];
    
    return response;
}

/**************************************************************************/
/*!
 @brief  Tries to read the PN532 ACK frame
 (not to be confused with the ACK signal)
 */
/**************************************************************************/
boolean PN532_I2C::readack(void) {
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
boolean PN532_I2C::sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout) {
    uint16_t timer = 0;
    
    // write the command
    sendcommand(cmd, cmdlen);
    
    // Wait for chip to say its ready!
    while (readstatus() != PN532_READY) {
        if (timeout != 0) {
            timer+=10;
            if (timer > timeout)
                return false;
        }
        delay(80);
    }
    
#ifdef PN532DEBUG
    Serial.println("IRQ received");
#endif
    
    // read acknowledgement
    if (!readack()) {
#ifdef PN532DEBUG
        Serial.println("No ACK frame received!");
#endif
        return false;
    }
    
    return true; // ack'd command
}



/**************************************************************************/
/*!
 @brief  Checks if the PN532 is ready
 
 @returns 0 if the PN532 is busy, 1 if it is free
 */
/**************************************************************************/
uint8_t PN532_I2C::readstatus(void) {
    uint8_t x = digitalRead(_irq);
    
    if (x == 1)
        return PN532_BUSY;
    else
        return PN532_READY;
}

/**************************************************************************/
/*!
 @brief  Reads n bytes of data from the PN532 via I2C
 
 @param  buff      Pointer to the buffer where data will be written
 @param  n         Number of bytes to be read
 */
/**************************************************************************/
void PN532_I2C::readdata(uint8_t* buff, uint8_t n) {
    uint16_t timer = 0;
    
    delay(2);
    
#ifdef PN532DEBUG
    Serial.print("Reading: ");
#endif
    // Start read (n+1 to take into account leading 0x01 with I2C)
    Wire.requestFrom((uint8_t)PN532_I2C_ADDRESS, (uint8_t)(n+2));
    // Discard the leading 0x01
    wirerecv();
    for (uint8_t i=0; i<n; i++) {
        delay(1);
        buff[i] = wirerecv();
#ifdef PN532DEBUG
        Serial.print(" 0x");
        Serial.print(buff[i], HEX);
#endif
    }
    // Discard trailing 0x00 0x00
    // wirerecv();
    
#ifdef PN532DEBUG
    Serial.println();
#endif
}

/**************************************************************************/
/*!
 @brief  Writes a command to the PN532, automatically inserting the
 preamble and required frame details (checksum, len, etc.)
 
 @param  cmd       Pointer to the command buffer
 @param  cmdlen    Command length in bytes
 */
/**************************************************************************/
void PN532_I2C::sendcommand(uint8_t* cmd, uint8_t cmdlen) {
    uint8_t checksum;
    
    cmdlen++;
    
#ifdef PN532DEBUG
    Serial.print("\nSending: ");
    Serial.print(cmdlen);
#endif
    
    delay(2);     // or whatever the delay is for waking up the board
    
    // I2C START
    Wire.beginTransmission(PN532_I2C_ADDRESS);
    checksum = PN532_PREAMBLE + PN532_PREAMBLE + PN532_STARTCODE2;
    wiresend(PN532_PREAMBLE);
    wiresend(PN532_PREAMBLE);
    wiresend(PN532_STARTCODE2);
    
    wiresend(cmdlen);
    wiresend(~cmdlen + 1);
    
    wiresend(PN532_HOSTTOPN532);
    checksum += PN532_HOSTTOPN532;
    
#ifdef PN532DEBUG
    Serial.print(" 0x"); Serial.print(PN532_PREAMBLE, HEX);
    Serial.print(" 0x"); Serial.print(PN532_PREAMBLE, HEX);
    Serial.print(" 0x"); Serial.print(PN532_STARTCODE2, HEX);
    Serial.print(" 0x"); Serial.print(cmdlen, HEX);
    Serial.print(" 0x"); Serial.print(~cmdlen + 1, HEX);
    Serial.print(" 0x"); Serial.print(PN532_HOSTTOPN532, HEX);
#endif
    
    for (uint8_t i=0; i<cmdlen-1; i++) {
        wiresend(cmd[i]);
        checksum += cmd[i];
#ifdef PN532DEBUG
        Serial.print(" _0x"); Serial.print(cmd[i], HEX);
#endif
    }
    
    wiresend(~checksum);
    wiresend(PN532_POSTAMBLE);  // I2C STOP
    Wire.endTransmission();
    
#ifdef PN532DEBUG
    Serial.print(" 0x"); Serial.print(~checksum, HEX);
    Serial.print(" 0x"); Serial.print(PN532_POSTAMBLE, HEX);
    Serial.println();
#endif
} 

/**************************************************************************/
/*!
 @brief  Sends a single byte via I2C
 
 @param  x    The byte to send
 */
/**************************************************************************/
void PN532_I2C::wiresend(uint8_t x){
#if ARDUINO >= 100
    Wire.write((uint8_t)x);
#else
    Wire.send(x);
#endif
}

/**************************************************************************/
/*!
 @brief  Reads a single byte via I2C
 */
/**************************************************************************/
uint8_t PN532_I2C::wirerecv(void){
#if ARDUINO >= 100
    return Wire.read();
#else
    return Wire.receive();
#endif
}

