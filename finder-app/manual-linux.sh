#!/bin/bash
# Script outline to install and build kernel.
# Author: Siddhant Jajoo.

set -e
set -u

OUTDIR=/tmp/aeld
KERNEL_REPO=git://git.kernel.org/pub/scm/linux/kernel/git/stable/linux-stable.git
KERNEL_VERSION=v5.1.10
BUSYBOX_VERSION=1_33_1
FINDER_APP_DIR=$(realpath $(dirname $0))
ARCH=arm64
CROSS_COMPILE=aarch64-none-linux-gnu-

if [ $# -lt 1 ]
then
	echo "Using default directory ${OUTDIR} for output"
else
	OUTDIR=$1
	echo "Using passed directory ${OUTDIR} for output"
fi

mkdir -p ${OUTDIR}

cd "$OUTDIR"
if [ ! -d "${OUTDIR}/linux-stable" ]; then
    #Clone only if the repository does not exist.
	echo "CLONING GIT LINUX STABLE VERSION ${KERNEL_VERSION} IN ${OUTDIR}"
	git clone ${KERNEL_REPO} --depth 1 --single-branch --branch ${KERNEL_VERSION}
fi
if [ ! -e ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ]; then
    cd linux-stable
    echo "Checking out version ${KERNEL_VERSION}"
    git checkout ${KERNEL_VERSION}

    # TODO: Add your kernel build steps here
    git reset --hard ${KERNEL_VERSION}
    make clean
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- mrproper
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- defconfig
    cp $FINDER_APP_DIR/fixyylloc.patch .
    git apply fixyylloc.patch
    make -j4 ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- all
    #make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- modules
    make ARCH=arm64 CROSS_COMPILE=aarch64-none-linux-gnu- dtbs
    echo kernel image made
fi
#: '
echo "Adding the Image in outdir"
cp ${OUTDIR}/linux-stable/arch/${ARCH}/boot/Image ${OUTDIR}
echo "Creating the staging directory for the root filesystem"
cd "$OUTDIR"

if [ -d "${OUTDIR}/rootfs" ]
then
	echo "Deleting rootfs directory at ${OUTDIR}/rootfs and starting over"
    sudo rm  -rf ${OUTDIR}/rootfs
fi

# TODO: Create necessary base directories
mkdir rootfs
cd rootfs
mkdir -p bin dev etc lib home lib64 sbin sys usr tmp var proc
mkdir -p usr/bin usr/lib usr/sbin var/log
cd "$OUTDIR"
if [ ! -d "${OUTDIR}/busybox" ]
then
git clone git://busybox.net/busybox.git
    cd busybox
    git checkout ${BUSYBOX_VERSION}
    # TODO:  Configure busybox

else
    cd busybox
fi

# TODO: Make and install busybox
make distclean
make defconfig
make ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE}
make CONFIG_PREFIX=${OUTDIR}/rootfs ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} install
#'
cd ${OUTDIR}/rootfs
echo "Library dependencies"
TOOLCHAIN_LIBC="$(dirname $(which ${CROSS_COMPILE}gcc))/../aarch64-none-linux-gnu/libc/"
files=$(aarch64-none-linux-gnu-readelf -a bin/busybox |grep "program interpreter"|sed -re "s/.*: ([^]]*).*/\1/")

files=(${files} $(aarch64-none-linux-gnu-readelf -a bin/busybox |grep 'Shared library'|sed -re 's/.*\[([^]]*).*/lib64\/\1/'))
if [ -z "${files}" ];then
    exit 1
fi
echo ${files[@]}
for file in "${files[@]}"; do
    echo "$(pwd)/${file}"
    cp "${TOOLCHAIN_LIBC}${file}" "$(pwd)/${file}"
done
# TODO: Add library dependencies to rootfs

# TODO: Make device nodes
sudo mknod -m 666 dev/null c 1 3
sudo mknod -m 666 dev/console c 5 1
# TODO: Clean and build the writer utility

cd $FINDER_APP_DIR
make clean
make CROSS_COMPILE=${CROSS_COMPILE}
# TODO: Copy the finder related scripts and executables to the /home directory
# on the target rootfs
scripts=(finder.sh writer conf finder-test.sh autorun-qemu.sh)
cp -rL ${scripts[@]} "${OUTDIR}/rootfs/home/"
# TODO: Chown the root directory
#sudo chown -R root:root "${OUTDIR}/rootfs"
# TODO: Create initramfs.cpio.gz
cd "$OUTDIR/rootfs"
find . | cpio -H newc -ov --owner root:root > $OUTDIR/initramfs.cpio
cd $OUTDIR
gzip -f initramfs.cpio
