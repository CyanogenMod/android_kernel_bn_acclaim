/*
 * Reboot reason special handler
 *
 * Copyright (C) 2011 Texas Instruments Incorporated - http://www.ti.com/
 *	Nishanth Menon
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.

 * This program is distributed "as is" WITHOUT ANY WARRANTY of any
 * kind, whether express or implied; without even the implied warranty
 * of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#include <linux/platform_device.h>
#include <linux/reboot.h>
#include <linux/notifier.h>
#include <linux/i2c/twl.h>

#include <mach/hardware.h>
#include <mach/omap4-common.h>
#include "omap4-sar-layout.h"

static int omap_reboot_notifier_call(struct notifier_block *this,
					unsigned long code, void *cmd)
{
	void __iomem *sar_base;
	char *reason = "normal";
	unsigned char vmmc_val = 0;

	sar_base = omap4_get_sar_ram_base();

	if (!sar_base)
		return notifier_from_errno(-ENOMEM);

	/* Save reboot mode in scratch memory */
	if (code == SYS_RESTART){
		if(cmd != NULL && strlen(cmd))
			reason = cmd;
		else
			reason = "reboot";
	}

	strncpy(sar_base + OMAP_REBOOT_REASON_OFFSET,
			reason, OMAP_REBOOT_REASON_SIZE);

	/*
	  Set warm reset sensitivity bit
	  and set VMMC voltage to 3.0V before
	  rebooting
	*/

	twl_i2c_write_u8(TWL6030_MODULE_ID0, 0x95, 0x9B);
	do {
		/*
		  Wait for the value to be written in the
		  VMMC_CFG_VOLTAGE register
		*/
		twl_i2c_read_u8(TWL6030_MODULE_ID0, &vmmc_val, 0x9B);
	} while (0x95 != vmmc_val);


	return NOTIFY_DONE;
}

static struct notifier_block omap_reboot_notifier = {
	.notifier_call = omap_reboot_notifier_call,
};

static int __init omap_reboot_reason_init(void)
{
	void __iomem *sar_base;

	sar_base = omap4_get_sar_ram_base();
	if (sar_base)
		strncpy(sar_base + OMAP_REBOOT_REASON_OFFSET,
			"", OMAP_REBOOT_REASON_SIZE);

	return register_reboot_notifier(&omap_reboot_notifier);
}
late_initcall(omap_reboot_reason_init);
