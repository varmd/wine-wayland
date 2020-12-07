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
  export LD_LIBRARY_PATH="/usr/wineland/lib32:$LD_LIBRARY_PATH"
  export VK_ICD_FILENAMES="/usr/wineland/vulkan/icd.d/intel_icd.i686.json:/usr/wineland/vulkan/icd.d/radeon_icd.i686.json:/usr/wineland/vulkan/icd.d/amd_icd.i686.json"
fi    

cd "$PWD_PATH"

#export variables
export WINEDEBUG
export WINE_VK_WAYLAND_WIDTH
export WINE_VK_WAYLAND_HEIGHT
export MANGOHUD
export MANGOHUD_CONFIG
export WINEDLLOVERRIDES
export WINE_VK_FULLSCREEN_GRAB_CURSOR
export WINE_VK_VULKAN_ONLY
export XCURSOR_SIZE
export WINE_VK_USE_CUSTOM_CURSORS


export WINEPREFIX=$PWD_PATH/wine
MANGO_PREFIX=$PWD_PATH/mangohud



if [ ! -d $PWD_PATH/dxvk ]; then
    mkdir dxvk
    cd dxvk
    curl  -L "https://github.com/doitsujin/dxvk/releases/download/v1.7.3/dxvk-1.7.3.tar.gz" > dxvk-1.7.3.tar.gz
    tar xf dxvk-1.7.3.tar.gz
    cd "$PWD_PATH"
fi

if [ ! -d $WINEPREFIX ]; then
  NEW_WINEPREFIX=1
  
  
  cd "$PWD_PATH"
  
  
  if [[ -z "$IS_64_EXE" ]]; then
    echo "is 32bit"
    WINE_CMD="wine"
    
    export WINEARCH=win32
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4
    
    cp -r dxvk/dxvk-1.7.3/x32/* wine/drive_c/windows/system32/
  else
    echo "is 64bit"
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4  
    cp -r dxvk/dxvk-1.7.3/x64/* wine/drive_c/windows/system32/
  fi
  
fi




if [[ -z "$MANGOHUD" ]]; then
  echo ""
else
if [ ! -d $MANGO_PREFIX ]; then
  
  
  cd "$PWD_PATH"
  if [ ! -d $PWD/mangohud ]; then
    mkdir mangohud
    cd mangohud
    curl  -L "https://github.com/flightlessmango/MangoHud/releases/download/v0.6.1/MangoHud-0.6.1.tar.gz" > mangohud.tar.gz
    tar xf mangohud.tar.gz
    tar xf MangoHud/MangoHud-package.tar
    cd "$PWD_PATH"
  fi
fi


  mkdir -p /run/user/$UID/mangohud-wine-wayland
  cp -r mangohud/usr/lib/mangohud/* /run/user/$UID/mangohud-wine-wayland
  cp -r mangohud/usr/share/vulkan/implicit_layer.d/*.json mangohud/mangohud.json
  
  
  

  if [[ -z "$IS_64_EXE" ]]; then
    echo "is 32bit mangohud"
    LIB="lib32"
  else
    echo "is 64bit mangohud"
    LIB="lib"
  fi
  
  sed -i "s/\/usr\/lib\/mangohud/\/run\/user\/${UID}\/mangohud-wine-wayland/g" mangohud/mangohud.json

  export VK_INSTANCE_LAYERS=VK_LAYER_MANGOHUD_overlay
  export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d:"$PWD_PATH/mangohud"
  export VK_LAYER_PATH="$PWD_PATH/mangohud"

fi #end mangohud


#export variables
export WINEFSYNC
export WINEESYNC

export WINEDEBUG=-all,+waylanddrv

rm $PWD_PATH/wine/drive_c/users/$USER/"My Documents"
mkdir -p $PWD_PATH/wine/drive_c/users/$USER/"My Documents"



cd "$GAME_PATH"

#sh setup_dxvk.sh install

#export WINEDLLOVERRIDES="d3dcompiler_47,d3d10,d3d9,dxgi,d3d11=n,b;winedbg=d"  

#mangohud





$WINE_CMD $GAME_EXE $GAME_OPTIONS  &> $LOG_PATH
