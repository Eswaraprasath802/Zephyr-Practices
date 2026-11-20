#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* we just found the device identifier  */
#define RLED_NODE DT_NODELABEL(redled)
#define GLED_NODE DT_NODELABEL(greenled)
#define BLED_NODE DT_NODELABEL(blueled)

/* from device idnetifier, we create a gpio spec (struct contains info such as (port, pin, flags))  */
static const struct gpio_dt_spec rled_spec = GPIO_DT_SPEC_GET(RLED_NODE, gpios);
static const struct gpio_dt_spec gled_spec = GPIO_DT_SPEC_GET(GLED_NODE, gpios);
static const struct gpio_dt_spec bled_spec = GPIO_DT_SPEC_GET(BLED_NODE, gpios);

/*First: we need the led instance created*/
int main(void)
{   
    /* check if devices are ready or available */
    int rc0= 0;
    int rc1= 0;
    int rc2= 0;
    rc0=gpio_is_ready_dt(&rled_spec);
    rc1=gpio_is_ready_dt(&gled_spec);
    rc2=gpio_is_ready_dt(&bled_spec);
    if (!rc0)
    {   
        // some error has occured, device is not ready
        printk("Red led not ready, error code [%d]", rc0);
        return rc0;
    }
    if (!rc1)
    {   
        // some error has occured, device is not ready
        printk("Green led not ready, error code [%d]", rc1);
        return rc1;
    }
    if (!rc2)
    {   
        // some error has occured, device is not ready
        printk("Blue led not ready, error code [%d]", rc2);
        return rc2;
    }
    //configure the pins states (output or input)
    //so by code we control the gpio pin state, since devicetree dont know the state of pin unless we define state
    rc0 = gpio_pin_configure_dt(&rled_spec, GPIO_OUTPUT_ACTIVE);
    rc1 = gpio_pin_configure_dt(&gled_spec, GPIO_OUTPUT_ACTIVE);
    rc2 = gpio_pin_configure_dt(&bled_spec, GPIO_OUTPUT_ACTIVE);

    if (rc0 < 0)
    {
        printk("GPIO Pin config error [%d]",rc0);
        return rc0;
    }
    if (rc1 < 0)
    {
        printk("GPIO Pin config error [%d]",rc1);
        return rc1;
    }
    if (rc2 < 0)
    {
        printk("GPIO Pin config error [%d]",rc2);
        return rc2;
    }
    
    while (1)
    {
        /* on some leds and off some leds */
        rc0 = gpio_pin_set_dt(&rled_spec, 1U);// unsigned 1
        if (rc0 < 0)
        {
            printk("Rled cannot set logical high, [%d]",rc0);
            return rc0;
        }
        k_msleep(1000);
        rc1 = gpio_pin_set_dt(&gled_spec, 1U);// unsigned 1
        if (rc1 < 0)
        {
            printk("Gled cannot set logical high, [%d]",rc1);
            return rc1;
        }
        k_msleep(1000);
        rc2 = gpio_pin_set_dt(&bled_spec, 1U);// unsigned 1
        if (rc2 < 0)
        {
            printk("Bled cannot set logical high, [%d]",rc2);
            return rc2;
        }
        k_msleep(1000);
        rc0 = gpio_pin_set_dt(&rled_spec, 0U);// unsigned 1
        if (rc0 < 0)
        {
            printk("Rled cannot set logical high, [%d]",rc0);
            return rc0;
        }
        k_msleep(1000);
        rc1 = gpio_pin_set_dt(&gled_spec, 0U);// unsigned 1
        if (rc1 < 0)
        {
            printk("Gled cannot set logical high, [%d]",rc1);
            return rc1;
        }
        k_msleep(1000);
        rc2 = gpio_pin_set_dt(&bled_spec, 0U);// unsigned 1
        if (rc2 < 0)
        {
            printk("Bled cannot set logical high, [%d]",rc2);
            return rc2;
        }
        k_msleep(1000);
        


    }
    printk("Hello World!\n");
    return 0;
}