#
# Copyright 2020-2024 varmd
#

WINE_VK_DXVK_VERSION="2.3"
WINE_WAYLAND_VERSION="9.3"
VKD3D_VERSION="2.11.1"
#MANGOHUD_URL="/v0.6.8/MangoHud-0.6.8.r0.gefdcc6d.tar.gz"
MANGOHUD_URL="/v0.7.1/MangoHud-0.7.1.tar.gz"
MANGOHUD_VERSION="MangoHud-0.7.1"
WINE_WAYLAND_TAG_NUM="3"

#for i in $(ls -d */); do echo ${i%%/}; done
#for i in $(set -- */; printf "%s\n" "${@%/}");
#do
#  echo ${i%%/};
#done

WINE_CMD="wine64"
#remove all exe programs
#should avoid stuck exe issues
killall -9 wineserver &> /dev/null
killall -9 wineserver &> /dev/null
killall -9 wineserver &> /dev/null
pgrep -f "\.exe" | xargs -L1 kill -9 &> /dev/null
pgrep -f "\.exe" | xargs -L1 kill -9 &> /dev/null
pgrep -f "\.exe" | xargs -L1 kill -9 &> /dev/null
pkill -9 "\.exe"

#needs to be here for GDI

export WINE_VK_WAYLAND_WIDTH=1920
export WINE_VK_WAYLAND_HEIGHT=1080


export XDG_RUNTIME_DIR=/run/user/$UID

if [  -z $1 ]; then
  echo "need folder name"
  exit;
fi

if [ ! -d $PWD/$1 ]; then
  echo "game folder does not exist"
  exit;
fi

echo "Starting $1";

PWD_PATH=$PWD/$1
LOG_PATH=$PWD/$1/log.log



WINEDLLOVERRIDES="openal32,d3dcompiler_47,d3d9,dxgi,d3d11,d3d12=n,b;dinput=d;winedbg=d;winemenubuilder.exe=d;mscoree=d;mshtml=d;cchromaeditorlibrary64=d;$WINEDLLOVERRIDES"

WINE_VK_WAYLAND_WIDTH=1920
WINE_VK_WAYLAND_HEIGHT=1080
MANGOHUD=1


export XCURSOR_SIZE="32"
export XCURSOR_THEME=Adwaita


source $1/options.conf

GAME_PATHNAME=$GAME_PATH
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


export LD_LIBRARY_PATH="/usr/lib/wineland/lib:$LD_LIBRARY_PATH"

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
export WINE_VK_USE_FSR
export XCURSOR_SIZE
export WINE_VK_USE_CUSTOM_CURSORS
export WINE_VK_ALWAYS_FULLSCREEN
export WINEFSYNC



export WINEPREFIX=$PWD_PATH/wine
MANGO_PREFIX=$PWD_PATH/mangohud

if [[ -z "$MANGOHUD" ]]; then

  echo ""

else

  # remove old mangohud
  if [ -f $PWD/mangohud/mangohud.json ]; then
    rm -rf $PWD/mangohud
  fi

  if [ ! -d $PWD/mangohud ]; then

    if [ ! -f $PWD_PATH/../${MANGOHUD_VERSION}.tar.gz ]; then
      echo "Downloading MangoHud"
      curl  -L "https://github.com/flightlessmango/MangoHud/releases/download${MANGOHUD_URL}" > $PWD_PATH/../${MANGOHUD_VERSION}.tar.gz
    fi

    cd "$PWD_PATH"
    mkdir mangohud
    cd mangohud
    cp $PWD_PATH/../${MANGOHUD_VERSION}.tar.gz mangohud.tar.gz
    tar xf mangohud.tar.gz
    tar xf MangoHud/MangoHud-package.tar
    cd "$PWD_PATH"

  fi

  cp -r mangohud/usr/lib/mangohud/* /run/user/$UID/mangohud-wine-wayland
  cp -r mangohud/usr/share/vulkan/implicit_layer.d/MangoHud.x86_64.json $PWD_PATH/mangohud/MangoHud.x86_64.json
  sed -i "s/\/usr\/lib\/mangohud/\/run\/user\/${UID}\/mangohud-wine-wayland/g" mangohud/MangoHud.x86_64.json

  mkdir -p /run/user/$UID/mangohud-wine-wayland

  echo "Using mangohud"
  export VK_INSTANCE_LAYERS=VK_LAYER_MANGOHUD_overlay_x86_64
  export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d:"$PWD_PATH/mangohud"
  export VK_LAYER_PATH="$PWD_PATH/mangohud"


fi #end mangohud




cd "$PWD_PATH"


if [ ! -f /usr/lib/wine/x86_64-unix/winewayland.so ]; then
  USE_LOCAL_WINE=1
fi

if [ -f /usr/lib/wine/x86_64-unix/winewayland.so ]; then
  unset USE_LOCAL_WINE
fi

#download local wine
if [[ "$USE_LOCAL_WINE" ]]; then

  #copy portable sh
  cp /usr/lib/wineland/ui/start-portable.sh $PWD_PATH

  if [ ! -d $PWD_PATH/winebin ]; then

    if [ ! -f $PWD_PATH/../wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst ]; then
      echo "Downloading 64bit wine-wayland from github"
      curl -L "https://github.com/varmd/wine-wayland/releases/download/v${WINE_WAYLAND_VERSION}.${WINE_WAYLAND_TAG_NUM}/wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst" > $PWD_PATH/../wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst
    fi

    mkdir -p $PWD_PATH/winebin
    cd ${PWD_PATH}/winebin
    cp ${PWD_PATH}/../wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst .
    tar xf wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst

    echo ""
    #copy sdl2 and alsa
    cp $PWD_PATH/winebin/usr/lib/wineland/lib/* $PWD_PATH/winebin/usr/lib/

    if [[ "$IS_64_EXE" ]]; then
      D=1
    else


      if [ ! -f $PWD_PATH/../lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst ]; then
        echo "Downloading 32bit wine wayland from github"
        curl -L "https://github.com/varmd/wine-wayland/releases/download/v${WINE_WAYLAND_VERSION}.${WINE_WAYLAND_TAG_NUM}/lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst" > $PWD_PATH/../lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst
      fi

      cp $PWD_PATH/../lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst .
      tar xf lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst

    fi

    rm -rf *.zst
    cd $PWD_PATH/wine/drive_c/"$GAME_PATHNAME"/$FINAL_PATH

  fi #no winebin

  #cleanup
  rm -rf $PWD_PATH/winebin/usr/lib/wine/x86_64-unix/*.a
  rm -rf $PWD_PATH/winebin/usr/include
  rm -rf $PWD_PATH/winebin/usr/share/man
  rm -rf $PWD_PATH/winebin/usr/share/applications
  rm -rf $PWD_PATH/winebin/usr/share/fonts/*fon
fi

#local wine
if [ -d $PWD_PATH/winebin ]; then

  #check for noexec and copy winebin so games still work with noexec for ~/.local/share/wineland
  GAME_PART=`df -P $PWD_PATH/winebin/usr/bin/$WINE_CMD | tail -1 | cut -d' ' -f 1`
  GAME_NOEXEC=`cat /proc/mounts | grep $GAME_PART | grep noexec`

  TMP_NOEXEC=`cat /proc/mounts | grep \/tmp | grep noexec`
  HOME_NOEXEC=`cat /proc/mounts | grep \/home | grep noexec`

  EXEC_LIB="$PWD_PATH/winebin/usr/lib:"

  rm -rf $HOME/.cache/wineland /tmp/wineland
  mkdir -p /tmp/wineland/ $HOME/.cache/wineland/

  if [[ "$GAME_NOEXEC" ]]; then
    echo "noexec detected, copying wine files to /tmp or ~/.cache"

    EXEC_LIB=""


    if [[ "$TMP_NOEXEC" ]]; then
      if [[ "$HOME_NOEXEC" ]]; then
        echo "noexec on both /tmp and /home detected. Please enable \
          exec permissions or install wine-wayland as a root user."
        exit;
      else #home cache
        cp -r $PWD_PATH/winebin $HOME/.cache/wineland/
        NOEXEC_BIN_PATH="$HOME/.cache/wineland/winebin/usr/bin:"
        NOEXEC_LIB_PATH="$HOME/.cache/wineland/winebin/usr/lib:"


      fi
    else #tmp
      cp -r $PWD_PATH/winebin /tmp/wineland
      NOEXEC_BIN_PATH="/tmp/wineland/winebin/usr/bin:"
      NOEXEC_LIB_PATH="/tmp/wineland/winebin/usr/lib:"
    fi

  fi


  mkdir -p /tmp/wineland/ $HOME/.cache/wineland/
  export LD_LIBRARY_PATH="${NOEXEC_LIB_PATH}${EXEC_LIB}$LD_LIBRARY_PATH"
  export PATH="${NOEXEC_BIN_PATH}$PWD_PATH/winebin/usr/bin:$PATH"


fi


cd "$PWD_PATH"

if [ ! -d $PWD_PATH/dxvk ]; then

    if [ ! -f $PWD_PATH/../dxvk-${WINE_VK_DXVK_VERSION}.tar.gz ]; then
      echo "Downloading dxvk"
      echo "https://github.com/doitsujin/dxvk/releases/download/v${WINE_VK_DXVK_VERSION}/dxvk-${WINE_VK_DXVK_VERSION}.tar.gz";
      curl  -L "https://github.com/doitsujin/dxvk/releases/download/v${WINE_VK_DXVK_VERSION}/dxvk-${WINE_VK_DXVK_VERSION}.tar.gz" > $PWD_PATH/../dxvk-$WINE_VK_DXVK_VERSION.tar.gz
    fi

    mkdir dxvk
    cd dxvk
    cp $PWD_PATH/../dxvk-$WINE_VK_DXVK_VERSION.tar.gz .
    tar xf dxvk-$WINE_VK_DXVK_VERSION.tar.gz
    cd "$PWD_PATH"
    REFRESH_DXVK=1
fi

if [[ "$IS_64_EXE" ]]; then
  if [ ! -d $PWD_PATH/vkd3d ]; then

      if [ ! -f $PWD_PATH/../vkd3d-${VKD3D_VERSION}.tar.zst ]; then
        echo "Downloading vkd3d"
        curl  -L "https://github.com/HansKristian-Work/vkd3d-proton/releases/download/v${VKD3D_VERSION}/vkd3d-proton-${VKD3D_VERSION}.tar.zst" > $PWD_PATH/../vkd3d-${VKD3D_VERSION}.tar.zst
      fi

      mkdir vkd3d
      cd vkd3d
      cp $PWD_PATH/../vkd3d-${VKD3D_VERSION}.tar.zst .
      tar xf vkd3d-${VKD3D_VERSION}.tar.zst
      cd "$PWD_PATH"
      REFRESH_VKD3D=1
  fi
fi

if [ ! -d $WINEPREFIX ]; then
  NEW_WINEPREFIX=1
  REFRESH_DXVK=1

  cd "$PWD_PATH"


  if [[ -z "$IS_64_EXE" ]]; then
    echo "32bit exe"
    WINE_CMD="wine"
    export WINEARCH=win64
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4

  else
    echo "64bit wine"
    export WINEARCH=win64
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4
  fi

  echo ${WINE_WAYLAND_VERSION} > $PWD_PATH/wine/version

  # Fixes no sound in some games
  $WINE_CMD winecfg /a
else

  #rm $PWD_PATH/wine/.update-timestamp
  #echo "disable" > $PWD_PATH/wine/.update-timestamp

  # speed up launch
  touch $PWD_PATH/wine/version
  CURR_VER=$(< $PWD_PATH/wine/version )
  if [ "$CURR_VER" != "$WINE_WAYLAND_VERSION" ]; then
    rm $PWD_PATH/wine/.update-timestamp
    WINE_VK_VULKAN_ONLY=1 wineboot -u &> /dev/null
    echo ${WINE_WAYLAND_VERSION} > $PWD_PATH/wine/version
  else
    echo "disable" > $PWD_PATH/wine/.update-timestamp
  fi


fi

if [[ -z "$REFRESH_DXVK" ]]; then
  echo -n ""
else
  if [[ -z "$IS_64_EXE" ]]; then
    echo "refreshing 32bit dxvk"
    cp -r dxvk/dxvk-${WINE_VK_DXVK_VERSION}/x32/* wine/drive_c/windows/syswow64/
  else
    echo "refreshing 64bit dxvk"
    cp -r dxvk/dxvk-${WINE_VK_DXVK_VERSION}/x64/* wine/drive_c/windows/system32/
  fi
fi

if [[ -z "$REFRESH_VKD3D" ]]; then
  echo -n ""
else
  if [[ -z "$IS_64_EXE" ]]; then
    echo "d3d12 not supported for 32bit"
  else
    echo "refreshing 64bit vkd3d for Directx12 to Vulkan"
    cp -r vkd3d/vkd3d-proton-${VKD3D_VERSION}/x64/* wine/drive_c/windows/system32/
  fi
fi


rm $PWD_PATH/wine/drive_c/users/$USER/"My Documents" &> /dev/null
rm $PWD_PATH/wine/drive_c/users/$USER/"Documents" &> /dev/null
mkdir -p $PWD_PATH/wine/drive_c/users/$USER/"My Documents"
mkdir -p $PWD_PATH/wine/drive_c/users/$USER/"Documents"


#remove z: for better security
rm -r $PWD_PATH/wine/dosdevices/z: &> /dev/null

#echo $PWD_PATH/"$GAME_PATHNAME"
#echo $PWD_PATH/wine/drive_c/"$GAME_PATHNAME"

#link to c:
ln -s $PWD_PATH/"$GAME_PATHNAME" $PWD_PATH/wine/drive_c/ 2> /dev/null

#hack for multiple level exes
FINAL_PATH="$(dirname "$GAME_EXE")"
FINAL_EXE="$(basename "$GAME_EXE")"

cd $PWD_PATH/wine/drive_c/"$GAME_PATHNAME"/$FINAL_PATH


#export variables before starting exe
export WINEDEBUG=fixme-all,-all,+waylanddrv

echo "Launching $1"
$WINE_CMD $FINAL_EXE $GAME_OPTIONS  &> $LOG_PATH

