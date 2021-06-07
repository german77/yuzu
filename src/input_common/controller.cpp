// Copyright 2020 yuzu Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

#pragma once
#include "input_common/controller.h"

namespace InputCommon {

Controller::Controller(bool player_number_) : player_number(player_number_) {
    ReloadInput();
}
void Controller::ReloadInput() {
    const auto& players = Settings::values.players.GetValue()[player_number];
        std::transform(players.buttons.begin() + Settings::NativeButton::BUTTON_HID_BEGIN,
                       players.buttons.begin() + Settings::NativeButton::BUTTON_HID_END,
                       buttons.begin(), Input::CreateDevice<Input::ButtonDevice>);
        std::transform(players.analogs.begin() + Settings::NativeAnalog::STICK_HID_BEGIN,
                       players.analogs.begin() + Settings::NativeAnalog::STICK_HID_END,
                       sticks.begin(), Input::CreateDevice<Input::AnalogDevice>);
        for (std::size_t index = 0; index < buttons.size(); ++index) {
            if (!buttons[index])
                return;
            Input::InputCallback button_callback{
                [this, index](Input::CallbackStatus callback) { SetButton(callback, index); }};
            buttons[index]->SetCallback(button_callback);
        }
        for (std::size_t index = 0; index < sticks.size(); ++index) {
            if (!buttons[index])
                return;
            Input::InputCallback stick_callback{
                [this, index](Input::CallbackStatus callback) { SetStick(callback, index); }};
            sticks[index]->SetCallback(stick_callback);
        }
}

void Controller::SetButton(Input::CallbackStatus callback, std::size_t index) {
    LOG_WARNING(Input, "Button {} called, status {}", index, callback.button_value);
    if (index >= controller.button_values.size()) {
        return;
    }

    bool value = false;
    switch (callback.type) {
    case Input::InputType::Analog:
        value = GetButtonFromAnalog(callback);
        break;
    case Input::InputType::Button:
        value = callback.button_value;
        break;
    case Input::InputType::Touch:
        value = GetButtonFromTouch(callback);
        break;
    default:
        LOG_ERROR(Input, "Conversion from type {} to button not implemented", callback.type);
        break;
    }
    controller.button_values[index] = value;
}

void Controller::SetStick(Input::CallbackStatus callback, std::size_t index) {
    LOG_WARNING(Input, "Button called {} {}", index, callback.type);
    if (index >= controller.stick_values.size()) {
        return;
    }

    float x = 0.0f;
    float y = 0.0f;
    switch (callback.type) {
    case Input::InputType::Stick:
        std::tie(x, y) = callback.stick_value;
        break;
    case Input::InputType::Motion:
        std::tie(x, y) = GetStickFromMotion(callback);
        break;
    case Input::InputType::Touch:
        std::tie(x, y) = GetStickFromTouch(callback);
        break;
    default:
        LOG_ERROR(Input, "Conversion from type {} to stick not implemented", callback.type);
        break;
    }

    float r = x * x + y * y;
    r = std::sqrt(r);

    if (r <= callback.deadzone) {
        controller.stick_values[index] = {0.0f, 0.0f};
        return;
    }

    // Adjust range of joystick
    const float deadzone_factor = 1 / r * (r - callback.deadzone) / (1 - callback.deadzone);
    x = x * deadzone_factor * callback.range;
    y = y * deadzone_factor * callback.range;
    r = r * deadzone_factor * callback.range;

    // Normalize joystick
    if (r > 1.0f) {
        x /= r;
        y /= r;
    }

    x = callback.invert_x ? x : -x;
    y = callback.invert_y ? y : -y;

    controller.stick_values[index] = {x, y};
}

void Controller::SetTrigger(Input::CallbackStatus callback, std::size_t index) {
    if (index >= controller.trigger_values.size()) {
        return;
    }

    float value = 0.0f;
    switch (callback.type) {
    case Input::InputType::Analog :
        value = callback.analog_value;
        break;
    case Input::InputType::Button:
        value = GetAnalogFromButton(callback);
        break;
    default:
        LOG_ERROR(Input, "Conversion from type {} to trigger not implemented", callback.type);
        break;
    }

    float r = std::abs(value);
    if (r <= callback.deadzone) {
        controller.trigger_values[index] = {0.0f, false};
        return;
    }

    // Adjust range of value
    const float deadzone_factor = 1 / r * (r - callback.deadzone) / (1 - callback.deadzone);
    value = value * deadzone_factor * callback.range;

    // Normalize value
    value = callback.positive ? value : -value;
    value = std::clamp(value, 0.0f, 1.0f);

    controller.trigger_values[index] = {value, value > callback.threshold};
}

void Controller::SetMotion(Input::CallbackStatus callback, std::size_t index) {
    if (index >= controller.motion_values.size()) {
        return;
    }

    switch (callback.type) {
    case Input::InputType::Motion :
        //motion_values[index] = callback.motion_value;
        break;
    default:
        LOG_ERROR(Input, "Conversion from type {} to motion not implemented", callback.type);
        break;
    }
}

void Controller::SetTouch(Input::CallbackStatus callback,
                          [[maybe_unused]] std::size_t index) {
    switch (callback.type) {
    case Input::InputType::Touch:
        //touch_values = callback.touch_value;
        break;
    default:
        LOG_ERROR(Input, "Conversion from type {} to touch not implemented", callback.type);
        break;
    }
}
bool Controller::GetButtonFromAnalog(const Input::CallbackStatus& callback) const {
    const float value = callback.analog_value;
    return value > callback.threshold;
}

bool Controller::GetButtonFromTouch([[maybe_unused]] const Input::CallbackStatus& callback) const {
    constexpr std::size_t finger_id = 0;
    bool active = false;
    //std::tie(std::ignore, std::ignore, active) = callback.touch_value[finger_id];
    return active;
}

float Controller::GetAnalogFromButton(const Input::CallbackStatus& callback) const {
    return callback.button_value ? 1.0f : 0.0f;
}

std::tuple<float, float> Controller::GetStickFromMotion(
    [[maybe_unused]]  const Input::CallbackStatus& callback) const {
    Common::Vec3f accel{};
    //std::tie(accel, std::ignore, std::ignore, std::ignore, std::ignore) = callback.motion_value;
    return {accel.x, accel.y};
}

std::tuple<float, float> Controller::GetStickFromTouch(
    [[maybe_unused]]  const Input::CallbackStatus& callback) const {
    //constexpr std::size_t finger_id = 0;
    //float x = 0;
    //float y = 0;
    //bool active = false;
    //std::tie(x,y,active)=callback.touch_value[finger_id];
    //if (active) {
    //    return {x, y};
    //}
    return {0.0f, 0.0f};
}


void Controller::SendVibration() {
    return;
}

void Controller::EnableController(bool enable) {
    is_connected=enable;
}


bool Controller::IsConnected() const {
    return is_connected;
}

std::size_t Controller::GetPlayerNumber() const {
    return player_number;
}

ControllerType Controller::GetControllerType() const {
    return controller_type;
}

Input::ButtonValues Controller::GetButtons() const {
    return controller.button_values;
}
Input::SticksValues Controller::GetSticks() const {
    return controller.stick_values;
}
Input::TriggerValues Controller::GetTriggers() const {
    return controller.trigger_values;
}
Input::MotionValues Controller::GetMotion() const {
    return controller.motion_values;
}
Input::TouchValues Controller::GetTouch() const {
    return controller.touch_values;
}

class ControllerPoller final : public Input::ControllerInputDevice {
public:
    explicit ControllerPoller(const Controller* controller_)
        :  controller(controller_) {}

    //~ControllerPoller() override;

    Input::ButtonValues GetButtonStatus() const override {
        return controller->GetButtons();
    }

private:
    const Controller* controller;
};

std::unique_ptr<Input::ControllerInputDevice> ControllerFactory::Create(
    [[maybe_unused]]const Common::ParamPackage& params) {
    return std::make_unique<ControllerPoller>(controller.get());
}

ControllerFactory::ControllerFactory(std::shared_ptr<Controller> controller_)
    : controller(std::move(controller_)) {}


} // namespace InputCommon
