/**************************************************************************/
/*! 
	@file     PN532_I2C.h
	@author   Odopod Inc.
	@license  BSD
	
	This code is refactored by Odopod (http://www.odopod.com) based on the 
	Adafruit NFC ShieldI2C Library (https://github.com/adafruit/Adafruit_NFCShield_I2C).
*/
/**************************************************************************/

#ifndef __PN532_I2C_INCLUDED__
#define __PN532_I2C_INCLUDED__

#include "PN532_Com.h"

#include "wire.h"

#define PN532_I2C_ADDRESS                   (0x48 >> 1)
#define PN532_I2C_READBIT                   (0x01)
#define PN532_I2C_READYTIMEOUT              (20)


class PN532_I2C : public PN532{
public:	
    PN532_I2C(uint8_t irq, uint8_t reset);
    void     begin(void);
    uint32_t getFirmwareVersion(void);
    
    boolean readack(void);
    
    boolean sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout = 1000);
    
	uint8_t readstatus(void);
	void    readdata(uint8_t* buffer, uint8_t length);
    void    sendcommand(uint8_t* cmd, uint8_t cmdlen);
	
private:
    uint8_t _irq, _reset;
   
    void    wiresend(uint8_t x);
    uint8_t wirerecv(void);
};

#endif
