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

static void rts5209_init_vendor_cfg(struct rtsx_pcr *pcr)
{
	u32 val;

	rtsx_pci_read_config_dword(pcr, 0x724, &val);
	dev_dbg(&(pcr->pci->dev), "Cfg 0x724: 0x%x\n", val);

	if (!(val & 0x80)) {
		if (val & 0x08)
			pcr->ms_pmos = false;
		else
			pcr->ms_pmos = true;
	}
}

static int rts5209_optimize_phy(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_phy_register(pcr, 0x00, 0xB966);
}

static int rts5209_turn_on_led(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, 0xFD58, 0x01, 0x00);
}

static int rts5209_turn_off_led(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, 0xFD58, 0x01, 0x01);
}

static int rts5209_enable_auto_blink(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, CARD_AUTO_BLINK, 0xFF, 0x0D);
}

static int rts5209_disable_auto_blink(struct rtsx_pcr *pcr)
{
	return rtsx_pci_write_register(pcr, CARD_AUTO_BLINK, 0x08, 0x00);
}

static const struct pcr_ops rts5209_pcr_ops = {
	.extra_init_hw = NULL,
	.optimize_phy = rts5209_optimize_phy,
	.turn_on_led = rts5209_turn_on_led,
	.turn_off_led = rts5209_turn_off_led,
	.enable_auto_blink = rts5209_enable_auto_blink,
	.disable_auto_blink = rts5209_disable_auto_blink,
};

/* SD Pull Control Enable:
 *     SD_DAT[3:0] ==> pull up
 *     SD_CD       ==> pull up
 *     SD_WP       ==> pull up
 *     SD_CMD      ==> pull up
 *     SD_CLK      ==> pull down
 */
static const u32 rts5209_sd_pull_ctl_enable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL1, 0xAA),
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0xAA),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xE9),
	0,
};

/* SD Pull Control Disable:
 *     SD_DAT[3:0] ==> pull down
 *     SD_CD       ==> pull up
 *     SD_WP       ==> pull down
 *     SD_CMD      ==> pull down
 *     SD_CLK      ==> pull down
 */
static const u32 rts5209_sd_pull_ctl_disable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL1, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xD5),
	0,
};

/* MS Pull Control Enable:
 *     MS CD       ==> pull up
 *     others      ==> pull down
 */
static const u32 rts5209_ms_pull_ctl_enable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL4, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL5, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL6, 0x15),
	0,
};

/* MS Pull Control Disable:
 *     MS CD       ==> pull up
 *     others      ==> pull down
 */
static const u32 rts5209_ms_pull_ctl_disable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL4, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL5, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL6, 0x15),
	0,
};

static const struct pcr_reg_val rts5209_rval1 = {
	.ldo_pwr_mask = LDO3318_PWR_MASK,
	.ldo_pwr_on = 0x00,
	.ldo_pwr_off = 0x06,
	.ldo_pwr_suspend = 0x04,

	.sd_pwr_mask = SD_POWER_MASK,
	.sd_partial_pwr_on = SD_PARTIAL_POWER_ON,
	.sd_pwr_on = SD_POWER_ON,
	.sd_pwr_off = SD_POWER_OFF,

	/* MS card also uses SD power here */
	.ms_pwr_mask = SD_POWER_MASK,
	.ms_partial_pwr_on = SD_PARTIAL_POWER_ON,
	.ms_pwr_on = SD_POWER_ON,
	.ms_pwr_off = SD_POWER_OFF,
};

static const struct pcr_reg_val rts5209_rval2 = {
	.ldo_pwr_mask = LDO3318_PWR_MASK,
	.ldo_pwr_on = 0x00,
	.ldo_pwr_off = 0x06,
	.ldo_pwr_suspend = 0x04,

	.sd_pwr_mask = SD_POWER_MASK,
	.sd_partial_pwr_on = SD_PARTIAL_POWER_ON,
	.sd_pwr_on = SD_POWER_ON,
	.sd_pwr_off = SD_POWER_OFF,

	.ms_pwr_mask = MS_POWER_MASK,
	.ms_partial_pwr_on = MS_PARTIAL_POWER_ON,
	.ms_pwr_on = MS_POWER_ON,
	.ms_pwr_off = MS_POWER_OFF,
};

void rts5209_init_params(struct rtsx_pcr *pcr)
{
	pcr->extra_caps = EXTRA_CAPS_SD_SDR50 |
		EXTRA_CAPS_SD_SDR104 | EXTRA_CAPS_MMC_8BIT;
	pcr->num_slots = 2;
	pcr->ops = &rts5209_pcr_ops;

	rts5209_init_vendor_cfg(pcr);
	if (pcr->ms_pmos)
		pcr->rval = &rts5209_rval2;
	else
		pcr->rval = &rts5209_rval1;

	pcr->ic_version = rts5209_get_ic_version(pcr);
	pcr->sd_pull_ctl_enable_tbl = rts5209_sd_pull_ctl_enable_tbl;
	pcr->sd_pull_ctl_disable_tbl = rts5209_sd_pull_ctl_disable_tbl;
	pcr->ms_pull_ctl_enable_tbl = rts5209_ms_pull_ctl_enable_tbl;
	pcr->ms_pull_ctl_disable_tbl = rts5209_ms_pull_ctl_disable_tbl;
}
