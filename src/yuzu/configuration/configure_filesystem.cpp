// Copyright 2019 yuzu Emulator Project
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <QFileDialog>
#include <QMessageBox>
#include "common/common_paths.h"
#include "common/file_util.h"
#include "common/settings.h"
#include "ui_configure_filesystem.h"
#include "yuzu/configuration/configure_filesystem.h"
#include "yuzu/uisettings.h"

ConfigureFilesystem::ConfigureFilesystem(QWidget* parent)
    : QWidget(parent), ui(std::make_unique<Ui::ConfigureFilesystem>()) {
    ui->setupUi(this);
    this->setConfiguration();

    connect(ui->nand_directory_button, &QToolButton::pressed, this,
            [this] { SetDirectory(DirectoryTarget::NAND, ui->nand_directory_edit); });
    connect(ui->sdmc_directory_button, &QToolButton::pressed, this,
            [this] { SetDirectory(DirectoryTarget::SD, ui->sdmc_directory_edit); });
    connect(ui->gamecard_path_button, &QToolButton::pressed, this,
            [this] { SetDirectory(DirectoryTarget::Gamecard, ui->gamecard_path_edit); });
    connect(ui->dump_path_button, &QToolButton::pressed, this,
            [this] { SetDirectory(DirectoryTarget::Dump, ui->dump_path_edit); });
    connect(ui->load_path_button, &QToolButton::pressed, this,
            [this] { SetDirectory(DirectoryTarget::Load, ui->load_path_edit); });
    connect(ui->tas_path_button, &QToolButton::pressed, this,
            [this] { SetDirectory(DirectoryTarget::TAS, ui->tas_path_edit); });

    connect(ui->reset_game_list_cache, &QPushButton::pressed, this,
            &ConfigureFilesystem::ResetMetadata);

    connect(ui->gamecard_inserted, &QCheckBox::stateChanged, this,
            &ConfigureFilesystem::UpdateEnabledControls);
    connect(ui->gamecard_current_game, &QCheckBox::stateChanged, this,
            &ConfigureFilesystem::UpdateEnabledControls);
}

ConfigureFilesystem::~ConfigureFilesystem() = default;

void ConfigureFilesystem::setConfiguration() {
    ui->nand_directory_edit->setText(
        QString::fromStdString(Common::FS::GetUserPath(Common::FS::UserPath::NANDDir)));
    ui->sdmc_directory_edit->setText(
        QString::fromStdString(Common::FS::GetUserPath(Common::FS::UserPath::SDMCDir)));
    ui->gamecard_path_edit->setText(QString::fromStdString(Settings::values.gamecard_path));
    ui->dump_path_edit->setText(
        QString::fromStdString(Common::FS::GetUserPath(Common::FS::UserPath::DumpDir)));
    ui->load_path_edit->setText(
        QString::fromStdString(Common::FS::GetUserPath(Common::FS::UserPath::LoadDir)));
    ui->tas_path_edit->setText(QString::fromStdString(Settings::values.tas_path));

    ui->gamecard_inserted->setChecked(Settings::values.gamecard_inserted);
    ui->gamecard_current_game->setChecked(Settings::values.gamecard_current_game);
    ui->tas_pause_on_load->setChecked(Settings::values.pauseTasOnLoad);
    ui->dump_exefs->setChecked(Settings::values.dump_exefs);
    ui->dump_nso->setChecked(Settings::values.dump_nso);

    ui->cache_game_list->setChecked(UISettings::values.cache_game_list);

    UpdateEnabledControls();
}

void ConfigureFilesystem::applyConfiguration() {
    Common::FS::GetUserPath(Common::FS::UserPath::NANDDir,
                            ui->nand_directory_edit->text().toStdString());
    Common::FS::GetUserPath(Common::FS::UserPath::SDMCDir,
                            ui->sdmc_directory_edit->text().toStdString());
    Common::FS::GetUserPath(Common::FS::UserPath::DumpDir,
                            ui->dump_path_edit->text().toStdString());
    Common::FS::GetUserPath(Common::FS::UserPath::LoadDir,
                            ui->load_path_edit->text().toStdString());

    Settings::values.tas_path = ui->tas_path_edit->text().toStdString();
    Settings::values.gamecard_inserted = ui->gamecard_inserted->isChecked();
    Settings::values.gamecard_current_game = ui->gamecard_current_game->isChecked();
    Settings::values.pauseTasOnLoad = ui->tas_pause_on_load->isChecked();
    Settings::values.dump_exefs = ui->dump_exefs->isChecked();
    Settings::values.dump_nso = ui->dump_nso->isChecked();

    UISettings::values.cache_game_list = ui->cache_game_list->isChecked();
}

void ConfigureFilesystem::SetDirectory(DirectoryTarget target, QLineEdit* edit) {
    QString caption;

    switch (target) {
    case DirectoryTarget::NAND:
        caption = tr("Select Emulated NAND Directory...");
        break;
    case DirectoryTarget::SD:
        caption = tr("Select Emulated SD Directory...");
        break;
    case DirectoryTarget::Gamecard:
        caption = tr("Select Gamecard Path...");
        break;
    case DirectoryTarget::Dump:
        caption = tr("Select Dump Directory...");
        break;
    case DirectoryTarget::Load:
        caption = tr("Select Mod Load Directory...");
        break;
    case DirectoryTarget::TAS:
        caption = tr("Select TAS File...");
        break;
    }

    QString str;
    if (target == DirectoryTarget::Gamecard) {
        str = QFileDialog::getOpenFileName(this, caption, QFileInfo(edit->text()).dir().path(),
                                           QStringLiteral("NX Gamecard;*.xci"));
    } else if (target == DirectoryTarget::TAS) {
        str = QFileDialog::getOpenFileName(this, caption, QFileInfo(edit->text()).dir().path(),
                                           QStringLiteral("Text File;*.txt"));
    } else {
        str = QFileDialog::getExistingDirectory(this, caption, edit->text());
        if (!str.isNull() && str.back() != QDir::separator()) {
            str.append(QDir::separator());
        }
    }

    if (str.isEmpty())
        return;

    edit->setText(str);
}

void ConfigureFilesystem::ResetMetadata() {
    if (!Common::FS::Exists(Common::FS::GetUserPath(Common::FS::UserPath::CacheDir) + DIR_SEP +
                            "game_list")) {
        QMessageBox::information(this, tr("Reset Metadata Cache"),
                                 tr("The metadata cache is already empty."));
    } else if (Common::FS::DeleteDirRecursively(
                   Common::FS::GetUserPath(Common::FS::UserPath::CacheDir) + DIR_SEP +
                   "game_list")) {
        QMessageBox::information(this, tr("Reset Metadata Cache"),
                                 tr("The operation completed successfully."));
        UISettings::values.is_game_list_reload_pending.exchange(true);
    } else {
        QMessageBox::warning(
            this, tr("Reset Metadata Cache"),
            tr("The metadata cache couldn't be deleted. It might be in use or non-existent."));
    }
}

void ConfigureFilesystem::UpdateEnabledControls() {
    ui->gamecard_current_game->setEnabled(ui->gamecard_inserted->isChecked());
    ui->gamecard_path_edit->setEnabled(ui->gamecard_inserted->isChecked() &&
                                       !ui->gamecard_current_game->isChecked());
    ui->gamecard_path_button->setEnabled(ui->gamecard_inserted->isChecked() &&
                                         !ui->gamecard_current_game->isChecked());
}

void ConfigureFilesystem::retranslateUi() {
    ui->retranslateUi(this);
}
