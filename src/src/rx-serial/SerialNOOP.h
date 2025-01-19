#pragma once

#include "SerialIO.h"

class SerialNOOP : public SerialIO {
public:
    explicit SerialNOOP() : SerialIO(nullptr, nullptr) {}
    virtual ~SerialNOOP() {}

    void queueLinkStatisticsPacket() override {}
    void queueMSPFrameTransmission(uint8_t* data) override {}
    uint32_t sendRCFrame(bool frameAvailable, uint32_t *channelData) override { return  DURATION_NEVER; }
    uint32_t sendRCFrameEncrypted(bool frameAvailable, uint8_t *channelDataEncrypted) override;

    void processSerialInput() override {}

private:
    void processBytes(uint8_t *bytes, uint16_t size) override {}
};
