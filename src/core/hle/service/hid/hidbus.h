// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once
#include "core/hle/service/service.h"

namespace Core {
class System;
}

namespace Kernel {
class KSharedMemory;
class KEvent;
class KReadableEvent;
}

namespace Service::HID {

class HidBus final : public ServiceFramework<HidBus> {
public:
    explicit HidBus(Core::System& system_);
    ~HidBus() override;

    enum class BusType : u8 {
        LeftJoyRail,
        RightJoyRail,
        InternalBus,

        MaxBusType,
    };

    struct BusHandle {
        u32_le abstracted_pad_id;
        u8 internal_index;
        u8 player_number;
        BusType bus_type;
        bool is_valid;
    };
    static_assert(sizeof(BusHandle) == 0x8, "BusHandle is an invalid size");

    enum class JoyPollingMode : u32 {
        GetJoyDisableSixAxisPollingDataAccessor,
        GetJoyEnableSixAxisPollingDataAccessor,
        GetJoyButtonOnlyPollingDataAccessor,
    };

    struct HidBusEntry {
        u8 flag;
        u8 padding_byte1;
        u8 padding_byte2;
        u8 padding_byte3;
        u32 resutl;
        u8 deviceEnabled;
        u8 isValid;
        u8 pollingEnabled;
        u8 unknow_padding;
        JoyPollingMode polling_mode;
        INSERT_PADDING_BYTES(0x70);
    };
    static_assert(sizeof(HidBusEntry) == 0x80, "HidBusEntry is an invalid size");

private:
    void GetBusHandle(Kernel::HLERequestContext& ctx);
    void IsExternalDeviceConnected(Kernel::HLERequestContext& ctx);
    void GetExternalDeviceId(Kernel::HLERequestContext& ctx);
    void SendCommandAsync(Kernel::HLERequestContext& ctx);
    void GetSendCommandAsynceResult(Kernel::HLERequestContext& ctx);
    void SetEventForSendCommandAsycResult(Kernel::HLERequestContext& ctx);
    void GetSharedMemoryHandle(Kernel::HLERequestContext& ctx);
    void GetPollingData(Kernel::HLERequestContext& ctx);
    void SetStatusManagerType(Kernel::HLERequestContext& ctx);
    //std::shared_ptr<Kernel::Handle>& GetReadableEvent();
    std::shared_ptr<Kernel::KEvent> send_command_asyc_event;
    std::shared_ptr<Kernel::Handle> bus_handle{};
    std::shared_ptr<Kernel::KSharedMemory> shared_memory;
};

} // namespace Service::HID
