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

## Building and Running
> [!WARNING]  
> The project is highly volatile and there is a chance you won't be able to compile it.
> If you do encounter issues, please open in issue, i would love to help you get it working!

### Linux
The engine and editor are designed in such a way to be cross-platorm but there are several missing implementation of various systems, simply because i'm too lazy to boot into Linux, implement and test them. This means that most likely it won't compile in Linux.

### MacOS
No

### Requirements
- A C++ compiler
- Vulkan SDK
  - You __MUST__ make sure the SDK is installed with the debug versions of the shader libraries!
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

To launch the editor, you first need to create a project. A project creator is "in the works" and will be runnable with the following command:
```shell
xmake r Editor # Run the Editor target without any args
```

However, currently you need an existing project. A sample is provided under `Editor/Assets/Projects/SampleProject`:
```shell
xmake r Editor -Project=Editor/Assets/Projects/SampleProject/SampleProject.fsnproj
```

Hopefully, the engine should now be running!

### Discord
Everybody and their mum has a discord server, so why not me? :)
[Join the Discord](https://discord.gg/K9QfYjKwng)
