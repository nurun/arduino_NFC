#define PN532_I2C_ADDRESS                   (0x48 >> 1)
#define PN532_I2C_READBIT                   (0x01)
#define PN532_I2C_BUSY                      (0x00)
#define PN532_I2C_READY                     (0x01)
#define PN532_I2C_READYTIMEOUT              (20)

class PN532_I2C{
public:
    PN532_I2C(uint8_t irq, uint8_t reset);
    void begin(void);
    uint32_t getFirmwareVersion(void);

}