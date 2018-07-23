# Dx11Base

![dx11appscreenshot](https://github.com/sebh/Dx11Base/blob/master/DX11Application.png)

A small dx11 base program I use to test shaders and techniques (windows Dx11 only).
It is meant to be simple and extended to your needs (nothing fancy here).

Features are
* Simple class helpers above DirectX 11.X functions.
* Live update of shaders upon saving (via ctrl+s)
* UI achieved with [imgui](https://github.com/ocornut/imgui)
* Performance measured with GPU timers and reported in UI (tested on intel and nvidia so far)
* Simple window and input management (could be improved)

When cloning the project the first time
* Update submodules (run `git submodule update`)
* In Visual Studio, change the _Application_ project _Working Directory_ from `$(ProjectDir)` to `$(SolutionDir)`

Submodules (do not forget to run `git submodule update`)
* [imgui](https://github.com/ocornut/imgui) V1.62

Have fun and do not hesitate to send back suggestions.

Seb
