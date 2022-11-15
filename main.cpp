/*
ESS система на основе Raspberry Pi Pico by romanmatv (c)2022

На данный момент работа происходит со встроенным светодиодом

LED_PIN - пин с логическим 1/0 - для включения/выключения света

gValueBlink - порог срабатывания системы

msBlink - частота мигания в мсек

sBlinkStop - через сколько секунд отключается мигание после последнего порога срабатывания
*/

#include <stdio.h>
#include "ADXL345.h"
#include <math.h>

/* ОСНОВНЫЕ ПАРАМЕТРЫ */
//Пин светодиода
const uint LED_PIN = PICO_DEFAULT_LED_PIN;
//Порог акселерометра, при котором срабатывает мигалка
const uint gValueBlink = 15;
//переодичность мигания в мс
const uint msBlink = 200;
//через сколько секунд отключать мигалку
const uint sBlinkStop = 7;

/* ПРОЧИЕ ПЕРЕМЕННЫЕ И КОНСТАНТЫ*/
//Флаг вкл/выкл светодиода
bool leden = 1;
//Через сколько раз мигания она останавливается
const uint cBlinkStop = (sBlinkStop * 1000) / msBlink;
//колличество сделанных миганий
uint countBlinks = 0;
//переменная таймера
struct repeating_timer timer;

/*Переменные акселерометра*/
ADXL345 accelerometer;
int interrupt_pin = 0;
sensors_event_t sensEvent;

void accelerometer_interrupt_handler(uint gpio, uint32_t events) {
    printf("interrupt! reasons: 0x%x\n",
     accelerometer.getInterruptSources()); // get interrupt reasons and clear latched motion interrupts
}

//Функция таймера блинка светодиода
bool blink_timer(struct repeating_timer *t) {
    countBlinks++;
    if (countBlinks>cBlinkStop){
        //Отключаем таймер мигания
        gpio_put(LED_PIN, 1);
        cancel_repeating_timer(&timer);
    }else{
        leden = !leden;
        gpio_put(LED_PIN, leden);
    }
    return true;
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

        if (max>gValueBlink){
            //printf("START BLINK\n");
            countBlinks = 0;
            if (!timer.alarm_id){
                add_repeating_timer_ms(msBlink, blink_timer, NULL, &timer);
            }
        }

        //printf("acc: %f\n", max);sleep_ms(100);
        //sleep_ms(100);
    }
}