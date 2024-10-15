# Fussion
<p align="center">
  <img src="Editor/Assets/Icons/logo_512.png" width="128" />
  <img src="https://github.com/user-attachments/assets/7a007570-5b04-4676-b666-a27a1ae8f20f" />
</p>

This is a very simple project that aims to help me learn how graphics work and how to develop a simple game engine. It is __NOT__ a serious tool but a learning project, so please keep that in mind.

## Status
Currently, only work on the "Editor" part is being done. Once i'm satisfied with that, the next step would be to get the "runtime" working, meaning
being able to load a "cooked" game from a single asset file.

## Architecture
The engine uses a "component" based system. Entities exist in the world and can have multiple components attached to them. It follows the logic of Unity's MonoBehaviour and not an ECS.
The reason for this is that i just feel more comfortable working with this type of components and speed is not a hude concern right now.

[wgpu-native](https://github.com/gfx-rs/wgpu-native) is used for rendering.

## Building and Running
> [!WARNING]  
> The project is highly volatile and there is a chance you won't be able to compile it.
> If you do encounter issues, please open in issue, i would love to help you get it working!

### Linux
Make sure the GLFW build dependencies are met by following GLFW [compilation guide](https://www.glfw.org/docs/latest/compile.html).

The project will build GLFW for X11 by default, mainly because RenderDoc doesn't support wayland. This can be overriden in `Vendor.lua`.

### MacOS
No

### Requirements
- A C++ compiler
- XMake

### Building the Editor

Building the editor will also build the engine library as a dependency.

Configure xmake to build either in release or debug mode:
```shell
xmake f -m debug|release
```

Once that is done, you should be able to build the engine by simple running xmake with no args:
```shell
xmake
```

### Running
If you run the Editor without any arguments, a project creator should appear.

You can then create a new project and the editor should automatically launch to the newly created
project!

### Discord
Everybody and their mum has a discord server, so why not me? :)
[Join the Discord](https://discord.gg/K9QfYjKwng)
