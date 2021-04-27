yuzu emulator
=============
Special thanks in this version goes to [Naii the Baf](https://github.com/Naii-the-Baf/yuzu/tree/naii-tas) for creating and providing the first TASing version of Yuzu, and averne for pointing out the perfect location for syncing the inputs!

If you have any issues with this custom build, ping me (MonsterDruide1) on the [SMO TASing Discord](https://discord.gg/U7tUnkxkAT).

### Setup TAS

Before you can start creating a TAS by your own, you have to match a few settings in the configuration of Yuzu:
1. Open the configuration menu using `Emulation` -> `Configure...`.
2. On the first page (`General` -> `General`), disable `Multicore CPU Emulation` and `Pause emulation when in background`.
3. Next, go to `System` -> `Filesystem`, and select the path to your script at the very bottom.
4. In the config of `CPU` -> `CPU`, the accuracy should be on `Accurate`.
5. (optional) For a small speed boost, you can set `Graphics` -> `Advanced` -> `Accuracy Level` to `Normal`. 

### Keymap

Hotkey | Description
------ | -----------
CTRL + F5 | Starts the playback of the selected script file. Make sure to set the `Input Device` to `TAS`! (see below, `Running TAS`)
CTRL + F6 | Refreshes the file and resets the playback progress.
CTRL + F7 | Record a new script file. See below for a guide on that.


### Recording TAS

**WARNING: Recording a TAS will override your current script file!**

Start by setting up your location/environment. When you're ready for recording, press `CTRL + F7` to start the recording. Now just execute whatever inputs you want to replicate later, and press the keys again (`CTRL + F7`) once you're done.

During this whole progress, the `Input Device` remains on a normal setting, like `Controller` or `Keyboard/Mouse`.

### Running TAS

First, set up the location/environment using your desired method of input (to be configured in `Controls`). When you are done and ready to start the script, open the `Controls` settings once again, and select `TAS` as the `Input Device`.

For debugging the script, you can enable `View` -> `Debugging` -> `Controller P1` to have an input display of Player 1. This helps in finding timing issues.

Now, before starting the playback, you may want to refresh the file and reset playback progress if you already played a TAS before. Just getting used to always reset before playbeck will avoid the issue of playing back an older version of the file, if you changed the script while Yuzu was still running.

Press `CTRL + F6` to refresh the file, then `CTRL + F5` to start the playback.

If you still run into any issues, feel free to ping me (MonsterDruide1) on the [SMO TASing Discord](https://discord.gg/U7tUnkxkAT).

## Default README

[![Travis CI Build Status](https://travis-ci.com/yuzu-emu/yuzu.svg?branch=master)](https://travis-ci.com/yuzu-emu/yuzu)
[![Azure Mainline CI Build Status](https://dev.azure.com/yuzu-emu/yuzu/_apis/build/status/yuzu%20mainline?branchName=master)](https://dev.azure.com/yuzu-emu/yuzu/)
[![Discord](https://img.shields.io/discord/398318088170242053?color=%237289DA&label=yuzu&logo=discord&logoColor=white)](https://discord.com/invite/u77vRWY)

yuzu is an experimental open-source emulator for the Nintendo Switch from the creators of [Citra](https://citra-emu.org/).

It is written in C++ with portability in mind, with builds actively maintained for Windows and Linux. The emulator is capable of running several commercial games.

yuzu only emulates a subset of Switch hardware and therefore most commercial games **do not** run at full speed or are not fully functional.

Do you want to check which games are compatible and which ones are not? Please visit our [Compatibility page](https://yuzu-emu.org/game/)!

yuzu is licensed under the GPLv2 (or any later version). Refer to the license.txt file included.

Check out our [website](https://yuzu-emu.org/)!

For development discussion, please join us on [Discord](https://discord.com/invite/u77vRWY).

### Development

Most of the development happens on GitHub. It's also where [our central repository](https://github.com/yuzu-emu/yuzu) is hosted.

If you want to contribute please take a look at the [Contributor's Guide](https://github.com/yuzu-emu/yuzu/wiki/Contributing) and [Developer Information](https://github.com/yuzu-emu/yuzu/wiki/Developer-Information). You should also contact any of the developers on Discord in order to know about the current state of the emulator.

If you want to contribute to the user interface translation, please check out the [yuzu project on transifex](https://www.transifex.com/yuzu-emulator/yuzu). We centralize translation work there, and periodically upstream translations.

### Building

* __Windows__: [Windows Build](https://github.com/yuzu-emu/yuzu/wiki/Building-For-Windows)
* __Linux__: [Linux Build](https://github.com/yuzu-emu/yuzu/wiki/Building-For-Linux)


### Support
We happily accept monetary donations, or donated games and hardware. Please see our [donations page](https://yuzu-emu.org/donate/) for more information on how you can contribute to yuzu. Any donations received will go towards things like:
* Switch consoles to explore and reverse-engineer the hardware
* Switch games for testing, reverse-engineering, and implementing new features
* Web hosting and infrastructure setup
* Software licenses (e.g. Visual Studio, IDA Pro, etc.)
* Additional hardware (e.g. GPUs as-needed to improve rendering support, other peripherals to add support for, etc.)

We also more than gladly accept used Switch consoles, preferably ones with firmware 3.0.0 or lower! If you would like to give yours away, don't hesitate to join our [Discord](https://discord.gg/VXqngT3) and talk to bunnei. You may also contact: donations@yuzu-emu.org.
