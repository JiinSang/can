/**
 * Reference Author: Jason White
 *
 * Editor: Jiin
 *
 * Compile:
 * gcc joystick.c -o joystick
 *
 * Run:
 * ./joystick [/dev/input/jsX]
 *
 * See also:
 * https://www.kernel.org/doc/Documentation/input/joystick-api.txt
 */
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <stdlib.h>
#include <string.h>


int state; // can state
FILE *fp;
char des_angle_str[3]={'0', '0'};
char VCUcommand[33]={'c','a','n','s','e','n','d',' ','c','a','n','0',' ','2','0','0','#','8','0','0','0','0','0','0','0','0','0','0','0','0','0','0','0'};//33
                                                                                             //20     //22
unsigned int des_angle = 90;
/**
 * Reads a joystick event from the joystick device.
 *
 * Returns 0 on success. Otherwise -1 is returned.
 */
int read_event(int fd, struct js_event *event)
{
    ssize_t bytes;

    bytes = read(fd, event, sizeof(*event));

    if (bytes == sizeof(*event))
        return 0;

    /* Error, could not read full event. */
    return -1;
}

/**
 * Returns the number of axes on the controller or 0 if an error occurs.
 */
size_t get_axis_count(int fd)
{
    __u8 axes;

    if (ioctl(fd, JSIOCGAXES, &axes) == -1)
        return 0;

    return axes;
}

/**
 * Returns the number of buttons on the controller or 0 if an error occurs.
 */
size_t get_button_count(int fd)
{
    __u8 buttons;
    if (ioctl(fd, JSIOCGBUTTONS, &buttons) == -1)
        return 0;

    return buttons;
}

/**
 * Current state of an axis.
 */
struct axis_state {
    short x, y;
};

/**
 * Keeps track of the current axis state.
 *
 * NOTE: This function assumes that axes are numbered starting from 0, and that
 * the X axis is an even number, and the Y axis is an odd number. However, this
 * is usually a safe assumption.
 *
 * Returns the axis that the event indicated.
 */
size_t get_axis_state(struct js_event *event, struct axis_state axes[3])
{
    size_t axis = event->number / 2;

    if (axis < 3)
    {
        if (event->number % 2 == 0)
            axes[axis].x = event->value;
        else
            axes[axis].y = event->value;
    }

    return axis;
}

int main(int argc, char *argv[])
{
    const char *device;
    int js;
    struct js_event event;
    struct axis_state axes[3] = {0};
    size_t axis;

    if (argc > 1)
        device = argv[1];
    else
        device = "/dev/input/js0";

    js = open(device, O_RDONLY);

    if (js == -1)
        perror("Could not open joystick");
    
    //---------can initial setup start-----------------//
    fp = popen("sudo /sbin/ip link set can0 up type can bitrate 500000", "r");
	if(fp==NULL)
	{
		perror("erro : ");
		exit(0);
	}
	state = pclose(fp);
    //---------can initial setup end-----------------//

    /* This loop will exit if the controller is unplugged. */
    while (read_event(js, &event) == 0)
    {
        switch (event.type)
        {
            case JS_EVENT_BUTTON:
                printf("Button %u %s\n", event.number, event.value ? "pressed" : "released");
                int i;
                if (event.number == 2) { //x : enabling remote control
                    for(i=0; i<6; i++) {
                        fp = popen("cansend can0 200#8000000000000000", "r");
                        printf("%s\n", VCUcommand);

                        state = pclose(fp);
                    }
                }

                else if (event.number == 3) { //y: forward control

                    VCUcommand[19] = '0';
                    VCUcommand[20] = '5';
                    fp = popen(VCUcommand, "r");
                    printf("%s\n", VCUcommand);

                    state = pclose(fp);
                }

                else if (event.number == 0) { //a=0 : backward control
                    VCUcommand[19] = 'F';
                    VCUcommand[20] = 'B';
                    fp = popen(VCUcommand, "r");
                    printf("%s\n", VCUcommand);

                    state = pclose(fp);
                }

                else if (event.number == 1) { //b: stop
                    VCUcommand[19] = '0';
                    VCUcommand[20] = '0';
                    fp = popen(VCUcommand, "r");
                    printf("%s\n", VCUcommand);

                    state = pclose(fp);
                }

                else if (event.number == 4) { //LB = 4 : turn left (40~130)
                    if((des_angle-5) >= 40 && (des_angle-5) <= 130){
                        des_angle = des_angle - 5;}

                    //change integer to string
                    sprintf(des_angle_str, "%x", des_angle); 

                    //assign value
                    if(des_angle_str[1] == '\0'){
                        VCUcommand[21] = '0';
                        VCUcommand[22] = *des_angle_str;
                    }
                    else if(des_angle_str[0] == '\0'&& des_angle_str[1] == '\0'){
                        VCUcommand[21] = '0';
                        VCUcommand[22] = '0';
                    }
                    else{
                        VCUcommand[21] = *des_angle_str;
                        VCUcommand[22] = *(des_angle_str+1);
                    }
                    

                    //printf("%s\n",VCUcommand);
                    fp = popen(VCUcommand, "r");
                    printf("%s\n", VCUcommand);

                    state = pclose(fp);
                }

                else if (event.number == 5) { //RB = 5 : turn right (90~130)
                    if((des_angle+5)>=40&&(des_angle+5)<=130){
                        des_angle = des_angle + 5;}

                    sprintf(des_angle_str, "%x", des_angle);

                    //printf("%s\n",VCUcommand);
                    //printf("[0]: %c\n",des_angle_str[0] );
                    //printf("[1]: %c\n",des_angle_str[1] );

                    /*
                    if(des_angle < 16){
                        char tmp;
                        tmp = des_angle_str[0];
                        des_angle_str[0] = des_angle_str[1];
                        des_angle_str[1] = tmp;
                    }                    
                    
                    printf("------------------------\n");
                    printf("[0]: %c\n",des_angle_str[0] );
                    printf("[1]: %c\n",des_angle_str[1] );
                    */

                    //assign value
                    if(des_angle_str[1] == '\0'){
                        VCUcommand[21] = '0';
                        VCUcommand[22] = *des_angle_str;
                    }
                    else if(des_angle_str[0] == '\0'&& des_angle_str[1] == '\0'){
                        VCUcommand[21] = '0';
                        VCUcommand[22] = '0';
                    }
                    else{
                        VCUcommand[21] = *des_angle_str;
                        VCUcommand[22] = *(des_angle_str+1);
                    }
                    

                    //printf("%s\n",VCUcommand);
                    fp = popen(VCUcommand, "r");
                    printf("%s\n", VCUcommand);

                    state = pclose(fp);
                }
                

                break;

            case JS_EVENT_AXIS:
                axis = get_axis_state(&event, axes);
                if (axis < 3)
                    printf("Axis %zu at (%6d, %6d)\n", axis, axes[axis].x, axes[axis].y);
                break;
            default:
                /* Ignore init events. */
                break;
        }
        
        fflush(stdout);
    }

    close(js);

    return 0;
}
