# Created by: varmd

pkgname=wine-wayland
pkgver=5.13
#pkgver=master
pkgrel=11
_winesrcdir="wine-wine-$pkgver"
#_winesrcdir="wine-master"

pkgdesc='Wine wayland'

url=''
arch=('x86_64')

options=('!staticlibs' '!strip' '!docs')

license=('LGPL')

depends=(
    'fontconfig'            
    'libxml2'              
    'freetype2'             
    'gcc-libs'              
    'desktop-file-utils'
)

makedepends=(
    'autoconf' 
    'ncurses' 
    'bison' 
    'perl' 
    'flex'
    'gcc'
    'libpng'                
    'mpg123'                
    'openal'    
    'alsa-lib'
    'mesa'
    'vulkan-icd-loader'    
    'vulkan-headers'    
    'gettext'               
    'faudio'            
    'zstd'            
)


source=("https://github.com/wine-mirror/wine/archive/wine-$pkgver.zip")
#source=("https://github.com/wine-mirror/wine/archive/master.zip")
    
sha256sums=('SKIP')


conflicts=('wine' 'wine-staging' 'wine-esync')

OPTIONS=(!strip !docs !libtool !zipman !purge !debug) 
makedepends=(${makedepends[@]} ${depends[@]})



prepare() {

  if [ -e "${srcdir}"/"${_winesrcdir}"/server/esync.c ]; then 
    msg2 "Stale src/ folder. Delete src/ folder or run makepkg --noextract."
    exit;    
  else
    cd ..
    rm -f "${srcdir}"/"${_winesrcdir}"/dlls/winewayland*

    ln -s $PWD/winewayland* "${srcdir}"/"${_winesrcdir}"/dlls/
    
    cd "${srcdir}"/"${_winesrcdir}"
    
    patch -Np1 < '../../enable-wayland.patch'  
    
    
    patch dlls/user32/driver.c < ../../winewayland.drv/patch/user32-driverc.patch
    patch dlls/user32/sysparams.c < ../../winewayland.drv/patch/user32-sysparamsc-new.patch
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
    
    
    rm configure
    autoconf

    mkdir -p "${srcdir}"/"${pkgname}"-64-build
    
  fi
	
}




build() {
	cd "${srcdir}"
  
  
  export CC=cc
  #Remove these - potentially buggy
  #export CFLAGS="${CFLAGS} -w -march=native -pipe -Ofast"
  #export LDFLAGS="${CFLAGS}"
	
  msg2 'Building Wine-64...'
	cd  "${srcdir}"/"${pkgname}"-64-build
	
  
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
		--without-udev \
		--without-netapi \
		--without-hal \
    --without-gsm \
    --without-opencl \
    --without-opengl \
    --without-sdl \
    --without-cups \
    --without-cms \
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
    --without-glu \
    --without-xcomposite \
    --without-xcursor \
    --without-hal \
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
    --with-vulkan \
    --with-faudio \
    --disable-win16 \
		--enable-win64 \
		--disable-tests
  fi
	
  CPUS=$(getconf _NPROCESSORS_ONLN)
  
	make -s -j $CPUS

}

package() {
	export PKGEXT='.pkg.tar.zst'
	cd "${srcdir}/${pkgname}"-64-build
	make -s	prefix="${pkgdir}/usr" \
			libdir="${pkgdir}/usr/lib" \
			dlldir="${pkgdir}/usr/lib/wine" install

}
