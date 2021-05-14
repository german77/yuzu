// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <cstring>
#include "common/common_types.h"
#include "core/core_timing.h"
#include "core/hle/service/hid/hidbus/ringcon.h"

namespace Service::HID {
constexpr std::size_t SHARED_MEMORY_OFFSET = 0x0;

Ring_controller::Ring_controller()  {}
Ring_controller::~Ring_controller() = default;


void Ring_controller::OnUpdate(const Core::Timing::CoreTiming& core_timing, u8* data,
                               std::size_t size) {
 /*   if (!is_hidbus_enabled) {
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
    cur_entry.polling_mode = JoyPollingMode::ButtonOnly;

    switch (cur_entry.polling_mode) {
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
    }

    entry_count++;
    std::memcpy(data, &hidbus_status, sizeof(hidbus_status));*/
}

} // namespace Service::HID
