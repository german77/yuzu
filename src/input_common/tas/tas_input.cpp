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
        LOG_DEBUG(Input, "LoadTasFile()");
        if (!newCommands.empty()) {
            newCommands.clear();
        }
        std::string file = "";
        Common::FS::ReadFileToString(true, Settings::values.tas_path, file);
        std::stringstream command_line(file);
        std::string line;
        int frameNo = 0;
        TASCommand empty = {.buttons = 0, .l_axis = {0.f, 0.f}, .r_axis = {0.f, 0.f}};
        while (std::getline(command_line, line, '\n')) {
            if (line.empty())
                continue;
            std::smatch m;

            std::stringstream linestream(line);
            std::string segment;
            std::vector<std::string> seglist;

            while (std::getline(linestream, segment, ' ')) {
                seglist.push_back(segment);
            }

            if (seglist.size() < 4)
                continue;

            while (frameNo < std::stoi(seglist.at(0))) {
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
        LOG_INFO(Input, "TAS file loaded!");
    }

    void Tas::WriteTasFile() {
        LOG_DEBUG(Input, "WriteTasFile()");
        std::string output_text = "";
        for (int frame = 0; frame < (signed)recordCommands.size(); frame++) {
            if (!output_text.empty())
                output_text += "\n";
            TASCommand line = recordCommands.at(frame);
            output_text += std::to_string(frame) + " " + WriteCommandButtons(line.buttons) + " " + WriteCommandAxis(line.l_axis) + " " + WriteCommandAxis(line.r_axis);
        }
        Common::FS::WriteStringToFile(true, Settings::values.tas_path, output_text);
        LOG_INFO(Input, "TAS file written to file!");
    }

    void Tas::RecordInput(u32 buttons, std::array<std::pair<float, float>, 2> axes) {
        lastInput = {buttons, flipY(axes[0]), flipY(axes[1])};
    }

    std::pair<float, float> Tas::flipY(std::pair<float, float> old) const {
        auto [x, y] = old;
        return {x, -y};
    }

    std::string Tas::GetStatusDescription() {
        if (Settings::values.tas_record) {
            return "Recording TAS: " + std::to_string(recordCommands.size());
        }
        if (Settings::values.tas_enable) {
            return "Playing TAS: " + std::to_string(current_command) + "/" + std::to_string(newCommands.size());
        }
        return "TAS not running: " + std::to_string(current_command) + "/" + std::to_string(newCommands.size());
    }

    void Tas::UpdateThread() {
        if (update_thread_running) {
            if (Settings::values.pauseTasOnLoad && Settings::values.cpuBoosted) {
                tas_data[0].buttons = 0;
                tas_data[0].axis = {};
            }

            if (Settings::values.tas_record) {
                recordCommands.push_back(lastInput);
            }
            if (!Settings::values.tas_record && !recordCommands.empty()) {
                WriteTasFile();
                Settings::values.tas_reset = true;
                refresh_tas_fle = true;
                recordCommands.clear();
            }
            if (Settings::values.tas_reset) {
                current_command = 0;
                if (refresh_tas_fle) {
                    LoadTasFile();
                    refresh_tas_fle = false;
                }
                Settings::values.tas_reset = false;
                LoadTasFile();
                LOG_DEBUG(Input, "tas_reset done");
            }
            if (Settings::values.tas_enable) {
                if ((signed)current_command < newCommands.size()) {
                    LOG_INFO(Input, "Playing TAS {}/{}", current_command, newCommands.size());
                    TASCommand command = newCommands[current_command++];
                    tas_data[0].buttons = command.buttons;
                    auto [l_axis_x, l_axis_y] = command.l_axis;
                    tas_data[0].axis[0] = l_axis_x;
                    tas_data[0].axis[1] = l_axis_y;
                    auto [r_axis_x, r_axis_y] = command.r_axis;
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

        while (std::getline(linestream, segment, ';')) {
            seglist.push_back(segment);
        }
        const float x = std::stof(seglist.at(0)) / 32767.f;
        const float y = std::stof(seglist.at(1)) / 32767.f;

        return { x, y };
    }

    u32 Tas::ReadCommandButtons(const std::string data) const {
        std::stringstream button_text(data);
        std::string line;
        u32 buttons = 0;
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

    std::string Tas::WriteCommandAxis(TasAnalog data) const {
        auto [x, y] = data;
        std::string line;
        line += std::to_string(static_cast<int>(x*32767));
        line += ";";
        line += std::to_string(static_cast<int>(y*32767));
        return line;
    }

    std::string Tas::WriteCommandButtons(u32 data) const {
        if (data == 0)
            return "NONE";

        std::string line;
        u32 index = 0;
        while (data > 0) {
            if ((data & 1) == 1) {
                for (auto [text, tas_button] : text_to_tas_button) {
                    if (tas_button == static_cast<TasButton>(1 << index)) {
                        if (line.size() > 0)
                            line += ";";
                        line += text;
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