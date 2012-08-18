#include "PN532_Com.h"

#define MIFARE_ISO14443A              (0x00)

// Mifare Commands
#define MIFARE_CMD_AUTH_A                   (0x60)
#define MIFARE_CMD_AUTH_B                   (0x61)
#define MIFARE_CMD_READ                     (0x30)
#define MIFARE_CMD_WRITE                    (0xA0)
#define MIFARE_CMD_TRANSFER                 (0xB0)
#define MIFARE_CMD_DECREMENT                (0xC0)
#define MIFARE_CMD_INCREMENT                (0xC1)
#define MIFARE_CMD_STORE                    (0xC2)

#define MIFARE_CLASSIC      0x000408 /* ATQA 00 04	 SAK 08 */
#define MIFARE_ULTRALIGHT   0x004400 /* ATQA 00 44	 SAK 00 */

class Mifare{
  public:
    uint8_t uid, uidLength, cardType;
    
    boolean readTargetID(void);
    boolean SAMConfig(void);

    uint8_t classic_AuthenticateBlock (int32_t blockNumber, uint8_t keyNumber, uint8_t * keyData);
    
    uint8_t readPayload(void);
    uint8_t writePayload(uint8_t * payload);
       
    uint8_t classic_readMemoryBlock(uint8_t blockaddress, uint8_t * block);
    uint8_t classic_writeMemoryBlock(uint8_t blockaddress, uint8_t * block);
    
    uint8_t ultralight_readMemoryPage(uint8_t pageaddress, uint8_t *page);
    uint8_t ultralight_writeMemoryPage(uint8_t pageaddress, uint8_t *page);
    
};