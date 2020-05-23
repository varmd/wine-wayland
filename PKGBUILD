# Created by: varmd

pkgname=wine-wayland
pkgver=5.9
pkgrel=2
_winesrcdir="wine-wine-$pkgver"

pkgdesc='Wine wayland'

url=''
arch=('x86_64')

options=('staticlibs' '!strip')

license=('LGPL')

depends=(
    'fontconfig'            
    'libxml2'              
    'freetype2'             
    'gcc-libs'              
    'desktop-file-utils'
    'gnutls'
)

makedepends=('git' 
    'autoconf' 
    'ncurses' 
    'bison' 
    'perl' 
    'flex'
    'gcc'
    'libpng'                
    'gnutls'                
    'mpg123'                
    'openal'    
    'alsa-lib'
    'mesa'
    'vulkan-icd-loader'    
    'vulkan-headers'    
    'freetype2'             
    'gettext'               
    'fontconfig'            
    'faudio'            
    'zstd'            
)


source=("https://github.com/wine-mirror/wine/archive/wine-$pkgver.zip")
    
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

    rm configure
    autoconf

    msg2 "Applying esync patches"
      cd "${srcdir}"
      cp -r ../esync .
      cd "${srcdir}"/"${_winesrcdir}"
      for _f in "${srcdir}"/esync/*.patch; do
        msg2 "Applying ${_f}"
        #git apply -C1 --verbose < ${_f}
        patch -Np1 < ${_f}
      done
    
    
    msg2 "Applying esync temp fix"
    patch -Np1 < '../../esync-no_kernel_obj_list.patch'  
    
    msg2 "Applying fsync"
    patch -Np1 < '../../fsync-mainline.patch'  
    
    #msg2 "Applying performance patches"
    patch -Np1 < '../../performance-disable-raw-clock.patch'  
    patch -Np1 < '../../performance-proton-improve-vulkan-alloc.patch'  
    
    mkdir -p "${srcdir}"/"${pkgname}"-64-build
    
  fi
	
}




build() {
	cd "${srcdir}"
  
  export COMMON_FLAGS="-w -march=native -pipe -Os"
  export LDFLAGS="-Os"
	
	export CFLAGS="${CFLAGS} -w"
	


  msg2 'Building Wine-64...'
	cd  "${srcdir}"/"${pkgname}"-64-build
	
  
  if [ -e Makefile ]; then 
    echo "Already configured"
  else
  #if [0]; then
  
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
	
	make -s -j 5

}

package() {
	export PKGEXT='.pkg.tar.zst'
	cd "${srcdir}/${pkgname}"-64-build
	make -s	prefix="${pkgdir}/usr" \
			libdir="${pkgdir}/usr/lib" \
			dlldir="${pkgdir}/usr/lib/wine" install

}
