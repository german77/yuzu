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
#include "core/hle/result.h"

namespace Service::HID {

enum class DataValid : u32_le {
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
    u8 sub;
    u8 main;
};

struct FactoryCalibration {
    s32_le os_max;
    s32_le hk_max;
    s32_le zero_min;
    s32_le zero_max;
};

struct UserCalibration {
    s16_le os_max;
    u16 os_max_crc;
    s16_le hk_max;
    u16 hk_crc;
    s16_le zero;
    u16 zero_crc;
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
        ReadId = 0x00020100,
        JoyPolling = 0x00020101,
        Unknown1 = 0x00020104,
        c20105 = 0x00020105,
        Unknown2 = 0x00020204,
        Unknown3 = 0x00020304,
        Unknown4 = 0x00020404,
        ReadUnkCal = 0x00020504,
        ReadManualCal = 0x00020A04,
        Unknown5 = 0x00021104,
        Unknown6 = 0x00021204,
        Unknown7 = 0x00021304,
        ReadUserCal = 0x00021A04,
        Unknown8 = 0x00023104,
        ReadTotalPushCount = 0x00023204,
        Unknown9 = 0x04013104,
        Unknown10 = 0x04011104,
        Unknown11 = 0x04011204,
        Unknown12 = 0x04011304,
        Error = 0xFFFFFFFF,
    };

    struct FirmwareVersionReply {
        DataValid status;
        FirmwareVersion firmware;
        INSERT_PADDING_BYTES(0x2);
    };
    static_assert(sizeof(FirmwareVersionReply) == 0x8, "FirmwareVersionReply is an invalid size");

    struct Cmd020105Reply {
        DataValid status;
        u8 data;
        INSERT_PADDING_BYTES(0x3);
    };
    static_assert(sizeof(Cmd020105Reply) == 0x8, "Cmd020105Reply is an invalid size");

    struct GetThreeByteReply {
        DataValid status;
        std::array<u8, 3> data;
        u8 crc;
    };
    static_assert(sizeof(GetThreeByteReply) == 0x8, "GetThreeByteReply is an invalid size");

    struct ReadUnkCalReply {
        DataValid status;
        u16_le data;
        INSERT_PADDING_BYTES(0x2);
    };
    static_assert(sizeof(ReadUnkCalReply) == 0x8, "ReadUnkCalReply is an invalid size");

    struct ReadManualCalReply {
        DataValid status;
        FactoryCalibration calibration;
    };
    static_assert(sizeof(ReadManualCalReply) == 0x14, "ReadManualCalReply is an invalid size");

    struct ReadUserCalReply {
        DataValid status;
        UserCalibration calibration;
        INSERT_PADDING_BYTES(0x4);
    };
    static_assert(sizeof(ReadUserCalReply) == 0x14, "ReadUserCalReply is an invalid size");

    struct ReadIdReply {
        DataValid status;
        u16_le id_l_x0;
        u16_le id_l_x0_2;
        u16_le id_l_x4;
        u16_le id_h_x0;
        u16_le id_h_x0_2;
        u16_le id_h_x4;
    };
    static_assert(sizeof(ReadIdReply) == 0x10, "ReadIdReply is an invalid size");

    enum class JoyPollingMode : u32 {
        SixAxisSensorDisable,
        SixAxisSensorEnable,
        ButtonOnly,
    };

    struct HidbusStatusManagerEntry {
        u8 is_connected{};
        INSERT_PADDING_BYTES(0x3);
        ResultCode is_connected_result{0};
        u8 is_enabled{};
        u8 is_in_focus{};
        u8 is_polling_mode{};
        u8 reserved{};
        JoyPollingMode polling_mode{};
        std::array<u8, 0x70> data{};
    };
    static_assert(sizeof(HidbusStatusManagerEntry) == 0x80,
                  "HidbusStatusManagerEntry is an invalid size");

    struct JoyButtonOnlyPollingData {
        std::array<u8, 0x2c> data;
        u8 out_size;
        INSERT_PADDING_BYTES(0x3);
        u64 sampling_number;
    };
    static_assert(sizeof(JoyButtonOnlyPollingData) == 0x38,
                  "JoyButtonOnlyPollingData is an invalid size");

    struct JoyButtonOnlyPollingEntry {
        u64 sampling_number;
        JoyButtonOnlyPollingData data;
    };
    static_assert(sizeof(JoyButtonOnlyPollingEntry) == 0x40,
                  "JoyButtonOnlyPollingEntry is an invalid size");

    struct DataAccessorHeader {
        ResultCode result{RESULT_UNKNOWN};
        INSERT_PADDING_WORDS(0x1);
        std::array<u8, 0x18> unused;
        u64 latest_entry;
        u64 total_entries;
    };
    static_assert(sizeof(DataAccessorHeader) == 0x30, "DataAccessorHeader is an invalid size");

    struct ButtonOnlyPollingDataAccessor {
        DataAccessorHeader header;
        std::array<JoyButtonOnlyPollingEntry, 0xb> entries;
    };

    u8 GetCrcValue(const std::vector<u8>& data) const;

    void GetFirmwareVersionReply(std::vector<u8>& data);
    void GetC020105Reply(std::vector<u8>& data);
    void GetReadTotalPushCountReply(std::vector<u8>& data);
    void GetReadUnkCalReply(std::vector<u8>& data);
    void GetReadManualCalReply(std::vector<u8>& data);
    void GetReadUserCalReply(std::vector<u8>& data);
    void GetReadIdReply(std::vector<u8>& data);

    RingConCommands command;
    HidbusStatusManagerEntry hidbus_entry{};
    ButtonOnlyPollingDataAccessor button_data{};
};
} // namespace Service::HID
