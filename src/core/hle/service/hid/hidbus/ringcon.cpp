// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstring>
#include "common/common_types.h"
#include "core/core_timing.h"
#include "core/hle/service/hid/hidbus/ringcon.h"

namespace Service::HID {
constexpr std::size_t SHARED_MEMORY_OFFSET = 0x0;
constexpr u8 DEVICE_ID = 0x20;
constexpr FirmwareVersion version = {0x2c, 0x0};

RingController::RingController() {}
RingController::~RingController() = default;

u8 RingController::GetCrcValue(const std::vector<u8>& data) const {
    u8 crc = 0;
    std::size_t data_len = data.size();
    std::size_t index = 0;
    u32 i;
    bool bit;
    u8 c;

    while (data_len--) {
        c = data[index++];
        for (i = 0x80; i > 0; i >>= 1) {
            bit = crc & 0x80;
            if (c & i) {
                bit = !bit;
            }
            crc <<= 1;
            if (bit) {
                crc ^= 0x8d;
            }
        }
        crc &= 0xff;
    }
    return crc & 0xff;
}

void RingController::OnUpdate(const Core::Timing::CoreTiming& core_timing, u8* data,
                              std::size_t size) {
    /* if (!is_hidbus_enabled) {
        return;
    }

    const auto& last_entry = hidbus_status.entries[last_entry_index];
    last_entry_index = (last_entry_index + 1) % 17;
    auto& cur_entry = hidbus_status.entries[last_entry_index];

    cur_entry.is_in_focus = true;
    cur_entry.is_connected = true;
    cur_entry.is_connected_result = RESULT_SUCCESS;
    cur_entry.is_enabled = true;
    cur_entry.is_polling_mode = true;
    cur_entry.polling_mode = JoyPollingMode::ButtonOnly;*/

    /* switch (cur_entry.polling_mode) {
     case JoyPollingMode::SixAxisSensorDisable: {
         JoyDisableSixAxisPollingEntry disable_six_entry{};
         std::memcpy(&cur_entry.data, &disable_six_entry, sizeof(disable_six_entry));
         break;
     }
     case JoyPollingMode::SixAxisSensorEnable: {
         JoyEnableSixAxisPollingEntry enable_sixaxis_entry{};
         std::memcpy(&cur_entry.data, &enable_sixaxis_entry, sizeof(enable_sixaxis_entry));
         break;
     }
     case JoyPollingMode::ButtonOnly: {
         JoyButtonOnlyPollingEntry button_polling_entry{};
         button_polling_entry.sampling_number = entry_count;
         button_polling_entry.data.out_size = 3;
         button_polling_entry.sampling_number = entry_count;
         std::memcpy(&cur_entry.data, &button_polling_entry, sizeof(button_polling_entry));
         break;
     }
     }*/

    // entry_count++;
    // std::memcpy(data, &hidbus_status, sizeof(hidbus_status));
}

u8 RingController::GetDeviceId() const {
    return DEVICE_ID;
}
void RingController::GetReply(std::vector<u8>& data) {
    switch (command) {
    case RingConCommands::GetFirmwareVersion: {
        LOG_ERROR(Service_HID, "firmware");
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(version.sub);
        data.push_back(version.main);
        data.push_back(0);
        data.push_back(0);
        /* FirmwareVersionResponse reply;
        reply.status = DataValid::Valid;
        reply.sub = version.sub;
        reply.main = version.main;
        std::memcpy(data.data(), &reply, sizeof(reply));*/
        return;
    }
    case RingConCommands::c20105:
        LOG_ERROR(Service_HID, "c20105");
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(1);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        break;
    case RingConCommands::GetTotalPushCount:
        LOG_ERROR(Service_HID, "GetTotalPushCount");
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(30);
        data.push_back(0);
        data.push_back(0);
        data.push_back(36);
        // data[7] = GetCrcValue({1,0,0});
        break;
    case RingConCommands::ReadUnkCal:
        LOG_ERROR(Service_HID, "ReadUnkCal");
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        break;
    case RingConCommands::ReadManuCal:
        LOG_ERROR(Service_HID, "ReadManuCal");
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        break;
    case RingConCommands::ReadUserCal:
        LOG_ERROR(Service_HID, "ReadUserCal");
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        break;
    case RingConCommands::PlayReport:
        LOG_ERROR(Service_HID, "PlayReport");
        data.push_back(0);
        data.push_back(0); //
        data.push_back(0); //
        data.push_back(0); //
        data.push_back(29);
        data.push_back(0);
        data.push_back(41);
        data.push_back(0);

        data.push_back(22);
        data.push_back(87);

        data.push_back(65);
        data.push_back(77);
        data.push_back(53);
        data.push_back(53);

        data.push_back(53);
        data.push_back(32);
        break;
    default:
        LOG_ERROR(Service_HID, "error");
        data[0] = 1;
    }
}

void RingController::SetCommand(const std::vector<u8>& data) {
    if (data[0] == 0 && data[1] == 0 && data[2] == 2) {
        LOG_ERROR(Service_HID, "firmware");
        command = RingConCommands::GetFirmwareVersion;
        return;
    }
    if (data[0] == 5 && data[1] == 1 && data[2] == 2) {
        LOG_ERROR(Service_HID, "c20105");
        command = RingConCommands::c20105;
        return;
    }
    if (data[0] == 4 && data[1] == 50 && data[2] == 2) {
        LOG_ERROR(Service_HID, "GetTotalPushCount");
        command = RingConCommands::GetTotalPushCount;
        return;
    }
    if (data[0] == 4 && data[1] == 5 && data[2] == 2) {
        LOG_ERROR(Service_HID, "ReadUnkCal");
        command = RingConCommands::ReadUnkCal;
        return;
    }
    if (data[0] == 4 && data[1] == 10 && data[2] == 2) {
        LOG_ERROR(Service_HID, "ReadManuCal");
        command = RingConCommands::ReadManuCal;
        return;
    }
    if (data[0] == 0 && data[1] == 1 && data[2] == 2) {
        LOG_ERROR(Service_HID, "PlayReport");
        command = RingConCommands::PlayReport;
        return;
    }
    if (data[0] == 4 && data[1] == 26 && data[2] == 2) {
        LOG_ERROR(Service_HID, "ReadUserCal");
        command = RingConCommands::ReadUserCal;
        return;
    }
    for (std::size_t i = 0; i < data.size(); i++) {
        LOG_ERROR(Service_HID, "data[{}]={}", i, data[i]);
    }
    command = RingConCommands::error;
}

} // namespace Service::HID
