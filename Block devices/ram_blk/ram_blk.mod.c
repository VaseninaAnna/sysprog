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

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0xe6d505d5, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0xb5a459dc, __VMLINUX_SYMBOL_STR(unregister_blkdev) },
	{ 0xbf1343fd, __VMLINUX_SYMBOL_STR(device_add_disk) },
	{ 0x28318305, __VMLINUX_SYMBOL_STR(snprintf) },
	{ 0x60e6f673, __VMLINUX_SYMBOL_STR(alloc_disk) },
	{ 0xd6ee688f, __VMLINUX_SYMBOL_STR(vmalloc) },
	{ 0x2a9814d8, __VMLINUX_SYMBOL_STR(blk_queue_make_request) },
	{ 0xe5eb9e5f, __VMLINUX_SYMBOL_STR(blk_alloc_queue) },
	{ 0xc499ae1e, __VMLINUX_SYMBOL_STR(kstrdup) },
	{ 0xcb09f265, __VMLINUX_SYMBOL_STR(kmem_cache_alloc_trace) },
	{ 0x95fd4f03, __VMLINUX_SYMBOL_STR(kmalloc_caches) },
	{ 0x71a50dbc, __VMLINUX_SYMBOL_STR(register_blkdev) },
	{ 0x16305289, __VMLINUX_SYMBOL_STR(warn_slowpath_null) },
	{ 0xe766a8f5, __VMLINUX_SYMBOL_STR(bio_endio) },
	{ 0x69acdf38, __VMLINUX_SYMBOL_STR(memcpy) },
	{ 0x67056d14, __VMLINUX_SYMBOL_STR(current_task) },
	{ 0x37a0cba, __VMLINUX_SYMBOL_STR(kfree) },
	{ 0xebe17e84, __VMLINUX_SYMBOL_STR(blk_cleanup_queue) },
	{ 0x999e8297, __VMLINUX_SYMBOL_STR(vfree) },
	{ 0x22e3f1ae, __VMLINUX_SYMBOL_STR(put_disk) },
	{ 0x76a7f0, __VMLINUX_SYMBOL_STR(del_gendisk) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xbdfb6dbb, __VMLINUX_SYMBOL_STR(__fentry__) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "F00BA6B78D4EF92E737AB3B");
