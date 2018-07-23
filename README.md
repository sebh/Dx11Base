# Dx11Base

![dx11appscreenshot](https://github.com/sebh/Dx11Base/blob/master/DX11Application.png)

A small DirectX 11 program I use to test shaders and techniques (windows only). It is meant to be simple and extended to your needs. Nothing fancy to see here: plenty of _engines_ already exist out there.

Features are
* Simple class helpers above DirectX 11.X functions
* Live update of shaders with saving via `ctrl+s`
* UI achieved with [Dear ImGui](https://github.com/ocornut/imgui)
* Performance measured with GPU timers and reported in UI (tested on intel and nvidia so far)
* Simple window and input management (could be improved)

When cloning the project the first time:
1. Update submodules (run `git submodule update`)
2. In Visual Studio, change the _Application_ project _Working Directory_ from `$(ProjectDir)` to `$(SolutionDir)`

Submodules
* [imgui](https://github.com/ocornut/imgui) V1.62

Have fun and do not hesitate to send back suggestions.

Seb
