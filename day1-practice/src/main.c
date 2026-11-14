#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/*we just found the device identifier*/
#define GLED_NODE DT_NODELABEL(gled)
#define BUTTON1  DT_NODELABEL(first_button)

/*from the identifier,we create a gpio spec(struct contains info such as (port pin flags))*/
static const struct gpio_dt_spec gled_spec = GPIO_DT_SPEC_GET(GLED_NODE, gpios);
static const struct gpio_dt_spec mybutton = GPIO_DT_SPEC_GET_OR(BUTTON1, gpios,{0});

static struct gpio_callback mybutton_cb_data;

void mybutton_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
        printk("my button pressed \n");  
    }
/*1. needs to create the led device instance */
int main(void)
{
    /*check if device is ready/available*/
    int rc = 0;
    rc = gpio_is_ready_dt(&gled_spec);
    if(!rc)
    {
        //some error has occured, mweans the device is ready
        printk("GLed not ready, error code [%d]", rc);
        return rc;
    }
    //cfg or register the pin states (output or input)
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


    /*
    1. device ready/ available
    2. configure pin
    3. configure interrupt, GPIO_INT_EDGE_TO_INACTIVE
    4. register the callback
    */
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


    // printk("Hello World! \n");
    // return 0;
}