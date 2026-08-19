# Conker's Bad Fur Day: Recompiled (PC Native Port)

A static recompilation project for *Conker's Bad Fur Day* (Nintendo 64) targeting modern PC platforms (Windows / Linux) with native 60+ FPS, widescreen/ultrawide support, modern graphics rendering (DirectX 12 / Vulkan via RT64), and modding capabilities.

---

## ⚖️ Legal Disclaimer

This project is an independent clean-room reverse engineering effort created for educational purposes, software interoperability, and digital preservation.

- **NO COPYRIGHTED ASSETS INCLUDED:** This repository does not distribute any game ROMs, proprietary assets, audio, textures, 3D models, or copyrighted binaries belonging to Nintendo, Rare Ltd., or Microsoft.
- **ROM REQUIREMENT:** Users and developers must supply their own legally acquired copy of *Conker's Bad Fur Day (USA)* (`baserom.us.z64`, SHA-1: `4CBADD3C4E0729DEC46AF64AD018050EADA4F47A`) to extract assets at build/runtime.

---

## 🛠️ Requirements & Toolchain

- **OS:** Windows 10/11 (64-bit) or Linux
- **Compiler:** MSVC (Visual Studio 2022 C++) or Clang / GCC
- **Python:** 3.10+ (with `splat64`, `spimdisasm`, `m2c`)
- **CMake:** 3.20+
- **Git**

---

## 🚀 Quick Start

### 1. Verify your ROM
Place your clean US ROM (`.n64` / `.v64` / `.z64`) in the root folder and run:
```bash
python check_rom.py
```
This generates `baserom.us.z64` verified with SHA-1 `4CBADD3C4E0729DEC46AF64AD018050EADA4F47A`.

### 2. Extract Segments (Splat)
```bash
cd conker_ref
python -m splat split conker.us.yaml
```

---

## 📜 License

This project is licensed under the [MIT License](LICENSE).
