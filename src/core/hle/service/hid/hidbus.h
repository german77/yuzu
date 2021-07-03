// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include "core/hle/service/hid/hidbus/ringcon.h"
#include "core/hle/service/service.h"

namespace Core::Timing {
struct EventType;
} // namespace Core::Timing

namespace Core {
class System;
} // namespace Core

namespace Kernel {
class KEvent;
class KReadableEvent;
} // namespace Kernel

namespace Service::HID {

enum class HidBusControllerTypes : std::size_t {
    RingController,
    Starlink,

    MaxControllers,
};

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

    enum class JoyPollingMode : u32 {
        SixAxisSensorDisable,
        SixAxisSensorEnable,
        ButtonOnly,
    };

    struct HidbusBusHandle {
        u32_le abstracted_pad_id;
        u8 internal_index;
        u8 player_number;
        BusType bus_type;
        bool is_valid;
    };
    static_assert(sizeof(HidbusBusHandle) == 0x8, "HidbusBusHandle is an invalid size");

    struct JoyPollingReceivedData {
        std::array<u8, 0x30> data;
        u64 out_size;
        u64 sampling_number;
    };
    static_assert(sizeof(JoyPollingReceivedData) == 0x40,
                  "JoyPollingReceivedData is an invalid size");

    struct DataAccessorHeader {
        ResultCode result;
        INSERT_PADDING_WORDS(0x1);
        std::array<u8, 0x18> unused;
        u64 latest_entry;
        u64 total_entries;
    };
    static_assert(sizeof(DataAccessorHeader) == 0x30, "DataAccessorHeader is an invalid size");

    struct JoyDisableSixAxisPollingData {
        std::array<u8, 0x26> data;
        u8 out_size;
        INSERT_PADDING_BYTES(0x1);
        u64 sampling_number;
    };
    static_assert(sizeof(JoyDisableSixAxisPollingData) == 0x30,
                  "JoyDisableSixAxisPollingData is an invalid size");

    struct JoyDisableSixAxisPollingEntry {
        u64 sampling_number;
        JoyDisableSixAxisPollingData data;
    };
    static_assert(sizeof(JoyDisableSixAxisPollingEntry) == 0x38,
                  "JoyDisableSixAxisPollingEntry is an invalid size");

    struct JoyEnableSixAxisPollingData {
        std::array<u8, 0x8> data;
        u8 out_size;
        INSERT_PADDING_BYTES(0x7);
        u64 sampling_number;
    };
    static_assert(sizeof(JoyEnableSixAxisPollingData) == 0x18,
                  "JoyEnableSixAxisPollingData is an invalid size");

    struct JoyEnableSixAxisPollingEntry {
        u64 sampling_number;
        JoyEnableSixAxisPollingData data;
    };
    static_assert(sizeof(JoyEnableSixAxisPollingEntry) == 0x20,
                  "JoyEnableSixAxisPollingEntry is an invalid size");

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

    struct CommonHeader {
        s64_le timestamp;
        s64_le total_entry_count;
        s64_le last_entry_index;
        s64_le entry_count;
    };
    static_assert(sizeof(CommonHeader) == 0x20, "CommonHeader is an invalid size");

    struct HidbusStatusManager {
        std::array<HidbusStatusManagerEntry, 19> entries{};
    };
    static_assert(sizeof(HidbusStatusManager) <= 0x1000, "HidbusStatusManager is an invalid size");

private:
    void GetBusHandle(Kernel::HLERequestContext& ctx);
    void IsExternalDeviceConnected(Kernel::HLERequestContext& ctx);
    void Initialize(Kernel::HLERequestContext& ctx);
    void EnableExternalDevice(Kernel::HLERequestContext& ctx);
    void GetExternalDeviceId(Kernel::HLERequestContext& ctx);
    void SendCommandAsync(Kernel::HLERequestContext& ctx);
    void GetSendCommandAsynceResult(Kernel::HLERequestContext& ctx);
    void SetEventForSendCommandAsycResult(Kernel::HLERequestContext& ctx);
    void GetSharedMemoryHandle(Kernel::HLERequestContext& ctx);
    void GetPollingData(Kernel::HLERequestContext& ctx);
    void SetStatusManagerType(Kernel::HLERequestContext& ctx);

    void UpdateHidbus(std::uintptr_t user_data, std::chrono::nanoseconds ns_late);

    Kernel::KEvent* send_command_asyc_event;
    bool is_hidbus_enabled{false};
    bool send_command_asyc{false};
    HidbusBusHandle bus_handle{};
    HidbusStatusManager hidbus_status{};
    std::shared_ptr<Core::Timing::EventType> hidbus_update_event;

    RingController ringcon{};
};

} // namespace Service::HID
