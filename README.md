## What is wine-wayland

Wine-wayland allows running DX9/DX11 and Vulkan games using pure wayland and Wine/DXVK.

## Why wine-wayland

 * You are tired of dealing with X11 and don't care about launchers
 * You want to remove X11 related packages from your PC
 * You want to experience potentially faster and smoother gaming on Wayland
 * You are concerned about insecure X11 games that can spy on other apps running on X11.

## Screenshot

![screenshot](https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot.png "Screenshot")
![screenshot1](https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot1.png "Screenshot1")

## Requirements

 * Archlinux or Manjaro
 * GPU with Vulkan and Wayland support
 * Mesa 20.1 or later with Wayland and Vulkan support
 * weston based compositor (tested on wayward), wlroots based compositor (tested on sway)
 * SDL and Faudio
 
## Download

You can download the 64bit only version from https://github.com/varmd/wine-wayland/releases. This version is automatically built via Github Actions. cd to download folder and install
    
    pacman -U wine-wayland*pkg*

## Installation

download from github, cd to zip directory

    makepkg
    pacman -U wine-wayland*pkg*


#### Installation of 32bit (optional, for 32bit games)

First compile and install regular wine-wayland, then in the same zip directory

    makepkg -p PKGBUILD-32 --noextract
    pacman -U lib32-wine-wayland*

## Using terminal to run games

    cd your-dir
    mkdir -p prefix/your-game
    cp -r YourGameFolder prefix/your-game/
   
Copy relevant 64bit or 32bit dxvk dlls to YourGameFolder or use winetricks.

Copy start-example.sh to your-dir and modify it for your-game, change your-game and YourGameFolder at the top of the file.

Rename start-example.sh to start-your-game.sh

Then in the terminal run sh start-your-game.sh

## Using wineland launcher to run games

From command line (or using file manager)

    mkdir -p ~/.local/share/wineland/your-game
    mv YourGameFolder ~/.local/share/wineland/your-game/
    
Then, go to your app launcher, click on the blue joystick icon. In the browser tab, click Edit below the card for your-game. Enter name for your game, YourGameFolder/game.exe for exe path. And -EpicPortal for game options (for EGS games). Set mangohud, fsync/esync, and other options if needed. See below screenshot for example of options for Subnautica.

![screenshot](https://raw.githubusercontent.com/varmd/wine-wayland/master/wineland/wineland-screenshot-2.png "Screenshot")

Afterwards, click Submit. Then click Launch.

You can obtain YourGameFolder from EGS, Steam or GOG. See the notes section below for links to command line downloaders and tools for these services.

For troubleshooting you can check the logs at YourGameFolder/log.log
  

## Keyboard shortcuts

* F11 - Enter fullscreen mode
* F10 - some games may not restrict cursor properly, manually restricts cursor to the game surface. 
* F9 - some games (such as NMS) that draw their own cursor may need this to lock the cursor pointer. Also enable export WINE_VK_HIDE_CURSOR=1 in the start-game.sh. After alt-tabbing, press F9 two times to reset cursor state


## Notes

* For Unity games make sure game folder is executable
* Some games may take a while to start
* Some games may crash if fullscreen is enabled/disabled. After crashing, look in the game settings folder and see if you can enable/disable fullscreen manually.
* If a game is not starting try to disable WINE\_VK\_VULKAN_ONLY variable and start the game to see if there are any error popups 
* While launchers are not working many games do not require launchers to run
* You can use https://github.com/derrod/legendary to download and run games from Epic Games Store
* You can use https://github.com/ValvePython/steamctl to download games from Steam
* GOG games can be extracted with innounp
* If a game is not starting, try wineserver -k, and start again
* Use export XCURSOR_SIZE="xx" and export XCURSOR_THEME=themename to set cursor theme and increase cursor size 
* Use export WINE_VK_NO_CLIP_CURSOR=1 to disable cursor locking for games that erroneously try to lock mouse cursor.
* Use export WINE_VK_FULLSCREEN_GRAB_CURSOR=1 to automatically enable cursor grab in fullscreen.
* Use export WINE_VK_ALWAYS_FULLSCREEN=1 to automatically set game to fullscreen without using F11. The WINE_VK_WAYLAND_WIDTH and WINE_VK_WAYLAND_HEIGHT must be set to your monitor's current resolution width and height or the game will crash.
* For best performance use kernel with the fsync patch, and add export WINEFSYNC=1 variable

## Caveats and issues

* No controller support - though some are working
* No GDI apps support
* No OpenGL
* No custom cursors


## Games confirmed working

* ABZU
* Dirt 3
* Subnautica
* Rebel Galaxy
* Endless Space
* Age of Wonders 3
* Stellaris
* EU4
* Path of Exile
* Pathfinder Kingmaker
* Crusader Kings 2
* Mutant Year Zero
* Tropico 6