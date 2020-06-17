# Dx11Base

![dx11appscreenshot](https://github.com/sebh/Dx11Base/blob/master/DX11Application.png)

A small DirectX 11 program I use to test shaders and techniques (windows only). It is meant to be simple and straightforward. Nothing fancy to see here: plenty of _engines_ already exist out there.

Features are
* Simple class helpers above DirectX 11.X functions
* Live update of shaders with saving via `ctrl+s`
* UI achieved with [Dear ImGui](https://github.com/ocornut/imgui)
* Performance measured with GPU timers and reported in UI (tested on intel and nvidia so far)
* Simple window and input management (could be improved)
* Works well with ![RenderDoc](https://renderdoc.org/)

When cloning the project the first time:
1. Update submodules (run `git submodule update`)
1. Open the solution 
2. In Visual Studio, change the _Application_ project _Working Directory_ from `$(ProjectDir)` to `$(SolutionDir)`
3. Make sure you select a windows SDK and a platform toolset you have locally on your computer for both projects
4. Select _Application_ as the startup project, hit F5

Submodules
* [imgui](https://github.com/ocornut/imgui) V1.62 supported

Have fun and do not hesitate to send back suggestions.

Seb
