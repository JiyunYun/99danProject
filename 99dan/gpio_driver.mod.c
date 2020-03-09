#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
 .name = KBUILD_MODNAME,
 .init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
 .exit = cleanup_module,
#endif
};

static const struct modversion_info ____versions[]
__attribute_used__
__attribute__((section("__versions"))) = {
	{ 0xfbad1725, "struct_module" },
	{ 0xc192d491, "unregister_chrdev" },
	{ 0x28118cb6, "__get_user_1" },
	{ 0x75b38522, "del_timer" },
	{ 0xf20dabd8, "free_irq" },
	{ 0x2cf190e3, "request_irq" },
	{ 0xde75b689, "set_irq_type" },
	{ 0x35fe47a1, "init_timer" },
	{ 0x8fafad7d, "pxa_gpio_mode" },
	{ 0x7d11c268, "jiffies" },
	{ 0x20187c7, "mod_timer" },
	{ 0xdd132261, "printk" },
	{ 0xdffa6002, "register_chrdev" },
};

static const char __module_depends[]
__attribute_used__
__attribute__((section(".modinfo"))) =
"depends=";

