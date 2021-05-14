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
class Ring_controller {
public:
    explicit Ring_controller();
    ~Ring_controller();

    // When the controller is requesting an update for the shared memory
    void OnUpdate(const Core::Timing::CoreTiming& core_timing, u8* data, std::size_t size);

private:
    enum class RingConCommands : u32_le {
        GetFirmwareVersion = 0x00020000,
        PlayReport = 0x00020100,
        JoyPolling = 0x00020101,
        Unknown1 = 0x00020104,
        Unknown2 = 0x00020105,
        Unknown3 = 0x00020204,
        Unknown4 = 0x00020304,
        Unknown5 = 0x00020404,
        Unknown6 = 0x00020504,
        Unknown7 = 0x00020A04,
        Unknown8 = 0x00021104,
        Unknown9 = 0x00021204,
        Unknown10 = 0x00021304,
        Unknown11 = 0x00021A04,
        Unknown12 = 0x00023104,
        Unknown13 = 0x00023204,
        Unknown14 = 0x04013104,
        Unknown15 = 0x04011104,
        Unknown16 = 0x04011204,
        Unknown17 = 0x04011304,

    };
};
} // namespace Service::HID
