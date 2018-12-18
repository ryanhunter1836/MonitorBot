//There may be more resistance on one side of the robot than the other, causing it to stray to one direction
//This program adjusts the power on each motor to make it drive in a stright line

#include <iostream>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <chrono>

using namespace std;

extern "C" {
        #include <roboticscape.h>
        #include <rc_usefulincludes.h> 
        int rc_initialize();
        int rc_cleanup();
        int rc_set_cpu_freq(rc_cpu_freq_t freq); 
        void rc_usleep(unsigned int us);
        uint64_t rc_nanos_since_boot();
        float rc_battery_voltage();
        int rc_get_encoder_pos(int ch);
        int rc_set_encoder_pos(int ch, int value);
        int rc_enable_motors();
        int rc_set_motor(int motor, float FORWARD);
        int rc_gpio_export(unsigned int gpio);
        int rc_gpio_set_dir(int gpio, rc_pin_direction_t dir);
        int rc_gpio_set_value(unsigned int gpio, int value);
        int rc_gpio_get_value(unsigned int gpio);
    }
    
int main()
{
    float rightSpeed = .7;
    float leftSpeed = .8;
    int rightCount = 0;
    int leftCount = 0;
    int difference = 1000;
    rc_initialize();
    rc_enable_motors();
    while(difference > 5  && (rightSpeed < 1) && (leftSpeed < 1))
    {
        while((rc_get_encoder_pos(1) < 2000))
        {
            rc_set_motor(1, rightSpeed);
            rc_set_motor(2, leftSpeed);
        }
        rightCount = rc_get_encoder_pos(1);
        leftCount = rc_get_encoder_pos(2);
        difference = abs(rightCount - leftCount);
        if(difference > 10)
        {
            if(rightCount > leftCount)
            {
                leftSpeed += .01;
            }
            if(leftCount > rightCount)
            {
                rightSpeed += .01;
            }
        }
        rc_set_motor(1, 0);
        rc_set_motor(2, 0);
        rc_set_encoder_pos(1, 0);
        rc_set_encoder_pos(2, 0);
    }
    cout << rightSpeed << endl;
    cout << leftSpeed << endl;
    rc_set_motor(1, 0);
    rc_set_motor(2, 0);
    rc_cleanup();
    return 0;
}
