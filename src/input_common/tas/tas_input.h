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

using TasAnalog = std::tuple<float, float, bool>;

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
    void RecordInput(u32 buttons, std::array<std::pair<float, float>, 2> axes, bool changed);

    InputCommon::ButtonMapping GetButtonMappingForDevice(const Common::ParamPackage& params) const;
    InputCommon::AnalogMapping GetAnalogMappingForDevice(const Common::ParamPackage& params) const;
    [[nodiscard]] TasData& GetTasState(std::size_t pad);
    [[nodiscard]] const TasData& GetTasState(std::size_t pad) const;

private:
    struct Command {
        u32 press_buttons{};
        u32 release_buttons{};
        TasAnalog r_axis{};
        TasAnalog l_axis{};
        u32 wait{};
    };
    struct InputCommand {
        u32 buttons{};
        std::array<std::pair<float, float>, 2> axes{};
        u32 wait{};
    };
    void WriteTasFile();
    void UpdateThread();
    u32 ReadCommandPress(const std::string line) const;
    u32 ReadCommandRelease(const std::string line) const;
    TasAnalog ReadCommandRaxis(const std::string line) const;
    TasAnalog ReadCommandLaxis(const std::string line) const;
    u32 ReadCommandWait(const std::string line) const;
    u32 ReadCommandButtons(const std::string line) const;
    std::string WriteCommandButtons(u32 data) const;

    std::thread update_thread;
    std::array<TasData, 7> tas_data;
    bool update_thread_running{true};
    bool refresh_tas_fle{false};
    std::vector<Command> commands{};
    std::vector<InputCommand> input_commands{};
    std::size_t current_command{0};
    u32 wait_for{0};
};
} // namespace TasInput