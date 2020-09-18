#
# Copyright 2020 varmd
#

#for i in $(ls -d */); do echo ${i%%/}; done
#for i in $(set -- */; printf "%s\n" "${@%/}"); 
#do 
#  echo ${i%%/}; 
#done

WINE_CMD="wine64"
#remove all exe programs
#should avoid stuck exe issues
pgrep -f "\.exe" | xargs -L1 kill -9

#needs to be here for GDI

export WINE_VK_WAYLAND_WIDTH=1920
export WINE_VK_WAYLAND_HEIGHT=1080
export WAYLAND_DISPLAY=wayland-0

export RADV_PERFTEST=aco


export XDG_RUNTIME_DIR=/run/user/$UID

if [  -z $1 ]; then
  echo "need folder name"
  exit;
fi

if [ ! -d $PWD/$1 ]; then
  echo "folder does not exist"
  exit;
fi

echo $1;

PWD_PATH=$PWD/$1
LOG_PATH=$PWD/$1/log.log



#export WINEDEBUG=+waylanddrv

WINEDLLOVERRIDES="d3dcompiler_47,d3d9,dxgi,d3d11=n,b;dinput=d;winedbg=d;winemenubuilder.exe=d;mscoree=d;mshtml=d"

WINE_VK_WAYLAND_WIDTH=1920
WINE_VK_WAYLAND_HEIGHT=1080
MANGOHUD=1
#WINEFSYNC=1

export XCURSOR_SIZE="32"
export XCURSOR_THEME=Adwaita


source $1/options.conf

GAME_PATH="$PWD/$1/$GAME_PATH"

ALREADY_LAUNCHED=`ps -aux | grep $GAME_EXE | grep -v grep`

echo $ALREADY_LAUNCHED

if [ ! -z $ALREADY_LAUNCHED ]; then
  echo "already running"
  exit;
fi


cd "$GAME_PATH"
#is 64bit

IS_64_EXE=`file $GAME_EXE | grep PE32+`


if [[ -z "$IS_64_EXE" ]]; then
  echo "is 32bit wine exe"
  WINE_CMD="wine"
fi    

cd "$PWD_PATH"

#export variables
export WINEDEBUG
export WINE_VK_WAYLAND_WIDTH
export WINE_VK_WAYLAND_HEIGHT
export MANGOHUD
export MANGOHUD_CONFIG
export WINEDLLOVERRIDES
export WINE_VK_ALWAYS_FULLSCREEN
export WINE_VK_VULKAN_ONLY
export XCURSOR_SIZE

export WINEPREFIX=$PWD_PATH/wine

if [ ! -d $WINEPREFIX ]; then
  NEW_WINEPREFIX=1
  
  
  cd "$PWD_PATH"
  if [ ! -d $PWD/dxvk ]; then
    mkdir dxvk
    cd dxvk
    curl  -L "https://github.com/doitsujin/dxvk/releases/download/v1.7.1/dxvk-1.7.1.tar.gz" > dxvk-1.7.1.tar.gz
    tar xf dxvk-1.7.1.tar.gz
    cd "$PWD_PATH"
  fi
  
  if [[ -z "$IS_64_EXE" ]]; then
    echo "is 32bit"
    WINE_CMD="wine"
    
    export WINEARCH=win32
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4
    
    cp -r dxvk/dxvk-1.7.1/x32/* wine/drive_c/windows/system32/
  else
    echo "is 64bit"
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4  
    cp -r dxvk/dxvk-1.7.1/x64/* wine/drive_c/windows/system32/
  fi
  
  
  
fi

#export variables
export WINEFSYNC
export WINEESYNC

 

rm $PWD_PATH/wine/drive_c/users/$USER/"My Documents"
mkdir -p $PWD_PATH/wine/drive_c/users/$USER/"My Documents"



cd "$GAME_PATH"

#sh setup_dxvk.sh install




#set

# ~/.local/bin/steamctl depot download --app appid -os windows -o foldername



#export WINEDLLOVERRIDES="d3dcompiler_47,d3d10,d3d9,dxgi,d3d11=n,b;winedbg=d"  

#mangohud


#export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d:"$PWD/$1/mangohud"
#export VK_INSTANCE_LAYERS=VK_LAYER_MANGOHUD_overlay
#export VK_LAYER_PATH="$PWD/$1/mangohud"
#export VK_LAYER_PATH=/home/.local/share/mangohud-link2
#export VK_LAYER_PATH=/home/alpha/mangohud
#echo $VK_LAYER_PATH

#if [ -z ${NEW_WINEPREFIX+x} ]; then
  #
#else
  #new wineprefix
#fi



echo $WINE_CMD

nohup $WINE_CMD $GAME_EXE $GAME_OPTIONS  &> $LOG_PATH & disown
