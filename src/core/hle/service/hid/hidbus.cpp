// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include "common/logging/log.h"
#include "core/core.h"
#include "core/hle/ipc_helpers.h"
#include "core/hle/kernel/k_event.h"
#include "core/hle/kernel/k_readable_event.h"
#include "core/hle/kernel/k_shared_memory.h"
#include "core/hle/kernel/k_writable_event.h"
#include "core/hle/kernel/kernel.h"
#include "core/hle/kernel/transfer_memory.h"
#include "core/hle/service/hid/errors.h"
#include "core/hle/service/hid/hidbus.h"
#include "core/hle/service/service.h"

namespace Service::HID {

HidBus::HidBus(Core::System& system_) : ServiceFramework{system_, "hidbus"} {

    // clang-format off
    static const FunctionInfo functions[] = {
            {1, &HidBus::GetBusHandle, "GetBusHandle"},
            {2, &HidBus::IsExternalDeviceConnected, "IsExternalDeviceConnected"},
            {3, nullptr, "Initialize"},
            {4, nullptr, "Finalize"},
            {5, nullptr, "EnableExternalDevice"},
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
    shared_memory = SharedFrom(&kernel.GetHidSharedMem());
}

HidBus::~HidBus() = default;

void HidBus::GetBusHandle(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto npad_id{rp.Pop<u32>()};
    const auto bus_type{rp.Pop<u64>()};
    const auto applet_resource_user_id{rp.Pop<u64>()};

    // bus_handle = SharedFrom(&kernel.GetHidSharedMem());

    LOG_ERROR(Service_HID, "(STUBBED) called. npad_id={} bus_type={}", npad_id, bus_type);

    IPC::ResponseBuilder rb{ctx, 3, 1};
    rb.Push(RESULT_SUCCESS);
    rb.Push(true);
    // rb.PushMoveObjects(GetReadableEvent());
}
//
// std::shared_ptr<Kernel::Handle>& GetReadableEvent() {
//    return bus_handle;
//}

void HidBus::IsExternalDeviceConnected(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle{rp.PopRaw<BusHandle>()};

    LOG_ERROR(Service_HID,
              "(STUBBED) called abstracted_pad_id={} bus_type={} internal_index={} "
              "player_number={} is_valid={}",
              bus_handle.abstracted_pad_id, bus_handle.bus_type, bus_handle.internal_index,
              bus_handle.player_number, bus_handle.is_valid);

    IPC::ResponseBuilder rb{ctx, 3, 1};
    rb.Push(RESULT_SUCCESS);
    rb.Push<bool>(true);
}

void HidBus::GetExternalDeviceId(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle{rp.PopRaw<BusHandle>()};

    LOG_ERROR(Service_HID, "(STUBBED) called");

    IPC::ResponseBuilder rb{ctx, 3};
    rb.Push(RESULT_SUCCESS);
    // 0x20 is the id for the ring controller
    rb.Push<u32>(0x20);
}

void HidBus::SendCommandAsync(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    // const auto input_buffer{rp.PopRaw<Type21InputBuffer>()};
    // const auto bus_handle{rp.PopRaw<BusHandle>()};

    LOG_ERROR(Service_HID, "(STUBBED) called");

    IPC::ResponseBuilder rb{ctx, 2};
    rb.Push(RESULT_SUCCESS);
};

void HidBus::GetSendCommandAsynceResult(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    // const auto input_buffer{rp.PopRaw<Type22OutputBuffer>()};
    // const auto bus_handle{rp.PopRaw<BusHandle>()};

    LOG_ERROR(Service_HID, "(STUBBED) called");

    IPC::ResponseBuilder rb{ctx, 3};
    rb.Push(RESULT_SUCCESS);
    rb.Push<u32>(0);
};

void HidBus::SetEventForSendCommandAsycResult(Kernel::HLERequestContext& ctx) {
    IPC::RequestParser rp{ctx};
    const auto bus_handle{rp.PopRaw<BusHandle>()};

    LOG_ERROR(Service_HID, "(STUBBED) called");

    IPC::ResponseBuilder rb{ctx, 2, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushCopyObjects(send_command_asyc_event->GetReadableEvent());
};

void HidBus::GetSharedMemoryHandle(Kernel::HLERequestContext& ctx) {
    LOG_ERROR(Service_HID, "called");

    IPC::ResponseBuilder rb{ctx, 2, 1};
    rb.Push(RESULT_SUCCESS);
    rb.PushCopyObjects(shared_memory);
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
