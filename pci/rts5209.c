/* Driver for Realtek PCI-Express card reader
 *
 * Copyright(c) 2009 Realtek Semiconductor Corp. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Author:
 *   Wei WANG <wei_wang@realsil.com.cn>
 *   No. 450, Shenhu Road, Suzhou Industry Park, Suzhou, China
 */

#include <linux/module.h>
#include <linux/mfd/rtsx_pci.h>

#include "rtsx_pcr.h"

static u8 rts5209_get_ic_version(struct rtsx_pcr *pcr)
{
	u8 val;

	val = rtsx_pci_readb(pcr, 0x1C);
	return val & 0x0F;
}

static int rts5209_extra_init_hw(struct rtsx_pcr *pcr)
{
	return 0;
}

static int rts5209_optimize_phy(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_phy_register(pcr, 0x00, 0xB966);
	if (err < 0)
		return err;

	return 0;
}

static int rts5209_turn_on_led(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, 0xFD58, 0x01, 0x00);
	if (err < 0)
		return err;

	return 0;
}

static int rts5209_turn_off_led(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, 0xFD58, 0x01, 0x01);
	if (err < 0)
		return err;

	return 0;
}

static int rts5209_enable_auto_blink(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, CARD_AUTO_BLINK, 0xFF, 0x0D);
	if (err < 0)
		return err;

	return 0;
}

static int rts5209_disable_auto_blink(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, CARD_AUTO_BLINK, 0x08, 0x00);
	if (err < 0)
		return err;

	return 0;
}

const struct pcr_ops rts5209_pcr_ops = {
	.get_ic_version = rts5209_get_ic_version,
	.extra_init_hw = rts5209_extra_init_hw,
	.optimize_phy = rts5209_optimize_phy,
	.turn_on_led = rts5209_turn_on_led,
	.turn_off_led = rts5209_turn_off_led,
	.enable_auto_blink = rts5209_enable_auto_blink,
	.disable_auto_blink = rts5209_disable_auto_blink,
};
