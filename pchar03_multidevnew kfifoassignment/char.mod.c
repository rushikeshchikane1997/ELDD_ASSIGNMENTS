#include <linux/module.h>
#define INCLUDE_VERMAGIC
#include <linux/build-salt.h>
#include <linux/elfnote-lto.h>
#include <linux/export-internal.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

#ifdef CONFIG_UNWINDER_ORC
#include <asm/orc_header.h>
ORC_HEADER;
#endif

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
	{ 0x4578f528, "__kfifo_to_user" },
	{ 0xeb233a45, "__kmalloc" },
	{ 0x139f2189, "__kfifo_alloc" },
	{ 0xdb760f52, "__kfifo_free" },
	{ 0x37a0cba, "kfree" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x147471ba, "class_create" },
	{ 0x28d0a616, "device_create" },
	{ 0x3b2621b7, "device_destroy" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0x47445572, "cdev_init" },
	{ 0x38d53355, "cdev_add" },
	{ 0x91873f54, "cdev_del" },
	{ 0x84853d0e, "class_destroy" },
	{ 0x6513c00f, "param_ops_int" },
	{ 0xbdfb6dbb, "__fentry__" },
	{ 0x122c3a7e, "_printk" },
	{ 0x5b8239ca, "__x86_return_thunk" },
	{ 0x30a80826, "__kfifo_from_user" },
	{ 0xa19b956, "__stack_chk_fail" },
	{ 0x69716abc, "module_layout" },
};

MODULE_INFO(depends, "");


MODULE_INFO(srcversion, "3EFDBB2AAFF70C2D79AF71E");
