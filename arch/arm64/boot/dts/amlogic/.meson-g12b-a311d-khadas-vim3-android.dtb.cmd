cmd_arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dtb := gcc -E -Wp,-MMD,arch/arm64/boot/dts/amlogic/.meson-g12b-a311d-khadas-vim3-android.dtb.d.pre.tmp -nostdinc -I./scripts/dtc/include-prefixes -undef -D__DTS__ -x assembler-with-cpp -o arch/arm64/boot/dts/amlogic/.meson-g12b-a311d-khadas-vim3-android.dtb.dts.tmp arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dts ; ./scripts/dtc/dtc -O dtb -o arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dtb -b 0 -iarch/arm64/boot/dts/amlogic/ -i./scripts/dtc/include-prefixes -Wno-interrupt_provider -Wno-unit_address_vs_reg -Wno-unit_address_format -Wno-avoid_unnecessary_addr_size -Wno-alias_paths -Wno-graph_child_address -Wno-simple_bus_reg -Wno-unique_unit_address -Wno-pci_device_reg  -d arch/arm64/boot/dts/amlogic/.meson-g12b-a311d-khadas-vim3-android.dtb.d.dtc.tmp arch/arm64/boot/dts/amlogic/.meson-g12b-a311d-khadas-vim3-android.dtb.dts.tmp ; cat arch/arm64/boot/dts/amlogic/.meson-g12b-a311d-khadas-vim3-android.dtb.d.pre.tmp arch/arm64/boot/dts/amlogic/.meson-g12b-a311d-khadas-vim3-android.dtb.d.dtc.tmp > arch/arm64/boot/dts/amlogic/.meson-g12b-a311d-khadas-vim3-android.dtb.d

source_arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dtb := arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dts

deps_arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dtb := \
  scripts/dtc/include-prefixes/dt-bindings/phy/phy.h \
  scripts/dtc/include-prefixes/dt-bindings/gpio/gpio.h \
  scripts/dtc/include-prefixes/dt-bindings/gpio/meson-g12a-gpio.h \
  scripts/dtc/include-prefixes/dt-bindings/input/input.h \
  scripts/dtc/include-prefixes/dt-bindings/input/linux-event-codes.h \
  scripts/dtc/include-prefixes/dt-bindings/interrupt-controller/irq.h \

arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dtb: $(deps_arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dtb)

$(deps_arch/arm64/boot/dts/amlogic/meson-g12b-a311d-khadas-vim3-android.dtb):
