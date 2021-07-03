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
constexpr FirmwareVersion version = {0x0, 0x2c};
constexpr FactoryCalibration factory_calibration = {5469, 742, 2159, 2444};
constexpr UserCalibration user_calibration = {-13570, 134, -13570, 134, 2188, 62};

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
    // if (!is_hidbus_enabled) {
    //    return;
    //}

    hidbus_entry.is_in_focus = true;
    hidbus_entry.is_connected = true;
    hidbus_entry.is_connected_result = RESULT_SUCCESS;
    hidbus_entry.is_enabled = true;
    hidbus_entry.is_polling_mode = true;
    hidbus_entry.polling_mode = JoyPollingMode::ButtonOnly;


     switch (hidbus_entry.polling_mode) {
     case JoyPollingMode::SixAxisSensorDisable: {
         //JoyDisableSixAxisPollingEntry disable_six_entry{};
         //std::memcpy(&cur_entry.data, &disable_six_entry, sizeof(disable_six_entry));
         break;
     }
     case JoyPollingMode::SixAxisSensorEnable: {
         //JoyEnableSixAxisPollingEntry enable_sixaxis_entry{};
         //std::memcpy(&cur_entry.data, &enable_sixaxis_entry, sizeof(enable_sixaxis_entry));
         break;
     }
     case JoyPollingMode::ButtonOnly: {
         button_data.header.total_entries = 2;
         button_data.header.latest_entry = 1;
         button_data.header.result = RESULT_SUCCESS;
         button_data.entries[0].sampling_number = button_data.entries[1].sampling_number + 1;
         button_data.entries[0].data.out_size = 3;
         button_data.entries[1].sampling_number = button_data.entries[0].sampling_number + 1;
         button_data.entries[1].data.out_size = 3;
         std::memcpy(&hidbus_entry.data, &button_data, sizeof(button_data));
         break;
     }
     }

    // entry_count++;
    std::memcpy(data, &hidbus_entry, sizeof(hidbus_entry));
}

u8 RingController::GetDeviceId() const {
    return DEVICE_ID;
}
void RingController::GetReply(std::vector<u8>& data) {
    switch (command) {
    case RingConCommands::GetFirmwareVersion:
        GetFirmwareVersionReply(data);
        return;
    case RingConCommands::c20105:
        GetC020105Reply(data);
        return;
    case RingConCommands::ReadTotalPushCount:
        GetReadTotalPushCountReply(data);
        return;
    case RingConCommands::ReadUnkCal:
        GetReadUnkCalReply(data);
        return;
    case RingConCommands::ReadManualCal:
        GetReadManualCalReply(data);
        return;
    case RingConCommands::ReadUserCal:
        GetReadUserCalReply(data);
        return;
    case RingConCommands::ReadId:
        GetReadIdReply(data);
        return;
    default:
        LOG_ERROR(Service_HID, "error");
        data.push_back(1);
    }
}

void RingController::SetCommand(const std::vector<u8>& data) {
    u32 command_id = data[0] + (data[1] << 8) + (data[2] << 16) + (data[3] << 24);
    constexpr std::array<RingConCommands, 7> commands = {
        RingConCommands::GetFirmwareVersion,
        RingConCommands::c20105,
        RingConCommands::ReadTotalPushCount,
        RingConCommands::ReadUnkCal,
        RingConCommands::ReadManualCal,
        RingConCommands::ReadUserCal,
        RingConCommands::ReadId,
    };
    for (RingConCommands cmd : commands) {
        if (command_id == static_cast<u32>(cmd)) {
            command = cmd;
            return;
        }
    }
    LOG_ERROR(Service_HID, "Command not implemented {}", command_id);
    command = RingConCommands::Error;
}

void RingController::GetFirmwareVersionReply(std::vector<u8>& data){
    LOG_ERROR(Service_HID, "firmware");

    FirmwareVersionReply reply;
    reply.status = DataValid::Valid;
    reply.firmware = version;
    data.resize(sizeof(reply));
    std::memcpy(data.data(), &reply, sizeof(reply));
}

void RingController::GetC020105Reply(std::vector<u8>& data) {
    LOG_ERROR(Service_HID, "c20105");

    Cmd020105Reply reply;
    reply.status = DataValid::Valid;
    reply.data = 1;
    data.resize(sizeof(reply));
    std::memcpy(data.data(), &reply, sizeof(reply));
}

void RingController::GetReadTotalPushCountReply(std::vector<u8>& data) {
    LOG_ERROR(Service_HID, "ReadTotalPushCount");

    GetThreeByteReply reply;
    reply.data = {30, 0, 0};
    reply.crc = GetCrcValue({30, 0, 0, 0}); // 36
    data.resize(sizeof(reply));
    std::memcpy(data.data(), &reply, sizeof(reply));
}

void RingController::GetReadUnkCalReply(std::vector<u8>& data) {
    LOG_ERROR(Service_HID, "ReadUnkCal");

    ReadUnkCalReply reply{};
    data.resize(sizeof(reply));
    std::memcpy(data.data(), &reply, sizeof(reply));
}

void RingController::GetReadManualCalReply(std::vector<u8>& data) {
    LOG_ERROR(Service_HID, "ReadManuCal");

    ReadManualCalReply reply;
    reply.status = DataValid::Valid;
    reply.calibration = factory_calibration;
    data.resize(sizeof(reply));
    std::memcpy(data.data(), &reply, sizeof(reply));
}

void RingController::GetReadUserCalReply(std::vector<u8>& data) {
    LOG_ERROR(Service_HID, "ReadUserCal");

    ReadUserCalReply reply;
    reply.status = DataValid::Valid;
    reply.calibration = user_calibration;
    data.resize(sizeof(reply));
    std::memcpy(data.data(), &reply, sizeof(reply));
}

void RingController::GetReadIdReply(std::vector<u8>& data) {
    LOG_ERROR(Service_HID, "ReadID");

    ReadIdReply reply;
    reply.status = DataValid::Valid;
    reply.id_l_x0 = 8;
    reply.id_l_x0_2 = 41;
    reply.id_l_x4 = 22294;
    reply.id_h_x0 = 19777;
    reply.id_h_x0_2 = 13621;
    reply.id_h_x4 = 8245;
    data.resize(sizeof(reply));
    std::memcpy(data.data(), &reply, sizeof(reply));
}

} // namespace Service::HID
