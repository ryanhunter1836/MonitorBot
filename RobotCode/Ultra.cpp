#define TRIGGER BLUE_GP0_PIN_3
#include <iostream>
#include <string>
using namespace std;

extern "C" {
        #include <roboticscape.h>
        #include <rc_usefulincludes.h> 
        int rc_initialize();
        int rc_cleanup();
        void rc_usleep(unsigned int ns);
        int rc_gpio_export(unsigned int gpio);
        int rc_gpio_set_dir(int gpio, rc_pin_direction_t dir);
        int rc_gpio_set_value(unsigned int gpio, int value);
        int rc_gpio_get_value(unsigned int gpio);
        int rc_set_cpu_freq(rc_cpu_freq_t freq);
    }
    
int main()
{
    /*
    Send pulse
    Wait for rising edge
    Get start time
    Wait for falling edge
    Get stop edge
    Calculate difference (amount of time pin was high)
    
    rc_initialize();
    rc_set_cpu_freq(FREQ_1000MHZ);
    rc_set_state(RUNNING);
    rc_gpio_export(TRIGGER);
    rc_gpio_set_dir(TRIGGER, OUTPUT_PIN);
    rc_gpio_set_value(TRIGGER, LOW);
    //const unsigned long timeout = 20000000;
    //while(rc_get_state()!=EXITING)
   //{
    rc_gpio_set_value(TRIGGER, HIGH);
    rc_usleep(500);
    rc_gpio_set_value(TRIGGER, LOW);
    rc_gpio_set_dir(TRIGGER, INPUT_PIN);
    
    while(rc_gpio_get_value(TRIGGER) == LOW);
    long startTime = rc_nanos_since_epoch();
    //long start = rc_nanos_since_epoch();
    
    while(rc_gpio_get_value(TRIGGER) == HIGH);
    //long startTime = rc_nanos_since_epoch();
    long travelTime = rc_nanos_since_epoch() - startTime;
    //cout << rc_gpio_get_value(TRIGGER) << endl;
    
    double distance = travelTime / 149284.0;
    //printf("%6d\n", travelTime);
    cout << distance <<endl;
    //} 
    rc_gpio_set_value(TRIGGER, LOW);
    rc_gpio_unexport(TRIGGER);
    rc_cleanup(); */
    cout << "Hello World" < endl;
    return -1;
}