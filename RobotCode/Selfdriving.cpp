//Feature that allows the robot to follow a predefined map
//STILL IN DEVELOPMENT

#include <iostream>
#include <thread>
#include <vector>

extern "C" 
{
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
    
using namespace std;
    
enum MOTORS
{
    LEFT = 1,
    RIGHT = 2
};

static void MotorController(int* data);


float LEFT_DUTY = .9;
float RIGHT_DUTY = .8;

int main()
{
    rc_initialize();
    int* data = new int;
    int* distance = new int;
    *data = 0;
    *distance = 0;
    thread motorThread(MotorController, data);
    int commands[3][2] = {{1, 25000}};
    //cout << "1" << endl;
    //loop through the commands 
    for(auto& element : commands)
    {
        int rightCount = 0;
        int leftCount = 0;
        int difference = 0;
        *data = element[0];
        cout << *data << endl;
        while(rc_get_encoder_pos(2) < element[1] && rc_get_state()!=EXITING)
        {
            cout << element[1] << endl;
            /*
            rightCount = rc_get_encoder_pos(1);
            leftCount = rc_get_encoder_pos(2);
            difference = abs(rightCount - leftCount);
            if(difference > 25)
            {
                if(rightCount < leftCount)
                {
                    LEFT_DUTY += .01;
                }
                if(leftCount < rightCount)
                {
                    LEFT_DUTY -= .01;
                }
            } */
            rc_usleep(10000);
        }
        *data = 0;
        rc_usleep(100000);
        rc_set_encoder_pos(1, 0);
        rc_set_encoder_pos(2, 0);
    }
    motorThread.join();
    rc_cleanup();
    delete data;
    delete distance;
    return 0;
}

static void MotorController(int* data)
{
    rc_enable_motors();
    while(rc_get_state()!=EXITING)
    {
        switch(*data)
    	{
    	    //stop
    		case 0:
    		    rc_set_motor(RIGHT, 0);
    		    rc_set_motor(LEFT, 0);
    		    rc_usleep(100000);
    		    break;
    		
    		//forwards
    		case 1:
    			rc_set_motor(RIGHT, RIGHT_DUTY);
    			rc_set_motor(LEFT, LEFT_DUTY);
    			rc_usleep(100000);
    			break;
    		
    		//backwards	
    		case 2:
    			rc_set_motor(RIGHT, -RIGHT_DUTY);
    			rc_set_motor(LEFT, -LEFT_DUTY);
    			rc_usleep(100000);
    			break;
    		
    		//left	
    		case 3:
    			rc_set_motor(LEFT, RIGHT_DUTY);
    			rc_set_motor(RIGHT, -LEFT_DUTY);
    			rc_usleep(100000);
    			break;
    		
    		//right
    		case 4:
    			rc_set_motor(LEFT, -RIGHT_DUTY);
    			rc_set_motor(RIGHT, LEFT_DUTY);
    			rc_usleep(100000);
    			break;
    		
    		//stop	
    		case 5:
    			rc_set_motor(LEFT, 0);
    			rc_set_motor(RIGHT, 0);
    			rc_usleep(100000);
    			break;
    			
    		//tilt head forwards
    		case 6:
    			rc_usleep(100000);
    			break;
    			
    		//tilt head backwards
    		case 7:
    			rc_usleep(100000);
    			break;
    			
    		//in case some error occurs
    		default:
    		    rc_usleep(100000);
    		    break;
    	}
    }
    
}
