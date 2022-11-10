#include <stdio.h>
#include "ADXL345.h"
#include <math.h>

ADXL345 accelerometer;
int interrupt_pin = 0;

sensors_event_t sensEvent;

uint tmr = 0;
const uint maxTmr = 200;

uint tmr2 = 0;
const uint maxTmr2 = 800;

void accelerometer_interrupt_handler(uint gpio, uint32_t events) {
    printf("interrupt! reasons: 0x%x\n",
     accelerometer.getInterruptSources()); // get interrupt reasons and clear latched motion interrupts
}

int main()
{
    stdio_init_all();

    const uint LED_PIN = PICO_DEFAULT_LED_PIN;
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);

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

    bool blink = false;

    bool leden = 1;

    while (true) {
        tmr++;
        if (blink){
            tmr2++;
        }
        accelerometer.getEvent(&sensEvent);

        //sensEvent.acceleration;
        float x = abs(sensEvent.acceleration.x);
        float y = abs(sensEvent.acceleration.y);
        float z = abs(sensEvent.acceleration.z);

        float max = MAX(MAX(x, y), z);

        if (max>15){
            blink = true;
            leden = 0;
            tmr2 = 0;
        }

        if (tmr>maxTmr){
            tmr = 0;
            if (blink) leden = !leden;
        }
        if (tmr2>maxTmr2){
            tmr2 = 0;
            blink = false;
            leden = 1;
        }

        gpio_put(LED_PIN, leden);

        
        /*printf("X: %f Y: %f Z: %f\n", 
            // note: these are raw values, to get real gravitational force, you can use accelerometer.getEvent()
            //accelerometer.getX(),
            //accelerometer.getY(),
            //accelerometer.getZ(),

            sensEvent.acceleration.x,
            sensEvent.acceleration.y,
            sensEvent.acceleration.z
        );*/
        printf("acc: %f\n", max);

        sleep_ms(100);
    }
}