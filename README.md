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
| Mouse wheel | Scale up/down the skeleton. |
| Left button + mouse wheel | Speed up/down the animation. |
| Left button drag | Move view point |
| Middle button | Reset scale, animation speed, and view point. |
| Right button + mouse wheel | Fast-forward/rewind the text. |

## Keyboard function
| Input  | Function  |
| --- | --- |
| <kbd>Esc</kbd> | Close the application. |
| <kbd>A</kbd> | Enable/disable premultiplied alpha. |
| <kbd>B</kbd> | Prefer/ignore blend-mode specified by slots. |
| <kbd>C</kbd> | Toggle text colour between black and white. |
| <kbd>T</kbd> | Show/hide text. |
| <kbd>↑</kbd> | Open the previous script. |
| <kbd>↓</kbd> | Open the next script. |
| <kbd>→</kbd> | Fast-forward the text. |
| <kbd>←</kbd> | Rewind the text. |

## External libraries
- [SDL2-2.30.3](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.3)
- [SDL2_image-2.8.2](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.2)
- [SDL2_ttf-2.22.0](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.22.0)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

## Build

1. Run `AllianceSagePlayer/deps/CMakeLists.txt` to obtain and modify external libraries.
2. Open `AllianceSagePlayer.sln` with Visual Studio.
3. Select `Build Solution` on menu item.

<details><summary>deps directory will be as follows</summary>

<pre>
AllianceSagePlayer
  ├ deps
  │  ├ SDL2_image-2.8.2 // SDL_image header and static lib for VC
  │  │  ├ include
  │  │  │  └ SDL2_image
  │  │  │    └ SDL_image.h
  │  │  └ lib
  │  │     └ x64
  │  │       └ SDL2_image.lib
  │  ├ SDL2_ttf-2.22.0 // SDL_ttf header and static lib for VC
  │  │  ├ include
  │  │  │  └ SDL2_ttf
  │  │  │    └ SDL_ttf.h 
  │  │  └ lib
  │  │     └ x64
  │  │       └ SDL2_ttf.lib
  │  ├ SDL2-2.30.3 // SDL headers and static lib for VC
  │  │  ├ include
  │  │  │  └ SDL2
  │  │  │    ├ begin_code.h 
  │  │  │    └ ...  
  │  │  └ lib
  │  │     └ x64
  │  │       └ SDL2.lib
  │  └ spine-cpp-3.8 // C++ Spine runtime for version 3.8.xx
  │     ├ include
  │     │  └ ...
  │     └ src
  │          └ ...
  ├ ...
  ├ AllianceSagePlayer.vcxproj
  └ ...
</pre>

</details>
