# AllianceSagePlayer

某寝室用。

## How to play

First, prepare the files, commented below, with proper directory.

<pre>
bundles
├ ...
├ cv // voice folder
│  ├ ...
│  ├ cv_1158
│  │  ├ HCG_loop1.m4a
│  │  └ ...
│  └ ...
├ ...
├ event_ja // script folder
│  ├ ...
│  ├ kizuna
│  │  ├ ...
│  │  ├ 1158_3.json // Select file like this.
│  │  ├ 1158_4.json
│  │  └ ...
│  ├ memory
│  │  ├ ...
│  │  ├ 7094.json
│  │  └ ...
│  └ ...
├ hscene // Scene Spine folder
├ ...
├ hscene15
│  ├ ...
│  ├ 1158_cg1.atlas
│  ├ 1158_cg1.json
│  ├ 1158_cg1.png
│  └ ...
└ ...
</pre>

Then, select a script file named as `*_3.json`, `*_4.json`, or `7*.json` from the application.  
The scene will be set up based on the specification of the selected script file.

## Mouse function

| Input | Function |
| --- | --- |
| Wheel scroll | Scale up/down the window. Combinating with `Ctrl` to zoom in/out. |
| Left pressed + wheel scroll | Speed up/down the animation. |
| Left drag | Move view-point |
| Middle click | Reset window scale, animation speed, and view-point to the default. |
| Right pressed + wheel scroll | Fast-forward/rewind the text. |

## Keyboard function

| Input | Function |
| --- | --- |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>A</kbd> | Enable/disable premultiplied alpha. |
| <kbd>B</kbd> | Prefer/ignore blend-mode specified by slots. |
| <kbd>C</kbd> | Toggle text colour between black and white. |
| <kbd>S</kbd> | Save the current frame as an image. |
| <kbd>T</kbd> | Show/hide text. |
| <kbd>↑</kbd> | Open the previous script. |
| <kbd>↓</kbd> | Open the next script. |
| <kbd>→</kbd> | Fast-forward the text. |
| <kbd>←</kbd> | Rewind the text. |

## External libraries

- [SDL3-3.4.0](https://github.com/libsdl-org/SDL/releases/tag/release-3.4.0)
- [SDL3_image-3.2.6](https://github.com/libsdl-org/SDL_image/releases/tag/release-3.2.6)
- [SDL3_ttf-3.2.2](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-3.2.2)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

## Build

Visual Studio is required.

1. Configure `src/deps/CMakeLists.txt` to obtain and modify external libraries.
2. Build Spine generic library for `x64-Debug` and for `x64-Release`.
3. Open `AllianceSagePlayer.sln`.
4. Select `Build Solution` on menu item.

<details><summary>deps directory will be as follows</summary>

<pre>
src
├ deps
│  ├ SDL3_image-3.2.6 // SDL_image header and static lib for VC
│  │  ├ include
│  │  │  └ SDL3_image
│  │  │    └ SDL_image.h
│  │  └ lib
│  │     └ x64
│  │       └ SDL3_image.lib
│  ├ SDL3_ttf-3.2.2 // SDL_ttf header and static lib for VC
│  │  ├ include
│  │  │  └ SDL3_ttf
│  │  │    └ SDL_ttf.h 
│  │  └ lib
│  │     └ x64
│  │       └ SDL3_ttf.lib
│  ├ SDL3-3.4.0 // SDL headers and static lib for VC
│  │  ├ include
│  │  │  └ SDL3
│  │  │    ├ begin_code.h 
│  │  │    └ ...  
│  │  └ lib
│  │     └ x64
│  │       └ SDL3.lib
│  └ spine-cpp-3.8 // Spine genetic C++ runtime for version 3.8.xx
│     ├ include
│     │  └ ...
│     └ src
│        └ ...
└ ...
</pre>

</details>
