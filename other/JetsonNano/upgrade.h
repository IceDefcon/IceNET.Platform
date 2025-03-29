/etc/ld.so.conf.d/nvidia-tegra.conf
/etc/systemd/nv-oem-config-post.sh
/etc/systemd/nv-oem-config.sh

Starting kernel ...

[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 4.9.337-tegra (buildbrain@mobile-u64-5499-d7000) (gcc version 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701] (Linaro GCC 7.3-2018.05) ) #1 SMP PREEMPT Mon Nov 4 23:41:41 PST 2024
[    0.000000] Boot CPU: AArch64 Processor [411fd071]
[    0.000000] OF: fdt:memory scan node memory@80000000, reg size 32,
[    0.000000] OF: fdt: - 80000000 ,  3f200000
[    0.000000] OF: fdt: - c0000000 ,  3ee00000
[    0.000000] Found tegra_fbmem: 00800000@92cb4000
[    0.000000] earlycon: uart8250 at MMIO32 0x0000000070006000 (options '')
[    0.000000] bootconsole [uart8250] enabled

Starting kernel ...

[    0.000000] Booting Linux on physical CPU 0x0
[    0.000000] Linux version 4.9.253-tegra (buildbrain@mobile-u64-5494-d8000) (gcc version 7.3.1 20180425 [linaro-7.3-2018.05 revision d29120a424ecfbc167ef90065c0eeb7f91977701] (Linaro GCC 7.3-2018.05) ) #1 SMP PREEMPT Wed Apr 20 14:25:12 PDT 2022
[    0.000000] Boot CPU: AArch64 Processor [411fd071]
[    0.000000] OF: fdt:memory scan node memory@80000000, reg size 32,
[    0.000000] OF: fdt: - 80000000 ,  7ee00000
[    0.000000] OF: fdt: - 100000000 ,  7f200000
[    0.000000] Found tegra_fbmem: 00800000@92cb4000
[    0.000000] earlycon: uart8250 at MMIO32 0x0000000070006000 (options '')
[    0.000000] bootconsole [uart8250] enabled
[    1.167220] tegradc tegradc.1: dpd enable lookup fail:-19
[    1.328510] imx219 7-0010: imx219_board_setup: error during i2c read probe (-121)
[    1.328576] imx219 7-0010: board setup failed
[    1.352412] imx219 8-0010: imx219_board_setup: error during i2c read probe (-121)
[    1.352469] imx219 8-0010: board setup failed
[    1.690357] cgroup: cgroup2: unknown option "nsdelegate"
[    2.167675] tegra_cec 70015000.tegra_cec: Sent <Text View On> res: -113.
[    2.174901] tegra_cec 70015000.tegra_cec: tegra_cec_init Done.
[    2.858720] using random self ethernet address
[    2.863262] using random host ethernet address
[    3.319399] random: crng init done
[    3.322830] random: 7 urandom warning(s) missed due to ratelimiting
[    3.436388] using random self ethernet address
[    3.458875] using random host ethernet address
[    3.830969] nvgpu: 57000000.gpu                     gk20a_pmu_isr:724  [ERR]  pmu exterr intr not implemented. Clearing interrupt.
[    3.842831] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:560  [ERR]  <<< FALCON id-0 DEBUG INFORMATION - START >>>
[    3.854073] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:504  [ERR]  FALCON IMEM BLK MAPPING (PA->VA) (96 TOTAL):
[    3.865260] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x00: 0x01000000 0x01000100 0x01000200 0x01000300
[    3.876912] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x04: 0x01000400 0x01000500 0x01000600 0x01000700
[    3.888588] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x08: 0x01000800 0x01000900 0x01000a00 0x01000b00
[    3.900226] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x0c: 0x01000c00 0x01000d00 0x01000e00 0x01000f00
[    3.911873] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x10: 0x01001000 0x01001100 0x01001200 0x01001300
[    3.923516] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x14: 0x01001400 0x01001500 0x01001600 0x01002d00
[    3.935162] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x18: 0x01002e00 0x01002f00 0x01003000 0x01003100
[    3.946807] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x1c: 0x01001c00 0x01001d00 0x01001e00 0x01001f00
[    3.958470] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x20: 0x01002000 0x01002100 0x01002200 0x01004b00
[    3.970125] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x24: 0x01004c00 0x01004d00 0x01004e00 0x01004f00
[    3.981776] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x28: 0x01005000 0x01005100 0x01002300 0x01002400
[    3.993405] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x2c: 0x01002500 0x01003200 0x01003300 0x01003400
[    4.005050] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x30: 0x01003500 0x01003600 0x01003700 0x01003800
[    4.016685] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x34: 0x01003900 0x01003a00 0x01003b00 0x01003c00
[    4.028328] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x38: 0x01003d00 0x01003e00 0x01003f00 0x01004000
[    4.039959] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x3c: 0x01004100 0x01004200 0x01004300 0x01004400
[    4.051601] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x40: 0x01004500 0x01004600 0x01004700 0x01004800
[    4.063231] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x44: 0x01004900 0x01004a00 0x01005200 0x01005300
[    4.074871] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x48: 0x01005400 0x01005500 0x01005600 0x01005700
[    4.086504] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x4c: 0x01005800 0x01005900 0x01005a00 0x01005b00
[    4.098148] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x50: 0x01005c00 0x01005d00 0x01005e00 0x01005f00
[    4.109778] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x54: 0x01006000 0x01006100 0x01006200 0x01006300
[    4.121420] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:518  [ERR]   0x58: 0x01006400 0x01006500 0x01006600 0x01002600
[    4.133050] nvgpu: 57000000.gpu           gk20a_falcon_dump_imblk:520  [ERR]   0x5c: 0x01002700 0x01002800 0x01002900 0x01002c00
[    4.144706] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:541  [ERR]  PC TRACE (TOTAL 7 ENTRIES. entry 0 is the most recent branch):
[    4.157392] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:549  [ERR]  FALCON_TRACEPC(0)  :  0x0000054a
[    4.157397] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:549  [ERR]  FALCON_TRACEPC(1)  :  0x00000545
[    4.157401] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:549  [ERR]  FALCON_TRACEPC(2)  :  0x00000154
[    4.157406] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:549  [ERR]  FALCON_TRACEPC(3)  :  0x00001041
[    4.157410] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:549  [ERR]  FALCON_TRACEPC(4)  :  0x000015e2
[    4.157414] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:549  [ERR]  FALCON_TRACEPC(5)  :  0x0000160e
[    4.157418] nvgpu: 57000000.gpu        gk20a_falcon_dump_pc_trace:549  [ERR]  FALCON_TRACEPC(6)  :  0x00000a59
[    4.157420] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:567  [ERR]  FALCON ICD REGISTERS DUMP
[    4.157425] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:575  [ERR]  FALCON_REG_PC : 0x1041
[    4.157429] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:582  [ERR]  FALCON_REG_SP : 0x43c4
[    4.157433] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:575  [ERR]  FALCON_REG_PC : 0x1051
[    4.157436] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:582  [ERR]  FALCON_REG_SP : 0x43c4
[    4.157440] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:575  [ERR]  FALCON_REG_PC : 0x1041
[    4.157443] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:582  [ERR]  FALCON_REG_SP : 0x43c4
[    4.157447] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:575  [ERR]  FALCON_REG_PC : 0x1041
[    4.157451] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:582  [ERR]  FALCON_REG_SP : 0x43c4
[    4.157454] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:589  [ERR]  FALCON_REG_IMB : 0xff5071
[    4.157458] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:595  [ERR]  FALCON_REG_DMB : 0xff50d8
[    4.157462] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:601  [ERR]  FALCON_REG_CSW : 0x300001
[    4.157465] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:607  [ERR]  FALCON_REG_CTX : 0x0
[    4.157469] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:613  [ERR]  FALCON_REG_EXCI : 0x693
[    4.157473] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:622  [ERR]  FALCON_REG_RSTAT[0] : 0xa5a5a5a5
[    4.157477] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:622  [ERR]  FALCON_REG_RSTAT[1] : 0xa5a5a5a5
[    4.157481] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:622  [ERR]  FALCON_REG_RSTAT[2] : 0xa5a5a5a5
[    4.157485] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:622  [ERR]  FALCON_REG_RSTAT[3] : 0xa5a5a5a5
[    4.157489] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:622  [ERR]  FALCON_REG_RSTAT[4] : 0xa5a5a5a5
[    4.157492] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:622  [ERR]  FALCON_REG_RSTAT[5] : 0x4b4
[    4.157494] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:625  [ERR]   FALCON REGISTERS DUMP
[    4.157498] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:627  [ERR]  falcon_falcon_os_r : 0
[    4.157501] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:629  [ERR]  falcon_falcon_cpuctl_r : 0x60
[    4.157505] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:631  [ERR]  falcon_falcon_idlestate_r : 0x0
[    4.157508] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:633  [ERR]  falcon_falcon_mailbox0_r : 0x0
[    4.157512] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:635  [ERR]  falcon_falcon_mailbox1_r : 0x0
[    4.157515] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:637  [ERR]  falcon_falcon_irqstat_r : 0x60
[    4.157519] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:639  [ERR]  falcon_falcon_irqmode_r : 0xfc24
[    4.157522] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:641  [ERR]  falcon_falcon_irqmask_r : 0x7879
[    4.157526] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:643  [ERR]  falcon_falcon_irqdest_r : 0x90372
[    4.157529] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:645  [ERR]  falcon_falcon_debug1_r : 0x10000
[    4.157532] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:647  [ERR]  falcon_falcon_debuginfo_r : 0x0
[    4.157536] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:649  [ERR]  falcon_falcon_bootvec_r : 0x3200
[    4.157539] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:651  [ERR]  falcon_falcon_hwcfg_r : 0x400cc060
[    4.157543] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:653  [ERR]  falcon_falcon_engctl_r : 0x0
[    4.157546] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:655  [ERR]  falcon_falcon_curctx_r : 0x707fffe7
[    4.157550] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:657  [ERR]  falcon_falcon_nxtctx_r : 0x707fffe7
[    4.157553] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:659  [ERR]  falcon_falcon_exterrstat_r : 0x85002efb
[    4.157557] nvgpu: 57000000.gpu           gk20a_falcon_dump_stats:661  [ERR]  falcon_falcon_exterraddr_r : 0x18405810
[    4.157562] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(0) : 0x0
[    4.157566] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(1) : 0x0
[    4.157569] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(2) : 0x0
[    4.157573] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(3) : 0x0
[    4.157576] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(4) : 0x0
[    4.157580] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(5) : 0x0
[    4.157583] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(6) : 0x0
[    4.157586] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(7) : 0x0
[    4.157590] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(8) : 0x0
[    4.157593] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(9) : 0x0
[    4.157597] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(10) : 0x0
[    4.157600] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:630  [ERR]  pwr_pmu_mailbox_r(11) : 0x0
[    4.157604] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:635  [ERR]  pwr_pmu_debug_r(0) : 0x0
[    4.157607] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:635  [ERR]  pwr_pmu_debug_r(1) : 0x0
[    4.157611] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:635  [ERR]  pwr_pmu_debug_r(2) : 0x2b
[    4.157614] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:635  [ERR]  pwr_pmu_debug_r(3) : 0x0
[    4.157618] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:639  [ERR]  pwr_pmu_bar0_error_status_r : 0x0
[    4.157621] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:652  [ERR]  pwr_pmu_bar0_fecs_error_r : 0xbadf1201
[    4.157625] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:655  [ERR]  pwr_falcon_exterrstat_r : 0x85002efb
[    4.157628] nvgpu: 57000000.gpu       gk20a_pmu_dump_falcon_stats:659  [ERR]  pwr_falcon_exterraddr_r : 0x18405810
[    4.157662] nvgpu: 57000000.gpu                   print_pmu_trace:88   [ERR]  dump PMU trace buffer
[    4.157667] nvgpu: 57000000.gpu       nvgpu_pmu_dump_falcon_stats:51   [ERR]  pmu state: 7
[    4.157669] nvgpu: 57000000.gpu       nvgpu_pmu_dump_falcon_stats:52   [ERR]  elpg state: 0
[    4.157673] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:129  [ERR]  gr_fecs_os_r : 0
[    4.157677] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:131  [ERR]  gr_fecs_cpuctl_r : 0x60
[    4.157680] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:133  [ERR]  gr_fecs_idlestate_r : 0x0
[    4.157684] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:135  [ERR]  gr_fecs_mailbox0_r : 0x0
[    4.157687] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:137  [ERR]  gr_fecs_mailbox1_r : 0x0
[    4.157690] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:139  [ERR]  gr_fecs_irqstat_r : 0x0
[    4.157693] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:141  [ERR]  gr_fecs_irqmode_r : 0x4
[    4.157697] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:143  [ERR]  gr_fecs_irqmask_r : 0x8704
[    4.157700] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:145  [ERR]  gr_fecs_irqdest_r : 0x0
[    4.157704] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:147  [ERR]  gr_fecs_debug1_r : 0x40
[    4.157707] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:149  [ERR]  gr_fecs_debuginfo_r : 0x0
[    4.157710] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:151  [ERR]  gr_fecs_ctxsw_status_1_r : 0x382
[    4.157713] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(0) : 0x1
[    4.157716] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(1) : 0x0
[    4.157719] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(2) : 0x1
[    4.157723] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(3) : 0x0
[    4.157726] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(4) : 0x1ffda0
[    4.157729] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(5) : 0x0
[    4.157732] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(6) : 0x0
[    4.157735] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(7) : 0x0
[    4.157738] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(8) : 0x0
[    4.157741] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(9) : 0x0
[    4.157744] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(10) : 0x0
[    4.157747] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(11) : 0x0
[    4.157750] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(12) : 0x0
[    4.157753] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(13) : 0x0
[    4.157756] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(14) : 0x0
[    4.157759] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:155  [ERR]  gr_fecs_ctxsw_mailbox_r(15) : 0x0
[    4.157762] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:159  [ERR]  gr_fecs_engctl_r : 0x0
[    4.157766] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:161  [ERR]  gr_fecs_curctx_r : 0x0
[    4.157769] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:163  [ERR]  gr_fecs_nxtctx_r : 0x0
[    4.157773] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:169  [ERR]  FECS_FALCON_REG_IMB : 0xbadfbadf
[    4.157776] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:175  [ERR]  FECS_FALCON_REG_DMB : 0xbadfbadf
[    4.157780] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:181  [ERR]  FECS_FALCON_REG_CSW : 0xbadfbadf
[    4.157784] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:187  [ERR]  FECS_FALCON_REG_CTX : 0xbadfbadf
[    4.157788] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:193  [ERR]  FECS_FALCON_REG_EXCI : 0xbadfbadf
[    4.157791] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:200  [ERR]  FECS_FALCON_REG_PC : 0xbadfbadf
[    4.157795] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:206  [ERR]  FECS_FALCON_REG_SP : 0xbadfbadf
[    4.157799] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:200  [ERR]  FECS_FALCON_REG_PC : 0xbadfbadf
[    4.157803] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:206  [ERR]  FECS_FALCON_REG_SP : 0xbadfbadf
[    4.157807] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:200  [ERR]  FECS_FALCON_REG_PC : 0xbadfbadf
[    4.157810] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:206  [ERR]  FECS_FALCON_REG_SP : 0xbadfbadf
[    4.157814] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:200  [ERR]  FECS_FALCON_REG_PC : 0xbadfbadf
[    4.157818] nvgpu: 57000000.gpu      gk20a_fecs_dump_falcon_stats:206  [ERR]  FECS_FALCON_REG_SP : 0xbadfbadf
[    4.157889] nvgpu: 57000000.gpu                  gk20a_ptimer_isr:50   [ERR]  PRI timeout: ADR 0x00122124 READ  DATA 0x00000000
[    4.157892] nvgpu: 57000000.gpu                  gk20a_ptimer_isr:56   [ERR]  FECS_ERRCODE 0xbadf5202
[    6.083019] Bridge firewalling registered

Ubuntu 18.04.6 LTS AI ttyS0
