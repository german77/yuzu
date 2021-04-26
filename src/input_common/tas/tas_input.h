// Copyright 2020 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#pragma once

#include <array>
#include <mutex>
#include <thread>

#include "common/common_types.h"
#include "core/frontend/input.h"
#include "input_common/main.h"

namespace TasInput {

using TasAnalog = std::tuple<float, float>;

enum class TasButton {
    BUTTON_A = 0x000001,
    BUTTON_B = 0x000002,
    BUTTON_X = 0x000004,
    BUTTON_Y = 0x000008,
    STICK_L = 0x000010,
    STICK_R = 0x000020,
    TRIGGER_L = 0x000040,
    TRIGGER_R = 0x000080,
    TRIGGER_ZL = 0x000100,
    TRIGGER_ZR = 0x000200,
    BUTTON_PLUS = 0x000400,
    BUTTON_MINUS = 0x000800,
    BUTTON_LEFT = 0x001000,
    BUTTON_UP = 0x002000,
    BUTTON_RIGHT = 0x004000,
    BUTTON_DOWN = 0x008000,
    BUTTON_SL = 0x010000,
    BUTTON_SR = 0x020000,
    BUTTON_HOME = 0x040000,
    BUTTON_CAPTURE = 0x080000,
};

static const std::array<std::pair<std::string, TasButton>, 20> text_to_tas_button = {
    std::pair{"KEY_A", TasButton::BUTTON_A},
    {"KEY_B", TasButton::BUTTON_B},
    {"KEY_X", TasButton::BUTTON_X},
    {"KEY_Y", TasButton::BUTTON_Y},
    {"KEY_LSTICK", TasButton::STICK_L},
    {"KEY_RSTICK", TasButton::STICK_R},
    {"KEY_L", TasButton::TRIGGER_L},
    {"KEY_R", TasButton::TRIGGER_R},
    {"KEY_PLUS", TasButton::BUTTON_PLUS},
    {"KEY_MINUS", TasButton::BUTTON_MINUS},
    {"KEY_DLEFT", TasButton::BUTTON_LEFT},
    {"KEY_DUP", TasButton::BUTTON_UP},
    {"KEY_DRIGHT", TasButton::BUTTON_RIGHT},
    {"KEY_DDOWN", TasButton::BUTTON_DOWN},
    {"KEY_SL", TasButton::BUTTON_SL},
    {"KEY_SR", TasButton::BUTTON_SR},
    {"KEY_CAPTURE", TasButton::BUTTON_CAPTURE},
    {"KEY_HOME", TasButton::BUTTON_HOME},
    {"KEY_ZL", TasButton::TRIGGER_ZL},
    {"KEY_ZR", TasButton::TRIGGER_ZR},
};

enum class TasAxes : u8 {
    StickX,
    StickY,
    SubstickX,
    SubstickY,
    Undefined,
};

struct TasData {
    u32 buttons{};
    std::array<float, 4> axis{};
};

class Tas {
public:
    Tas();
    ~Tas();

    void RefreshTasFile();
    void LoadTasFile();
    void RecordInput(u32 buttons, std::array<std::pair<float, float>, 2> axes);
    void UpdateThread();

    InputCommon::ButtonMapping GetButtonMappingForDevice(const Common::ParamPackage& params) const;
    InputCommon::AnalogMapping GetAnalogMappingForDevice(const Common::ParamPackage& params) const;
    [[nodiscard]] TasData& GetTasState(std::size_t pad);
    [[nodiscard]] const TasData& GetTasState(std::size_t pad) const;

private:
    struct TASCommand {
        u32 buttons{};
        TasAnalog l_axis{};
        TasAnalog r_axis{};
    };
    void WriteTasFile();
    TasAnalog ReadCommandAxis(const std::string line) const;
    u32 ReadCommandButtons(const std::string line) const;
    std::string WriteCommandButtons(u32 data) const;
    std::string WriteCommandAxis(TasAnalog data) const;
    std::pair<float, float> flipY(std::pair<float, float> old) const;

    std::array<TasData, 7> tas_data;
    bool update_thread_running{true};
    bool refresh_tas_fle{false};
    std::vector<TASCommand> newCommands{};
    std::vector<TASCommand> recordCommands{};
    std::size_t current_command{0};
    TASCommand lastInput{};
};
} // namespace TasInput