#
# Portable launcher
#

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

echo "Starting $1";

PWD_PATH=$PWD
LOG_PATH=$PWDlog.log



WINEDLLOVERRIDES="d3dcompiler_47,d3d9,dxgi,d3d11,d3d12=n,b;gameux=d;dinput=d;winedbg=d;winemenubuilder.exe=d;mscoree=d;mshtml=d;cchromaeditorlibrary64=d;$WINEDLLOVERRIDES"

WINE_VK_WAYLAND_WIDTH=1920
WINE_VK_WAYLAND_HEIGHT=1080
MANGOHUD=1


export XCURSOR_SIZE="32"
export XCURSOR_THEME=Adwaita


source ./options.conf

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







mkdir -p /run/user/$UID/mangohud-wine-wayland
cp -r mangohud/usr/lib/mangohud/* /run/user/$UID/mangohud-wine-wayland
cp -r mangohud/usr/share/vulkan/implicit_layer.d/MangoHud.json mangohud/mangohud.json

if [[ -z "$MANGOHUD" ]]; then
  echo ""
else

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

fi






cd "$PWD_PATH"

#download local wine

if [ ! -d $PWD_PATH/winebin ]; then
  echo "winebin is missing"
  exit;
fi


rm $PWD_PATH/wine/drive_c/"$GAME_PATHNAME"

#link to c:
echo $PWD_PATH
ln -s $PWD_PATH/"$GAME_PATHNAME" $PWD_PATH/wine/drive_c/ 2> /dev/null

cd $PWD_PATH/wine/drive_c/"$GAME_PATHNAME"/$FINAL_PATH


#local wine

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
        _NOEXEC_BIN_PATH="$HOME/.cache/wineland/winebin/usr/bin"
        _NOEXEC_LIB32_PATH="$HOME/.cache/wineland/winebin/usr/lib/wineland/lib32"

      fi
    else #tmp
      cp -r $PWD_PATH/winebin /tmp/wineland
      NOEXEC_BIN_PATH="/tmp/wineland/winebin/usr/bin:"
      NOEXEC_LIB_PATH="/tmp/wineland/winebin/usr/lib:"
      _NOEXEC_LIB_PATH="/tmp/wineland/winebin/usr/lib"
      NOEXEC_LIB32_PATH="/tmp/wineland/winebin/usr/lib/wineland/lib32:"
      _NOEXEC_LIB32_PATH="/tmp/wineland/winebin/usr/lib/wineland/lib32"
      _NOEXEC_BIN_PATH="/tmp/wineland/winebin/usr/bin"
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

      export VK_ICD_FILENAMES="$_NOEXEC_LIB_PATH/wineland/vulkan/icd.d/radeon_icd.i686.json"

      sed -i "s#\"/usr/lib#\"$_NOEXEC_LIB_PATH#g" ${_NOEXEC_LIB_PATH}/wineland/vulkan/icd.d/*json
      #fix libc
      patchelf-for-wineland --set-interpreter ${_NOEXEC_LIB32_PATH}/ld-linux.so.2 ${_NOEXEC_BIN_PATH}/wine
    else
      #replace with originals from backup
      cp $PWD_PATH/winebin/usr/lib/wineland/vulkan/orig/*json $PWD_PATH/winebin/usr/lib/wineland/vulkan/icd.d/

      sed -i "s#\"/usr/lib#\"${PWD_PATH}/winebin/usr/lib#g" $PWD_PATH/winebin/usr/lib/wineland/vulkan/icd.d/*json
      #fix libc
      patchelf-for-wineland --set-interpreter $PWD_PATH/winebin/usr/lib/wineland/lib32/ld-linux.so.2 $PWD_PATH/winebin/usr/bin/wine
    fi
  fi




cd "$PWD_PATH"


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


rm $PWD_PATH/wine/drive_c/users/$USER/"My Documents" &> /dev/null
rm $PWD_PATH/wine/drive_c/users/$USER/"Documents" &> /dev/null
mkdir -p $PWD_PATH/wine/drive_c/users/$USER/"My Documents"
mkdir -p $PWD_PATH/wine/drive_c/users/$USER/"Documents"


#remove z: for better security
rm -r $PWD_PATH/wine/dosdevices/z: &> /dev/null

#hack for multiple level exes
FINAL_PATH="$(dirname "$GAME_EXE")"
FINAL_EXE="$(basename "$GAME_EXE")"

cd $PWD_PATH/wine/drive_c/"$GAME_PATHNAME"/$FINAL_PATH



#export variables before starting exe
export WINEDEBUG=fixme-all,-all,+waylanddrv

echo "Launching $1"
$WINE_CMD $FINAL_EXE $GAME_OPTIONS  &> $LOG_PATH


