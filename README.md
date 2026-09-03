# Apollo Kernel — Linux 5.10 for Redmi K30S Ultra

Mainline Linux 5.10 kernel port for the **Xiaomi Redmi K30S Ultra** (codename `apollo`, also known as Mi 10T / apollon).

| Spec | Details |
|------|---------|
| SoC | Qualcomm SM8250 / Snapdragon 865 |
| CPU | 1x 2.84 GHz Kryo 585 Prime + 3x 2.42 GHz + 4x 1.80 GHz |
| GPU | Adreno 650 |
| Display | 6.67" IPS LCD, 1080x2400, 144Hz |
| RAM | 6/8 GB LPDDR5 |
| Storage | 128/256 GB UFS 3.0 |
| Battery | 5000 mAh |
| PMIC | pm8150a + pm8150b + pm8150l |
| Touch | Novatek NT36xxx (SPI) |
| Audio | WCD9380 codec + CS35L41 x2 |
| Charging | BQ25970 charge pump (33W) |
| Vibrator | AW8697 (I2C) |
| WiFi/BT | QCA6390 |

---

## Hardware Support Status

| Subsystem | Status | Driver | Notes |
|-----------|--------|--------|-------|
| CPU | Compiles | cpufreq | All 8 cores, Kryo 585 |
| GPU | Compiles | msm/adreno | Adreno 650, DRM_MSM |
| UART | Compiles | qcom_geni | uart12 debug console (ttyMSM0) |
| UFS | Compiles | ufs_qcom | UFS 3.0 storage |
| I2C | Compiles | i2c_qcom_geni | GENI I2C buses 1/13/15 |
| SPI | Compiles | spi_qcom_geni | GENI SPI bus 4 (touch) |
| Regulators | Compiles | qcom_rpmh | RPMh regulators, labibb |
| Clock | Compiles | gcc/dispcc | GCC, RPMh, dispcc-sm8250 |
| Interconnect | Compiles | icc_qcom | NoC bandwidth scaling |
| Thermal | Compiles | qcom_tsens | TSENS sensors |
| Display | Compiles (WIP) | msm/dpu + dsi | MDSS/DPU/DSI DT backported from pmOS, panel timing untested |
| Backlight | Compiles | qcom_wled | pm8150l WLED 3-string |
| Touchscreen | Compiles (WIP) | nt36xxx (rewritten) | Minimal SPI driver, firmware update deferred |
| Audio | WIP | — | WCD9380/CS35L41 not in 5.10, needs backport from 6.x |
| Charging | Compiles (WIP) | bq2597x (rewritten) | Minimal charge pump, fast charge negotiation deferred |
| Fuel Gauge | Compiles | qcom_smb | pm8150b SPMI fuel gauge |
| Vibrator | Compiles (WIP) | aw8697 (rewritten) | Minimal FF_RUMBLE, custom waveforms deferred |
| WiFi | WIP | ath11k | QCA6390 AHB match missing, needs CNSS2 backport |
| Bluetooth | Compiles | hci_qca | BT_QCA driver, DT added |
| NFC | Compiles | nxp_nci | NQ310/NQ440 on i2c1 |
| Sensors | Compiles | bmi160 | BMI160 accel/gyro on i2c13, light/prox added |
| USB | Compiles | dwc3 + qmp | USB 3.0 with QMP PHY |
| Camera | WIP | camss | CAMSS compiled, sensor drivers need backport |
| Fingerprint | Not started | — | Goodix/FPC, needs investigation |
| Suspend/Resume | Compiles | — | Basic support, deep sleep untested |
| GPS | Compiles | — | Via ADSP QMI, depends on remoteproc |

> **Status legend**: Compiles = driver builds into kernel, functionality not verified on hardware. WIP = driver partially ported or needs more work. Not started = no driver yet.

---

## Building

### Prerequisites
```bash
# Ubuntu/Debian
sudo apt install bc bison flex libelf-dev libssl-dev \
    build-essential gcc-aarch64-linux-gnu device-tree-compiler
```

### Toolchain
Proton Clang 13 (at `toolchains/proton-clang/`):
```bash
export PATH=/home/user/apollo-kernel/toolchains/proton-clang/bin:$PATH
```

### Quick Build
```bash
cd kernel/apollo-5.10
./build.sh defconfig   # Generate .config
./build.sh all         # Build Image + dtbs
```

### Manual Build
```bash
export ARCH=arm64
export CROSS_COMPILE=aarch64-linux-gnu-
make CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm \
     OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip \
     apollo_defconfig
make -j$(nproc) CC=clang LD=ld.lld AR=llvm-ar NM=llvm-nm \
     OBJCOPY=llvm-objcopy OBJDUMP=llvm-objdump STRIP=llvm-strip \
     Image dtbs
```

### Build Script Options
```
./build.sh defconfig   Generate .config
./build.sh menuconfig  Interactive config
./build.sh dtbs        Build DTBs only
./build.sh Image       Build kernel Image only
./build.sh all         Build Image + dtbs (default)
./build.sh clean       Clean build artifacts
./build.sh distclean   Full clean
./build.sh zip         Build + create AnyKernel3 flashable zip
```

### Output
Artifacts copied to `/home/user/apollo-kernel/output/`:
- `Image` — uncompressed kernel
- `Image.gz` — gzipped kernel
- `apollo-sm8250.dtb` — device tree blob

---

## Flashing

### AnyKernel3 (Recommended)
```bash
./build.sh zip
# Flash resulting .zip in TWRP / custom recovery
```

The AnyKernel3 zip:
- Replaces kernel (Image.gz) and dtb in boot partition
- Preserves original ramdisk
- Device check: `apollo`
- Block: `/dev/block/bootdevice/by-name/boot`

### Fastboot (manual)
```bash
# Extract boot.img, replace kernel+dtb, repack
fastboot flash boot boot.img
fastboot reboot
```

Console: UART12 @ 115200n8 (ttyMSM0), earlycon at 0xa88000

---

## Project Structure

```
apollo-kernel/
├── kernel/apollo-5.10/           # Kernel source
│   ├── arch/arm64/configs/apollo_defconfig
│   ├── arch/arm64/boot/dts/qcom/apollo-sm8250.dts
│   ├── arch/arm64/boot/dts/qcom/sm8250.dtsi (modified: +dispcc, +apps_smmu)
│   ├── drivers/input/touchscreen/nt36xxx/   # Ported touch driver
│   ├── drivers/input/misc/aw8697/           # Ported vibrator driver
│   ├── drivers/power/supply/ti/             # Ported charger driver
│   ├── include/drm/drm_notifier_mi.h        # Stub for Qualcomm DRM notifier
│   ├── build.sh
│   ├── PORTING_LOG.md
│   ├── README.md
│   └── .github/workflows/build.yml
├── toolchains/proton-clang/       # Clang 13 toolchain
├── reference/lineage-4.19/        # 4.19 reference kernel
├── pmos-mainline/                 # postmarketOS sm8250 reference
├── anykernel/                      # AnyKernel3 template
└── output/                         # Build artifacts
```

---

## CI

GitHub Actions builds on every push: ubuntu-latest + clang, Image + dtbs, artifacts uploaded (30-day retention). See `.github/workflows/build.yml`.

---

## Known Issues & TODO

1. **Audio**: WCD9380/CS35L41 drivers not in 5.10. Need backport from Linux 6.x (mainlined ~5.18+).
2. **WiFi**: QCA6390 requires CNSS2 driver. ath11k in 5.10 only supports IPQ8074/IPQ6018 AHB.
3. **Display**: Panel timing may need adjustment for 144Hz. DSI PHY calibration values from 4.19 should be verified.
4. **Camera**: Sensor drivers (IMX682, S5K3L6) need backport. CAMSS sm8250 support is partial.
5. **Touch**: Firmware update and Xiaomi touchfeature (game mode) not ported.
6. **Charging**: QC3/PD fast charge negotiation not implemented. Basic charge status only.
7. **Vibrator**: Custom waveform effects (DAGU) not ported. Basic rumble only.

---

## References

- postmarketOS sm8250-mainline: https://gitlab.com/sm8250-mainline
- LineageOS 4.19 apollo kernel (reference)
- AOSP Common Kernel android12-5.10
- Proton Clang: https://github.com/kdrag0n/proton-clang
- AnyKernel3: https://github.com/osm0sis/AnyKernel3

## License

GPL-2.0
