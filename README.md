<p align="center">
  <img src="banner.jpg" alt="Conker: Recompiled" width="100%" />
</p>

<h1 align="center">🍺 CONKER64: RECOMPILED 🐿️</h1>

<p align="center">
  <b>The Unofficial Native PC Port of <i>Conker's Bad Fur Day</i> for Modern Systems.</b><br>
  Built via Static Recompilation, C++20, SDL2, and Hardware-Accelerated Rendering.
</p>

<p align="center">
  <a href="README.es.md"><img src="https://img.shields.io/badge/Idioma-Espa%C3%B1ol-red?style=for-the-badge&logo=googletranslate" alt="Leer en Español" /></a>
  <img src="https://img.shields.io/badge/Platform-Windows%20%7C%20Linux-blue?style=for-the-badge&logo=windows" />
  <img src="https://img.shields.io/badge/Target_FPS-60%20%2F%20120%20%2F%20Unlocked-brightgreen?style=for-the-badge" />
  <img src="https://img.shields.io/badge/Aspect_Ratio-16%3A9%20%2F%20Ultrawide-orange?style=for-the-badge" />
  <img src="https://img.shields.io/badge/License-MIT-green?style=for-the-badge" />
</p>

---

## ⚡ What is this?

In 2001, **Rareware** pushed the Nintendo 64 hardware to its absolute breaking point with *Conker's Bad Fur Day*. Between massive dynamic voice lines, MP3 audio streams, advanced facial animations, and complex geometry, the original console was on its knees, frequently dipping into painful **12–18 FPS**.

**CONKER64: RECOMPILED** aims to fix that legacy permanently. Instead of running through traditional high-overhead emulation, this project uses **Static Recompilation (MIPS ➔ Native x86_64 C++)** to translate the original game logic directly into native machine code.

### 🌟 Target Features
- 🚀 **Buttery Smooth Framerate:** Native 60 FPS, 120 FPS, and Unlocked Refresh Rates.
- 🖥️ **Widescreen & Ultrawide:** True 16:9 / 21:9 support with proper FOV scaling.
- 🎮 **Modern Input & Low Latency:** Full DualSense, Xbox Controller, Switch Pro Controller, and Keyboard/Mouse mapping via SDL2.
- 🎨 **Visual Enhancements:** Support for RT64 (Ray Tracing), HD Texture Injection, and Modern Post-Processing.
- 💾 **Native Save Persistence:** Official 16Kbit EEPROM hardware emulation directly to `%APPDATA%\ConkerRecompiled`.
- 🛠️ **Modding API:** Direct C++ hook injection for custom levels, skins, and community gameplay patches.

---

## 📊 Current Project Status

- [x] **Verified ROM Pipeline:** Clean bit-perfect extraction of US NTSC release (`SHA-1: 4CBADD3C4E0729DEC46AF64AD018050EADA4F47A`).
- [x] **Rareware Proprietary Decompression:** Custom RZIP + XOR decryption (`0x8039CCCA`) implemented and functional.
- [x] **Native x64 Application Framework:** SDL2 engine shell running in C++20 with real-time V-Sync frame loop.
- [x] **Virtual RDRAM & Hardware VI:** 8MB Expansion Pak bus with 4:3 Pillarbox texture decoding in GPU.
- [x] **Hardware Save System:** Official `libultra` `osEeprom` hooks mapped to Windows AppData.
- [x] **MIPS Instruction Disassembly:** Segment symbol analysis of `init` and `code` binaries.
- [ ] **Gfx / RSP Microcode Emulation:** Intercepting display lists for Vulkan / DirectX 12 render passes.
- [ ] **Audio DMA Streamer:** Multi-channel PCM / MP3 voice playback routing.

---

## 🛠️ Building from Source

### Prerequisites
- **Visual Studio 2022 / 2026** (Desktop development with C++)
- **CMake** 3.20+
- **Python** 3.10+ (with `splat64`, `spimdisasm`, `m2c`)
- **Git**

### Step-by-Step
```bash
# 1. Clone the repository
git clone https://github.com/Conker64Recomp/conker64-recompiled.git
cd conker64-recompiled/recomp

# 2. Compile Native Executable
build_game.bat

# 3. Launch
.\build\Conker.exe
```

---

## ⚖️ Legal Notice & Disclaimer

This project is a clean-room reverse engineering and digital preservation effort created strictly for educational and interoperability purposes.

- **NO COPYRIGHTED ASSETS ARE HOSTED OR DISTRIBUTED:** This repository contains no game ROMs, proprietary textures, 3D meshes, copyrighted audio, or proprietary binaries owned by Nintendo Co., Ltd., Rare Ltd., or Microsoft Corporation.
- **ROM REQUIREMENT:** Users must supply their own legally acquired copy of *Conker's Bad Fur Day (USA)* (`baserom.us.z64`) to build and play.
- All product and company names are trademarks™ or registered® trademarks of their respective holders.

---

<p align="center">
  <i>Developed with ❤️ by the Conker Recompiled Contributors. Bad Fur Day lives on!</i>
</p>
