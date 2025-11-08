
# all:  bootloader kernel userland image

# bootloader:
# 	cd Bootloader; make all

# kernel:
# 	cd Kernel; make all

# userland:
# 	cd Userland; make all

# image: kernel bootloader userland
# 	cd Image; make all

# clean:
# 	cd Bootloader; make clean
# 	cd Image; make clean
# 	cd Kernel; make clean
# 	cd Userland; make clean

# .PHONY: bootloader image collections kernel userland all clean

all: bootloader kernel userland image

buddy: bootloader kernel_buddy userland image_buddy

bootloader:
	cd Bootloader; $(MAKE) all

kernel:
	cd Kernel; $(MAKE) all

kernel_buddy:
	cd Kernel; $(MAKE) all MM=USE_BUDDY

userland:
	cd Userland; $(MAKE) all

image: kernel bootloader userland
	cd Image; $(MAKE) all

image_buddy: kernel_buddy bootloader userland
	cd Image; $(MAKE) all MM=USE_BUDDY

format:
	@echo "Aplicando clang-format…"
	@find Kernel Userland SharedLibraries \
		\( -name '*.c' -o -name '*.h' \) -exec clang-format -i {} +

clean:
	cd Bootloader; make clean
	cd Image; make clean
	cd Kernel; make clean
	cd Userland; make clean

.PHONY: bootloader image image_buddy kernel kernel_buddy userland all buddy clean
