# Apollo Kernel 5.10 Porting Log

**Device:** Xiaomi Redmi K30S Ultra (apollo)
**SoC:** Qualcomm SM8250 / Snapdragon 865
**Kernel:** Linux 5.10.236 (AOSP android12-5.10-2025-05)
**Toolchain:** Proton Clang 13

---

## 2026-09-03: Initial Bring-up

### Environment Setup
- Installed build dependencies: bc 1.07.1, bison 3.8.2, flex 2.6.4, m4 1.4.18, libelf-dev 0.186
- Method: Downloaded .deb packages from Ubuntu 22.04 archive, extracted to ~/.local
- Created `aarch64-linux-gnu-gcc` wrapper script pointing to clang with `--target=aarch64-linux-gnu`
- Set BISON_PKGDATADIR to ~/.local/usr/share/bison

### Defconfig (apollo_defconfig)
- Based on AOSP 5.10 arch/arm64/configs/defconfig
- Key changes (m → y):
  - CONFIG_SCSI_UFS_QCOM=y
  - CONFIG_I2C_QCOM_GENI=y
  - CONFIG_SPI_QCOM_GENI=y
  - CONFIG_PHY_QCOM_QMP=y
  - CONFIG_INTERCONNECT_QCOM_SM8250=y
  - CONFIG_QCOM_SPMI_ADC5=y
  - CONFIG_QCOM_Q6V5_PAS=y
  - CONFIG_QCOM_Q6V5_ADSP=y
  - CONFIG_DRM_MSM=y
- CONFIG_DRM=y (changed from m)
- CONFIG_BACKLIGHT_CLASS_DEVICE=y
- CONFIG_CMDLINE="console=ttyMSM0,115200n8 earlycon=msm_geni_serial,0xa88000 rw"
- CONFIG_CMDLINE_EXTEND=y
- Disabled many non-QCOM platform drivers (pinctrl, I2C, SPI, regulator, GPIO, DRM) to reduce build time

### Device Tree (apollo-sm8250.dts)
- Based on sm8250-mtp.dts
- model = "Xiaomi Redmi K30S Ultra"
- compatible = "xiaomi,apollo", "qcom,sm8250"
- PMIC: pm8150 (a) + pm8150b (b) + pm8150l (c) — no pm8009
- Enabled: uart12 (debug), ufs_mem_hc, ufs_mem_phy, i2c1/13/15, spi4
- Enabled: qupv3_id_0/1/2, adsp, slpi, cdsp, gpu
- gpio-keys: volume_up on pm8150_gpios 6
- Reserved GPIOs: <28 4>, <40 4>
- Added to arch/arm64/boot/dts/qcom/Makefile

### Build Results
- `make apollo_defconfig`: ✅ Success
- `make dtbs`: ✅ Success — apollo-sm8250.dtb (57659 bytes)
- `make -j2 Image dtbs`: ✅ **SUCCESS** — Full kernel compiled with zero errors
  - Total objects: 4360
  - Image size: 32,326,144 bytes (30.8 MB uncompressed)
  - Image.gz size: ~11 MB
  - Build time: ~38 minutes (2 CPU cores, 4GB RAM)
  - vmlinux linked, System.map generated
  - DRM_MSM driver (including DPU with sm8250 support) compiled successfully

### Display Driver Research
- AOSP 5.10 DRM_MSM driver **already includes sm8250 DPU support** (DPU_HW_VER_600, `sm8250_cfg_init()` in dpu_hw_catalog.c)
- Missing: MDSS/DPU/DSI device tree nodes in sm8250.dtsi (AOSP 5.10 base predates mainline sm8250 display DT)
- Next step: Backport MDSS DT nodes from mainline Linux 5.14+ or postmarketOS sm8250-mainline
- DSI panel driver for the 1080x2400 144Hz IPS LCD also needed

### Build Artifacts (output/)
- `Image` — uncompressed kernel (30.8 MB)
- `Image.gz` — gzipped kernel (11 MB)
- `apollo-sm8250.dtb` — device tree blob (57 KB)
- `kernel.config` — actual .config used
- `BUILD_INFO.txt` — build metadata
1. **Display (WIP)**: AOSP 5.10 sm8250.dtsi has NO MDSS/display controller nodes. DRM_MSM driver is compiled but lacks DT binding. Need to backport MDSS DTS nodes from mainline/postmarketOS or 4.19.
2. **Touchscreen (WIP)**: Novatek NT36xxx SPI driver needs to be ported from 4.19 (drivers/input/touchscreen/nt36xxx/). SPI4 bus is enabled in DTS.
3. **Audio (WIP)**: WCD9380 codec + CS35L41 amplifiers not present in AOSP 5.10 sound/soc/. Need backport from mainline or 4.19.
4. **Charging (WIP)**: BQ25970 charge pump driver needs porting from 4.19.
5. **Vibrator (WIP)**: AW8697 I2C driver needs porting from 4.19.
6. **WiFi/BT (WIP)**: QCA6390 cnss2 driver is complex, needs significant porting work.
7. **No QCOM_Q6V5_SLPI/CDSP Kconfig in 5.10**: Only ADSP, MSS, PAS, WCSS exist. SLPI/CDSP remoteproc may use different driver names.

### Compilation Errors Encountered
1. **bison m4sugar not found**: `bison: /usr/share/bison/m4sugar/m4sugar.m4: cannot open`
   - Fix: Set `BISON_PKGDATADIR=$HOME/.local/usr/share/bison`

2. **aarch64-linux-gnu-gcc not found**: `scripts/Kconfig.include:39: compiler 'aarch64-linux-gnu-gcc' not found`
   - Fix: Created wrapper script `aarch64-linux-gnu-gcc` → `clang --target=aarch64-linux-gnu`

---

## Driver Porting Plan (Priority Order)

### Phase 1: Boot (Current)
- [x] defconfig
- [x] Device tree (basic)
- [x] UART console
- [x] UFS storage
- [ ] Kernel Image compiles
- [ ] Boot to initramfs/adb shell

### Phase 2: Input
- [ ] Touchscreen (NT36xxx SPI)
- [ ] Volume/power keys
- [ ] Vibrator (AW8697)

### Phase 3: Display
- [ ] MDSS DTS nodes (backport from mainline)
- [ ] DSI panel driver
- [ ] Backlight (pm8150l WLED)
- [ ] 144Hz refresh rate support

### Phase 4: Audio
- [ ] WCD9380 codec
- [ ] CS35L41 speaker amplifiers (x2)
- [ ] Audio routing

### Phase 5: Power
- [ ] BQ25970 charge pump
- [ ] pm8150b fuel gauge
- [ ] Battery stats

### Phase 6: Connectivity
- [ ] WiFi (QCA6390 cnss2)
- [ ] Bluetooth
- [ ] NFC (NQ)

### Phase 7: Extras
- [ ] Camera (very complex, likely WIP long-term)
- [ ] Fingerprint
- [ ] Sensors (accelerometer, gyro, proximity, light)
- [ ] USB

---

## References
- postmarketOS sm8250-mainline: https://gitlab.com/sm8250-mainline
- LineageOS 4.19 apollo kernel: reference/lineage-4.19/
- AOSP common kernel 5.10: android12-5.10-2025-05

---
## 2026-09-04: Full Driver Porting (Phase 2)

### P0 - Display (MDSS/DPU/DSI)
**Method**: Backport device tree nodes from postmarketOS sm8250-mainline (v6.2)
- Cloned postmarketOS repo to `/home/user/apollo-kernel/pmos-mainline`
- Extracted MDSS/DPU/DSI register maps from `qcom,sm8250-mdss.yaml` and `qcom,sm8250-dpu.yaml`
- Added to `sm8250.dtsi`:
  - `dispcc@af00000` (qcom,sm8250-dispcc) — display clock controller
  - `apps_smmu@15000000` (qcom,sm8250-smmu-500) — SMMU for MDSS
- Added to `apollo-sm8250.dts`:
  - `mdss@ae00000` (qcom,sm8250-mdss) — top-level MDSS
  - `dpu@ae01000` (qcom,sm8250-dpu) — DPU with mdp_opp_table (200/300/345/460 MHz)
  - `dsi0@ae94000` (qcom,mdss-dsi-ctrl) — DSI controller with dsi_opp_table
  - `dsi0_phy@ae94400` (qcom,dsi-phy-7nm) — DSI 7nm PHY
  - `panel@0` (novatek,nt36675 + simple-panel) — 1080x2400 144Hz IPS LCD, reset gpio12
  - `wled@d800` (qcom,pm8150l-wled) — 3-string backlight on pm8150l
  - `labibb` (qcom,labibb) — LCD bias (lab/ibb) regulator
- **Key finding**: 5.10 DRM_MSM already has sm8250 DPU support (DPU_HW_VER_600), dispcc-sm8250 clock driver, DSI 7nm PHY driver, SM8250_MMCX power domain. Only DT nodes were missing.
- **Status**: Compiles. Display output not verified on hardware (WIP — needs panel timing tuning).

### P1 - Touchscreen (Novatek NT36xxx SPI)
**Method**: Rewrote as minimal 5.10 mainline driver (original 4.19 driver too deeply tied to Qualcomm proprietary framework)
- Original 4.19 driver dependencies that don't exist in mainline 5.10:
  - `drm/drm_notifier_mi.h` — Qualcomm display notifier (created stub)
  - `../xiaomi/xiaomi_touch.h` — Xiaomi touch feature framework (game mode, etc.)
  - `mt_spi.h` — MediaTek SPI header (unused, removed)
  - Custom procfs entries using `file_operations` (5.10 requires `proc_ops`)
  - Pen/stylus support (nvt_pen_charge_state_*)
  - Xiaomi touchfeature workqueue
- **Decision**: Rewrote `nt36xxx.c` as clean minimal driver:
  - Standard SPI probe/remove with devm
  - Multi-touch input device (10 slots, ABS_MT_POSITION_X/Y)
  - Threaded IRQ handler with SPI read of touch data
  - GPIO reset control via gpiod
  - Regulator control (vdd/vcc)
  - Suspend/resume power management
  - Compatible strings: novatek,nt36xxx / nt36523 / nt36675
- Removed: nt36xxx_ext_proc.c, nt36xxx_fw_update.c, nt36xxx_mp_ctrlram.c (firmware update and manufacturing test features)
- **Status**: Compiles with 0 errors. Basic touch functionality expected. Firmware update and advanced features deferred.

### P2 - Audio (WCD9380 + CS35L41)
**Method**: Investigated mainline 5.10 support — **not available**
- 5.10 `sound/soc/codecs/`: No wcd938x.c, no cs35l41.c
- 5.10 `sound/soc/qcom/`: No sm8250 machine driver
- 4.19 audio stack is in `techpack/audio/` (Qualcomm ASoC):
  - `kona.c` machine driver (263KB) — deeply tied to Q6 DSP, ADSP, GLINK, RPMh
  - WCD9380 codec driver depends on Qualcomm-specific clock/regulator framework
  - CS35L41 driver depends on Qualcomm SoundWire slave framework
- **Attempted**: Direct copy of techpack/audio → failed due to missing Qualcomm headers (asoc.h, platform.h, etc.) and Kconfig dependencies
- **Status**: WIP. Audio not functional. Mainline 5.10 lacks WCD9380/CS35L41 drivers. Full port requires backporting Qualcomm ASoC framework from 4.19 or using 6.x mainline drivers (which have wcd938x support since ~5.18).
- **Recommendation**: Backport from Linux 6.x where WCD9380 and CS35L41 are mainlined.

### P3 - Charging (BQ25970 + fuel gauge)
**Method**: Rewrote BQ2597x as minimal 5.10 driver
- Original 4.19 driver used custom `POWER_SUPPLY_PROP_TI_*` properties (TI-specific extensions not in mainline)
- Caused duplicate case values when mapped to standard properties
- **Decision**: Rewrote `bq2597x_charger.c` with standard power supply properties only:
  - STATUS, PRESENT, ONLINE, VOLTAGE_NOW, CURRENT_NOW, TEMP
  - CONSTANT_CHARGE_CURRENT, CONSTANT_CHARGE_VOLTAGE
  - regmap-based I2C register access
  - Compatible: ti,bq25970 / ti,bq2597x
- pm8150b fuel gauge: 5.10 has `CONFIG_BATTERY_QCOM_SMB` and related drivers. Fuel gauge via SPMI PMIC should work with mainline driver. DT node for pm8150b fuel gauge added.
- **Status**: BQ2597x compiles with 0 errors. Basic charging status reporting. Fast charge negotiation (QC3/PD) deferred.

### P4 - Vibrator (AW8697)
**Method**: Rewrote as minimal 5.10 driver
- Original 4.19 driver issues:
  - `struct timeval` removed in 5.10 (replaced with timespec64)
  - `do_gettimeofday()` removed (replaced with ktime_get_ts64)
  - `pm_qos_add_request()`/`pm_qos_remove_request()` global API removed in 5.10
  - PM_QOS_CPU_DMA_LATENCY removed
  - Extra files: aw8697_dagu.c (DAGU waveform), ringbuffer.c (effect storage)
- **Decision**: Rewrote `aw8697.c` as minimal force-feedback driver:
  - I2C + regmap register access
  - FF_RUMBLE force feedback via input device
  - Single workqueue for vibration play
  - Compatible: awinic,aw8697
- Removed: aw8697_dagu.c, ringbuffer.c/h (advanced waveform effects)
- **Status**: Compiles with 0 errors. Basic rumble vibration expected. Custom waveform effects deferred.

### P5 - WiFi/BT (QCA6390)
**Method**: Investigated mainline ath11k support
- 5.10 `drivers/net/wireless/ath/ath11k/`: Has AHB bus support but `of_match_table` only contains:
  - `qcom,ipq8074-wifi`
  - `qcom,ipq6018-wifi`
  - **No QCA6390 / sm8250 compatible string**
- QCA6390 on mobile platforms uses CNSS2 (Qualcomm Connectivity SubSystem) driver, not standard ath11k AHB
- 4.19 cnss2 driver is in `drivers/net/wireless/cnss2/` — deeply tied to Qualcomm remoteproc, QMI, PCIe
- Bluetooth: 5.10 has `BT_HCIUART_QCA` and `BT_QCA` drivers. DT node for QCA6390 BT added.
- **Attempted**: Added `qcom,qca6390-wifi` to ath11k AHB of_match → compiled but driver won't probe correctly without CNSS2 firmware loading mechanism
- **Status**: WiFi WIP. ath11k compiled but QCA6390 mobile support requires CNSS2 backport. Bluetooth driver compiled (hci_qca), needs DT verification.

### P6 - Sensors (BMI160 + light/proximity)
**Method**: Use mainline 5.10 drivers
- BMI160: 5.10 has `drivers/iio/imu/bmi160/` with both I2C and SPI support
  - Config: CONFIG_BMI160=y, CONFIG_BMI160_I2C=y
  - DT node added on i2c13 @0x69
  - **Status**: Compiles. Accelerometer + gyroscope expected to work.
- Light/proximity sensor: apollo uses Rohm BH1745 or similar (confirmed from 4.19 DTS)
  - 5.10 has `drivers/iio/light/` with various Rohm/AMS drivers
  - **Status**: DT node added, driver compiled. Exact sensor model needs hardware verification.

### P7 - Camera (CAMSS/ISP)
**Method**: Investigated mainline 5.10 CAMSS
- 5.10 `drivers/media/platform/qcom/camss/`: Has CAMSS driver but only supports:
  - msm8916, msm8996, msm8998, sdm845, sm8250 (partial)
  - sm8250 referenced in Kconfig but DTS bindings may be incomplete
- apollo camera sensors: Sony IMX682 (main), Samsung S5K3L6 (ultrawide), etc.
  - Sensor drivers (imx682, s5k3l6) not in mainline 5.10
- **Status**: WIP. CAMSS driver compiled (CONFIG_VIDEO_QCOM_CAMSS=y). Sensor drivers need backport from 4.19 or mainline 6.x. CSI PHY and ISP configuration for sm8250 may need additional DT nodes.

### P8 - Other (NFC, USB, GPS, suspend)
- **NFC**: NQ310/NQ440. 5.10 has `drivers/nfc/nxp-nci/` (CONFIG_NFC_NXP_NCI=y). DT node added on i2c1 @0x28. **Status**: Compiles.
- **USB**: dwc3. 5.10 has `drivers/usb/dwc3/` and `drivers/usb/phy/phy-qcom-qmp.c`. DT node added (usb0@a600000 + qmp_usb3_phy@a700000). **Status**: Compiles.
- **GPS**: Qualcomm IZat. Uses QMI via ADSP. No standalone driver needed. **Status**: Depends on ADSP remoteproc (enabled).
- **Suspend/resume**: Basic kernel suspend support enabled. Deep sleep (rpmh, interconnect) needs hardware verification. **Status**: Compiles, untested.

### Kconfig Integration Fixes
- Parent Kconfig files in 5.10 end with `endif` (not `endmenu` as in 4.19)
- Fixed by inserting `source` lines before `endif`:
  - `drivers/input/touchscreen/Kconfig` → nt36xxx/Kconfig
  - `drivers/input/misc/Kconfig` → aw8697/Kconfig
  - `drivers/power/supply/Kconfig` → ti/Kconfig
- Makefile additions:
  - `drivers/power/supply/Makefile` → `obj-$(CONFIG_CHARGER_BQ2597X) += ti/`

### Build Environment Notes
- Full rebuild after config change: ~40 minutes (2 cores, 4GB RAM)
- Build was killed once by OOM at ~1449 objects; restarted and continued from cached objects
- Individual driver compile testing used to catch errors before full build
