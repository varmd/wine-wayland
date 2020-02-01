## what is wine-wayland

Wine-wayland allows playing DXVK and Vulkan games using pure wayland and Wine 4.21.

## why wine-wayland

 * You are tired of dealing with Xorg and don't care about launchers and controllers
 * You want to remove Xorg related packages from your PC
 * You want to experience potentially faster and smoother gaming on Wayland
 * You are concerned about insecure Xorg games that can spy on other apps running on Xorg.

## screenshot

![screenshot] (https://raw.githubusercontent.com/varmd/wine-wayland/master/screenshot.png "Screenshot")

## requirements

 * Archlinux or Manjaro
 * AMD GPU with Vulkan support
 * Mesa 19.3 or later with Wayland, Vulkan and EGL support
 * Wayland compositor - tested on weston 
 * SDL and Faudio
 * Esync or Fsync support

## installation

download from github, cd to zip directory

    makepkg
    pacman -U wine-wayland*pkg*


#### installation of 32bit (optional, for 32bit games)

First compile and install regular wine-wayland, then in the same zip directory

    makepkg -P PKGBUILD-32 --noextract
    pacman -U lib32-wine-wayland*

## running

    cd your-dir
    mkdir -p prefix/your-game
    cp -r YourGameFolder prefix/your-game/
   
copy relevant 64bit or 32bit dxvk dlls to prefix/your-game
Copy start-example.sh to your-dir and modify for your-game, change your-game and YourGameFolder at the top of the file.
rename start-example.sh to start-your-game.sh
Then in the terminal run sh start-your-game.sh
On first boot, click cancel on Install Gecko and Mono


## keyboard shortcuts

* F11 - Fullscreen mode
* F10 - some games may not restrict cursor properly, manually restricts cursor to the game surface. After alt-tabbing, press two times
* F9 - some games (such as NMS) that have their own cursor may need this to lock the cursor pointer. Also enable export WINE_VK_HIDE_CURSOR=1 in the start-game.sh


## notes
* For Unity games make sure game folder is executable
* Some games may take a while to start
* Some games may crash if fullscreen is enabled/disabled. After crashing, look in the game settings folder and see if you can enable/disable fullscreen manually.
* If a game is not starting try to disable WINE\_VK\_VULKAN_ONLY variable and start the game to see if there are any error popups 
* While launchers are not working many games from launchers do not require launchers to be running. You can download these games in a container with normal wine, and copy game folders to the host os.
* For GOG games, these can be extracted with innounp
* If a game is not starting, try wineserver -k, and start again

## caveats and issues

* No controller support
* No GDI apps support
* Launchers are not working
* No OpenGL support
* No custom cursors
