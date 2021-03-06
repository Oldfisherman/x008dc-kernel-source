

//#ifdef BUILD_LK
//	#include <platform/mt_gpio.h>
//#elif defined(BUILD_UBOOT)
//	#include <asm/arch/mt_gpio.h>
//#else
//	#include <mach/mt_gpio.h>
//#endif
#ifdef BUILD_LK
#include <platform/gpio_const.h>
#include <platform/mt_gpio.h>
#include <platform/upmu_common.h>
#else
#include <linux/string.h>
    #if defined(BUILD_UBOOT)
        #include <asm/arch/mt_gpio.h>
    #else
        #include <mt_gpio.h>
    #endif
#endif
#include "lcm_drv.h"
/* --------------------------------------------------------------------------- */
/* Local Constants */
/* --------------------------------------------------------------------------- */
#define LCM_DSI_CMD_MODE									0
#define FRAME_WIDTH  (720)
#define FRAME_HEIGHT (1280)
#define LCM_ID_OTM1289A                                     0x40 //(0x1289)

#define REGFLAG_DELAY             								0xFC
#define REGFLAG_END_OF_TABLE      							0xFD   // END OF REGISTERS MARKER
#ifndef TRUE
    #define TRUE 1
#endif
#ifndef FALSE
    #define FALSE 0
#endif
//static unsigned int lcm_esd_test = FALSE;      ///only for ESD test
/* --------------------------------------------------------------------------- */
/* Local Variables */
/* --------------------------------------------------------------------------- */

static LCM_UTIL_FUNCS lcm_util = { 0 };

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))



/* --------------------------------------------------------------------------- */
/* Local Functions */
/* --------------------------------------------------------------------------- */
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update) lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update) lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums) lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size) lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)


/* #define LCM_DSI_CMD_MODE */
//static struct platform_device * pltfm_dev ;

extern struct pinctrl *lcmbiasctrl ;
extern struct pinctrl_state *lcmbias_enable;
extern struct pinctrl_state *lcmbias_disable;
struct LCM_setting_table {
	unsigned cmd;
	unsigned char count;
	unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

	/*
	   Note :

	   Data ID will depends on the following rule.

	   count of parameters > 1      => Data ID = 0x39
	   count of parameters = 1      => Data ID = 0x15
	   count of parameters = 0      => Data ID = 0x05

	   Structure Format :

	   {DCS command, count of parameters, {parameter list}}
	   {REGFLAG_DELAY, milliseconds of time, {}},

	   ...

	   Setting ending by predefined flag

	   {REGFLAG_END_OF_TABLE, 0x00, {}}
	 */

	{ 0x00, 1, {0x00} },
	{ 0xFF, 3, {0x12, 0x89, 0x01} },
	
	{ 0x00, 1, {0x80} },
	{ 0xFF, 2, {0x12, 0x89} },
	
	{ 0x00, 1, {0x90} },
	{ 0xFF, 1, {0xB0} },
 
	{ 0x00, 1, {0x80} },
	{ 0xC0, 8, {0x4A, 0x00, 0x10, 0x10, 0x96, 0x01, 0x68, 0x40} },
	{ 0x00, 1, {0x90} },
	{ 0xC0, 3, {0x3B, 0x01, 0x09} },

	{ 0x00, 1, {0x8B} },
	{ 0xC0, 1, {0x04} },
	
	{ 0x00, 1, {0x8C} },
	{ 0xC0, 1, {0x00} },
	
	{ 0x00, 1, {0x80} },
	{ 0xC1, 1, {0x33} },
	
	{ 0x00, 1, {0x85} },
	{ 0xC5, 3, {0x0A, 0x0A, 0x36} },
	
	{ 0x00, 1, {0x00} },
	{ 0xD8, 2, {0x2E, 0x02E} },
	
	{ 0x00, 1, {0x95} },
	{ 0xC5, 1, {0x27} },

/*	
	{ 0x00, 1, {0x00} },
	{ 0xD9, 1, {0x80} },
	
	{ 0x00, 1, {0x01} },
	{ 0xD9, 1, {0x94} },
*/	
	{ 0x00, 1, {0x84} },
	{ 0xC4, 1, {0x02} },

	/* Oscillator frequency should be 60Hz(0x55) for command mode
	 * but for video mode, the frequency should be greater than 60Hz.
	 * Use 65Hz(0x66) for video mode
	 * Remark: 60Hz(0x55), 65Hz(0x66), 70Hz(0x77) */


	{ 0x00, 1, {0x93} },
	{ 0xC4, 1, {0x04} },
	
	{ 0x00, 1, {0x96} },
	{ 0xF5, 1, {0xE7} },
	
	{ 0x00, 1, {0xA0} },
	{ 0xF5, 1, {0x4A} },
	
	{ 0x00, 1, {0x8A} },
	{ 0xC0, 1, {0x11} },
	
	{ 0x00, 1, {0x83} },
	{ 0xF5, 1, {0x81} },
	
	{ 0x00, 1, {0x90} },
	{ 0xC4, 2, {0x90, 0x05} },
	
	{ 0x00, 1, {0x80} },
	{ 0xCB, 15, {0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 0x00, 1, {0x90} },
	{ 0xCB, 7, {0x14, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	
	{ 0x00, 1, {0x80} },
	{ 0xCC, 14, {0x1E, 0x1d, 0x19, 0x1a, 0x0a, 0x0c, 0x0e, 0x10, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 0x00, 1, {0x90} },
	{ 0xCC, 15, {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1e, 0x1d, 0x19, 0x1a, 0x09, 0x0b, 0x0d, 0x0f} },
	{ 0x00, 1, {0xA0} },
	{ 0xCC, 13, {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 0x00, 1, {0xB0} },
	{ 0xCC, 14, {0x1d, 0x1e, 0x19, 0x1a, 0x0f, 0x0d, 0x0b, 0x09, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00} },
	{ 0x00, 1, {0xC0} },
	{ 0xCC, 15, {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1d, 0x1e, 0x19, 0x1a, 0x10, 0x0e, 0x0c, 0x0a} },
	{ 0x00, 1, {0xD0} },
	{ 0xCC, 13, {0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00} },
	
	{ 0x00, 1, {0x80} },
	{ 0xCE, 6, {0x85, 0x03, 0x00, 0x84, 0x89, 0x88} },	
	{ 0x00, 1, {0x90} },
	{ 0xCE, 9, {0x34, 0xfd, 0x00, 0x04, 0xff, 0x04, 0xfe, 0x04, 0xff} },
	{ 0x00, 1, {0xA0} },
	{ 0xCE, 15, {0x30, 0x83, 0x86, 0x00, 0x00, 0x00, 0x82, 0x85, 0x00, 0x81, 0x84, 0x00, 0x80, 0x83, 0x00} },
	{ 0x00, 1, {0xB0} },
	{ 0xCE, 15, {0x30, 0x00, 0x82, 0x00, 0x00, 0x00, 0x01, 0x81, 0x00, 0x02, 0x00, 0x00, 0x03, 0x01, 0x00} },
	{ 0x00, 1, {0xF0} },
	{ 0xCE, 6, {0x3a, 0x15, 0x01, 0x80, 0x10, 0x00} },
	
	{ 0x00, 1, {0x00} },
	{ 0xE1, 16, {0x08, 0x17, 0x23, 0x33, 0x41, 0x5F, 0x5E, 0x75, 0x80, 0x6D, 0x75, 0x5B, 0x42, 0x2B, 0x1A, 0x08} },
	{ 0x00, 1, {0x00} },
	{ 0xE2, 16, {0x08, 0x17, 0x23, 0x33, 0x41, 0x5F, 0x5E, 0x75, 0x80, 0x6D, 0x75, 0x5B, 0x42, 0x2B, 0x1A, 0x08} },
	
	{ 0x00, 1, {0x83} },
	{ 0xC5, 1, {0x55} },
	
	{ 0x00, 1, {0x81} },
	{ 0xC4, 1, {0xE9} },
	
	{ 0x00, 1, {0x90} },
	{ 0xC4, 2, {0x96, 0X05} },
	
	{ 0x00, 1, {0x00} },
	{ 0xFF, 3, {0xFF, 0xFF, 0xFF,} },
	{ 0x35, 1, {0x00} },	

	{ 0x11, 0, {} },
	{ REGFLAG_DELAY, 120, {} },
	//{ 0x35, 1, {0} },
	{ 0x29, 0, {} },
	{ REGFLAG_DELAY, 40, {} },


	/* Note */
	/* Strongly recommend not to set Sleep out, Display On here. */
	/* That will cause messed frame to be shown as later the backlight is on. */


	/* Setting ending by predefined flag */
	//{ REGFLAG_END_OF_TABLE, 0x00, {} }
};

#if 0
static struct LCM_setting_table lcm_sleep_out_setting[] = {
	/* Sleep Out */
	{ 0x11, 1, {0x00} },
	{ REGFLAG_DELAY, 20, {} },

	/* Display ON */
	{ 0x35, 0, {} },
	{ REGFLAG_DELAY, 120, {} },
	{ REGFLAG_END_OF_TABLE, 0x00, {} }
};
#endif

static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    {0x28, 0, {}},
    {REGFLAG_DELAY, 200, {}},
    {0x10, 0, {}},
    {REGFLAG_DELAY, 100, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};
static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {

		unsigned cmd;

		cmd = table[i].cmd;

		switch (cmd) {

		case REGFLAG_DELAY:
			MDELAY(table[i].count);
			break;

		case REGFLAG_END_OF_TABLE:
			break;

		default:
			dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}

}



/* --------------------------------------------------------------------------- */
/* LCM Driver Implementations */
/* --------------------------------------------------------------------------- */

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->physical_width = 65;
	params->physical_height = 115 ;
	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	/* enable tearing-free */
	params->dbi.te_mode = LCM_DBI_TE_MODE_VSYNC_ONLY;
	params->dbi.te_edge_polarity = LCM_POLARITY_RISING;


	params->dsi.mode = SYNC_PULSE_VDO_MODE;


	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	/* Not support in MT6573 */

	//params->dsi.DSI_WMEM_CONTI = 0x3C;
	//params->dsi.DSI_RMEM_CONTI = 0x3E;



	/* Video mode setting */
	params->dsi.intermediat_buffer_num = 2;

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;
	params->dsi.packet_size = 256;


	params->dsi.vertical_sync_active = 10;
	params->dsi.vertical_backporch = 40;
	params->dsi.vertical_frontporch = 40;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 10;
	params->dsi.horizontal_backporch = 30;
	params->dsi.horizontal_frontporch = 30;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	/* Bit rate calculation */
	params->dsi.HS_TRAIL=15;
	params->dsi.PLL_CLOCK = 212;


	params->dsi.cont_clock=1;   
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0A;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
	
	params->dsi.lcm_esd_check_table[1].cmd = 0x0E;
	params->dsi.lcm_esd_check_table[1].count = 1;
	params->dsi.lcm_esd_check_table[1].para_list[0] = 0x80;
}



static void lcm_init(void)
{
#ifdef GPIO_LCD_BIAS_ENP_PIN
	mt_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
#endif
	pinctrl_select_state(lcmbiasctrl, lcmbias_enable); 
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(200);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(120);

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}



static void lcm_suspend(void)
{
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);

	#ifdef GPIO_LCD_BIAS_ENP_PIN
	mt_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
// zhaozhensen@wind-mobi.com 20160322 begin
	mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ZERO);
// zhaozhensen@wind-mobi.com 20160322 end
#endif
	pinctrl_select_state(lcmbiasctrl, lcmbias_disable); 
	MDELAY(10);
	MDELAY(10);


#ifdef BUILD_LK
	printf("uboot %s\n", __func__);
#else
	pr_debug("kernel %s\n", __func__);
#endif
}


static void lcm_resume(void)
{
#ifdef BUILD_LK
	printf("uboot %s\n", __func__);
#else
	pr_debug("kernel %s\n", __func__);
#endif
/* push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1); */
	lcm_init();
}
static unsigned int lcm_compare_id(void)
{
	unsigned int id=0,id1=0;
	unsigned char buffer[3];
	unsigned int data_array[16];  
	// pinctrl_select_state(lcmbiasctrl, lcmbias_enable); 
#ifdef GPIO_LCD_BIAS_ENP_PIN
	mt_set_gpio_mode(GPIO_LCD_BIAS_ENP_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_LCD_BIAS_ENP_PIN, GPIO_DIR_OUT);
	mt_set_gpio_out(GPIO_LCD_BIAS_ENP_PIN, GPIO_OUT_ONE);
#endif
	SET_RESET_PIN(1);  //NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50); 

//	data_array[1]=0x00001500;
//	dsi_set_cmdq(data_array, 1, 1);
//	data_array[0]=0x00043902;
//	data_array[1]=0x018912ff;
//	dsi_set_cmdq(data_array, 2, 1);
//	MDELAY(10);

	data_array[0]=0x00023700; 
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xDA, buffer, 1);
	id1= buffer[0]; //should be 0x01


	id=id1;

#if defined(BUILD_LK)||defined(BUILD_UBOOT)
	printf(" LK otm1289a txd  debug: %s id=%x id1=%x id2=%x \n",__func__,id, id1, id2);
#else
	printk(" kernel otm1289a txd horse debug: %s id=%x  \n",__func__,id);
#endif	
	
//	return 1;
	if(LCM_ID_OTM1289A==id)
		return 1;
	else
		return 0;
}

extern atomic_t ESDCheck_byCPU;
static unsigned int lcm_ata_check(unsigned char *buf)		
{
#ifndef BUILD_LK
	unsigned int id=0,id1=0;
	unsigned char buffer[3];
	unsigned int data_array[16]; 

	data_array[1]=0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);	

    atomic_set(&ESDCheck_byCPU,1);
    read_reg_v2(0xDA, buffer, 1);
	atomic_set(&ESDCheck_byCPU,0);
	id1= buffer[0]; //should be 0x01
	id=id1;
#if defined(BUILD_LK)||defined(BUILD_UBOOT)
	printf(" LK otm1289a txd  debug: %s id=%x id1=%x id2=%x \n",__func__,id, id1, id2);
#else
	printk(" kernel otm1289a txd horse debug: %s id=%x  \n",__func__,id);
#endif		
	if(LCM_ID_OTM1289A==id)
		return 1;
	else
		return 0;	
#else 
		return 0;
#endif
}


LCM_DRIVER otm1289a_hsd_dsi_vdo_lcm_drv = {

	.name = "otm1289a_hsd_dsi_vdo",
	.set_util_funcs = lcm_set_util_funcs,
	.compare_id = lcm_compare_id,
	.get_params = lcm_get_params,
	.init = lcm_init,
	.suspend = lcm_suspend,
	.resume = lcm_resume,
	.ata_check = lcm_ata_check,
};
