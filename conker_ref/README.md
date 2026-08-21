# Conker's Bad Fur Day Decompilation & Asset Specification

![Conker's Bad Fur Day (US) Progress](https://img.shields.io/badge/dynamic/json?url=https%3A%2F%2Fconker.deco.mp%2Flatest.json&color=critical&label=Conker's%20Bad%20Fur%20Day%20(US)&query=$.progress[0].sections[3].percent&suffix=%25) ![all Functions](https://img.shields.io/badge/funcs-1365%2F5916-blue)

| Section | Decompiled Functions | Status |
|---|---|---|
| `init.us` | 231 / 536 | Matching |
| `game.us` (RZIP Code) | 1114 / 5338 | In Progress |
| `debugger.us` | 20 / 42 | Matching |

---

## 📖 About This Reference

This folder contains the complete assembly, disassembly mappings, asset offsets, and Splat configuration schemas for the **US release of Conker's Bad Fur Day** (`baserom.us.z64`, SHA-1: `4cbadd3c4e0729dec46af64ad018050eada4f47a`).

It is used as the foundational reference data for the **Conker64 Recompiled** project (`recomp/`).

---

## 📦 ROM Memory Layout & Asset Packages

| Offset Range | Type | Description |
|---|---|---|
| `0x00000000 - 0x00000040` | Header | N64 Rom Header & Checksum |
| `0x00000040 - 0x00001000` | IPL3 Boot | Bootloader code |
| `0x00001000 - 0x00042450` | Init Segment | Entry point, memory manager, OS initialization |
| `0x00042450 - 0x0019EA88` | `game.us.rzip` | 508 RZIP compressed MIPS code subsegments (XOR `0x8039CCCA`) |
| `0x001A37E0 - 0x00AB1941` | Master Asset Block | 7,760 compressed per-actor asset records |
| `0x00AB1A40 - 0x00AF4918` | `assets00` | Foliage & Environment geometry |
| `0x00AF4918 - 0x00BB1BA0` | `assets01` | UI, Icons, Font glyphs, HUD props |
| `0x00BB1BA0 - 0x00F8F278` | `assets02` | Level geometry & Interactive props |
| `0x00F8F278 - 0x00F9E660` | `assets03` | Conker Character 3D Model & Display Lists |
| `0x00F9E660 - 0x011542A8` | `assets04` | Large level structures & Skyboxes |
| `0x011542A8 - 0x0117FE50` | `assets05` | Enemy actors & NPCs |
| `0x0117FE50 - 0x012043B0` | `assets06` | Cutscene sequences & intro choreography |
| `0x01204780 - 0x0125CED0` | `assets09` | Multiplayer characters & maps |
| `0x0125CED0 - 0x0129E780` | `assets0A` | Bat's Tower / Barn Boys |
| `0x0129E780 - 0x012A1638` | `assets0B` | Windy / Poop Mountain |
| `0x012A1638 - 0x012DEDF0` | `assets0C` | It's War / Tediz Base |
| `0x012DEFB8 - 0x012E2CF8` | `assets0E` | Sloprano / Great Mighty Poo |
| `0x012E2CF8 - 0x012F5F40` | `assets0F` | Uga Buga / Raptor Arena |
| `0x012F5F40 - 0x012FBCF0` | `assets10` | Spooky / Count Batula Castle |
| `0x012FBCF0 - 0x012FF550` | `assets11` | Heist / Bank Vault |
| `0x01300000 - 0x03FA5000` | `assets16` | 453 MP3 compressed audio & speech streams |

---

## 🛠️ Extraction Tools

- `tools/splat_ext/rzip.py`: Custom Rareware RZIP decrypter supporting XOR `0x8039CCCA`.
- `tools/extract_compressed.py`: Decompresses individual asset files.
- `conker.us.yaml`: Master Splat segmentation configuration.
