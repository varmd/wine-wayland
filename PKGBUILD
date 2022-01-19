# Created by: varmd

RELEASE=7.0
_pkgname=('wine-wayland')
pkgname=('wine-wayland' 'wineland' )

pkgver=`echo $RELEASE | sed s~-~~`
pkgrel=1
_winesrcdir="wine-wine-$RELEASE"

pkgdesc='Wine wayland'

url=''
arch=('x86_64')

options=('!staticlibs' '!strip' '!docs')
license=('LGPL')

export LANG=en_US.utf8
LANG=en_US.utf8
export PKGEXT='.pkg.tar.zst'

depends=(
  'adwaita-icon-theme'
  'fontconfig'
  'libxml2'
  'freetype2'
  'gcc-libs'
  'desktop-file-utils'
  'libpng'
  'mpg123'
  'openal'
  'alsa-lib'
  'mesa'
  'vulkan-icd-loader'
  #'faudio'

  'wayland'
  'wayland-protocols'
)

makedepends=(
  'autoconf'
  'ncurses'
  'bison'
  'perl'
  'flex'
  'gcc'
  'vulkan-headers'
  'gettext'
  'zstd'
)


source=(
  "https://github.com/wine-mirror/wine/archive/wine-$RELEASE.zip"
  "https://github.com/civetweb/civetweb/archive/v1.12.zip"
  "https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.0.16.zip"
  "https://github.com/FNA-XNA/FAudio/archive/refs/tags/21.10.zip"
)

sha256sums=('SKIP' 'SKIP' 'SKIP' 'SKIP')


STRBUILD32="builder"


if [ "$USER" = "$STRBUILD32" ]; then
  WINE_BUILD_32=1
fi


if [ -z "${WINE_BUILD_32:-}" ]; then
  #msg2 "Not building wine 32"
  D=1
else
  source ./PKGBUILD-32
  #msg2 "Also building wine 32"
fi



OPTIONS=(!strip !docs !libtool !zipman !purge !debug)
makedepends=(${makedepends[@]} ${depends[@]})




build_sdl2() {
  cd "${srcdir}"

  cd $(find . -maxdepth 1 -type d -name "*SDL*" | sed 1q)

  mkdir -p $srcdir/sdl2-install
  
  mkdir -p build
  cd build


  export PKG_CONFIG_PATH="$srcdir/sdl2-install/usr/lib/pkgconfig"

  cmake .. \
      -DCMAKE_INSTALL_PREFIX=/usr \
      -DSDL_STATIC=OFF \
      -DSDL_DLOPEN=ON \
      -DSDL_USE_LIBDBUS=OFF \
      -DARTS=OFF \
      -DESD=OFF \
      -DNAS=OFF \
      -DALSA=ON \
      -DJACK=OFF \
      -DDBUS=OFF \
      -DHAPTIC=ON \
      -DJOYSTICK=ON \
      -DJOYSTICK_HIDAPI=ON \
      -DPULSEAUDIO=OFF \
      -DPULSEAUDIO_SHARED=OFF \
      -DVIDEO_WAYLAND=OFF \
      -DVIDEO_X11=OFF \
      -DVIDEO_OPENGL=OFF \
      -DVIDEO_VULKAN=OFF \
      -DVIDEO_KMSDRM=OFF \
      -DVIDEO_OPENGLES=OFF \
      -DX11_SHARED=OFF \
      -DRPATH=OFF \
      -DCLOCK_GETTIME=ON

  CPUS=$(getconf _NPROCESSORS_ONLN)

  make -j $CPUS
  make DESTDIR="${srcdir}/sdl2-install/" install

  cd ${srcdir}/sdl2-install/usr/lib/pkgconfig
  sed -i "s~/usr/lib~${srcdir}/sdl2-install/usr/lib~g" sdl2.pc
  sed -i "s~/usr/include~${srcdir}/sdl2-install/usr/include~g" sdl2.pc
}

build_faudio() {
  cd "${srcdir}"

  cd $(find . -maxdepth 1 -type d -name "*FAudio*" | sed 1q)

  mkdir -p $srcdir/faudio-install
  
  mkdir -p build
  cd build


  export PKG_CONFIG_PATH="$srcdir/sdl2-install/usr/lib/pkgconfig"
  export LD_LIBRARY_PATH="/usr/lib:${srcdir}/sdl2-install/usr/lib:$LD_LIBRARY_PATH"

  cmake .. -G Ninja \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DCMAKE_BUILD_TYPE=Release \
    -DSDL2_INCLUDE_DIRS=$srcdir/sdl2-install/usr/include/SDL2 \
    -DSDL2_LIBRARIES=$srcdir/sdl2-install/usr/lib/libSDL2.so \
    -DCMAKE_INSTALL_LIBDIR=lib

  CPUS=$(getconf _NPROCESSORS_ONLN)

  ninja -j $CPUS
  DESTDIR="${srcdir}/faudio-install/" ninja install

  cd ${srcdir}/faudio-install/usr/lib/pkgconfig
  sed -i "s~/usr~${srcdir}/faudio-install/usr~g" FAudio.pc
}




prepare() {

  if [ -e "${srcdir}"/"${_winesrcdir}"/server/esync.c ]; then
    msg2 "Stale src/ folder. Delete src/ folder or run makepkg --noextract."
    exit;
  else
    cd ..
    
    #rm -rf  "${srcdir}"/"${_winesrcdir}"/dlls/winevulkan
    #ln -s $PWD/winevulkan "${srcdir}"/"${_winesrcdir}"/dlls/
    
    rm -f "${srcdir}"/"${_winesrcdir}"/dlls/winewayland*

    ln -s $PWD/winewayland* "${srcdir}"/"${_winesrcdir}"/dlls/

    cd "${srcdir}"/"${_winesrcdir}"

    patch -Np1 < '../../enable-wayland.patch'
    
    #fix civ 6
    #TODO remove when fixed in Wine
    patch -Np1 < '../../fix-civ6.patch'


    patch dlls/user32/driver.c < ../../winewayland.drv/patch/user32-driverc.patch
    
    #patch dlls/user32/sysparams.c < ../../winewayland.drv/patch/0002-user32-sysparamsc-new2.patch
    #patch dlls/user32/sysparams.c < ../../winewayland.drv/patch/0004-user32-sysparams-fix-valid-adapter.patch
    
    patch programs/explorer/desktop.c < ../../winewayland.drv/patch/explorer-desktopc.patch    
    


    cd "${srcdir}"/"${_winesrcdir}"


    cp ../../esync2/esync-copy/ntdll/* dlls/ntdll/unix/
    cp ../../esync2/esync-copy/server/* server/

    cp ../../esync2/fsync-copy/ntdll/* dlls/ntdll/unix/
    cp ../../esync2/fsync-copy/server/* server/


    for _f in ../../esync2/ok/server/*.patch; do
      msg2 "Applying ${_f}"
      patch -Np1 < ${_f}
    done

    for _f in ../../esync2/ok/*.patch; do
      msg2 "Applying ${_f}"
      patch -Np1 < ${_f}
    done


    for _f in ../../esync2/fsync/*.patch; do
      msg2 "Applying ${_f}"
      patch -Np1 < ${_f}
    done
    
    #fix -lrt compilation
    patch -Np1 < ../../esync2/fix-rt.patch
    
    
    
    msg2 "Applying FSR patches"
    for _f in ../../patches/fsr/*.patch; do
      msg2 "Applying ${_f}"
      patch -Np1 < ${_f}
    done
    cp '../../patches/fsr/vulkan-fsr-include.c' dlls/winevulkan/
    
    #patch -Np1 < '../../patches/fsr/fsr-wine-include.patch'
    #patch -Np1 < '../../patches/fsr/fsr-vulkanc.patch'
    #patch -Np1 < '../../patches/fsr/fsr-make-vulkan.patch'
    #patch -Np1 < '../../patches/fsr/fsr-vulkan_thunks.patch'
    #patch -Np1 < '../../patches/fsr/fsr-Makefile.patch'
    
    
    
    # speed up

    sed -i '/programs\/explorer/d' configure.ac
    sed -i '/programs\/iexplore/d' configure.ac
    sed -i '/programs\/dxdiag/d' configure.ac

    sed -i '/programs\/hh/d' configure.ac
    sed -i '/programs\/powershell/d' configure.ac
    sed -i '/programs\/winemenubuilder/d' configure.ac
    sed -i '/programs\/wordpad/d' configure.ac
    
    sed -i '/programs\/winedbg/d' configure.ac
    sed -i '/programs\/winemine/d' configure.ac
    sed -i '/wineps/d' configure.ac
    
    sed -i '/programs\/taskmgr/d' configure.ac
    sed -i '/winhlp32/d' configure.ac
    sed -i '/programs\/notepad/d' configure.ac
    sed -i '/programs\/aspnet/d' configure.ac
    sed -i '/programs\/xpsprint/d' configure.ac
    sed -i '/programs\/oleview/d' configure.ac
    sed -i '/programs\/progman/d' configure.ac
    sed -i '/programs\/clock/d' configure.ac
    sed -i '/programs\/wmplayer/d' configure.ac
    sed -i '/programs\/spoolsv/d' configure.ac
    sed -i '/programs\/schtasks/d' configure.ac
    sed -i '/systeminfo/d' configure.ac
    
    
    
    #misc exe
    sed -i '/programs\/whoami/d' configure.ac
    sed -i '/programs\/eject/d' configure.ac
    sed -i '/programs\/shutdown/d' configure.ac
    sed -i '/programs\/csript/d' configure.ac
    sed -i '/programs\/dplaysvr/d' configure.ac
    
    sed -i '/dlls\/d3d8/d' configure.ac    
    sed -i '/dlls\/dxerr8/d' configure.ac
    sed -i '/dlls\/dx8vb/d' configure.ac
    sed -i '/dlls\/opencl/d' configure.ac
    sed -i '/msstyles/d' configure.ac
    
    #ie stuff
    #sed -i '/dlls\/shdocvw/d' configure.ac
    #sed -i '/dlls\/ieframe/d' configure.ac
    sed -i '/dhtmled\.ocx/d' configure.ac
    sed -i '/inetcpl\.cpl/d' configure.ac
    
    #mshtml
    sed -i '/mshtml/d' configure.ac
    sed -i '/actxprxy/d' configure.ac
    sed -i '/msscript\.ocx/d' configure.ac
    sed -i '/wshom\.ocx/d' configure.ac
    
    #wlan
    sed -i '/dlls\/wlanui/d' configure.ac

    #misc dll
    
    sed -i '/\/adsldp/d' configure.ac
    sed -i '/\/tests/d' configure.ac
    sed -i '/dlls\/d3d12/d' configure.ac
    sed -i '/dlls\/jscript/d' configure.ac
    sed -i '/dlls\/vbscript/d' configure.ac
    sed -i '/dlls\/hhctrl/d' configure.ac
    
    sed -i '/dlls\/gameux/d' configure.ac


    rm configure
    autoconf

    mkdir -p "${srcdir}"/"${_pkgname}"-64-build
    
    

  fi

}




build() {


  if [ -z "${WINE_BUILD_32_DEV_SKIP_64:-}" ]; then
    echo "Building 64bit"
  else
    return 0;
  fi

  #build sdl2 here to avoid x11 dependencies from official archlinux sdl2
  build_sdl2
  
  #build faudio here to avoid x11 dependencies from official archlinux
  build_faudio

  export PKG_CONFIG_PATH="${srcdir}/sdl2-install/usr/lib/pkgconfig:${srcdir}/faudio-install/usr/lib/pkgconfig:/usr/lib/pkgconfig"
  export LD_LIBRARY_PATH="/usr/lib:${srcdir}/sdl2-install/usr/lib:${srcdir}/faudio-install/usr/lib:$LD_LIBRARY_PATH"

  msg2 'Building Wine-64...'
	cd  "${srcdir}"/${_pkgname}-64-build

  if [ -e Makefile ]; then
    echo "Already configured"
  else
  ../${_winesrcdir}/configure \
		--prefix='/usr' \
		--libdir='/usr/lib' \
		--without-x \
		--without-capi \
		--without-dbus \
		--without-gphoto \
		--without-gssapi \
		--without-netapi \
    --without-opencl \
    --without-opengl \
    --without-cups \
    --without-vkd3d \
    --without-xinerama \
    --without-xrandr \
    --without-sane \
    --without-osmesa \
    --without-gettext \
    --without-fontconfig \
    --without-cups \
    --disable-win16 \
    --without-gphoto \
    --without-xcomposite \
    --without-xcursor \
    --without-xfixes \
    --without-xshape \
    --without-xrender \
    --without-xinput \
    --without-xinput2 \
    --without-xrender \
    --without-xxf86vm \
    --without-xshm \
    --without-v4l2 \
    --without-usb \
    --with-sdl2 \
    --with-vulkan \
    --with-faudio \
    --disable-win16 \
		--enable-win64 \
		--disable-tests
  fi

  CPUS=$(getconf _NPROCESSORS_ONLN)
  if ((CPUS > 10)); then
    CPUS=8;
  fi
	make -s -j $CPUS

}

package_wineland() {

  
  depends=(
    'adwaita-icon-theme'
    'fontconfig'
    'freetype2'
    'gcc-libs'
    'desktop-file-utils'
    'mpg123'
    'openal'
    'alsa-lib'
    'mesa'
    'vulkan-icd-loader'
    #'faudio'
    'libpng'
    'libxml2'
    'lib32-glibc'
  )


  #build civetweb for wineland
  cd $srcdir
  cd civetweb-1.12
  make build WITH_IPV6=0 USE_LUA=0 PREFIX="$pkgdir/usr"
  
  mkdir -p ${pkgdir}/usr/bin
  mkdir -p ${pkgdir}/usr/lib/wineland
  cp ${srcdir}/civetweb*/civetweb ${pkgdir}/usr/lib/wineland/wineland-civetweb
  cd ${srcdir}
  cp -r ../wineland ${pkgdir}/usr/lib/wineland/ui
  cp -r ../wineland/joystick.svg ${pkgdir}/usr/lib/wineland/ui/joystick.svg
  cp -r ../wineland/wineland ${pkgdir}/usr/bin/wineland
  chmod +x ${pkgdir}/usr/bin/wineland

  mkdir -p ${pkgdir}/usr/share/applications
  cp -r ../wineland/wineland.desktop ${pkgdir}/usr/share/applications/wineland.desktop

}


package_wine-wayland() {

  

  if [ -z "${WINE_BUILD_32_DEV_SKIP_64:-}" ]; then
    echo "Building 64bit"
  else
    return 0;
  fi


  depends=(
    'adwaita-icon-theme'
    'fontconfig'
    'libxml2'
    'freetype2'
    'gcc-libs'
    'desktop-file-utils'
    'libpng'
    'mpg123'
    'openal'
    'alsa-lib'
    'mesa'
    'vulkan-icd-loader'
    #'faudio'
  )

  conflicts=('wine' 'wine-staging' 'wine-esync')
  provides=('wine')


	cd "${srcdir}/${_pkgname}"-64-build
	make -s	prefix="${pkgdir}/usr" \
			libdir="${pkgdir}/usr/lib" \
			dlldir="${pkgdir}/usr/lib/wine" install

  #Cleanup
  rm -rf $pkgdir/usr/include
  rm -rf $pkgdir/usr/share/man
  rm -rf $pkgdir/usr/lib/wine/x86_64-unix/*.a
  rm -rf $pkgdir/usr/lib/wine/x86_64-unix/*.def
  cd $pkgdir/usr/lib/wine/x86_64-unix/
  strip -s *
  
  #SDL2
  mkdir -p $pkgdir/usr/lib/wineland/lib
  rm -rf $pkgdir/usr/include/SDL2*
  rm -rf $pkgdir/usr/bin/sdl2*
  rm -rf $pkgdir/usr/lib/cmake
  rm -rf $pkgdir/usr/lib/pkgconfig
  rm -rf $pkgdir/usr/share/aclocal
  rm -rf $pkgdir/usr/share/applications
  cp --preserve=links ${srcdir}/sdl2-install/usr/lib/libSDL2* $pkgdir/usr/lib/wineland/lib/
  rm -rf $pkgdir/usr/lib/libSDL2*
  
}
