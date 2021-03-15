/*
 * overcurrent-protection.c
 *
 * Created: 25-Feb-21 15:47:49
 * Author : M19939
 */ 

#define F_CPU 4000000ul
#define CLK_PER 4000000ul
#define OPAMP_MAX_SETTLE_TIME 0x7F
#define VREF_AC_MV 4096
#define AC_TRIGGER_VOLTAGE_MV_INIT 1100

#include <avr/io.h>
#include <math.h>
#include <util/delay.h>
#include <avr/cpufunc.h>
#include <avr/interrupt.h>


void opamp_init(void);
void tcd_init(void);
void ac_init(void);
void fan_start(void);
void button_init(void);
void ac_trig_event_enable(void);
void ccl_init(void);

int main(void)
{
	opamp_init();
	tcd_init();
	ac_init();
	ccl_init();
	fan_start();
	button_init();
	PORTC.DIRSET = PIN4_bm | PIN5_bm;
	PORTC.OUTCLR = PIN4_bm | PIN5_bm;
	PORTD.DIRSET = PIN2_bm;
	sei();
	while (1)
	{
	}
}

void opamp_init(void)
{
	/*Disable input on op amp output pin*/
	PORTD.PIN5CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTE.PIN2CTRL = PORT_ISC_INPUT_DISABLE_gc;
	PORTE.PIN1CTRL = PORT_ISC_INPUT_DISABLE_gc;
	
	/*Set up op amp*/
	OPAMP.CTRLA = OPAMP_ENABLE_bm;
	OPAMP.TIMEBASE = (uint8_t) ceil(CLK_PER*0.000001)-1; /*Number of peripheral clock cycles that amounts to 1us*/
	
	//OP2 setup
	OPAMP.OP2CTRLA = OPAMP_RUNSTBY_bm | OPAMP_ALWAYSON_bm | OPAMP_OP2CTRLA_OUTMODE_NORMAL_gc;
	OPAMP.OP2SETTLE = OPAMP_MAX_SETTLE_TIME; //As the settle time is unknown, the maximums should be set
	OPAMP.OP2INMUX = OPAMP_OP2INMUX_MUXNEG_WIP_gc | OPAMP_OP2INMUX_MUXPOS_LINKOUT_gc;
	OPAMP.OP2RESMUX = OPAMP_OP2RESMUX_MUXWIP_WIP5_gc | OPAMP_OP2RESMUX_MUXBOT_GND_gc|OPAMP_OP2RESMUX_MUXTOP_OUT_gc;
	
	//OP1 setup
	OPAMP.OP1CTRLA = OPAMP_RUNSTBY_bm | OPAMP_ALWAYSON_bm | OPAMP_OP1CTRLA_OUTMODE_NORMAL_gc;
	OPAMP.OP1SETTLE = OPAMP_MAX_SETTLE_TIME; //As the settle time is unknown, the maximums should be set
	OPAMP.OP1INMUX = OPAMP_OP1INMUX_MUXNEG_WIP_gc | OPAMP_OP1INMUX_MUXPOS_INP_gc;
	OPAMP.OP1RESMUX = OPAMP_OP1RESMUX_MUXWIP_WIP7_gc | OPAMP_OP1RESMUX_MUXBOT_INN_gc | OPAMP_OP1RESMUX_MUXTOP_OUT_gc;
}

void ccl_init(void)
{
	CCL.SEQCTRL0 = CCL_SEQSEL_RS_gc; // Create a RS latch
	/*Set up LUT0*/
	CCL.LUT0CTRLB = CCL_INSEL0_AC0_gc; // AC0 and mask all other bits
	CCL.TRUTH0 = 0x02; // When AC is hight hight output, otherwise low output
	CCL.LUT0CTRLA = CCL_ENABLE_bm | CCL_OUTEN_bm;
	PORTA.DIRSET = PIN3_bm;
	
	/*Set up LUT1*/
	EVSYS.USERCCLLUT1A = EVSYS_USER_CHANNEL1_gc;
	CCL.LUT1CTRLB = CCL_INSEL0_EVENTA_gc; // Event A as input and mask all other bits
	CCL.TRUTH1 = 0x02; // When Event A is hight hight output, otherwise low output
	CCL.LUT1CTRLA = CCL_ENABLE_bm;
	
	CCL.CTRLA = CCL_ENABLE_bm;
}

void tcd_init(void)
{
	PORTMUX.TCDROUTEA = PORTMUX_TCD0_ALT2_gc;
	PORTF.DIRSET=PIN0_bm;
	
	TCD0.CTRLB = TCD_WGMODE_TWORAMP_gc;
	
	TCD0.CMPASET = 0; /* Compare A Set: 0 */
	
	TCD0.CMPACLR = 1000; /* Compare A Clear: 1000 */

	TCD0.CMPBSET = 0; /* Compare B Set: 0 */

	TCD0.CMPBCLR = 1000; /* Compare B Clear: 1000 */

	ccp_write_io((void*)&(TCD0.FAULTCTRL),1 << TCD_CMPAEN_bp /* Compare A enable: enabled */
	| 0 << TCD_CMPA_bp /* Compare A value: disabled */
	| 0 << TCD_CMPB_bp /* Compare B value: disabled */
	| 0 << TCD_CMPBEN_bp /* Compare B enable: disabled */
	| 0 << TCD_CMPC_bp /* Compare C value: disabled */
	| 0 << TCD_CMPCEN_bp /* Compare C enable: disabled */
	| 0 << TCD_CMPD_bp /* Compare D vaule: disabled */
	| 0 << TCD_CMPDEN_bp /* Compare D enable: disabled */);

	TCD0.EVCTRLA
	= TCD_CFG_ASYNC_gc     /* Asynchronous Event output qualification enabled */
	| TCD_ACTION_FAULT_gc  /* Event trigger a fault */
	| TCD_EDGE_RISE_HIGH_gc /* The rising edge or high level of event generates retrigger or fault action */
	| 1 << TCD_TRIGEI_bp;  /* Trigger event enable: enabled */

	TCD0.INPUTCTRLA=TCD_INPUTMODE_WAITSW_gc; //Wait for a reset command
	TCD0.INPUTCTRLB=TCD_INPUTMODE_WAITSW_gc; //Wait for a reset command
	
	while ((TCD0.STATUS & TCD_ENRDY_bm) == 0); // Wait for Enable Ready to be high.

	TCD0.CTRLA = 0 << TCD_ENABLE_bp      /* Enable: disabled */
	| TCD_CLKSEL_OSCHF_gc   /*  */
	| TCD_CNTPRES_DIV1_gc   /* Sync clock divided by 1 */
	| TCD_SYNCPRES_DIV1_gc;
	

}

void tcd_enable(void)
{
	TCD0.CTRLA = TCD_ENABLE_bm;
	TCD0.CTRLE = TCD_RESTART_bm;
}

uint8_t ac_calculate_trigger_voltage(uint16_t mV)
{
	uint8_t triggerVoltage = (((uint32_t)mV*256)/VREF_AC_MV);
	return triggerVoltage;
}

void ac_init(void)
{
	PORTA.DIRSET=PIN7_bm;
	VREF.ACREF = VREF_REFSEL_4V096_gc;

	AC0.MUXCTRL = 0 << AC_INVERT_bp     /* Invert AC Output: disabled */
	| AC_MUXNEG_DACREF_gc /* DAC Reference */
	| AC_MUXPOS_AINP2_gc; /* Positive Pin 2 */


	AC0.DACREF = ac_calculate_trigger_voltage(AC_TRIGGER_VOLTAGE_MV_INIT); /* DAC Voltage Reference: 0x64 */

	AC0.CTRLA = 1 << AC_ENABLE_bp /* Enable: enabled */
	| AC_HYSMODE_NONE_gc   /* No hysteresis */
	| AC_POWER_PROFILE0_gc /* Power profile 0, lowest consumption and highest response time. */
	| 1 << AC_OUTEN_bp     /* Output Buffer Enable: enabled */
	| 0 << AC_RUNSTDBY_bp; /* Run in Standby Mode: disabled */
}

void ac_calibration(void)
{
	PORTB.DIRSET = PIN3_bm;
	PORTB.OUTCLR = PIN3_bm;
	uint8_t calibrating = 1;
	uint16_t ac_trigger_voltage_mv=AC_TRIGGER_VOLTAGE_MV_INIT;
	AC0.INTCTRL = AC_INTMODE_NORMAL_POSEDGE_gc;
	while (calibrating)
	{
		ac_trigger_voltage_mv += 50;
		AC0.DACREF = ac_calculate_trigger_voltage(ac_trigger_voltage_mv);
		_delay_ms(100);//Allow some time for the flag to be raised
		if (!(AC0.STATUS & AC_CMPIF_bm))
		{
			calibrating=0;
			PORTB.OUTSET =PIN3_bm;
		}
		AC0.STATUS=AC_CMPIF_bm;
	}
	
}

void ac_trig_event_enable(void)
{
	AC0.INTCTRL = AC_INTMODE_NORMAL_POSEDGE_gc | AC_CMP_bm;
	EVSYS.SWEVENTA = EVSYS_SWEVENTA_CH1_gc;
	EVSYS.CHANNEL0=EVSYS_CHANNEL0_CCL_LUT0_gc; //Output from the RS latch as trigger
	EVSYS.USERTCD0INPUTA=EVSYS_USER_CHANNEL0_gc;
	EVSYS.USEREVSYSEVOUTD = EVSYS_USER_CHANNEL0_gc;
}

void ac_trig_event_disable(void)
{
	AC0.INTCTRL &= ~AC_CMP_bm;
	EVSYS.CHANNEL0=EVSYS_CHANNEL0_OFF_gc;
	EVSYS.USERTCD0INPUTA = EVSYS_USER_OFF_gc;
	EVSYS.USEREVSYSEVOUTD = EVSYS_USER_OFF_gc;
}

void fan_start(void)
{
	ac_trig_event_disable();
	tcd_enable();
	ac_calibration();
	_delay_ms(1000);
	ac_trig_event_enable();

}

void button_init(void)
{
	PORTB.DIRCLR=PIN2_bm;
	PORTB.PIN2CTRL=PORT_ISC_FALLING_gc | PORT_PULLUPEN_bm;
	sei();
}

ISR(PORTB_PORT_vect)
{
	fan_start();
	PORTB.INTFLAGS=0xff;
}

ISR(AC0_AC_vect)
{
	AC0.STATUS = AC_CMPIF_bm; // Clear interrupt flag
	PORTC.OUTTGL = PIN4_bm;
}