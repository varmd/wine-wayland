FOLDER1=your-game
FOLDER2=YourGameFolder
EXE_PATH=your-game.exe

#enable aco
export RADV_PERFTEST=aco


#enable esync or fsync
#export WINEESYNC=1
export WINEFSYNC=1

#enable/disable winedebug
export WINEDEBUG=-all

rm -rf /tmp/esync-fd
mkfifo /tmp/esync-fd

#enable/disable winedebug for waylanddrv
#export WINEDEBUG=+waylanddrv 

#enable for games that have their own cursor
#export WINE_VK_HIDE_CURSOR=1

export WINEPREFIX=$PWD/prefix/$FOLDER1/.wine

#create and update wineprefix on first run of your game
if [ ! -d $WINEPREFIX ]; then
  wineboot -u
fi

#enables vulkan only windows, only disable to see any GDI error popups
export WINE_VK_VULKAN_ONLY=1

#set width/height of vulkan window. must be common resolution
export WINE_VK_WAYLAND_WIDTH=1920
export WINE_VK_WAYLAND_HEIGHT=1080




#dxvk options here
export DXVK_CONFIG_FILE=$PWD/dxvk.conf
export DXVK_LOG_LEVEL=none
#export DXVK_LOG_LEVEL=error
#export DXVK_HUD=fps


#enable/disable dxvk_hud
export DXVK_HUD=1

#enable disable mangohud if available
#export MANGOHUD=1
#export MANGOHUD_CONFIG=cpu_temp,gpu_temp,height=100,font_size=20

#---Modify folder names here

cd "prefix/$FOLDER1/$FOLDER2"


WINEDLLOVERRIDES="dxgi,d3d11=n,b,dinput=d,winedbg=d"  wine64 $EXE_PATH #add command line arguments if needed 

#or uncomment for 32bit
#WINEDLLOVERRIDES="dxgi,d3d11=n,b,dinput=d,winedbg=d"  wine your32bitgame.exe #add command line arguments if neededPortal 
