#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define GLED_NODE DT_NODELABEL(gled)
static const struct gpio_dt_spec gled = GPIO_DT_SPEC_GET(GLED_NODE, gpios);

/* Message format: what producers send and the consumer receives */
struct sensor_msg {
    uint8_t  id;        /* which producer */
    uint32_t ts;        /* uptime in ms */
    int32_t  value;
};

/* Queue: holds up to 8 messages, each sizeof(struct sensor_msg), 4-byte aligned */
K_MSGQ_DEFINE(sensor_q, sizeof(struct sensor_msg), 8, 4);

/* Per-producer settings, passed to the thread through p1 */
struct producer_cfg {
    uint8_t  id;
    uint32_t period_ms;
};
static const struct producer_cfg soil_cfg = { .id = 1, .period_ms = 500 };
static const struct producer_cfg ldr_cfg  = { .id = 2, .period_ms = 300 };

static void producer(void *p1, void *p2, void *p3)
{
    const struct producer_cfg *cfg = p1;
    uint32_t count = 0;

    while (1) {
        struct sensor_msg msg = {
            .id    = cfg->id,
            .ts    = k_uptime_get_32(),
            .value = 500 + (count++ * 37) % 200,   /* fake reading */
        };

        /* wait up to 10 ms for space; never block forever */
        int ret = k_msgq_put(&sensor_q, &msg, K_MSEC(10));
        if (ret != 0) {
            printk("producer %u: queue full, message dropped\n", cfg->id);
        }
        k_msleep(cfg->period_ms);
    }
}

static void consumer(void *p1, void *p2, void *p3)
{
    struct sensor_msg msg;

     gpio_pin_configure_dt(&gled, GPIO_OUTPUT_INACTIVE);

    while (1) {
        /* sleeps here until a message arrives (no polling) */
        k_msgq_get(&sensor_q, &msg, K_FOREVER);

        gpio_pin_toggle_dt(&gled);
        printk("[%u ms] sensor %u = %d  (queued: %u)\n",
               msg.ts, msg.id, msg.value, k_msgq_num_used_get(&sensor_q));
    }
}

int main(void)
{
    printk("message passing demo started\n");
    return 0;
}
K_THREAD_DEFINE(soil_th, 1024, producer, &soil_cfg, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(ldr_th,  1024, producer, &ldr_cfg,  NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(cons_th, 1024, consumer, NULL,      NULL, NULL, 4, 0, 0);