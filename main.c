#include "EPD_Test.h"   // Examples
#include "run_File.h"

#include "led.h"
#include "waveshare_PCF85063.h" // RTC
#include "DEV_Config.h"

#include <time.h>
#include "pico/low_power.h"
#include "pico/aon_timer.h"
extern const char *fileList;
extern char pathName[];

#define enChargingRtc 0

// How long to wait
#ifndef AWAKE_TIME_MS
#define AWAKE_TIME_MS 10000
#endif
#ifndef SLEEP_TIME_MS
#define SLEEP_TIME_MS 10000
#endif

#define SLEEP_TIME_1MINUTE_MS 60*1000
#define SLEEP_TIME_15MINUTE_MS 15*60*1000

static uint32_t __persistent_data(run_count);
static bool __persistent_data(aon_timer_started);

/*
Mode 0: Automatically get pic folder names and sort them
Mode 1: Automatically get pic folder names but not sorted
Mode 2: pic folder name is not automatically obtained, users need to create fileList.txt file and write the picture name in TF card by themselves
*/
#define Mode 0


float measureVBAT(void)
{
    float Voltage=0.0;
    const float conversion_factor = 3.3f / (1 << 12);
    uint16_t result = adc_read();
    Voltage = result * conversion_factor * 3;
    printf("Raw value: 0x%03x, voltage: %f V\n", result, Voltage);
    return Voltage;
}

void chargeState_callback() 
{
    if(DEV_Digital_Read(VBUS)) {
        if(!DEV_Digital_Read(CHARGE_STATE)) {  // is charging
            ledCharging();
        }
        else {  // charge complete
            ledCharged();
        }
    }
}

void run_display(Time_data Time, Time_data alarmTime, char hasCard)
{
    if(hasCard) {
        setFilePath();
        EPD_7in3f_display_BMP(pathName, measureVBAT());   // display bmp
    }
    else {
        EPD_7in3f_display(measureVBAT());
    }

    PCF85063_clear_alarm_flag();    // clear RTC alarm flag
    rtcRunAlarm(Time, alarmTime);  // RTC run alarm
}

void timer_set() {
    // Must start the aon timer if needed
    printf("Current time: %llu\n", aon_timer_get_absolute_time());
    if (!aon_timer_started) {
        low_power_start_aon_timer_at_time_ms(1776858754000);
        aon_timer_started = true;
    }
#if AWAKE_TIME_MS < 10000
    // pause for at least 10s to allow the debugger to attach on power up to allow the device to be re-programmed
    printf("Waiting a bit to allow debugger to attach\n");
    sleep_ms(10000 - AWAKE_TIME_MS);
#endif
    //hard_assert(status_led_init());
    //status_led_set_state(true);

    // Scratch register survives power down
    printf("Wake up, test run: %u\n", run_count++);

    // Stay awake for a few seconds
    printf("Awake for %dms\n", AWAKE_TIME_MS);
    sleep_ms(AWAKE_TIME_MS);

    // power off
    printf("Low power for %dms\n", SLEEP_TIME_1MINUTE_MS);
    //status_led_set_state(false);
    int rc = low_power_pstate_for_ms(SLEEP_TIME_1MINUTE_MS, NULL, NULL);
    //status_led_set_state(true);
    printf("low_power_pstate_for_ms returned error %d\n", rc);
    hard_assert(false); // should never get here!
}

int main(void)
{
    Time_data Time = {2024-2000, 3, 31, 0, 0, 0};
    Time_data alarmTime = Time;
    // alarmTime.seconds += 10;
    alarmTime.minutes += 3;
    // alarmTime.hours +=1;
    char isCard = 0;
  
    printf("Init...\r\n");
    if(DEV_Module_Init() != 0) {  // DEV init
        return -1;
    }
    
    watchdog_enable(8*1000, 1);    // 8s
    DEV_Delay_ms(1000);
    PCF85063_init();    // RTC init
    rtcRunAlarm(Time, alarmTime);  // RTC run alarm
    gpio_set_irq_enabled_with_callback(CHARGE_STATE, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, chargeState_callback);
    /*
    if(measureVBAT() < 3.1) {   // battery power is low
        printf("low power ...\r\n");
        PCF85063_alarm_Time_Disable();
        ledLowPower();  // LED flash for Low power
        powerOff(); // BAT off
        return 0;
    }
    else {
        printf("work ...\r\n");
        ledPowerOn();
    }
    */
    if(!sdTest()) 
    {
        isCard = 1;
        if(Mode == 0)
        {
            sdScanDir();
            file_sort();
        }
        if(Mode == 1)
        {
            sdScanDir();
        }
        if(Mode == 2)
        {
            file_cat();
        }
        
    }
    else 
    {
        isCard = 0;
    }

    if(!DEV_Digital_Read(VBUS)) {    // no charge state
        run_display(Time, alarmTime, isCard);
    }
    else {  // charge state
        chargeState_callback();
        while(DEV_Digital_Read(VBUS)) {
            measureVBAT();
            
            #if enChargingRtc
            if(!DEV_Digital_Read(RTC_INT)) {    // RTC interrupt trigger
                printf("rtc interrupt\r\n");
                run_display(Time, alarmTime, isCard);
            }
            #endif

            if(!DEV_Digital_Read(BAT_STATE)) {  // KEY pressed
                printf("key interrupt\r\n");
                run_display(Time, alarmTime, isCard);
            }
            DEV_Delay_ms(200);
        }
    }
    
    watchdog_disable();
    printf("watch dog disabled and power off ...\r\n");
    timer_set();
    powerOff(); // BAT off

    return 0;
}
