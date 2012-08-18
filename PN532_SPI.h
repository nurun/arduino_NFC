#include "PN532_Com.h"
#include "PN532.h"

#define PN532_SPI_STATREAD                  (0x02)
#define PN532_SPI_DATAWRITE                 (0x01)
#define PN532_SPI_DATAREAD                  (0x03)
#define PN532_SPI_READY                     (0x01)


class PN532_SPI : public PN532{
public:
    PN532_SPI(uint8_t clk, uint8_t miso, uint8_t mosi, uint8_t ss);
    void     begin(void);
    uint32_t getFirmwareVersion(void);
    
    boolean readack(void);

    boolean sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout = 1000);
    
private:
    uint8_t _clk, _mosi, _miso, _ss;

    uint8_t readstatus(void);
    void    readdata(uint8_t* buff, uint8_t n);
    void    sendcommand(uint8_t* cmd, uint8_t cmdlen);
    
    void    spiwrite(uint8_t x);
    uint8_t spiread(void);
};