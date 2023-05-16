#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;
BUILD_LTO_INFO;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__section(".gnu.linkonce.this_module") = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

#ifdef CONFIG_RETPOLINE
MODULE_INFO(retpoline, "Y");
#endif


static const struct modversion_info ____versions[]
__used __section("__versions") = {
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x3213f038, "mutex_unlock" },
	{ 0x92997ed8, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x88db9f48, "__check_object_size" },
	{ 0x6b10bee1, "_copy_to_user" },
	{ 0x7682ba4e, "__copy_overflow" },
	{ 0xbb9ed3bf, "mutex_trylock" },
	{ 0x13c49cc2, "_copy_from_user" },
	{ 0xa916b694, "strnlen" },
	{ 0xcbd4898c, "fortify_panic" },
	{ 0xc1352057, "__register_chrdev" },
	{ 0xaee657ee, "__class_create" },
	{ 0x6bc3fbc0, "__unregister_chrdev" },
	{ 0xf3e489db, "device_create" },
	{ 0x645620c0, "class_destroy" },
	{ 0xcefb0c9f, "__mutex_init" },
	{ 0xbdee1658, "device_destroy" },
	{ 0x7377b2e, "class_unregister" },
	{ 0x541a6db8, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "75F00D4301C23F59695E9AC");
