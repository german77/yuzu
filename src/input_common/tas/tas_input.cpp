// Copyright 2021 yuzu Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.


#include <chrono>
#include <cstring>
#include <functional>
#include <random>
#include <thread>
#include <boost/asio.hpp>
#include <regex>

#include "common/settings.h"
#include "common/logging/log.h"
#include "input_common/tas/tas_input.h"
#include "common/file_util.h"

namespace TasInput {

    Tas::Tas() {
        LoadTasFile();
    }

    Tas::~Tas() {
        update_thread_running = false;
    }

    void Tas::RefreshTasFile() {
        refresh_tas_fle = true;
    }
    void Tas::LoadTasFile() {
        if (!newCommands.empty()) {
            newCommands.clear();
        }
        std::string file = "";
        LOG_ERROR(Input, "file reloaded {}", Common::FS::ReadFileToString(true, Settings::values.tas_path, file));
        std::stringstream command_line(file);
        std::string line;
        int frameNo = 0;
        TASCommand empty = {.buttons = 0, .l_axis = {0.f, 0.f, true}, .r_axis = {0.f, 0.f, true}};
        while (std::getline(command_line, line, '\n')) {
            std::smatch m;

            std::stringstream linestream(line);
            std::string segment;
            std::vector<std::string> seglist;

            while (std::getline(linestream, segment, ' ')) {
                seglist.push_back(segment);
            }

            while (frameNo != std::stoi(seglist.at(0))) {
                newCommands.push_back(empty);
                frameNo++;
            }

            TASCommand command = {
                .buttons = ReadCommandButtons(seglist.at(1)),
                .l_axis = ReadCommandAxis(seglist.at(2)),
                .r_axis = ReadCommandAxis(seglist.at(3)),
            };
            newCommands.push_back(command);
            frameNo++;
        }
    }

    void Tas::WriteTasFile() {
        std::string output_text = "";
        InputCommand prev_line{};
        for (const InputCommand line : input_commands) {
            LOG_CRITICAL(Input, "button:{} wait:{}", line.buttons, line.wait);
            u32 press_line_bits = 0;
            u32 release_line_bits = 0;
            u32 buttons = line.buttons;
            u32 buttons_old = prev_line.buttons;
            u32 index = 0;
            while (buttons > 0 || buttons_old > 0) {
                if ((buttons & 1) != (buttons_old & 1)) {
                    if (buttons & 1) {
                        press_line_bits += 1 << index;
                    }
                    else {
                        release_line_bits += 1 << index;
                    }
                }
                buttons >>= 1;
                buttons_old >>= 1;
                index++;
            }
            if (press_line_bits != 0) {
                output_text += "press" + WriteCommandButtons(press_line_bits);
            }
            if (release_line_bits != 0) {
                output_text += " release" + WriteCommandButtons(release_line_bits);
            }
            {
                auto [x, y] = line.axes[0];
                auto [old_x, old_y] = prev_line.axes[0];
                if (x != old_x || y != old_y) {
                    output_text += " laxis " + std::to_string(x) + ";" + std::to_string(-y);
                }
            }
            {
                auto [x, y] = line.axes[1];
                auto [old_x, old_y] = prev_line.axes[1];
                if (x != old_x || y != old_y) {
                    output_text += " raxis " + std::to_string(x) + ";" + std::to_string(-y);
                }
            }
            if (line.wait > 0) {
                output_text += " wait " + std::to_string(line.wait);
            }
            output_text += "\n";
            prev_line = line;
        }
        LOG_CRITICAL(Input, "{}", output_text);
        Common::FS::WriteStringToFile(true, Settings::values.tas_path, output_text);
    }

    void Tas::RecordInput(u32 buttons, std::array<std::pair<float, float>, 2> axes, bool changed) {
        //UpdateThread();
        if (!Settings::values.tas_record) {
            return;
        }
        if (changed) {
            input_commands.push_back({ buttons, axes, 0 });
        }
        else {
            if (!input_commands.empty()) {
                //std::pair<float, float> blank_axes = {0.0f, 0.0f};
                //input_commands.push_back({0, {blank_axes, blank_axes}, 0});
                input_commands.back().wait++;
            }
        }
        LOG_ERROR(Input, "button {} {};{} {};{} {}", buttons, axes[0].first, axes[0].second, axes[1].first, axes[1].second, changed);
    }

    void Tas::UpdateThread() {
        if (update_thread_running) {
            if (!Settings::values.tas_record && !input_commands.empty()) {
                WriteTasFile();
                Settings::values.tas_reset = true;
                refresh_tas_fle = true;
                input_commands.clear();
            }
            if (Settings::values.tas_reset) {
                current_command = 0;
                if (refresh_tas_fle) {
                    LoadTasFile();
                    refresh_tas_fle = false;
                }
                Settings::values.tas_reset = false;
                LoadTasFile();
                LOG_ERROR(Input, "reset");
            }
            if (Settings::values.tas_enable) {
                if ((signed)current_command < newCommands.size()) {
                    TASCommand command = newCommands[current_command++];
                    LOG_ERROR(Input, "updateThread(): {}, buttons {}", current_command-1, command.buttons);
                    tas_data[0].buttons = command.buttons;
                    auto [l_axis_x, l_axis_y, l_axis_changed] = command.l_axis;
                    tas_data[0].axis[0] = l_axis_x;
                    tas_data[0].axis[1] = l_axis_y;
                    auto [r_axis_x, r_axis_y, r_axis_changed] = command.r_axis;
                    tas_data[0].axis[2] = r_axis_x;
                    tas_data[0].axis[3] = r_axis_y;
                } else {
                    Settings::values.tas_enable = false;
                    current_command = 0;
                    tas_data[0].buttons = 0;
                    tas_data[0].axis = {};
                }
            } else {
                tas_data[0].buttons = 0;
                tas_data[0].axis = {};
            }
        }
    }

    TasAnalog Tas::ReadCommandAxis(const std::string line) const {
        std::stringstream linestream(line);
        std::string segment;
        std::vector<std::string> seglist;
        LOG_DEBUG(Input, "axis {}", line);

        while (std::getline(linestream, segment, ';')) {
            seglist.push_back(segment);
        }
        const float x = std::stof(seglist.at(0)) / 32767.f;
        const float y = std::stof(seglist.at(1)) / 32767.f;

        return { x, y, true };
    }

    u32 Tas::ReadCommandButtons(const std::string data) const {
        std::stringstream button_text(data);
        std::string line;
        u32 buttons = 0;
        static const std::array<std::pair<std::string, TasButton>, 20>
            text_to_tas_button = {
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
        while (std::getline(button_text, line, ';')) {
            for (auto [text, tas_button] : text_to_tas_button) {
                if (text == line) {
                    buttons |= static_cast<u32>(tas_button);
                    break;
                }
            }
        }
        return buttons;
    }

    std::string Tas::WriteCommandButtons(u32 data) const {
        LOG_CRITICAL(Input, "input {}", data);
        std::string line;
        static const std::array<std::pair<std::string, TasButton>, 20> tas_to_text_button = {
            std::pair{"A", TasButton::BUTTON_A}, {"B", TasButton::BUTTON_B},
            {"X", TasButton::BUTTON_X},          {"Y", TasButton::BUTTON_Y},
            {"LSTICK", TasButton::STICK_L},      {"RSTICK", TasButton::STICK_R},
            {"L", TasButton::TRIGGER_L},         {"R", TasButton::TRIGGER_R},
            {"PLUS", TasButton::BUTTON_PLUS},    {"MINUS", TasButton::BUTTON_MINUS},
            {"LEFT", TasButton::BUTTON_LEFT},    {"UP", TasButton::BUTTON_UP},
            {"RIGHT", TasButton::BUTTON_RIGHT},  {"DOWN", TasButton::BUTTON_DOWN},
            {"SL", TasButton::BUTTON_SL},        {"SR", TasButton::BUTTON_SR},
            {"CAPT", TasButton::BUTTON_CAPTURE}, {"HOME", TasButton::BUTTON_HOME},
            {"ZL", TasButton::TRIGGER_ZL},       {"ZR", TasButton::TRIGGER_ZR},
        };
        u32 index = 0;
        while (data > 0) {
            if ((data & 1) == 1) {
                for (auto [text, tas_button] : tas_to_text_button) {
                    if (tas_button == static_cast<TasButton>(1 << index)) {
                        line += " " + text;
                        break;
                    }
                }
            }
            index++;
            data >>= 1;
        }
        return line;
    }

    InputCommon::ButtonMapping Tas::GetButtonMappingForDevice(
        const Common::ParamPackage& params) const {
        // This list is missing ZL/ZR since those are not considered buttons.
        // We will add those afterwards
        // This list also excludes any button that can't be really mapped
        static constexpr std::array<std::pair<Settings::NativeButton::Values, TasButton>, 20>
            switch_to_tas_button = {
                std::pair{Settings::NativeButton::A, TasButton::BUTTON_A},
                {Settings::NativeButton::B, TasButton::BUTTON_B},
                {Settings::NativeButton::X, TasButton::BUTTON_X},
                {Settings::NativeButton::Y, TasButton::BUTTON_Y},
                {Settings::NativeButton::LStick, TasButton::STICK_L},
                {Settings::NativeButton::RStick, TasButton::STICK_R},
                {Settings::NativeButton::L, TasButton::TRIGGER_L},
                {Settings::NativeButton::R, TasButton::TRIGGER_R},
                {Settings::NativeButton::Plus, TasButton::BUTTON_PLUS},
                {Settings::NativeButton::Minus, TasButton::BUTTON_MINUS},
                {Settings::NativeButton::DLeft, TasButton::BUTTON_LEFT},
                {Settings::NativeButton::DUp, TasButton::BUTTON_UP},
                {Settings::NativeButton::DRight, TasButton::BUTTON_RIGHT},
                {Settings::NativeButton::DDown, TasButton::BUTTON_DOWN},
                {Settings::NativeButton::SL, TasButton::BUTTON_SL},
                {Settings::NativeButton::SR, TasButton::BUTTON_SR},
                {Settings::NativeButton::Screenshot, TasButton::BUTTON_CAPTURE},
                {Settings::NativeButton::Home, TasButton::BUTTON_HOME},
                {Settings::NativeButton::ZL, TasButton::TRIGGER_ZL},
                {Settings::NativeButton::ZR, TasButton::TRIGGER_ZR},
        };

        InputCommon::ButtonMapping mapping{};
        for (const auto& [switch_button, tas_button] : switch_to_tas_button) {
            Common::ParamPackage button_params({ {"engine", "tas"} });
            button_params.Set("pad", params.Get("pad", 0));
            button_params.Set("button", static_cast<int>(tas_button));
            mapping.insert_or_assign(switch_button, std::move(button_params));
        }

        return mapping;
    }

    InputCommon::AnalogMapping Tas::GetAnalogMappingForDevice(
        const Common::ParamPackage& params) const {

        InputCommon::AnalogMapping mapping = {};
        Common::ParamPackage left_analog_params;
        left_analog_params.Set("engine", "tas");
        left_analog_params.Set("pad", params.Get("pad", 0));
        left_analog_params.Set("axis_x", static_cast<int>(TasAxes::StickX));
        left_analog_params.Set("axis_y", static_cast<int>(TasAxes::StickY));
        mapping.insert_or_assign(Settings::NativeAnalog::LStick, std::move(left_analog_params));
        Common::ParamPackage right_analog_params;
        right_analog_params.Set("engine", "tas");
        right_analog_params.Set("pad", params.Get("pad", 0));
        right_analog_params.Set("axis_x", static_cast<int>(TasAxes::SubstickX));
        right_analog_params.Set("axis_y", static_cast<int>(TasAxes::SubstickY));
        mapping.insert_or_assign(Settings::NativeAnalog::RStick, std::move(right_analog_params));
        return mapping;
    }

    TasData& Tas::GetTasState(std::size_t pad) {
        return tas_data[pad];
    }

    const TasData& Tas::GetTasState(std::size_t pad) const {
        return tas_data[pad];
    }
} // namespace TasInput