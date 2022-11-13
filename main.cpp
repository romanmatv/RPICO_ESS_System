#include <stdio.h>
#include "ADXL345.h"
#include <math.h>

ADXL345 accelerometer;
int interrupt_pin = 0;

sensors_event_t sensEvent;

/*uint tmr = 0;
const uint maxTmr = 200;

uint tmr2 = 0;
const uint maxTmr2 = 800;*/

//флаг мигания
bool blink = false;
//Пин светодиода
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
//Флаг вкл/выкл светодиода
bool leden = 1;
//Порог акселерометра, при котором срабатывает мигалка
const uint gValueBlink = 15;

struct repeating_timer timer;

void accelerometer_interrupt_handler(uint gpio, uint32_t events) {
    printf("interrupt! reasons: 0x%x\n",
     accelerometer.getInterruptSources()); // get interrupt reasons and clear latched motion interrupts
}

//Функция таймера блинка светодиода
bool blink_timer(struct repeating_timer *t) {
    if (blink) leden = !leden;
    gpio_put(LED_PIN, leden);
    return true;
}

//Функция таймера отключения блинка
int64_t blinkoff_callback(alarm_id_t id, void *user_data) {
    printf("BLINK OFF\n");
    blink = false;
    leden = true;
    // Can return a value here in us to fire in the future
    cancel_repeating_timer(&timer);
    gpio_put(LED_PIN, 1);
    return 0;
}

int main()
{
    stdio_init_all();

    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    
    gpio_put(LED_PIN, 1);

    accelerometer = ADXL345();
    accelerometer.begin(ADXL345_DEFAULT_ADDRESS, // I2C address
                    i2c_default, // Pico I2C bus (0 is default)
                    PICO_DEFAULT_I2C_SDA_PIN, // SDA pin (4 is default)
                    PICO_DEFAULT_I2C_SCL_PIN); // SCL pin (5 is default)

    accelerometer.setRange(ADXL345_RANGE_2_G); // set 16 g range
    accelerometer.setFreefallInterruptTime(350); // 350 ms minimum
    accelerometer.setFreefallInterruptTreshold(600); // below 600 mg
    accelerometer.setInterrupt(ADXL345_INT_FREE_FALL, true); // enable free fall interrupt

    gpio_set_irq_enabled_with_callback(interrupt_pin, GPIO_IRQ_EDGE_RISE, true, &accelerometer_interrupt_handler);


    while (true) {

        accelerometer.getEvent(&sensEvent);

        //sensEvent.acceleration;
        float x = abs(sensEvent.acceleration.x);
        float y = abs(sensEvent.acceleration.y);
        float z = abs(sensEvent.acceleration.z);

        float max = MAX(MAX(x, y), z);

        /* 
        TODO: 
            устранить баг множественного назначения таймеров
            Таймер блинка тупо не назначать, если он уже есть
            А таймер остановки блинка - обнулять
            А вообще, все это можно заменить одним таймером, который будет сам обнуляться после n раз равное sec_stop / sec_blink
        */
        if (max>gValueBlink){
            printf("START BLINK\n");
            blink = true;
            // Call alarm_callback in 2 seconds
            add_alarm_in_ms(5000, blinkoff_callback, NULL, false);
            add_repeating_timer_ms(300, blink_timer, NULL, &timer);
        }

        printf("acc: %f\n", max);

        sleep_ms(100);
    }
}