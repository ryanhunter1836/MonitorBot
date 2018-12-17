#include <roboticscape.h>
#include <rc_usefulincludes.h> 

int main()
{
    rc_initialize();
    rc_enable_motors();
    while(rc_get_state()!=EXITING)
    {
        rc_set_motor(3, .25);
        /*rc_set_motor(4, .25);
        rc_usleep(10000);
        rc_set_motor(3, -.25);
        rc_set_motor(4, .25);
        rc_usleep(10000);
        rc_set_motor(3, -.25);
        rc_set_motor(4, -.25);
        rc_usleep(10000);
        rc_set_motor(3, .25);
        rc_set_motor(4, -.25);
        rc_usleep(10000); */
    }
    return 0;
}