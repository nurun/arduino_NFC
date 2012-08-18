class PN532{
public:
    void        begin(void);
    uint32_t    getFirmwareVersion(void);
    boolean     readack(void);
    boolean     sendCommandCheckAck(uint8_t *cmd, uint8_t cmdlen, uint16_t timeout = 1000);
};