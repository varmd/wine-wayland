#
# Copyright 2020-2022 varmd
#

WINE_VK_DXVK_VERSION="2.0"
WINE_WAYLAND_VERSION="7.21"
VKD3D_VERSION="2.7"
MANGOHUD_URL="/v0.6.8/MangoHud-0.6.8.r0.gefdcc6d.tar.gz"
WINE_WAYLAND_TAG_NUM="2"

#partition check
#df -P ~/.local/share/wineland/game/winebin/usr/bin/wine64 | tail -1 | cut -d' ' -f 1


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



WINEDLLOVERRIDES="d3dcompiler_47,d3d9,dxgi,d3d11,d3d12=n,b;gameux=d;dinput=d;winedbg=d;winemenubuilder.exe=d;mscoree=d;mshtml=d;cchromaeditorlibrary64=d;$WINEDLLOVERRIDES"

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


if [[ -z "$IS_64_EXE" ]]; then
  echo "32bit game"
  WINE_CMD="wine"
  export LD_LIBRARY_PATH="/usr/lib/wineland/lib32:$LD_LIBRARY_PATH"
  export VK_ICD_FILENAMES="/usr/lib/wineland/vulkan/icd.d/intel_icd.i686.json:/usr/lib/wineland/vulkan/icd.d/radeon_icd.i686.json"
else
  echo "64bit game"
  export LD_LIBRARY_PATH="/usr/lib/wineland/lib:$LD_LIBRARY_PATH"
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
  if [ ! -d $MANGO_PREFIX ]; then
  cd "$PWD_PATH"
    if [ ! -d $PWD/mangohud ]; then
      mkdir mangohud
      cd mangohud
      curl  -L "https://github.com/flightlessmango/MangoHud/releases/download${MANGOHUD_URL}" > mangohud.tar.gz
      tar xf mangohud.tar.gz
      tar xf MangoHud/MangoHud-package.tar
      cd "$PWD_PATH"
    fi
  fi


  mkdir -p /run/user/$UID/mangohud-wine-wayland
  cp -r mangohud/usr/lib/mangohud/* /run/user/$UID/mangohud-wine-wayland
  cp -r mangohud/usr/share/vulkan/implicit_layer.d/MangoHud.json mangohud/mangohud.json




  if [[ -z "$IS_64_EXE" ]]; then
    echo "Using 32bit mangohud"
    LIB="lib32"
  else
    echo "Using 64bit mangohud"
    LIB="lib64"
  fi

  sed -i "s/\/usr\/lib\/mangohud/\/run\/user\/${UID}\/mangohud-wine-wayland/g" mangohud/mangohud.json
  sed -i "s/\$LIB/${LIB}/g" mangohud/mangohud.json

  export VK_INSTANCE_LAYERS=VK_LAYER_MANGOHUD_overlay
  export VK_LAYER_PATH=/usr/share/vulkan/explicit_layer.d:"$PWD_PATH/mangohud"
  export VK_LAYER_PATH="$PWD_PATH/mangohud"
  #disable wined3d which runs even with dxvk on and crashes due to opengl
  #wined3d=d disabled as it crashes witcher1
  #WINEDLLOVERRIDES="gameux=d;wined3d=d;$WINEDLLOVERRIDES"

fi #end mangohud




cd "$PWD_PATH"



if [ ! -f /usr/lib/wine/x86_64-unix/winewayland.drv.so ]; then
  USE_LOCAL_WINE=1
fi

if [ -f /usr/lib/wine/x86_64-unix/winewayland.so ]; then
  unset USE_LOCAL_WINE
fi

#download local wine
if [[ "$USE_LOCAL_WINE" ]]; then
  if [ ! -d $PWD_PATH/winebin ]; then

    if [ ! -f $PWD_PATH/../wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst ]; then
      echo "Downloading 64bit wine-wayland"
      curl -L "https://github.com/varmd/wine-wayland/releases/download/v${WINE_WAYLAND_VERSION}.${WINE_WAYLAND_TAG_NUM}/wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst" > $PWD_PATH/../wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst
    fi

    mkdir -p $PWD_PATH/winebin
    cd ${PWD_PATH}/winebin
    cp ${PWD_PATH}/../wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst .
    tar xf wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst

    if [[ "$IS_64_EXE" ]]; then
      echo ""
      #copy sdl2
      cp $PWD_PATH/winebin/usr/lib/wineland/lib/* $PWD_PATH/winebin/usr/lib/
    else


      if [ ! -f $PWD_PATH/../lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst ]; then
        echo "Downloading 32bit wine wayland"
        curl -L "https://github.com/varmd/wine-wayland/releases/download/v${WINE_WAYLAND_VERSION}.${WINE_WAYLAND_TAG_NUM}/lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst" > $PWD_PATH/../lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst
      fi

      cp $PWD_PATH/../lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst .
      tar xf lib32-wine-wayland-${WINE_WAYLAND_VERSION}-1-x86_64.pkg.tar.zst

      #copy vulkan json files to backups
      mkdir $PWD_PATH/winebin/usr/lib/wineland/vulkan/orig
      cp $PWD_PATH/winebin/usr/lib/wineland/vulkan/icd.d/*json $PWD_PATH/winebin/usr/lib/wineland/vulkan/orig/



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
  GAME_PART=`df -P $PWD_PATH/winebin/usr/bin/wine64 | tail -1 | cut -d' ' -f 1`
  GAME_NOEXEC=`cat /proc/mounts | grep $GAME_PART | grep noexec`

  TMP_NOEXEC=`cat /proc/mounts | grep \/tmp | grep noexec`
  HOME_NOEXEC=`cat /proc/mounts | grep \/home | grep noexec`

  EXEC_LIB="$PWD_PATH/winebin/usr/lib:"
  EXEC_LIB32="$PWD_PATH/winebin/usr/lib/wineland/lib32:"

  rm -rf $HOME/.cache/wineland /tmp/wineland
  mkdir -p /tmp/wineland/ $HOME/.cache/wineland/

  if [[ "$GAME_NOEXEC" ]]; then
    echo "noexec detected, copying wine files to /tmp or ~/.cache"

    EXEC_LIB=""
    EXEC_LIB32=""

    if [[ "$TMP_NOEXEC" ]]; then
      if [[ "$HOME_NOEXEC" ]]; then
        echo "noexec on both /tmp and /home detected. Please enable \
          exec permissions or install wine-wayland as a root user."
        exit;
      else #home cache
        cp -r $PWD_PATH/winebin $HOME/.cache/wineland/
        NOEXEC_BIN_PATH="$HOME/.cache/wineland/winebin/usr/bin:"
        NOEXEC_LIB_PATH="$HOME/.cache/wineland/winebin/usr/lib:"
        _NOEXEC_LIB_PATH="$HOME/.cache/wineland/winebin/usr/lib"
        NOEXEC_LIB32_PATH="$HOME/.cache/wineland/winebin/usr/lib/wineland/lib32:"

      fi
    else #tmp
      cp -r $PWD_PATH/winebin /tmp/wineland
      NOEXEC_BIN_PATH="/tmp/wineland/winebin/usr/bin:"
      NOEXEC_LIB_PATH="/tmp/wineland/winebin/usr/lib:"
      _NOEXEC_LIB_PATH="/tmp/wineland/winebin/usr/lib"
      NOEXEC_LIB32_PATH="/tmp/wineland/winebin/usr/lib/wineland/lib32:"
    fi

  fi


  mkdir -p /tmp/wineland/ $HOME/.cache/wineland/



  if [[ "$IS_64_EXE" ]]; then

    echo "Using local wine 64bit"

    export LD_LIBRARY_PATH="${NOEXEC_LIB_PATH}${EXEC_LIB}$LD_LIBRARY_PATH"
    export PATH="${NOEXEC_BIN_PATH}$PWD_PATH/winebin/usr/bin:$PATH"
  else
    echo "Using local wine 32bit"

    export LD_LIBRARY_PATH="${NOEXEC_LIB_PATH}${NOEXEC_LIB32_PATH}${EXEC_LIB32}${EXEC_LIB}$LD_LIBRARY_PATH"
    export PATH="${NOEXEC_BIN_PATH}$PWD_PATH/winebin/usr/bin:$PATH"

    export VK_ICD_FILENAMES="$PWD_PATH/winebin/usr/lib/wineland/vulkan/icd.d/intel_icd.i686.json:$PWD_PATH/winebin/usr/lib/wineland/vulkan/icd.d/radeon_icd.i686.json"




    if [[ "$GAME_NOEXEC" ]]; then
      #replace with originals from backup
      cp ${_NOEXEC_LIB_PATH}/wineland/vulkan/orig/*json ${_NOEXEC_LIB_PATH}/wineland/vulkan/icd.d/

      export VK_ICD_FILENAMES="$_NOEXEC_LIB_PATH/wineland/vulkan/icd.d/intel_icd.i686.json:$_NOEXEC_LIB_PATH/wineland/vulkan/icd.d/radeon_icd.i686.json"

      sed -i "s#\"/usr/lib#\"$_NOEXEC_LIB_PATH#g" ${_NOEXEC_LIB_PATH}/wineland/vulkan/icd.d/*json
    else
      #replace with originals from backup
      cp $PWD_PATH/winebin/usr/lib/wineland/vulkan/orig/*json $PWD_PATH/winebin/usr/lib/wineland/vulkan/icd.d/

      sed -i "s#\"/usr/lib#\"${PWD_PATH}/winebin/usr/lib#g" $PWD_PATH/winebin/usr/lib/wineland/vulkan/icd.d/*json
    fi

  fi

fi


cd "$PWD_PATH"

if [ ! -d $PWD_PATH/dxvk ]; then
    echo "Downloading dxvk"
    mkdir dxvk
    cd dxvk
    echo "https://github.com/doitsujin/dxvk/releases/download/v${WINE_VK_DXVK_VERSION}/dxvk-${WINE_VK_DXVK_VERSION}.tar.gz";
    curl  -L "https://github.com/doitsujin/dxvk/releases/download/v${WINE_VK_DXVK_VERSION}/dxvk-${WINE_VK_DXVK_VERSION}.tar.gz" > dxvk-$WINE_VK_DXVK_VERSION.tar.gz
    tar xf dxvk-$WINE_VK_DXVK_VERSION.tar.gz
    cd "$PWD_PATH"
    REFRESH_DXVK=1
fi

if [ ! -d $PWD_PATH/vkd3d ]; then
    echo "Downloading vkd3d"
    mkdir vkd3d
    cd vkd3d


    curl  -L "https://github.com/HansKristian-Work/vkd3d-proton/releases/download/v${VKD3D_VERSION}/vkd3d-proton-${VKD3D_VERSION}.tar.zst" > vkd3d-${VKD3D_VERSION}.tar.zst
    tar xf vkd3d-${VKD3D_VERSION}.tar.zst
    cd "$PWD_PATH"
    REFRESH_VKD3D=1
fi

if [ ! -d $WINEPREFIX ]; then
  NEW_WINEPREFIX=1
  REFRESH_DXVK=1

  cd "$PWD_PATH"


  if [[ -z "$IS_64_EXE" ]]; then
    echo "32bit wine"
    WINE_CMD="wine"

    export WINEARCH=win32
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4

  else
    echo "64bit wine"
    WINE_VK_VULKAN_ONLY=1 wineboot -u
    sleep 4
  fi

  # Fixes no sound in some games
  wine64 winecfg /a
else

  #rm $PWD_PATH/wine/.update-timestamp
  #echo "disable" > $PWD_PATH/wine/.update-timestamp
  WINE_VK_VULKAN_ONLY=1 wineboot -u &> /dev/null
  echo ""


fi

if [[ -z "$REFRESH_DXVK" ]]; then
  echo -n ""
else
  if [[ -z "$IS_64_EXE" ]]; then
    echo "refreshing 32bit dxvk"
    cp -r dxvk/dxvk-${WINE_VK_DXVK_VERSION}/x32/* wine/drive_c/windows/system32/
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


