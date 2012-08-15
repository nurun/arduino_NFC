#define PN532_SPI_STATREAD                  (0x02)
#define PN532_SPI_DATAWRITE                 (0x01)
#define PN532_SPI_DATAREAD                  (0x03)
#define PN532_SPI_READY                     (0x01)


class PN532_I2C{
public:
    PN532(uint8_t cs, uint8_t clk, uint8_t mosi, uint8_t miso);
    void begin(void);
    uint32_t getFirmwareVersion(void);
    
}