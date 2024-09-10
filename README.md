# AllianceSagePlayer
某寝室用。

## How to play
1. Run the programme. A file-select-dialogue will be shown.
2. Select a script file named as `*_3.txt`, `*_4.txt`, or `7*.txt`.
 <pre>
event_ja
  ├ kizuna
  │  ├ ...
  │  ├ 1158_3.txt
  │  ├ 1158_4.txt
  │  └ ...
  ├ memory
  │  ├ ...
  │  ├ 7094.txt
  │  └ ...
  └ ...
 </pre>

 Then, voice files and spine files are loaded according to the specification of the selected script file.
 <pre>
bundles
  ├ ...
  ├ cv
  │  ├ cv_XXXX // voice folder specified in the script file
  │  └ ...
  ├ ...
  ├ event_ja
  │  ├ ...
  │  ├ kizuna
  │  ├ ...
  │  ├ memory
  │  └ ...
  ├ XXXXXX // spine folder specified in the script file.
  ├ XXXXXX01
  └ ...
 </pre>

## Mouse function
| Input  | Function  |
| --- | --- |
| Mouse wheel | Scale up/down |
| Left button + mouse wheel | Speed up/down the animation. |
| Left button click | Switch to the next animation. |
| Left button drag | Move view point |
| Middle button | Reset scaling, animation speed, and view point. |
| Right button + mouse wheel | Show the next/previous text. |
| Right button + left button | Move Window |

## Keyboard function
| Input  | Function  |
| --- | --- |
| A | Enable/disable premultiplied alpha. |
| B | Prefer/ignore blend-mode specified by slots. |
| C | Switch text colour between black and white. |
| T | Show/hide text. |
| Esc | Close the application. |
| Up | Open the next script. |
| Down | Open the previous script. |

## Build dependency
- [SDL2-2.30.3](https://github.com/libsdl-org/SDL/releases/tag/release-2.30.3)
- [SDL2_image-2.8.2](https://github.com/libsdl-org/SDL_image/releases/tag/release-2.8.2)
- [SDL2_ttf-2.22.0](https://github.com/libsdl-org/SDL_ttf/releases/tag/release-2.22.0)
- [spine-cpp-3.8](https://github.com/EsotericSoftware/spine-runtimes/tree/3.8)

When building, supply the above libraries under project folder `/AllianceSagePlayer/deps`.
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
  │  └ spine-cpp-3.8 // sources and headers of spine-cpp 3.8
  │     ├ include
  │     │  └ ...
  │     └ src
  │          └ ...
  ├ ...
  ├ AllianceSagePlayer.vcxproj
  └ ...
</pre>
