#include "PN532_Com.h"
#include "PN532.h"
#include "wire.h"

#define PN532_I2C_ADDRESS                   (0x48 >> 1)
#define PN532_I2C_READBIT                   (0x01)
#define PN532_I2C_BUSY                      (0x00)
#define PN532_I2C_READY                     (0x01)
#define PN532_I2C_READYTIMEOUT              (20)


class PN532_I2C : public PN532{
public:
    PN532_I2C(uint8_t irq, uint8_t reset);
    void     begin(void);
    uint32_t getFirmwareVersion(void);
    
    boolean readack(void);
    
    boolean sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout = 1000);
    
private:
    uint8_t _irq, _reset;
    
    uint8_t readstatus(void);
    void    readdata(uint8_t* buff, uint8_t n);
    void    sendcommand(uint8_t* cmd, uint8_t cmdlen);
    
    void    wiresend(uint8_t x);
    uint8_t wirerecv(void);
};