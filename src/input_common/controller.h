// Copyright 2020 yuzu Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once
#include "common/settings.h"
#include "core/frontend/input.h"

namespace InputCommon {

using ButtonDevices =
    std::array<std::unique_ptr<Input::ButtonDevice>, Settings::NativeButton::NumButtons>;
using StickDevices =
    std::array<std::unique_ptr<Input::AnalogDevice>, Settings::NativeAnalog::NUM_STICKS_HID>;

enum class ControllerType {
    ProController,
    LeftJoycon,
    RightJoycon,
    DualJoycon,
    Handheld,
    GameCube,
    Pokeball,
};

class Controller {
public:
    Controller(bool player_number_);

    void SendVibration();
    void ReloadInput();
    void EnableController(bool enable);

    bool IsConnected() const;
    std::size_t GetPlayerNumber() const;
    ControllerType GetControllerType() const;
    Input::ButtonValues GetButtons() const;
    Input::SticksValues GetSticks() const;
    Input::TriggerValues GetTriggers() const;
    Input::MotionValues GetMotion() const;
    Input::TouchValues GetTouch() const;

private:
    void SetButton(Input::CallbackStatus callback, std::size_t index);
    void SetStick(Input::CallbackStatus callback, std::size_t index);
    void SetTrigger(Input::CallbackStatus callback, std::size_t index);
    void SetMotion(Input::CallbackStatus callback, std::size_t index);
    void SetTouch(Input::CallbackStatus callback, std::size_t index);

    bool GetButtonFromAnalog(const Input::CallbackStatus& callback) const;
    bool GetButtonFromTouch(const Input::CallbackStatus& callback) const;

    float GetAnalogFromButton(const Input::CallbackStatus& callback) const;

    std::tuple<float, float> GetStickFromMotion(const Input::CallbackStatus& callback) const;
    std::tuple<float, float> GetStickFromTouch(const Input::CallbackStatus& callback) const;


    bool is_connected{};
    const std::size_t player_number;
    ControllerType controller_type;

    Input::ControllerStatus controller;

    ButtonDevices buttons;
    StickDevices sticks;
    //VibrationArray vibrations;
    //MotionArray motions;
};

class ControllerFactory final : public Input::Factory<Input::ControllerInputDevice> {
public:
    explicit ControllerFactory(std::shared_ptr<Controller> controller_);

    /**
     * Creates a button device from a button press
     * @param params contains parameters for creating the device:
     *     - "code": the code of the key to bind with the button
     */
    std::unique_ptr<Input::ControllerInputDevice> Create(
        const Common::ParamPackage& params) override;

private:
    std::shared_ptr<Controller> controller;
};
} // namespace InputCommon
