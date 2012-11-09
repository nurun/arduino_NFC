/**************************************************************************/
/*! 
	@file     Mifare.h
	@author   Odopod, a Nurun Company
	@license  BSD
*/
/**************************************************************************/

#ifndef __MIFARE_INCLUDED__
#define __MIFARE_INCLUDED__

#include "PN532_Com.h"

#define MIFARE_ISO14443A              (0x00)

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE_CLASSIC            (0xA0)
#define MIFARE_CMD_WRITE_ULTRALIGHT         (0xA2)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)
#define STOP_BYTE                           (0XFE)

#define MIFARE_CLASSIC      0x000408 /* ATQA 00 04	 SAK 08 */
#define MIFARE_ULTRALIGHT   0x004400 /* ATQA 00 44	 SAK 00 */

#define KEY_A	1
#define KEY_B	2

//#define MIFAREDEBUG 1

extern PN532 * board;

class Mifare{
  public:
	Mifare();
    
    static uint8_t keyA[6];
    static uint8_t keyB[6];
    static uint8_t useKey;
    static uint32_t cardType;
    
	boolean SAMConfig(void);
    uint8_t* readTarget(uint16_t timeout = 0);
    
    boolean readPayload(uint8_t * output , uint8_t lengthLimit);
    boolean writePayload(uint8_t * payload, uint8_t length);
    
  private:
    boolean classic_formatForNDEF(void);
    boolean classic_authenticateBlock (uint32_t blockNumber);
    
    boolean classic_readPayload(uint8_t * output, uint8_t lengthLimit);
    boolean classic_writePayload(uint8_t * payload, uint8_t length);
    boolean classic_readMemoryBlock(uint8_t blockaddress, uint8_t * block);
    boolean classic_writeMemoryBlock(uint8_t blockaddress, uint8_t * block);
    
    boolean ultralight_readPayload(uint8_t * output, uint8_t lengthLimit);
    boolean ultralight_writePayload(uint8_t * payload, uint8_t length);
    boolean ultralight_readMemoryBlock(uint8_t blockaddress, uint8_t *block);
    boolean ultralight_writeMemoryBlock(uint8_t blockaddress, uint8_t *block);
    
};

#endif