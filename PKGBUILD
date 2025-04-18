# Created by: varmd

STRBUILD32="builder"
if [ "$USER" = "$STRBUILD32" ]; then
  WINE_BUILD_32=1
fi

if [ -z "${WINE_BUILD_32:-}" ]; then
  pkgname=('wine-wayland' 'wineland')
else
  pkgname=('wine-wayland' 'wineland' 'lib32-wine-wayland')
fi

RELEASE=10.5
_pkgname=('wine-wayland')


pkgver=`echo $RELEASE | sed s~-~~`
pkgrel=1
_winesrcdir="wine-wine-$RELEASE"

pkgdesc='Wine wayland'

url=''
arch=('x86_64')

options=('!staticlibs' '!docs' '!debug' '!lto')
license=('LGPL')

export PKGEXT='.pkg.tar.zst'

#export COMPRESSZST=(zstd -10 -c -z -q - )

depends=(
  'adwaita-cursors'
  'fontconfig'
  'freetype2'
  'gcc-libs'
  'desktop-file-utils'
  'libpng'
  'alsa-lib'
  'mesa'
  'vulkan-icd-loader'
  'wayland'
  'wayland-protocols'
)

makedepends=(
  'autoconf'
  'ncurses'
  'bison'
  'cmake'
  'perl'
  'flex'
  'gcc'
  'vulkan-headers'
  'gettext'
  'mingw-w64-gcc'
)


source=(
  "https://github.com/wine-mirror/wine/archive/wine-$RELEASE.zip"
  "https://github.com/civetweb/civetweb/archive/v1.15.tar.gz"
  "https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.0.16.zip"

)

sha256sums=(
 'SKIP' 'SKIP' 'SKIP'
 #'SKIP'
)

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
      -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
      -DCLOCK_GETTIME=ON

  CPUS=$(getconf _NPROCESSORS_ONLN)

  make -j $CPUS
  make DESTDIR="${srcdir}/sdl2-install/" install

  cd ${srcdir}/sdl2-install/usr/lib/pkgconfig
  sed -i "s~/usr/lib~${srcdir}/sdl2-install/usr/lib~g" sdl2.pc
  sed -i "s~/usr/include~${srcdir}/sdl2-install/usr/include~g" sdl2.pc
}






prepare() {

  if [ -e "${srcdir}"/"${_winesrcdir}"/dlls/winevulkan/vulkan-fsr-include.c ]; then
    msg2 "Stale src/ folder. Delete src/ folder or run makepkg --noextract."
    exit;
  else
    cd ..

    # Remove collabora driver for now
    rm -rf "${srcdir}"/"${_winesrcdir}"/dlls/winewayland*
    ln -s $PWD/winewayland.drv "${srcdir}"/"${_winesrcdir}"/dlls/winewayland.drv

    # Add shims

    rm -rf "${srcdir}"/"${_winesrcdir}"/dlls/opengl32
    cp -r shims/opengl32 "${srcdir}"/"${_winesrcdir}"/dlls/
    rm -rf "${srcdir}"/"${_winesrcdir}"/dlls/winspool.drv
    ln -s $PWD/shims/winspool.drv "${srcdir}"/"${_winesrcdir}"/dlls/




    cd "${srcdir}"/"${_winesrcdir}"

    patch -Np1 < '../../patches/enable-wayland.patch'

    msg2 "Patching broken alsa"
    patch -Np1 < '../../patches/fix-alsa-winecfg.patch'

    cd "${srcdir}"/"${_winesrcdir}"

# FSYNC

#    cp ../../patches/fsync/fsync-copy/ntdll/* dlls/ntdll/unix/
#    cp ../../patches/fsync/fsync-copy/server/* server/

#    cd "${srcdir}"/"${_winesrcdir}"


#    cp ../../patches/fsync/fsync-copy/ntdll/* dlls/ntdll/unix/
#    cp ../../patches/fsync/fsync-copy/server/* server/

 #   msg2 "Fsync"

 #   for _f in ../../patches/fsync/fsync/*.patch; do
 #     msg2 "Applying ${_f}"
 #     patch -Np1 < ${_f}
 #   done

 #   for _f in ../../patches/fsync/fsync/ntdll/*.patch; do
#      msg2 "Applying ${_f}"
 #     patch -Np1 < ${_f}
 #  done



#    for _f in ../../patches/fsync/misc/*.patch; do
#      msg2 "Applying ${_f}"
#      patch -Np1 < ${_f}
#    done

   msg2 "NTSYNC"


   patch -Np1 < '../../patches/ntsync-7226.patch'



    msg2 "Applying FSR patches"
    for _f in ../../patches/fsr/*.patch; do
      msg2 "Applying ${_f}"
      patch -Np1 < ${_f}
    done
    ln -s  $PWD/'../../patches/fsr/vulkan-fsr-include.c' dlls/winevulkan/



# speed up

#    sed -i '/programs\/explorer/d' configure.ac
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
    sed -i '/programs\/wusa/d' configure.ac
    sed -i '/programs\/robocopy/d' configure.ac
    sed -i '/programs\/cabarc/d' configure.ac
    sed -i '/programs\/mofcomp/d' configure.ac
    sed -i '/programs\/mshta/d' configure.ac
    sed -i '/programs\/dism/d' configure.ac
    sed -i '/programs\/certutil/d' configure.ac
    sed -i '/programs\/dpnsvr/d' configure.ac
    sed -i '/programs\/fc/d' configure.ac
    sed -i '/programs\/findstr/d' configure.ac
    sed -i '/programs\/sdbinst/d' configure.ac
    sed -i '/programs\/termsv/d' configure.ac
    sed -i '/programs\/xcopy/d' configure.ac
    sed -i '/programs\/extrac32/d' configure.ac
    sed -i '/programs\/expand/d' configure.ac
    sed -i '/programs\/attrib/d' configure.ac
    sed -i '/programs\/arp/d' configure.ac
    sed -i '/programs\/makecab/d' configure.ac
    sed -i '/programs\/pnputil/d' configure.ac
    sed -i '/programs\/ngen/d' configure.ac

    sed -i '/systeminfo/d' configure.ac



    #misc exe
    sed -i '/programs\/whoami/d' configure.ac
    sed -i '/programs\/eject/d' configure.ac
    sed -i '/programs\/shutdown/d' configure.ac
    sed -i '/programs\/csript/d' configure.ac
    sed -i '/programs\/wsript/d' configure.ac
    sed -i '/programs\/dplaysvr/d' configure.ac
    sed -i '/programs\/winefile/d' configure.ac
    sed -i '/programs\/where/d' configure.ac
    sed -i '/programs\/fsutil/d' configure.ac

    sed -i '/dlls\/d3d8/d' configure.ac
    sed -i '/dlls\/dxerr8/d' configure.ac
    sed -i '/dlls\/dx8vb/d' configure.ac
    sed -i '/dlls\/opencl/d' configure.ac
    sed -i '/msstyles/d' configure.ac

    #ie stuff

    sed -i '/dlls\/shdocvw/d' configure.ac
    sed -i '/WINAPI IEParseDisplayNameWithBCW/d' dlls/shell32/shfldr_desktop.c
    sed -i "s~IEParseDisplayNameWithBCW~S_OK;//~g" dlls/shell32/shfldr_desktop.c
    sed -i "s~shdocvw~ ~g" dlls/shell32/Makefile.in

    sed -i '/dlls\/ieframe/d' configure.ac
    sed -i '/dhtmled\.ocx/d' configure.ac
    sed -i '/inetcpl\.cpl/d' configure.ac

    #mshtml
    sed -i '/mshtml/d' configure.ac
    sed -i '/actxprxy/d' configure.ac
    sed -i '/msscript\.ocx/d' configure.ac
    sed -i '/wshom\.ocx/d' configure.ac

    #wlan
    sed -i '/dlls\/wlanui/d' configure.ac

    #msi
    sed -i '/dlls\/appwiz\.cpl/d' configure.ac
    sed -i '/dlls\/msisys\.ocx/d' configure.ac
    sed -i '/dlls\/msi)/d' configure.ac
    sed -i '/dlls\/msident/d' configure.ac
    sed -i '/dlls\/msimsg/d' configure.ac
    sed -i '/dlls\/msimtf/d' configure.ac
    sed -i '/dlls\/msisip/d' configure.ac
    sed -i '/programs\/msiexec/d' configure.ac
    sed -i '/programs\/msidb/d' configure.ac
    sed -i '/programs\/winemsibuilder/d' configure.ac

    #misc dll

    sed -i '/\/adsldp/d' configure.ac
    sed -i '/\/tests/d' configure.ac
    sed -i '/dlls\/d3d12/d' configure.ac
    sed -i '/dlls\/scrobj/d' configure.ac
    sed -i '/dlls\/jscript/d' configure.ac
    sed -i '/dlls\/vbscript/d' configure.ac
    sed -i '/programs\/cscript/d' configure.ac
    sed -i '/programs\/wscript/d' configure.ac
    sed -i '/dlls\/hhctrl/d' configure.ac
    sed -i '/dlls\/spoolss/d' configure.ac
    sed -i '/dlls\/localspl/d' configure.ac

    sed -i '/dlls\/gameux/d' configure.ac

    sed -i '/dlls\/wmphoto/d' configure.ac

    # Test
      # Direct3d Retained Mode
      sed -i '/dlls\/d3drm/d' configure.ac
      sed -i '/dlls\/adsldp/d' configure.ac
      sed -i '/dlls\/activeds/d' configure.ac
      sed -i '/dlls\/cards/d' configure.ac
      sed -i '/dlls\/d3dx10/d' configure.ac

      #d3dx9
      sed -i '/dlls\/d3dx9_24/d' configure.ac
      sed -i '/dlls\/d3dx9_25/d' configure.ac
      sed -i '/dlls\/d3dx9_26/d' configure.ac
      sed -i '/dlls\/d3dx9_27/d' configure.ac
      sed -i '/dlls\/d3dx9_28/d' configure.ac
      sed -i '/dlls\/d3dx9_29/d' configure.ac
      sed -i '/dlls\/d3dx9_30/d' configure.ac
      sed -i '/dlls\/d3dxcompiler_33/d' configure.ac
      sed -i '/dlls\/d3dxcompiler_34/d' configure.ac
      sed -i '/dlls\/d3dxcompiler_35/d' configure.ac
      sed -i '/dlls\/d3dxcompiler_36/d' configure.ac


      # Webservices
      sed -i '/dlls\/wsdapi/d' configure.ac
      sed -i '/dlls\/webservices/d' configure.ac
      # MSADO
      sed -i '/dlls\/msado15/d' configure.ac
      sed -i '/dlls\/msdaps/d' configure.ac
      # Riched
      sed -i '/dlls\/riched20/d' configure.ac
      sed -i '/dlls\/riched32/d' configure.ac
      sed -i '/dlls\/msftedit/d' configure.ac

      sed -i '/dlls\/mapi32/d' configure.ac
      sed -i '/dlls\/winemapi/d' configure.ac
      sed -i '/dlls\/qdvd/d' configure.ac
      sed -i '/dlls\/wiaservc/d' configure.ac
      sed -i '/dlls\/prntvpt/d' configure.ac

      sed -i '/dlls\/msvcrtd/d' configure.ac

      #wbem
      sed -i '/dlls\/wbemprox/d' configure.ac
      sed -i '/dlls\/wbemdisp/d' configure.ac
      #windows update
      sed -i '/programs\/wuauserv/d' configure.ac
      #misc exe
      sed -i '/programs\/winebrowser/d' configure.ac
      sed -i '/programs\/write/d' configure.ac
      sed -i '/programs\/dpvsetup/d' configure.ac
      sed -i '/programs\/msinfo32/d' configure.ac
      sed -i '/programs\/winver/d' configure.ac
      sed -i '/programs\/uninstaller/d' configure.ac

      ### WMI command-line (WMIC)
      sed -i '/programs\/wmic/d' configure.ac
      ### Display information about the Microsoft-Windows-Eventlog event publisher
      sed -i '/programs\/wmic/d' configure.ac

      #misc cpl + exe
      sed -i '/bthprops/d' configure.ac
      sed -i '/irprops/d' configure.ac
      sed -i '/joy\./d' configure.ac
      sed -i '/programs\/chcp\./d' configure.ac


      ###misc dll
      #Wine Bluetooth driver
      sed -i '/dlls\/winebth/d' configure.ac
      #MSCMS - Color Management System for Wine
      sed -i '/dlls\/mscms/d' configure.ac

      #Speech Application Programming Interface or SAPI
      sed -i '/dlls\/sapi/d' configure.ac
      #Background Intelligent Transfer Service Proxy
      sed -i '/dlls\/qmgrprxy/d' configure.ac

      #UI Automation Core
      sed -i '/dlls\/uiautomationcore/d' configure.ac


      #.Net
      sed -i '/dlls\/diasymreader/d' configure.ac

      #indeo
      sed -i '/ir50_32/d' configure.ac

      #acl
      sed -i '/acledit/d' configure.ac

      #misc tools
      sed -i '/tools\/winedump/d' configure.ac








    rm configure
    autoconf

    mkdir -p "${srcdir}"/"${_pkgname}"-64-build



  fi

}




build() {

  if [ -z "${WINE_BUILD_32:-}" ]; then
    D=1
  else
    msg2 "Also building wine32"
  fi

  #build sdl2 here to avoid x11 dependencies from official archlinux sdl2
  build_sdl2


  export PKG_CONFIG_PATH="$PKG_CONFIG_PATH:${srcdir}/sdl2-install/usr/lib/pkgconfig:/usr/lib/pkgconfig"
  export LD_LIBRARY_PATH="/usr/lib:${srcdir}/sdl2-install/usr/lib"

  export CFLAGS="$CFLAGS"

  CFLAGS="${CFLAGS/-Wp,-D_FORTIFY_SOURCE=2/}"
  CFLAGS="${CFLAGS/-O2/}"

  export CFLAGS="${CFLAGS/-fno-plt/} -O2 "
  export CFLAGS="${CFLAGS/-flto/}"
  export CFLAGS="${CFLAGS/-ffat-lto-objects/}"

  export LDFLAGS="${LDFLAGS/,-z,relro,-z,now/}"

  if [ -z "${WINE_BUILD_32:-}" ]; then
    I386=
  else
    I386=i386,
  fi


  msg2 'Building Wine-64...'
	cd  "${srcdir}"/${_pkgname}-64-build

  if [ -e Makefile ]; then
    echo ""
  else
  ../${_winesrcdir}/configure \
		--prefix='/usr' \
		--libdir='/usr/lib' \
		--enable-archs=${I386}x86_64 \
		--without-x \
    --without-oss \
		--without-capi \
		--without-dbus \
		--without-gphoto \
		--without-gssapi \
		--without-netapi \
    --without-opencl \
    --without-opengl \
    --without-cups \
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
    --without-xshm \
    --without-v4l2 \
    --without-usb \
    --without-pulse \
    --with-alsa \
    --with-sdl \
    --with-vulkan \
		--enable-win64 \
		--enable-wayland \
		--with-mingw \
		--disable-tests
  fi

  CPUS=$(getconf _NPROCESSORS_ONLN)
  if ((CPUS > 10)); then
    CPUS=6;
  fi
	make -s -j $CPUS

}

package_lib32-wine-wayland() {

  provides=('lib32-wine-wayland')

  depends=(
    'wine-wayland'
  )

  mkdir -p $pkgdir/usr/lib/wine
  mv $srcdir/i386-windows "$pkgdir"/usr/lib/wine/i386-windows/

  #cleanup
  rm -rf "$pkgdir"/usr/lib/wine/i386-windows/*.a
  #i686-w64-mingw32-strip --strip-unneeded "$pkgdir"/usr/lib/wine/i386-windows/*.dll

}


package_wineland() {

  depends=(
    'adwaita-icon-theme'
    'fontconfig'
    'freetype2'
    'gcc-libs'
    'desktop-file-utils'
    'alsa-lib'
    'mesa'
    'vulkan-icd-loader'
  )

  #build civetweb for wineland
  cd $srcdir
  cd civetweb-1.15
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

  CPUS=$(getconf _NPROCESSORS_ONLN)
  if ((CPUS > 10)); then
    CPUS=6;
  fi

  if [ -z "${WINE_BUILD_32_DEV_SKIP_64:-}" ]; then
    echo "Building 64bit complete"
  else
    return 0;
  fi


  depends=(
    'adwaita-icon-theme'
    'fontconfig'
    'freetype2'
    'gcc-libs'
    'desktop-file-utils'
    'libpng'
    'alsa-lib'
    'mesa'
    'vulkan-icd-loader'
  )

  conflicts=('wine' 'wine-staging')
  provides=('wine')


	cd "${srcdir}/${_pkgname}"-64-build
	make -j $CPUS -s prefix="${pkgdir}/usr" \
			libdir="${pkgdir}/usr/lib" \
			dlldir="${pkgdir}/usr/lib/wine" install

  #Cleanup
  rm -rf $pkgdir/usr/include
  rm -rf $pkgdir/usr/share/man
  rm -rf $pkgdir/usr/lib/wine/x86_64-unix/*.a
  rm -rf $pkgdir/usr/lib/wine/x86_64-unix/*.def
  cd $pkgdir/usr/lib/wine/x86_64-unix/
  strip -s *

  x86_64-w64-mingw32-strip --strip-unneeded "$pkgdir"/usr/lib/wine/x86_64-windows/*

  if [ -z "${WINE_BUILD_32:-}" ]; then
    #msg2 "Not building wine 32"
    cp $pkgdir/usr/bin/wine $pkgdir/usr/bin/wine64
  else
    cp $pkgdir/usr/bin/wine $pkgdir/usr/bin/wine64
    i686-w64-mingw32-strip --strip-unneeded "$pkgdir"/usr/lib/wine/i386-windows/*.dll
    rm -rf $srcdir/i386-windows/
    mv "$pkgdir"/usr/lib/wine/i386-windows/ $srcdir/
  fi



  #SDL2
  mkdir -p $pkgdir/usr/lib/wineland/lib
  rm -rf $pkgdir/usr/include/SDL2*
  rm -rf $pkgdir/usr/bin/sdl2*
  rm -rf $pkgdir/usr/bin/aserver
  rm -rf $pkgdir/usr/lib/cmake
  rm -rf $pkgdir/usr/lib/pkgconfig
  rm -rf $pkgdir/usr/share/aclocal
  rm -rf $pkgdir/usr/share/applications
  cp --preserve=links ${srcdir}/sdl2-install/usr/lib/libSDL2* $pkgdir/usr/lib/wineland/lib/

  #cp --preserve=links ${srcdir}/alsa-lib-install/usr/lib/liba* $pkgdir/usr/lib/wineland/lib/
  rm -rf $pkgdir/usr/lib/libSDL2*
  rm -rf $pkgdir/usr/lib/wineland/*.a
  rm -rf $pkgdir/usr/lib/wine/x86_64-windows/*.a
  rm -rf $pkgdir/usr/lib/wine/x86_64-windows/d3d11.dll
  rm -rf $pkgdir/usr/lib/wine/x86_64-windows/d3d9.dll

  rm -rf $pkgdir/usr/lib/wine/x86_64-windows/dxgi.dll

  # Obsolete for 64bit games
  rm -rf $pkgdir/usr/lib/wine/x86_64-windows/wined3d.dll
  rm -rf $pkgdir/usr/lib/wine/x86_64-windows/ddraw.dll

  # Useless for games
  rm -rf $pkgdir/usr/bin/widl
  rm -rf $pkgdir/usr/bin/wrc
  rm -rf $pkgdir/usr/bin/winemaker
  rm -rf $pkgdir/usr/bin/winebuild
  rm -rf $pkgdir/usr/bin/winegcc

  #misc nls
  rm  $pkgdir/usr/share/wine/nls/c_949.nls
  rm  $pkgdir/usr/share/wine/nls/c_708.nls


}


