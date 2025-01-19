#include "SerialIO.h"

class SerialCRSF : public SerialIO {
public:
    explicit SerialCRSF(Stream &out, Stream &in) : SerialIO(&out, &in) {}
    virtual ~SerialCRSF() {}

    uint32_t sendRCFrame(bool frameAvailable, uint32_t *channelData) override;
    uint32_t sendRCFrameEncrypted(bool frameAvailable, uint8_t *channelDataEncrypted);
    void queueMSPFrameTransmission(uint8_t* data) override;
    void queueLinkStatisticsPacket() override;
    void sendQueuedData(uint32_t maxBytesToSend) override;

private:
    void processBytes(uint8_t *bytes, uint16_t size) override;
};
