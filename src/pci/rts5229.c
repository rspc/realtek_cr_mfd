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

static u8 rts5229_get_ic_version(struct rtsx_pcr *pcr)
{
	u8 val;

	rtsx_pci_read_register(pcr, 0xFE90, &val);
	return val & 0x0F;
}

static int rts5229_extra_init_hw(struct rtsx_pcr *pcr)
{
	int err;

	rtsx_pci_init_cmd(pcr);

	/* Switch LDO3318 source from DV33 to card_3v3 */
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, 0xFE78, 0x03, 0x00);
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, 0xFE78, 0x03, 0x01);
	/* LED shine disabled, set initial shine cycle period */
	rtsx_pci_add_cmd(pcr, WRITE_REG_CMD, 0xFC1E, 0x0F, 0x02);

	err = rtsx_pci_send_cmd(pcr, 100);
	if (err < 0)
		return err;

	return 0;
}

static int rts5229_optimize_phy(struct rtsx_pcr *pcr)
{
	int err;

	/* Optimize RX sensitivity */
	err = rtsx_pci_write_phy_register(pcr, 0x00, 0xBA42);
	if (err < 0)
		return err;

	return 0;
}

static int rts5229_turn_on_led(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, 0xFC1F, 0x02, 0x02);
	if (err < 0)
		return err;

	return 0;
}

static int rts5229_turn_off_led(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, 0xFC1F, 0x02, 0x00);
	if (err < 0)
		return err;

	return 0;
}

static int rts5229_enable_auto_blink(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, 0xFC1E, 0x08, 0x08);
	if (err < 0)
		return err;

	return 0;
}

static int rts5229_disable_auto_blink(struct rtsx_pcr *pcr)
{
	int err;

	err = rtsx_pci_write_register(pcr, 0xFC1E, 0x08, 0x00);
	if (err < 0)
		return err;

	return 0;
}

static const struct pcr_ops rts5229_pcr_ops = {
	.extra_init_hw = rts5229_extra_init_hw,
	.optimize_phy = rts5229_optimize_phy,
	.turn_on_led = rts5229_turn_on_led,
	.turn_off_led = rts5229_turn_off_led,
	.enable_auto_blink = rts5229_enable_auto_blink,
	.disable_auto_blink = rts5229_disable_auto_blink,
};

/* SD Pull Control Enable:
 *     SD_DAT[3:0] ==> pull up
 *     SD_CD       ==> pull up
 *     SD_WP       ==> pull up
 *     SD_CMD      ==> pull up
 *     SD_CLK      ==> pull down
 */
static const u32 rts5229_sd_pull_ctl_enable_tbl1[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0xAA),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xE9),
	0,
};

/* For RTS5229 version C */
static const u32 rts5229_sd_pull_ctl_enable_tbl2[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0xAA),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xD9),
	0,
};

/* SD Pull Control Disable:
 *     SD_DAT[3:0] ==> pull down
 *     SD_CD       ==> pull up
 *     SD_WP       ==> pull down
 *     SD_CMD      ==> pull down
 *     SD_CLK      ==> pull down
 */
static const u32 rts5229_sd_pull_ctl_disable_tbl1[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xD5),
	0,
};

/* For RTS5229 version C */
static const u32 rts5229_sd_pull_ctl_disable_tbl2[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL2, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL3, 0xE5),
	0,
};

/* MS Pull Control Enable:
 *     MS CD       ==> pull up
 *     others      ==> pull down
 */
static const u32 rts5229_ms_pull_ctl_enable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL5, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL6, 0x15),
	0,
};

/* MS Pull Control Disable:
 *     MS CD       ==> pull up
 *     others      ==> pull down
 */
static const u32 rts5229_ms_pull_ctl_disable_tbl[] = {
	RTSX_REG_PAIR(CARD_PULL_CTL5, 0x55),
	RTSX_REG_PAIR(CARD_PULL_CTL6, 0x15),
	0,
};

static const struct pcr_reg_val rts5229_rval = {
	.ldo_pwr_mask = LDO3318_PWR_MASK,
	.ldo_pwr_on = 0x06,
	.ldo_pwr_off = 0x00,
	.ldo_pwr_suspend = 0x02,

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

void rts5229_init_params(struct rtsx_pcr *pcr)
{
	pcr->extra_caps = EXTRA_CAPS_SD_SDR50 | EXTRA_CAPS_SD_SDR104;
	pcr->num_slots = 2;
	pcr->ops = &rts5229_pcr_ops;
	pcr->rval = &rts5229_rval;

	pcr->ic_version = rts5229_get_ic_version(pcr);
	if (pcr->ic_version == IC_VER_C) {
		pcr->sd_pull_ctl_enable_tbl = rts5229_sd_pull_ctl_enable_tbl2;
		pcr->sd_pull_ctl_disable_tbl = rts5229_sd_pull_ctl_disable_tbl2;
	} else {
		pcr->sd_pull_ctl_enable_tbl = rts5229_sd_pull_ctl_enable_tbl1;
		pcr->sd_pull_ctl_disable_tbl = rts5229_sd_pull_ctl_disable_tbl1;
	}
	pcr->ms_pull_ctl_enable_tbl = rts5229_ms_pull_ctl_enable_tbl;
	pcr->ms_pull_ctl_disable_tbl = rts5229_ms_pull_ctl_disable_tbl;
}
