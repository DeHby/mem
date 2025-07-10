# mem

[![Standard](https://img.shields.io/badge/C%2B%2B-11-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B11)
[![License](https://img.shields.io/badge/license-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Build status](https://ci.appveyor.com/api/projects/status/81uujm7atdjy5odx/branch/master?svg=true)](https://ci.appveyor.com/project/0x1F9F1/mem/branch/master)

mem is a cross-platform, header-only C++11 library made to aid reverse engineering.

<br/>

---

## 🔧 Enhancements

This repository builds upon the original [`mem`](https://github.com/0x1F9F1/mem) project, with several improvements aimed at extending its flexibility and Windows-specific capabilities:

- 🧩 **Abstracted Memory Access**
Introduced a modular abstraction for memory access, laying groundwork for multi-process or remote memory interaction.
- 🎉 **Windows-Focused Remote Memory Support**
Implemented practical support for **remote memory access**, such as scanning or interacting with memory regions of external processes on Windows platforms.
- 🔍 **Remote-Aware Scanner**
Enhanced the original `scanner` infrastructure to support scanning remote memory with minimal interface changes.
- 🗂 **Reorganized Directory Structure**
Restructured the file and module layout for better clarity, modularity, and ease of extension.

While cross-platform compatibility is preserved for the base structure, most extended functionalities currently target **Windows environments**.