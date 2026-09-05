cmd_usr/initramfs_data.o := clang -Wp,-MMD,usr/.initramfs_data.o.d -nostdinc -isystem /home/user/apollo-kernel/toolchains/proton-clang/lib/clang/13.0.0/include -I./arch/arm64/include -I./arch/arm64/include/generated  -I./include -I./arch/arm64/include/uapi -I./arch/arm64/include/generated/uapi -I./include/uapi -I./include/generated/uapi -include ./include/linux/kconfig.h -D__KERNEL__ -mlittle-endian -DKASAN_SHADOW_SCALE_SHIFT= -Qunused-arguments -fmacro-prefix-map=./= -D__ASSEMBLY__ -fno-PIE --target=aarch64-linux-gnu --prefix=/home/user/apollo-kernel/toolchains/proton-clang/bin/aarch64-linux-gnu- --gcc-toolchain=/home/user/apollo-kernel/toolchains/proton-clang -fno-integrated-as -Werror=unknown-warning-option -fno-asynchronous-unwind-tables -fno-unwind-tables -DKASAN_SHADOW_SCALE_SHIFT= -Wa,-gdwarf-2    -c -o usr/initramfs_data.o usr/initramfs_data.S

source_usr/initramfs_data.o := usr/initramfs_data.S

deps_usr/initramfs_data.o := \
    $(wildcard include/config/64bit.h) \
  include/linux/kconfig.h \
    $(wildcard include/config/cc/version/text.h) \
    $(wildcard include/config/cpu/big/endian.h) \
    $(wildcard include/config/booger.h) \
    $(wildcard include/config/foo.h) \

usr/initramfs_data.o: $(deps_usr/initramfs_data.o)

$(deps_usr/initramfs_data.o):
