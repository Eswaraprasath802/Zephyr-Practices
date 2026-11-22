#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>


#define GLED_NODE DT_NODELABEL(gled)
#define BUTTON1  DT_NODELABEL(first_button)
#define PWM_BLINKY DT_NODELABEL(pwm_led)


static const struct gpio_dt_spec gled_spec = GPIO_DT_SPEC_GET(GLED_NODE, gpios);
static const struct gpio_dt_spec mybutton = GPIO_DT_SPEC_GET_OR(BUTTON1, gpios,{0});
const struct pwm_dt_spec pwm_blinky_led = PWM_DT_SPEC_GET(PWM_BLINKY);


void set_pwm_brightness(uint8_t brightness);

static struct gpio_callback mybutton_cb_data;
#define FAN_STATES 4
uint8_t fan_state_step=0;
uint8_t fan_speed=0;

void mybutton_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
       fan_state_step=(fan_state_step+1)%FAN_STATES;

       switch(fan_state_step){
        case 0:
            fan_speed=0;
            break;
         
        case 1:
            fan_speed=25;
            break;

        case 2:
            fan_speed=75;
            break;

        case 3:
            fan_speed=100;
            break;

        default:
            printk("The step size is invalid");
            break;
       }
       set_pwm_brightness(fan_speed);
    }
void set_pwm_brightness(uint8_t brightness){
    pwm_set_pulse_dt(&pwm_blinky_led,(pwm_blinky_led.period*brightness)/100);

}
static uint8_t pwm_breathing_brightness=0;

enum dir {UP,DOWN};
enum dir direction=UP;

// struct k_timer my_timer;
void control_brightness();
// K_TIMER_DEFINE(my_timer,control_brightness, NULL);

void control_brightness(){
    set_pwm_brightness(pwm_breathing_brightness);
    if (direction==UP){
        pwm_breathing_brightness++;
    }
    if (direction==DOWN){
         pwm_breathing_brightness--;
    }
    if(pwm_breathing_brightness==0){
        direction=UP;
    }
    if(pwm_breathing_brightness==100){
        direction=DOWN;
    }
}

int main(void)
{
    int rc = 0;
    rc= pwm_is_ready_dt	(&pwm_blinky_led);
    if(!rc){
        printk("PWM not ready, error code [%d]\n", rc);
        return rc;
    }	
    rc = gpio_is_ready_dt(&gled_spec);
    if(!rc)
    {
        printk("GLed not ready, error code [%d]\n", rc);
        return rc;
    }
    rc = gpio_pin_configure_dt(&gled_spec, GPIO_OUTPUT_ACTIVE);
    if(rc < 0)
    {
        printk("Gled pin confi error [%d]\n", rc);
        return rc;
    }
    rc = gpio_is_ready_dt(&mybutton);
    if(!rc) {
        printk("Button device is not ready\n");
        return rc;
    }
    /* k_timer_start(&my_timer, K_MSEC(1000), K_MSEC(10));
    printk("PWM period in nanosecond from a device tree %u \n",pwm_blinky_led.period);
    rc=pwm_set_pulse_dt(&pwm_blinky_led,pwm_blinky_led.period);
    if(rc<0){
        printk("set pulse failed\n");
    }
        */
    rc = gpio_pin_configure_dt(&mybutton, GPIO_INPUT);
    if(rc < 0)  
    {
        printk("Can't cfg mybutton pin type\n");
        return rc;
    }
    rc = gpio_pin_interrupt_configure_dt(&mybutton, GPIO_INT_EDGE_TO_INACTIVE);
    if(rc != 0)
    {
        printk("Can't cfg interrupt for mybutton [%d]\n", rc);
        return rc;
    }
    gpio_init_callback(&mybutton_cb_data, mybutton_cb, BIT(mybutton.pin));
    
    rc = gpio_add_callback_dt(&mybutton, &mybutton_cb_data);
    if(rc) {
        printk("Unable to add callback for my button\n");
        return rc;
    }

    while (1)
    {
        /*we toggle the led */
        rc = gpio_pin_set_dt(&gled_spec, 1U);
        if(rc < 0) {
            printk("Gled cant set logical HIGH\n");
            return rc;
        }
        k_msleep(1000);
        rc = gpio_pin_set_dt(&gled_spec, 0U);
        if(rc < 0) {
            printk("Gled cant set logical LOW\n");
            return rc;
        }
        k_msleep(1000);
    }

}
 /*#define MY_STACK_SIZE 4096
#define MY_PRIORITY 5
extern void pwm_breathing(void *, void *, void *){
    while(1){
        control_brightness();
        k_msleep(50);
        printk("The brightness of led %u and direction %u\n",pwm_breathing_brightness,direction);
    }

}
K_THREAD_DEFINE(breathing_th1, MY_STACK_SIZE,
                pwm_breathing, NULL, NULL, NULL,
                MY_PRIORITY, 0, 300);
*/