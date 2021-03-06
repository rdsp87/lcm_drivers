/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <platform/mt_pwm.h>
	#ifdef LCD_DEBUG
		#define LCM_DEBUG(format, ...)   printf("uboot ssd2825" format "\n", ## __VA_ARGS__)
	#else
		#define LCM_DEBUG(format, ...)
	#endif

#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt6577_gpio.h>
	#include <asm/arch/mt6577_pwm.h>

	#ifdef LCD_DEBUG
		#define LCM_DEBUG(format, ...)   printf("uboot ssd2825" format "\n", ## __VA_ARGS__)
	#else
		#define LCM_DEBUG(format, ...)
	#endif
#else
	#include <mach/mt_gpio.h>
	#include <mach/mt_pwm.h>
	#include <mach/mt_pm_ldo.h>
	#ifdef LCD_DEBUG
		#define LCM_DEBUG(format, ...)   printk("kernel ssd2825" format "\n", ## __VA_ARGS__)
	#else
		#define LCM_DEBUG(format, ...)
	#endif
#endif

#include "lcm_drv.h"

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(800)

#define REGFLAG_DELAY             							0XFE
#define REGFLAG_END_OF_TABLE      							0xFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   			lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)        

static unsigned int lcm_read(void);

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};

static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},

    // Sleep Mode On
	{0x10, 1, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
		memset(params, 0, sizeof(LCM_PARAMS));
	
		params->type   = LCM_TYPE_DSI;

		params->width  = FRAME_WIDTH;
		params->height = FRAME_HEIGHT;

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
		// Highly depends on LCD driver capability.
		// Not support in MT6573
		params->dsi.packet_size=256;
		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
#if 0
		params->dsi.vertical_sync_active				= 3;
		params->dsi.vertical_backporch					= 3;
		params->dsi.vertical_frontporch 				= 3;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 50;
		params->dsi.horizontal_frontporch				= 50;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
#else
params->dsi.vertical_sync_active				= 3;
		params->dsi.vertical_backporch					= 5;
		params->dsi.vertical_frontporch 				= 8;
params->dsi.vertical_active_line				= FRAME_HEIGHT; 

params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 30;// 20;
		params->dsi.horizontal_frontporch				= 30;// 20;
params->dsi.horizontal_active_pixel 			= FRAME_WIDTH;
#endif
		// Bit rate calculation
		params->dsi.pll_div1=26;//26;		// fref=26MHz, fvco=fref*(div1+1)	(div1=0~63, fvco=500MHZ~1GHz)
		params->dsi.pll_div2=1; 		// div2=0~15: fout=fvo/(2*div2)

}


static void lcm_init(void)
{
	unsigned int data_array[16];
	
#ifdef BUILD_UBOOT 
#elif defined(BUILD_LK) 
#else
	hwPowerOn(MT65XX_POWER_LDO_VCAM_IO, VOL_1800, "1V8_LCD_VIO_MTK_S" ); 
	hwPowerOn(MT65XX_POWER_LDO_VCAMD, VOL_2800, "2V8_LCD_VCC_MTK_S" );
#endif 
	MDELAY(5);
	mt_set_gpio_mode(GPIO18, GPIO_MODE_00); /* LCD ENABLE PIN -> LOW */
	mt_set_gpio_pull_enable(GPIO18, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO18, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO18,GPIO_OUT_ZERO);
	MDELAY(5);
	mt_set_gpio_out(GPIO18,GPIO_OUT_ONE);
	MDELAY(10);

	/*For DI Issue, Use dsi_set_cmdq() instead of dsi_set_cmdq_v2()*/
	
	data_array[0] = 0x00043902; // SET EXTC
	data_array[1] = 0x7983ffb9;
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00203902; //SET POWER
	data_array[1] = 0x345000b1;
	data_array[2] = 0x11084fe8;
	data_array[3] = 0x372f1111;
	data_array[4] = 0x05423fbf;

	data_array[5] = 0xe600f16a;
	data_array[6] = 0xe6e6e6e6;
	data_array[7] = 0x0a050400;
	data_array[8] = 0x6f05040b;
	dsi_set_cmdq(&data_array, 9, 1);

	data_array[0] = 0x000e3902; //SET DISP
	data_array[1] = 0x3c0000b2;
	data_array[2] = 0xB219110b;
	data_array[3] = 0x110bff00;
	data_array[4] = 0x00002019;
	dsi_set_cmdq(&data_array, 5, 1);

	data_array[0] = 0x00203902; //SET CYC
	data_array[1] = 0x000800b4;
	data_array[2] = 0x00001032;
	data_array[3] = 0x00000000;
	data_array[4] = 0x400a3700;
	data_array[5] = 0x300a3704;
	data_array[6] = 0x0a513c14;
	data_array[7] = 0x300a400a;
	data_array[8] = 0x0a554014;
	dsi_set_cmdq(&data_array, 9, 1);

	data_array[0] = 0x00053902; //SET VCOM
	data_array[1] = 0x00a200b6;
	data_array[2] = 0x000000a2;
	dsi_set_cmdq(&data_array, 3, 1);

	data_array[0] = 0x00043902; //SET TE
	data_array[1] = 0x005010b7;
	dsi_set_cmdq(&data_array, 2, 1);

	data_array[0] = 0x00023902; //SET PANEL
	data_array[1] = 0x00000bcc;
	dsi_set_cmdq(&data_array, 2, 1);


	data_array[0] = 0x00303902; //SET GIP SIGNAL
	data_array[1] = 0x080300d5;
	data_array[2] = 0x00000100;
	data_array[3] = 0x88880003;
	data_array[4] = 0x45678888;
	data_array[5] = 0x23010123;
	data_array[6] = 0x88888888;
	data_array[7] = 0x88888888;
	data_array[8] = 0x45678888;
	data_array[9] = 0x23010123;
	data_array[10] = 0x88888888;
	data_array[11] = 0x01018888;
	data_array[12] = 0x00030000;
	dsi_set_cmdq(&data_array, 13, 1);

	data_array[0] = 0x00043902; //SET PTBA
	data_array[1] = 0x047005de;
	dsi_set_cmdq(&data_array, 2, 1);

#if 0 //evb_rev_b
	data_array[0] = 0x00233902; //Set GAMMA
	data_array[1] = 0x141105e0;
	data_array[2] = 0x203f3f37;
	data_array[3] = 0x0d0e084f;
	data_array[4] = 0x14121412;
	data_array[5] = 0x11051c1d;
	data_array[6] = 0x3f3f3714;
	data_array[7] = 0x0e084f20;
	data_array[8] = 0x1214120d;
	data_array[9] = 0x001c1d14;
	dsi_set_cmdq(&data_array, 10, 1);
#else //REV_A
/*
	data_array[0] = 0x00243902; //Set GAMMA
	data_array[1] = 0x0c0179e0;
	data_array[2] = 0x3f3f3d0f;
	data_array[3] = 0x0b06471e;
	data_array[4] = 0x1214120f;
	data_array[5] = 0x01181215;
	data_array[6] = 0x3f3d0f0c;
	data_array[7] = 0x06471e3f;
	data_array[8] = 0x14120f0b;
	data_array[9] = 0x18121512;
	dsi_set_cmdq(&data_array, 10, 1);
*/

#if 1 //USE_RC7
	data_array[0] = 0x00053902; //Set TCONOPT_Himax 121012
	data_array[1] = 0x000000C7;
	data_array[2] = 0x00000003;
	dsi_set_cmdq(&data_array, 3, 1);
#endif
	data_array[0] = 0x00243902; //Set GAMMA
	data_array[1] = 0x0c0179e0;
	data_array[2] = 0x3b2a290e;
	data_array[3] = 0x0b06421a;
	data_array[4] = 0x1214110e;
	data_array[5] = 0x00181215;
	data_array[6] = 0x28270604;
	data_array[7] = 0x063f123b;
	data_array[8] = 0x14110e0b;
	data_array[9] = 0x18121512;
	dsi_set_cmdq(&data_array, 10, 1);

#endif

	data_array[0] = 0x00110500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);
	data_array[0] = 0x00290500;
	dsi_set_cmdq(&data_array, 1, 1);

}

static void lcm_suspend(void)
{
#if 0
	unsigned int data_array[16];
	
	data_array[0] = 0x00280500; //Display OFF
	dsi_set_cmdq(&data_array, 1, 1);
	data_array[0] = 0x00100500; //Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(10);
#else

#ifdef BUILD_UBOOT
#elif defined(BUILD_LK)
	mt_set_gpio_mode(GPIO18, GPIO_MODE_00); /* LCD RESET, mipi sequence issue.(temporary code) */
	mt_set_gpio_pull_enable(GPIO18, GPIO_PULL_ENABLE);
	mt_set_gpio_dir(GPIO18, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO18,GPIO_OUT_ZERO);
	MDELAY(5);
	mt_set_gpio_out(GPIO18,GPIO_OUT_ONE);
	MDELAY(10);

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	MDELAY(10);
#else
	unsigned int data_array[1];
	data_array[0] = 0x00100500;  // sleep in command
	dsi_set_cmdq(&data_array, 1, 1);

    UDELAY(200);
    data_array[0] = 0x00000100;  // Vsync Start
	dsi_set_cmdq(&data_array, 1, 1);

	MDELAY(40);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(10);

	hwPowerDown(MT65XX_POWER_LDO_VCAMD, "2V8_LCD_VCC_MTK_S" );
	hwPowerDown(MT65XX_POWER_LDO_VCAM_IO, "1V8_LCD_VIO_MTK_S" );
#endif
#endif
}


static void lcm_resume(void)
{
#if 0
	unsigned int data_array[16];

	data_array[0] = 0x00110500; //Sleep Out
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);
	data_array[0] = 0x00290500; //Display On
	dsi_set_cmdq(&data_array, 1, 1);
#endif
	lcm_init();
//	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}


static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//                                  
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_setpwm(unsigned int divider)
{
	// TBD
}


static unsigned int lcm_getpwm(unsigned int divider)
{
	// ref freq = 15MHz, B0h setting 0x80, so 80.6% * freq is pwm_clk;
	// pwm_clk / 255 / 2(lcm_setpwm() 6th params) = pwm_duration = 23706
	unsigned int pwm_clk = 23706 / (1<<divider);	
	return pwm_clk;
}

static unsigned int lcm_read(void)
{
	unsigned int id = 0;
	unsigned char buffer[2]={0x88};
	unsigned int array[16];

	array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
//	id = read_reg(0xF4);
	read_reg_v2(0x52, buffer, 1);
	id = buffer[0]; //we only need ID
	//LCD_DEBUG("\n\n\n\n\n\n\n\n\n\n[soso]%s, lcm_read = 0x%08x\n", __func__, id);
    //return (LCM_ID == id)?1:0;
}


LCM_DRIVER hx8379a_dsi_vdo_drv = 
{
    .name			= "hx8379a_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};

