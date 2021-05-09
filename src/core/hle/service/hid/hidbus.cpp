// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/logging/log.h"
#include "core/core.h"
#include "core/core_timing.h"
#include "core/core_timing_util.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/k_event.h"
#include "core/hle/kernel/k_readable_event.h"
#include "core/hle/kernel/k_shared_memory.h"
#include "core/hle/kernel/k_writable_event.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/k_transfer_memory.h"
#include "core/hle/service/hid/errors.h"
#include "core/hle/service/hid/hidbus.h"
#include "core/hle/service/service.h"

namespace Service::HID {

constexpr auto hidbus_update_ns = std::chrono::nanoseconds{4 * 1000 * 1000}; // (4ms, 1000Hz)

HidBus::HidBus(Core::System& system_) : ServiceFramework{system_, "hidbus"} {

    // clang-format off
    static const FunctionInfo functions[] = {
            {1, &HidBus::GetBusHandle, "GetBusHandle"},
            {2, &HidBus::IsExternalDeviceConnected, "IsExternalDeviceConnected"},
            {3, &HidBus::Initialize, "Initialize"},
            {4, nullptr, "Finalize"},
            {5, &HidBus::EnableExternalDevice, "EnableExternalDevice"},
            {6, &HidBus::GetExternalDeviceId, "GetExternalDeviceId"},
            {7, &HidBus::SendCommandAsync, "SendCommandAsync"},
            {8, &HidBus::GetSendCommandAsynceResult, "GetSendCommandAsynceResult"},
            {9, &HidBus::SetEventForSendCommandAsycResult, "SetEventForSendCommandAsycResult"},
            {10, &HidBus::GetSharedMemoryHandle, "GetSharedMemoryHandle"},
            {11, nullptr, "EnableJoyPollingReceiveMode"},
            {12, nullptr, "DisableJoyPollingReceiveMode"},
            {13, &HidBus::GetPollingData, "GetPollingData"},
            {14, &HidBus::SetStatusManagerType, "SetStatusManagerType"},
    };
    // clang-format on

    RegisterHandlers(functions);

    auto& kernel = system.Kernel();
    send_command_asyc_event = Kernel::KEvent::Create(kernel);
    send_command_asyc_event->Initialize("hidbus:SendCommandAsycEvent");

    // Register update callbacks
    hidbus_update_event = Core::Timing::CreateEvent(
        "hidbus::UpdateCallback",
        [this](std::uintptr_t user_data, std::chrono::nanoseconds ns_late) {
            const auto guard = LockService();
            UpdateHidbus(user_data, ns_late);
        });

    system_.CoreTiming().ScheduleEvent(hidbus_update_ns, hidbus_update_event);
}

HidBus::~HidBus() {
    system.CoreTiming().UnscheduleEvent(hidbus_update_event, 0);
}

void HidBus::UpdateHidbus(std::uintptr_t user_data, std::chrono::nanoseconds ns_late) {
    auto& core_timing = system.CoreTiming();

    UpdateSharedMemory(core_timing);

    // We can send the answer right away no need to wait
    if (send_command_asyc) {
        send_command_asyc_event->GetWritableEvent().Signal();
        send_command_asyc = false;
    }

    // If ns_late is higher than the update rate ignore the delay
    if (ns_late > hidbus_update_ns) {
        ns_late = {};
    }

    core_timing.ScheduleEvent(hidbus_update_ns - ns_late, hidbus_update_event);
}

void HidBus::UpdateSharedMemory(const Core::Timing::CoreTiming& core_timing) {
    if (!is_hidbus_enabled) {
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
    case JoyPollingMode::SixAxisSensorDisable:{
        JoyDisableSixAxisPollingEntry disable_six_entry{};
        std::memcpy(&cur_entry.data, &disable_six_entry, sizeof(disable_six_entry));
        break;
    }
    case JoyPollingMode::SixAxisSensorEnable:{
        JoyEnableSixAxisPollingEntry enable_sixaxis_entry{};
        std::memcpy(&cur_entry.data, &enable_sixaxis_entry, sizeof(enable_sixaxis_entry));
        break;
    }
    case JoyPollingMode::ButtonOnly:{
        JoyButtonOnlyPollingEntry button_polling_entry{};
        button_polling_entry.sampling_number = entry_count;
        button_polling_entry.data.out_size = 3;
        button_polling_entry.sampling_number = entry_count;
        std::memcpy(&cur_entry.data, &button_polling_entry, sizeof(button_polling_entry));
        break;
    }
    }

    entry_count++;
    std::memcpy(system.Kernel().GetHidSharedMem().GetPointer(), &hidbus_status,
                sizeof(hidbus_status));
}


void HidBus::GetBusHandle(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    struct Parameters {
        u32 npad_id;
        u64 bus_type;
        u64 applet_resource_user_id;
    };
    const auto parameters{rp.PopRaw<Parameters>()};

    LOG_ERROR(Service_HID, "called. npad_id={} bus_type={}", parameters.npad_id,
              parameters.bus_type);
    bus_handle.is_valid = true;
    bus_handle.player_number = static_cast<u8>(parameters.npad_id);
    bus_handle.bus_type = BusType::RightJoyRail;
    IPC::ResponseBuilder rb{ctx, 5};
    rb.Push(RESULT_SUCCESS);
    rb.Push(true);
    rb.PushRaw(bus_handle);
}

void HidBus::IsExternalDeviceConnected(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle_{rp.PopRaw<HidbusBusHandle>()};

    LOG_ERROR(Service_HID,
              "(STUBBED) called abstracted_pad_id={} bus_type={} internal_index={} "
              "player_number={} is_valid={}",
              bus_handle_.abstracted_pad_id, bus_handle_.bus_type, bus_handle_.internal_index,
              bus_handle_.player_number, bus_handle_.is_valid);

    IPC::ResponseBuilder rb{ctx, 3};
    rb.Push(RESULT_SUCCESS);
    rb.Push<bool>(true);
}

void HidBus::Initialize(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle_{rp.PopRaw<HidbusBusHandle>()};

    LOG_ERROR(Service_HID,
              "(STUBBED) called abstracted_pad_id={} bus_type={} internal_index={} "
              "player_number={} is_valid={}",
              bus_handle_.abstracted_pad_id, bus_handle_.bus_type, bus_handle_.internal_index,
              bus_handle_.player_number, bus_handle_.is_valid);

    is_hidbus_enabled = true;
    IPC::ResponseBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
}

void HidBus::EnableExternalDevice(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    struct Parameters {
        bool enable;
        std::array<u8, 7> pad;
        HidbusBusHandle bus_handle;
        u64 inval;
        u64 applet_resource_user_id;
    };
    const auto parameters{rp.PopRaw<Parameters>()};

    LOG_ERROR(Service_HID,
              "(STUBBED) called enable={}, pad={}",
              parameters.enable, parameters.pad[0]);

    IPC::ResponseBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
}

void HidBus::GetExternalDeviceId(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle_{rp.PopRaw<HidbusBusHandle>()};

    LOG_ERROR(Service_HID,
              "(STUBBED) called abstracted_pad_id={} bus_type={} internal_index={} "
              "player_number={} is_valid={}",
              bus_handle_.abstracted_pad_id, bus_handle_.bus_type, bus_handle_.internal_index,
              bus_handle_.player_number, bus_handle_.is_valid);

    IPC::ResponseBuilder rb{ctx, 3};
    rb.Push(RESULT_SUCCESS);
    // 0x20 is the id for the ring controller
    rb.Push<u32>(0x20);
}

void HidBus::SendCommandAsync(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto data = ctx.ReadBuffer();
    const auto bus_handle_{rp.PopRaw<HidbusBusHandle>()};

    LOG_ERROR(Service_HID,
              "(STUBBED) called data_size={}, abstracted_pad_id={} bus_type={} internal_index={} "
              "player_number={} is_valid={}",
              data.size(),
              bus_handle_.abstracted_pad_id, bus_handle_.bus_type, bus_handle_.internal_index,
              bus_handle_.player_number, bus_handle_.is_valid);

    for (std::size_t i = 0; i < data.size(); i++) {
        LOG_ERROR(Service_HID, "data[{}]={}", i, data[i]);
    }

    send_command_asyc = true;

    IPC::ResponseBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
};

void HidBus::GetSendCommandAsynceResult(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle_{rp.PopRaw<HidbusBusHandle>()};
    std::array<u8, 0x8> data;

     LOG_ERROR(Service_HID,
               "(STUBBED) called  abstracted_pad_id={} bus_type={} internal_index={} "
               "player_number={} is_valid={}", bus_handle_.abstracted_pad_id, bus_handle_.bus_type,
               bus_handle_.internal_index, bus_handle_.player_number, bus_handle_.is_valid);

    u64 data_size = ctx.WriteBuffer(data);
    IPC::ResponseBuilder rb{ctx, 4};
    rb.Push(RESULT_SUCCESS);
    rb.Push<u64>(data_size);
};

void HidBus::SetEventForSendCommandAsycResult(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle{rp.PopRaw<HidbusBusHandle>()};

    LOG_ERROR(Service_HID, "called");

    IPC::ResponseBuilder rb{ctx, 2, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushCopyObjects(send_command_asyc_event->GetReadableEvent());
};

void HidBus::GetSharedMemoryHandle(Kernel::HLERequestContext& ctx) {
    LOG_ERROR(Service_HID, "called");

    IPC::ResponseBuilder rb{ctx, 2, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushCopyObjects(&system.Kernel().GetHidBusSharedMem());
}

void HidBus::GetPollingData(Kernel::HLERequestContext& ctx) {
    LOG_ERROR(Service_HID, "(STUBBED) called");
};

void HidBus::SetStatusManagerType(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto input{rp.PopRaw<u32>()};

    LOG_ERROR(Service_HID, "(STUBBED) called input={}", input);

    IPC::ResponseBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
};
} // namespace Service::HID
