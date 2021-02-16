// Copyright 2017 Citra Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <memory>
#include <thread>
#include "common/param_package.h"
#include "input_common/analog_from_button.h"
#include "input_common/gcadapter/gc_adapter.h"
#include "input_common/keyboard.h"
#include "input_common/main.h"
#include "input_common/motion_from_button.h"
#include "input_common/mouse/mouse_input.h"
#include "input_common/mouse/mouse_poller.h"
#include "input_common/touch_from_button.h"
#include "input_common/joycon/jc_adapter.h"
#include "input_common/udp/client.h"
#ifdef HAVE_SDL2
#include "input_common/sdl/sdl.h"
#endif

namespace InputCommon {

struct InputSubsystem::Impl {
    void Initialize() {
        //jcadapter = JCAdapter::Init();
        //gcadapter = GCAdapter::Init();
        //udp = CemuhookUDP::Init();
        //mouse = Mouse::Init();
        mouse = std::make_shared<Mouse>();

#ifdef HAVE_SDL2
        sdl = SDL::Init();
#endif
    }

    void Shutdown() {
        //jcadapter.reset();
        //gcadapter.reset();
        //udp.reset();
        mouse.reset();
#ifdef HAVE_SDL2
        sdl.reset();
#endif
    }

    [[nodiscard]] std::vector<Common::ParamPackage> GetInputDevices() const {
        std::vector<Common::ParamPackage> devices = {
            Common::ParamPackage{{"display", "Any"}, {"class", "any"}},
            Common::ParamPackage{{"display", "Keyboard/Mouse"}, {"class", "keyboard"}},
        };
#ifdef HAVE_SDL2
        const auto sdl_devices = sdl->GetInputDevices();
        devices.insert(devices.end(), sdl_devices.begin(), sdl_devices.end());
#endif
        //const auto udp_devices = udp->GetInputDevices();
        //devices.insert(devices.end(), udp_devices.begin(), udp_devices.end());
        //const auto gcpad_devices = gcadapter->GetInputDevices();
        //devices.insert(devices.end(), gcpad_devices.begin(), gcpad_devices.end());
        //const auto jcpad_devices = jcadapter->GetInputDevices();
        //devices.insert(devices.end(), jcpad_devices.begin(), jcpad_devices.end());
        return devices;
    }

    [[nodiscard]] AnalogMapping GetAnalogMappingForDevice(
        const Common::ParamPackage& params) const {
        if (!params.Has("class") || params.Get("class", "") == "any") {
            return {};
        }
        //if (params.Get("class", "") == "gcpad") {
        //    return gcadapter->GetAnalogMappingForDevice(params);
        //}
        //if (params.Get("class", "") == "jcpad") {
        //    return jcadapter->GetAnalogMappingForDevice(params);
        //}
#ifdef HAVE_SDL2
        if (params.Get("class", "") == "sdl") {
            return sdl->GetAnalogMappingForDevice(params);
        }
#endif
        return {};
    }

    [[nodiscard]] ButtonMapping GetButtonMappingForDevice(
        const Common::ParamPackage& params) const {
        if (!params.Has("class") || params.Get("class", "") == "any") {
            return {};
        }
        //if (params.Get("class", "") == "gcpad") {
        //    return gcadapter->GetButtonMappingForDevice(params);
        //}
        //if (params.Get("class", "") == "jcpad") {
        //    return jcadapter->GetButtonMappingForDevice(params);
        //}
#ifdef HAVE_SDL2
        if (params.Get("class", "") == "sdl") {
            return sdl->GetButtonMappingForDevice(params);
        }
#endif
        return {};
    }

    [[nodiscard]] MotionMapping GetMotionMappingForDevice(
        const Common::ParamPackage& params) const {
        if (!params.Has("class") || params.Get("class", "") == "any") {
            return {};
        }
        if (params.Get("class", "") == "cemuhookudp") {
            // TODO return the correct motion device
            return {};
        }
        return {};
    }

    std::shared_ptr<Keyboard> keyboard;
#ifdef HAVE_SDL2
    std::unique_ptr<SDL::State> sdl;
#endif
    //std::shared_ptr<JCAdapter::State> jcadapter;
    //std::shared_ptr<CemuhookUDP::State> udp;
    //std::shared_ptr<GCAdapter::State> gcadapter;
    std::shared_ptr<Mouse> mouse;
};

InputSubsystem::InputSubsystem() : impl{std::make_unique<Impl>()} {}

InputSubsystem::~InputSubsystem() = default;

void InputSubsystem::Initialize() {
    impl->Initialize();
}

void InputSubsystem::Shutdown() {
    impl->Shutdown();
}

Keyboard* InputSubsystem::GetKeyboard() {
    return impl->keyboard.get();
}

const Keyboard* InputSubsystem::GetKeyboard() const {
    return impl->keyboard.get();
}

Mouse* InputSubsystem::GetMouse() {
    return impl->mouse.get();
}

const Mouse* InputSubsystem::GetMouse() const {
    return impl->mouse.get();
}

std::vector<Common::ParamPackage> InputSubsystem::GetInputDevices() const {
    return impl->GetInputDevices();
}

AnalogMapping InputSubsystem::GetAnalogMappingForDevice(const Common::ParamPackage& device) const {
    return impl->GetAnalogMappingForDevice(device);
}

ButtonMapping InputSubsystem::GetButtonMappingForDevice(const Common::ParamPackage& device) const {
    return impl->GetButtonMappingForDevice(device);
}

MotionMapping InputSubsystem::GetMotionMappingForDevice(const Common::ParamPackage& device) const {
    return impl->GetMotionMappingForDevice(device);
}

void InputSubsystem::ReloadInputDevices() {
    //if (!impl->udp) {
    //    return;
    //}
    //impl->udp->ReloadSockets();
}

std::vector<std::unique_ptr<Polling::DevicePoller>> InputSubsystem::GetPollers(
    Polling::DeviceType type) const {
    std::vector<std::unique_ptr<Polling::DevicePoller>> pollers;
    //const auto udp_pollers = impl->udp->GetPollers(type);
    //pollers.insert(pollers.end(), udp_pollers.begin(), udp_pollers.end());
    //const auto gcpad_pollers = impl->gcadapter->GetPollers(type);
    //pollers.insert(pollers.end(), gcpad_pollers.begin(), gcpad_pollers.end());
    //const auto jcpad_pollers = impl->jcadapter->GetPollers(type);
    //pollers.insert(pollers.end(), jcpad_pollers.begin(), jcpad_pollers.end());
    //const auto mouse_pollers = impl->mouse->GetPollers(type);
    //pollers.insert(pollers.end(), mouse_pollers.begin(), mouse_pollers.end());
#ifdef HAVE_SDL2
    const auto sdl_pollers = impl->sdl->GetPollers(type);
    pollers.insert(pollers.end(), sdl_pollers.begin(), sdl_pollers.end());
#endif
    return pollers;
}

std::string GenerateKeyboardParam(int key_code) {
    Common::ParamPackage param{
        {"engine", "keyboard"},
        {"code", std::to_string(key_code)},
    };
    return param.Serialize();
}

std::string GenerateAnalogParamFromKeys(int key_up, int key_down, int key_left, int key_right,
                                        int key_modifier, float modifier_scale) {
    Common::ParamPackage circle_pad_param{
        {"engine", "analog_from_button"},
        {"up", GenerateKeyboardParam(key_up)},
        {"down", GenerateKeyboardParam(key_down)},
        {"left", GenerateKeyboardParam(key_left)},
        {"right", GenerateKeyboardParam(key_right)},
        {"modifier", GenerateKeyboardParam(key_modifier)},
        {"modifier_scale", std::to_string(modifier_scale)},
    };
    return circle_pad_param.Serialize();
}
} // namespace InputCommon
