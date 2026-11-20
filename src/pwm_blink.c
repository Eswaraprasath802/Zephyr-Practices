#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/pwm.h>


#define GLED_NODE DT_NODELABEL(gled)
#define BUTTON1  DT_NODELABEL(first_button)
#define PWM_BLINKY DT_NODELABEL(pwm_led)


static const struct gpio_dt_spec gled_spec = GPIO_DT_SPEC_GET(GLED_NODE, gpios);
static const struct gpio_dt_spec mybutton = GPIO_DT_SPEC_GET_OR(BUTTON1, gpios,{0});
const struct pwm_dt_spec pwm_blinky_led = PWM_DT_SPEC_GET(PWM_BLINKY);


static struct gpio_callback mybutton_cb_data;

void mybutton_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
        printk("my button pressed \n");  
    }

int main(void)
{
    int rc = 0;
    rc= pwm_is_ready_dt	(&pwm_blinky_led);
    if(!rc){
        printk("PWM not ready, error code [%d]", rc);
        return rc;
    }	

    rc = gpio_is_ready_dt(&gled_spec);
    if(!rc)
    {
        printk("GLed not ready, error code [%d]", rc);
        return rc;
    }
    rc = gpio_pin_configure_dt(&gled_spec, GPIO_OUTPUT_ACTIVE);
    if(rc < 0)
    {
        printk("Gled pin confi error [%d]", rc);
        return rc;
    }
    rc = gpio_is_ready_dt(&mybutton);
    if(!rc) {
        printk("Button device is not ready");
        return rc;
    }
    printk("PWM period in nanosecond from a device tree %u \n",pwm_blinky_led.period);
    rc=pwm_set_pulse_dt(&pwm_blinky_led,pwm_blinky_led.period);
    if(rc<0){
        printk("set pulse failed");
    }
    rc = gpio_pin_configure_dt(&mybutton, GPIO_INPUT);
    if(rc < 0)  
    {
        printk("Can't cfg mybutton pin type");
        return rc;
    }
    rc = gpio_pin_interrupt_configure_dt(&mybutton, GPIO_INT_EDGE_TO_INACTIVE);
    if(rc != 0)
    {
        printk("Can't cfg interrupt for mybutton [%d]", rc);
        return rc;
    }
    gpio_init_callback(&mybutton_cb_data, mybutton_cb, BIT(mybutton.pin));
    
    rc = gpio_add_callback_dt(&mybutton, &mybutton_cb_data);
    if(rc) {
        printk("Unable to add callback for my button");
        return rc;
    }

    while (1)
    {
        /*we toggle the led */
        rc = gpio_pin_set_dt(&gled_spec, 1U);
        if(rc < 0) {
            printk("Gled cant set logical HIGH");
            return rc;
        }
        k_msleep(1000);
        rc = gpio_pin_set_dt(&gled_spec, 0U);
        if(rc < 0) {
            printk("Gled cant set logical LOW");
            return rc;
        }
        k_msleep(1000);
    }

}