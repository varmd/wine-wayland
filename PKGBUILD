# Created by: varmd

pkgname=wine-wayland
pkgver=4.21
pkgrel=75
_winesrcdir='wine-wine-4.21'
_esyncsrcdir='esync'
_where=$PWD

pkgdesc=''

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
    'libxml2'               
    'fontconfig'            
    'faudio'            
)


source=("https://github.com/wine-mirror/wine/archive/wine-4.21.zip")
    
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
    cp -r $PWD/winewayland* "${srcdir}"/"${_winesrcdir}"/dlls/
    
    cd "${srcdir}"/"${_winesrcdir}"
    
    patch -Np1 < '../../enable-wayland.patch'  
    
    patch dlls/user32/driver.c < ../../winewayland.drv/patch/user32-driverc.patch
    patch dlls/user32/sysparams.c < ../../winewayland.drv/patch/user32-sysparamsc.patch
    patch programs/explorer/desktop.c < ../../winewayland.drv/patch/explorer-desktopc.patch
    
    
    cd "${srcdir}"/"${_winesrcdir}"

    rm configure
    autoconf

    msg2 "Applying esync patches"
      cd "${srcdir}"
      cp -r ../esync .
      cd "${srcdir}"/"${_winesrcdir}"
      for _f in "${srcdir}"/"${_esyncsrcdir}"/*.patch; do
        msg2 "Applying ${_f}"
        git apply -C1 --verbose < ${_f}
      done
    
    
    msg2 "Applying esync temp fix"
    patch -Np1 < '../../esync-no_kernel_obj_list.patch'  
    
    msg2 "Applying fsync"
    patch -Np1 < '../../fsync-mainline.patch'  
    #git apply -C1 --verbose < '../../fsync-mainline.patch'  
    
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
	
  
  
  #if [0]; then
  
  ../${_winesrcdir}/configure \
		--prefix='/usr' \
		--libdir='/usr/lib' \
		--without-x \
		--without-dbus \
		--without-gphoto \
		--without-gstreamer \
		--without-gssapi \
		--without-udev \
		--without-netapi \
		--without-hal \
    --without-gsm \
    --without-opencl \
    --with-opengl \
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
    --with-vulkan \
    --with-faudio \
		--enable-win64 \
    --without-netapi \
		--disable-tests
  #fi
	
	make -s -j 5

}

package() {
	
	cd "${srcdir}/${pkgname}"-64-build
	make -s	prefix="${pkgdir}/usr" \
			libdir="${pkgdir}/usr/lib" \
			dlldir="${pkgdir}/usr/lib/wine" install

}
