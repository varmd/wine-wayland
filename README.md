## What is wine-wayland

Wine-wayland allows running DX9/DX11/DX12 and Vulkan games using pure Wayland and Wine/DXVK.

## Why wine-wayland

 * You are tired of dealing with X11 and don't care about launchers
 * You want to remove X11 related packages from your PC
 * You want to experience potentially faster and smoother gaming on Wayland
 * You are concerned about insecure X11 games that can spy on other apps running on X11
 * You want to remove all the lib32 packages from your PC and still be able to play most 32-bit games
 * You want to use a launcher which does not require GTK, QT or Electron.

## Screenshot

![screenshot](https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot.png "Screenshot")
![screenshot1](https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot1.png "Subnautica")
![screenshot2](https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot2.png "CivilizationVI")

## Requirements

 * Archlinux or Manjaro, Linux 5.16+
 * AMD GPU with Vulkan and Wayland support, 4GB+ VRAM. Intel, Nvidia not tested
 * Mesa with Wayland and Vulkan support
 * Weston based compositor (tested on wayward), wlroots based compositor (tested on sway), Gnome not tested

## Installation with wineland

From v6.11, it's possible to install only the wineland launcher, without installing wine-wayland. The launcher
will then download wine-wayland, dxvk and mangohud from Github and run games without system-wide installation of wine-wayland.

You can download the wineland launcher package from https://github.com/varmd/wine-wayland/releases. This version is automatically built via Github Actions. cd to download folder and install. After installation, refer to the section below on how to configure games for the wineland launcher.

    pacman -U wineland*pkg*


## Download

You can download the 64-bit only archlinux package from https://github.com/varmd/wine-wayland/releases. This version is automatically built via Github Actions. cd to download folder and install

    pacman -U wine-wayland*pkg*

## Download of 32-bit (optional, for 32-bit games)

You can download the optional 32-bit version from https://github.com/varmd/wine-wayland/releases. It is automatically built via Github Actions. Download both the 64-bit and 32-bit archlinux packages, cd to download folder and install

    pacman -U *wine-wayland*pkg*

## Compile

download or clone from github, cd to zip directory

    makepkg
    pacman -U wine-wayland*pkg*


#### Compile 32-bit add-on (optional, for 32-bit games)

In wine-wayland directory

    WINE_BUILD_32=1 makepkg
    pacman -U *wine-wayland*pkg*

## Using wineland launcher to run games

From command line (or using file manager) create a wrapper folder for the game folder and DXVK, wine, logs, etc.

    mkdir -p ~/.local/share/wineland/your-game
    mv YourGameFolder ~/.local/share/wineland/your-game/

"your-game" above should be lowercase, no spaces tag. For example, for Subnautica it would be subnautica. Then go to your launcher, click on the blue joystick icon. In the browser tab, click Edit below the card for your-game. Enter name for your game, YourGameFolder/game.exe for exe path. And -EpicPortal for game options (for EGS games). Set mangohud, fsync and other options as needed. See below screenshot for example of options for Subnautica.

![screenshot](https://raw.githubusercontent.com/varmd/wine-wayland/master/wineland/wineland-screenshot-2.png "Screenshot")

Click Submit. Then click Launch.

You can obtain YourGameFolder from EGS, Steam or GOG. See the notes section below for links to command line downloaders and tools for these services.

For troubleshooting check the logs at your-game/log.log

## Using terminal to run games with wineland launcher

After setting up your game with the steps above, you can
run your games from the terminal.

    wineland your-game

## Using wineland to setup portable game installtion

First uninstall wine-wayland and lib32-wine-wayland packages if installed, then setup and run your game using
wineland. Afterwards, you can uninstall wineland and wine-wayland and run the game by going to the wrapper folder and running
start-portable.sh. For example for Subnautica `cd ~/.local/share/wineland/subnautica` and `sh start-portable.sh`


## Using terminal to run games without wineland launcher

    cd your-dir
    mkdir -p prefix/your-game
    cp -r YourGameFolder prefix/your-game/

Copy relevant 64-bit or 32-bit dxvk dlls to YourGameFolder or use winetricks.

Copy start-example.sh to your-dir and modify it for your-game, change your-game and YourGameFolder at the top of the file.

Rename start-example.sh to start-your-game.sh

Then in the terminal run sh start-your-game.sh

### Environment variables when running games without the wineland launcher

* Use `export LD_LIBRARY_PATH="/usr/lib/wineland/lib32:$LD_LIBRARY_PATH"` and `export VK_ICD_FILENAMES="/usr/lib/wineland/vulkan/icd.d/intel_icd.i686.json:/usr/lib/wineland/vulkan/icd.d/radeon_icd.i686.json"` when running 32-bit wine apps outside of the wineland launcher
* Use `export WINE_VK_VULKAN_ONLY=1` if a game is not starting or there is no keyboard/mouse focus
* Use `export XCURSOR_SIZE="xx"` and `export XCURSOR_THEME=themename` to set cursor theme and increase cursor size
* Use `export WINE_VK_HIDE_CURSOR=1` to hide cursors, when games do not hide cursors - for example when using a controller
* Use `export WINE_VK_USE_CUSTOM_CURSORS=1` to enable experimental custom game cursors. This will disable cursor size and theme
* Use `export WINE_VK_NO_CLIP_CURSOR=1` to disable cursor locking for games that erroneously try to lock mouse cursor.
* Use `export WINE_VK_FULLSCREEN_GRAB_CURSOR=1` to automatically enable cursor grab in fullscreen.
* Use `export WINE_VK_ALWAYS_FULLSCREEN=1` to automatically set game to fullscreen without using F11.
* Use `export WINE_VK_USE_FSR=1` to enable FSR.
* Use `export WINEFSYNC=1` to enable FSYNC for better performance in most games.

## Keyboard shortcuts

* F11 - Enter fullscreen mode
* F10 - some games may not restrict cursor properly, manually restricts cursor to the game surface.
* F9 - some games (such as NMS) that draw their own cursor may need this to lock the cursor pointer.


## Notes

* Some games require the game folder to be executable
* You can use https://github.com/derrod/legendary to download and run games from Epic Games Store
* You can use https://github.com/ValvePython/steamctl to download games from Steam
* GOG games can be extracted with innoextract
* If a game is not starting, try wineserver -k, and start again, or click Launch again in the wineland launcher
* For FSR to work, in the game settings disable any in-game fullscreen option, set resolution to lower than monitor's resolution, and disable any dynamic resolution options. Then restart the game.


## Caveats and issues

* No GDI apps support - though popups and simple launchers should work
* No OpenGL

## Games confirmed working

* ABZU
* Dirt 3 - (Since 7.21 - install bundled OpenAL32)
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
* Wasteland 2
* Torchlight 1
* Divinity Original Sin 2
* Dungeons 3
* Seven
* Pillars of Eternity
* Ziggurat 1 (add -force-d3d11 in Exe options)
* Warframe (see #25)
* Shogun Total War 2
* Imperator Rome
* The Witcher 1
* The Witcher 3
* Cyberpunk 2077 (needs vkd3d dll, see https://github.com/varmd/wine-wayland/issues/31)
* Torchlight2
* Civilization 6 - use SidMeiersCivilizationVI/Base/Binaries/Win64EOS/CivilizationVI.exe for exe path (EGS version)
* GTA 5
* Thea 2: The Shattering


### Changelog

#### Release 9.3

 * Update to Wine 9.3
 * Update FSYNC to Wine 9.3
 * Update FSR to Wine 9.3
 * Update DXVK, VKD3D, Mangohud
 * Switch to the new 32-bit on 64-bit Wine feature for 32-bit games,
   this will require reinstalling any existing 32-bit games.
 * Reduce download size to 20MB for 64-bit, 17MB for 32-bit.
 * Reduce installation size to 120MB for 64-bit and 104MB for 32-bit.
 * Misc fixes and improvements

#### Release 8.1

 * Add portable mode to run games without wineland or wine-wayland
 * Update to Wine 8.2
 * Update FSYNC to Wine 8.2
 * Update FSR to Wine 8.2
 * Update VKD3D
 * Remove lib32-glibc runtime dependency for wineland
 * Disable FSR for 32-bit games
 * Misc fixes and improvements

#### Release 7.21

 * Update to Wine 7.21
 * Update FSYNC to Wine 7.21
 * Update FSR to Wine 7.21
 * Update DXVK, Mangohud, VKD3D
 * Reduce installation size to 140MB for 64bit.
 * Remove lib32-alsa-lib, lib32-libelf, lib32-expat from 32bit build.
 * Misc fixes and improvements



