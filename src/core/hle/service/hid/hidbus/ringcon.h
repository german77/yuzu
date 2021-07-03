// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include "common/bit_field.h"
#include "common/common_types.h"
#include "common/settings.h"
#include "common/swap.h"
#include "core/frontend/input.h"

namespace Service::HID {

enum class DataValid {
    Valid,
    BadCRC,
    Cal,
};

enum class ErrorFlag {
    BadUserCalUpdate = 0,
    BadFlag = 4,
    BadUserCal = 5,
    BadManualCal = 6,
};

struct FirmwareVersion {
    u8 main;
    u8 sub;
};

struct FactoryCalibration {
    s16 os_max;
    s16 hk_max;
    s16 zero_min;
    s16 zero_max;
};

class RingController {
public:
    explicit RingController();
    ~RingController();

    // When the controller is requesting an update for the shared memory
    void OnUpdate(const Core::Timing::CoreTiming& core_timing, u8* data, std::size_t size);

    u8 GetDeviceId() const;
    void SetCommand(const std::vector<u8>& data);
    void GetReply(std::vector<u8>& data);

private:
    enum class RingConCommands : u32_le {
        GetFirmwareVersion = 0x00020000,
        PlayReport = 0x00020100,
        JoyPolling = 0x00020101,
        Unknown1 = 0x00020104,
        c20105 = 0x00020105,
        Unknown3 = 0x00020204,
        Unknown4 = 0x00020304,
        Unknown5 = 0x00020404,
        ReadUnkCal = 0x00020504,
        ReadManuCal = 0x00020A04,
        Unknown8 = 0x00021104,
        Unknown9 = 0x00021204,
        Unknown10 = 0x00021304,
        ReadUserCal = 0x00021A04,
        Unknown12 = 0x00023104,
        GetTotalPushCount = 0x00023204,
        Unknown14 = 0x04013104,
        Unknown15 = 0x04011104,
        Unknown16 = 0x04011204,
        Unknown17 = 0x04011304,
        error = 0xFFFFFFFF,
    };

    struct FirmwareVersionResponse {
        DataValid status;
        INSERT_PADDING_BYTES(0x3);
        u8 sub;
        u8 main;
        INSERT_PADDING_BYTES(0x2);
    };

    u8 GetCrcValue(const std::vector<u8>& data) const;

    RingConCommands command;
};
} // namespace Service::HID
