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
	{ 0x6bc3fbc0, __VMLINUX_SYMBOL_STR(__unregister_chrdev) },
	{ 0xd890979e, __VMLINUX_SYMBOL_STR(kernel_write) },
	{ 0x663b5397, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0xd2b09ce5, __VMLINUX_SYMBOL_STR(__kmalloc) },
	{ 0xf9a482f9, __VMLINUX_SYMBOL_STR(msleep) },
	{ 0x2cd7da0a, __VMLINUX_SYMBOL_STR(alloc_disk) },
	{ 0x1fdc7df2, __VMLINUX_SYMBOL_STR(_mcount) },
	{ 0x8889b9d6, __VMLINUX_SYMBOL_STR(blk_cleanup_queue) },
	{ 0xf33847d3, __VMLINUX_SYMBOL_STR(_raw_spin_unlock) },
	{ 0x3a9b6fb9, __VMLINUX_SYMBOL_STR(blk_unregister_region) },
	{ 0xeec47ab0, __VMLINUX_SYMBOL_STR(blk_queue_max_hw_sectors) },
	{ 0x43a53735, __VMLINUX_SYMBOL_STR(__alloc_workqueue_key) },
	{ 0x47229b5c, __VMLINUX_SYMBOL_STR(gpio_request) },
	{ 0x84bc974b, __VMLINUX_SYMBOL_STR(__arch_copy_from_user) },
	{ 0x442a7b0a, __VMLINUX_SYMBOL_STR(skb_clone) },
	{ 0xdc65d6a2, __VMLINUX_SYMBOL_STR(gpio_to_desc) },
	{ 0x5641485b, __VMLINUX_SYMBOL_STR(tty_termios_encode_baud_rate) },
	{ 0x88bfa7e, __VMLINUX_SYMBOL_STR(cancel_work_sync) },
	{ 0xe4c9a89, __VMLINUX_SYMBOL_STR(device_destroy) },
	{ 0xa719d0ed, __VMLINUX_SYMBOL_STR(__register_chrdev) },
	{ 0x6e7c92ab, __VMLINUX_SYMBOL_STR(filp_close) },
	{ 0x1f18fa3c, __VMLINUX_SYMBOL_STR(mutex_unlock) },
	{ 0x6a0eee43, __VMLINUX_SYMBOL_STR(spi_setup) },
	{ 0x91715312, __VMLINUX_SYMBOL_STR(sprintf) },
	{ 0x98ae31bb, __VMLINUX_SYMBOL_STR(__alloc_pages_nodemask) },
	{ 0x312b4459, __VMLINUX_SYMBOL_STR(kthread_create_on_node) },
	{ 0xabc31512, __VMLINUX_SYMBOL_STR(mutex_trylock) },
	{ 0xdcb764ad, __VMLINUX_SYMBOL_STR(memset) },
	{ 0xe50231f8, __VMLINUX_SYMBOL_STR(blk_alloc_queue) },
	{ 0xa563261, __VMLINUX_SYMBOL_STR(spi_busnum_to_master) },
	{ 0x97fdbab9, __VMLINUX_SYMBOL_STR(_raw_spin_unlock_irqrestore) },
	{ 0x907caf90, __VMLINUX_SYMBOL_STR(__mutex_init) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0x8cef9c25, __VMLINUX_SYMBOL_STR(kthread_stop) },
	{ 0xec9592fd, __VMLINUX_SYMBOL_STR(blk_queue_max_discard_sectors) },
	{ 0x82963b98, __VMLINUX_SYMBOL_STR(del_gendisk) },
	{ 0xf181bca0, __VMLINUX_SYMBOL_STR(blkdev_get_by_path) },
	{ 0x3b2b3f21, __VMLINUX_SYMBOL_STR(blk_register_region) },
	{ 0xa1c864c4, __VMLINUX_SYMBOL_STR(skb_push) },
	{ 0x1b0dd6f8, __VMLINUX_SYMBOL_STR(mutex_lock) },
	{ 0x8c03d20c, __VMLINUX_SYMBOL_STR(destroy_workqueue) },
	{ 0xc5e16b4b, __VMLINUX_SYMBOL_STR(gpiod_direction_input) },
	{ 0x71a50dbc, __VMLINUX_SYMBOL_STR(register_blkdev) },
	{ 0x860bb889, __VMLINUX_SYMBOL_STR(dev_remove_pack) },
	{ 0x3d2f5ee8, __VMLINUX_SYMBOL_STR(device_create) },
	{ 0x2469810f, __VMLINUX_SYMBOL_STR(__rcu_read_unlock) },
	{ 0xf3cac39b, __VMLINUX_SYMBOL_STR(gpiod_direction_output_raw) },
	{ 0xd6b8e852, __VMLINUX_SYMBOL_STR(request_threaded_irq) },
	{ 0x43b0c9c3, __VMLINUX_SYMBOL_STR(preempt_schedule) },
	{ 0x111c985e, __VMLINUX_SYMBOL_STR(init_net) },
	{ 0x7500497c, __VMLINUX_SYMBOL_STR(get_disk) },
	{ 0x42160169, __VMLINUX_SYMBOL_STR(flush_workqueue) },
	{ 0x985089b4, __VMLINUX_SYMBOL_STR(contig_page_data) },
	{ 0x1d1da57, __VMLINUX_SYMBOL_STR(bio_endio) },
	{ 0xedd0cd89, __VMLINUX_SYMBOL_STR(radix_tree_delete) },
	{ 0xb7af5aa6, __VMLINUX_SYMBOL_STR(nf_register_net_hook) },
	{ 0x827d6d22, __VMLINUX_SYMBOL_STR(spi_sync) },
	{ 0xe868b352, __VMLINUX_SYMBOL_STR(nf_unregister_net_hook) },
	{ 0xb5a459dc, __VMLINUX_SYMBOL_STR(unregister_blkdev) },
	{ 0x41c82305, __VMLINUX_SYMBOL_STR(blk_queue_bounce_limit) },
	{ 0x3725893a, __VMLINUX_SYMBOL_STR(radix_tree_gang_lookup) },
	{ 0x368d22b, __VMLINUX_SYMBOL_STR(__free_pages) },
	{ 0x24f5e56f, __VMLINUX_SYMBOL_STR(blkdev_put) },
	{ 0xa5827f00, __VMLINUX_SYMBOL_STR(__alloc_skb) },
	{ 0x102f9dc2, __VMLINUX_SYMBOL_STR(blk_queue_make_request) },
	{ 0x907a21b4, __VMLINUX_SYMBOL_STR(put_device) },
	{ 0xb35dea8f, __VMLINUX_SYMBOL_STR(__arch_copy_to_user) },
	{ 0x2c73595, __VMLINUX_SYMBOL_STR(blk_queue_physical_block_size) },
	{ 0x883dc848, __VMLINUX_SYMBOL_STR(kfree_skb) },
	{ 0xcd12de83, __VMLINUX_SYMBOL_STR(spi_alloc_device) },
	{ 0x16f4869b, __VMLINUX_SYMBOL_STR(put_disk) },
	{ 0x3d9ee9f0, __VMLINUX_SYMBOL_STR(clear_page) },
	{ 0xeaf93a21, __VMLINUX_SYMBOL_STR(crypto_destroy_tfm) },
	{ 0xd89757a, __VMLINUX_SYMBOL_STR(wake_up_process) },
	{ 0x3e2286cd, __VMLINUX_SYMBOL_STR(__radix_tree_insert) },
	{ 0xffdb220c, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x5cd885d5, __VMLINUX_SYMBOL_STR(_raw_spin_lock) },
	{ 0x96220280, __VMLINUX_SYMBOL_STR(_raw_spin_lock_irqsave) },
	{ 0xb3f7646e, __VMLINUX_SYMBOL_STR(kthread_should_stop) },
	{ 0xfe990052, __VMLINUX_SYMBOL_STR(gpio_free) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0x4829a47e, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0xfcbd75e8, __VMLINUX_SYMBOL_STR(gpiod_to_irq) },
	{ 0x5ac2fc94, __VMLINUX_SYMBOL_STR(gpiod_set_raw_value) },
	{ 0x9754ec10, __VMLINUX_SYMBOL_STR(radix_tree_preload) },
	{ 0xfa05263e, __VMLINUX_SYMBOL_STR(class_destroy) },
	{ 0x19c85478, __VMLINUX_SYMBOL_STR(radix_tree_lookup) },
	{ 0x2e0d2f7f, __VMLINUX_SYMBOL_STR(queue_work_on) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0xd50f48a8, __VMLINUX_SYMBOL_STR(device_add_disk) },
	{ 0x922016c5, __VMLINUX_SYMBOL_STR(dev_add_pack) },
	{ 0xdbcbd137, __VMLINUX_SYMBOL_STR(kill_bdev) },
	{ 0xe5a51fd6, __VMLINUX_SYMBOL_STR(consume_skb) },
	{ 0x25a9c4e, __VMLINUX_SYMBOL_STR(dev_queue_xmit) },
	{ 0x8d522714, __VMLINUX_SYMBOL_STR(__rcu_read_lock) },
	{ 0x79778196, __VMLINUX_SYMBOL_STR(skb_put) },
	{ 0xf79ccf8b, __VMLINUX_SYMBOL_STR(crypto_alloc_base) },
	{ 0x1166021f, __VMLINUX_SYMBOL_STR(__class_create) },
	{ 0xd3af9adb, __VMLINUX_SYMBOL_STR(flush_dcache_page) },
	{ 0x1bdef168, __VMLINUX_SYMBOL_STR(dma_ops) },
	{ 0x88db9f48, __VMLINUX_SYMBOL_STR(__check_object_size) },
	{ 0xf20dabd8, __VMLINUX_SYMBOL_STR(free_irq) },
	{ 0xe8000410, __VMLINUX_SYMBOL_STR(page_endio) },
	{ 0xc45319e5, __VMLINUX_SYMBOL_STR(tty_set_termios) },
	{ 0x5ccfcaf1, __VMLINUX_SYMBOL_STR(filp_open) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "36CF58F0DA7E38B8F695392");
