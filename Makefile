
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

pvs:
	@echo "Ejecutando análisis PVS-Studio..."
	@$(MAKE) clean
	pvs-studio-analyzer trace -- $(MAKE) -j
	pvs-studio-analyzer analyze -o PVS.log -j 4 \
		-e Bootloader \
		-e Image \
		-e Kernel/asm \
		-e Userland/SampleCodeModule/asm \
		-e Userland/SampleCodeModule/obj \
		-e Userland/SampleCodeModule/tests
	@echo "Análisis completo. Log generado: PVS.log"

pvs-html:
	@echo "Generando reporte HTML..."
	plog-converter -a GA:1,2,3 -t fullhtml -o PVS-report PVS.log
	@echo "Reporte HTML generado en: PVS-report/index.html"

pvs-clean:
	@echo "Limpiando archivos PVS-Studio..."
	rm -f PVS.log compile_commands.json strace_out
	rm -rf PVS-report
	@echo "Limpieza PVS completa."

.PHONY: bootloader image image_buddy kernel kernel_buddy userland all buddy clean format pvs pvs-html pvs-clean