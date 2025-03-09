#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x95f28b28, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x827d6d22, __VMLINUX_SYMBOL_STR(spi_sync) },
	{ 0x907a21b4, __VMLINUX_SYMBOL_STR(put_device) },
	{ 0x6a0eee43, __VMLINUX_SYMBOL_STR(spi_setup) },
	{ 0xcd12de83, __VMLINUX_SYMBOL_STR(spi_alloc_device) },
	{ 0xa563261, __VMLINUX_SYMBOL_STR(spi_busnum_to_master) },
	{ 0x1fdc7df2, __VMLINUX_SYMBOL_STR(_mcount) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";

