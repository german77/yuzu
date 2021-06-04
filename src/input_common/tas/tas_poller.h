// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <memory>
#include "core/frontend/input.h"
#include "input_common/tas/tas_input.h"

namespace InputCommon {

/// A button device factory that creates button devices from TAS
class TasButtonFactory final : public Input::Factory<Input::ButtonDevice> {
public:
    explicit TasButtonFactory(std::shared_ptr<TasInput::Tas> tas_input_);

    /**
     * Creates a button device from a button press
     * @param params contains parameters for creating the device:
     *     - "code": the code of the key to bind with the button
     */
    std::unique_ptr<Input::ButtonDevice> Create(const Common::ParamPackage& params) override;

private:
    std::shared_ptr<TasInput::Tas> tas_input;
};

/// An analog device factory that creates analog devices from TAS
class TasAnalogFactory final : public Input::Factory<Input::AnalogDevice> {
public:
    explicit TasAnalogFactory(std::shared_ptr<TasInput::Tas> tas_input_);

    std::unique_ptr<Input::AnalogDevice> Create(const Common::ParamPackage& params) override;

private:
    std::shared_ptr<TasInput::Tas> tas_input;
};

/// A motion device factory that creates motion devices from TAS
class TasMotionFactory final : public Input::Factory<Input::MotionDevice> {
public:
    explicit TasMotionFactory(std::shared_ptr<TasInput::Tas> tas_input_);

    std::unique_ptr<Input::MotionDevice> Create(const Common::ParamPackage& params) override;

private:
    std::shared_ptr<TasInput::Tas> tas_input;
};

} // namespace InputCommon