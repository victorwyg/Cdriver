#include <linux/build-salt.h>
#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

BUILD_SALT;

MODULE_INFO(vermagic, VERMAGIC_STRING);
MODULE_INFO(name, KBUILD_MODNAME);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
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
__used
__attribute__((section("__versions"))) = {
	{ 0x10d06f3b, "module_layout" },
	{ 0xe5135656, "platform_driver_unregister" },
	{ 0x911060d2, "__platform_driver_register" },
	{ 0xdb7305a1, "__stack_chk_fail" },
	{ 0x1e047854, "warn_slowpath_fmt" },
	{ 0xf4fa543b, "arm_copy_to_user" },
	{ 0x97255bdf, "strlen" },
	{ 0x91715312, "sprintf" },
	{ 0x8f678b07, "__stack_chk_guard" },
	{ 0x37a0cba, "kfree" },
	{ 0x7c32d0f0, "printk" },
	{ 0x47229b5c, "gpio_request" },
	{ 0x470efb25, "of_get_named_gpio_flags" },
	{ 0x5736a32a, "of_find_node_opts_by_path" },
	{ 0x7772b225, "device_create" },
	{ 0xb489cb8d, "__class_create" },
	{ 0x99d31bfb, "cdev_add" },
	{ 0xb9aea4e3, "cdev_init" },
	{ 0xe3ec2f2b, "alloc_chrdev_region" },
	{ 0x2bac60c1, "kmem_cache_alloc" },
	{ 0xccf15c6e, "kmalloc_caches" },
	{ 0xb06f864f, "gpiod_get_raw_value" },
	{ 0x7ed47eaa, "gpiod_direction_input" },
	{ 0x39a12ca7, "_raw_spin_unlock_irqrestore" },
	{ 0xfc036f08, "gpiod_set_raw_value" },
	{ 0xe392d11e, "gpiod_direction_output_raw" },
	{ 0xe027f59b, "gpio_to_desc" },
	{ 0x5f849a69, "_raw_spin_lock_irqsave" },
	{ 0x8e865d3c, "arm_delay_ops" },
	{ 0x376e15d8, "try_module_get" },
	{ 0x54cc369d, "module_put" },
	{ 0x6091b333, "unregister_chrdev_region" },
	{ 0xf7d082ff, "cdev_del" },
	{ 0xa3f23a4d, "class_destroy" },
	{ 0x676c0ac6, "device_destroy" },
	{ 0xefd6cf06, "__aeabi_unwind_cpp_pr0" },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "5F487B66C6F07B2311D83D8");
