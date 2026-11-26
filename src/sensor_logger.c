#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#define USE_LOCK 1          /* set to 0 to see the race condition */
#define LOG_SIZE 32

#define GREEN_LED DT_NODELABEL(gled)
static const struct gpio_dt_spec gled = GPIO_DT_SPEC_GET(GREEN_LED,gpios);

struct sensor_msg {
   uint8_t id; 
   uint32_t ts;
   int32_t value; 
  };

K_MSGQ_DEFINE(sensor_q, sizeof(struct sensor_msg), 8, 4);

/* ---- shared data, protected by log_mtx ---- */
static struct sensor_msg log_buf[LOG_SIZE];
static size_t log_head, log_count;

struct stats {
    uint32_t count[3];      /* index = sensor id (1, 2) */
    int32_t  sum[3];
    uint32_t total;         /* invariant: count[1] + count[2] == total */
};
static struct stats st;

K_MUTEX_DEFINE(log_mtx);

static inline void lock(void)   { if (USE_LOCK) k_mutex_lock(&log_mtx, K_FOREVER); }
static inline void unlock(void) { if (USE_LOCK) k_mutex_unlock(&log_mtx); }

/* ---- producers ---- */
struct producer_cfg { uint8_t id; uint32_t period_ms; };
static const struct producer_cfg soil_cfg = { 1, 500 };
static const struct producer_cfg ldr_cfg  = { 2, 300 };

static void producer(void *p1, void *p2, void *p3)
{
    const struct producer_cfg *cfg = p1;
    uint32_t n = 0;

    while (1) {
        struct sensor_msg m = {
            .id = cfg->id,
            .ts = k_uptime_get_32(),
            .value = 500 + (n++ * 37) % 200,
        };
        if (k_msgq_put(&sensor_q, &m, K_MSEC(10)) != 0) {
            printk("producer %u: queue full, dropped\n", cfg->id);
        }
        k_msleep(cfg->period_ms);
    }
}

/* ---- logger: queue -> ring buffer + stats ---- */
static void logger(void *p1, void *p2, void *p3)
{
    struct sensor_msg m;

    gpio_pin_configure_dt(&gled, GPIO_OUTPUT_INACTIVE);

    while (1) {
        k_msgq_get(&sensor_q, &m, K_FOREVER);

        lock();
        log_buf[log_head] = m;
        log_head = (log_head + 1) % LOG_SIZE;
        if (log_count < LOG_SIZE) log_count++;

        st.count[m.id]++;
        k_busy_wait(20000);          /* pretend this is a slow flash write (20 ms) */
        st.sum[m.id] += m.value;
        st.total++;                  /* stats are only consistent after this line */
        unlock();

        gpio_pin_toggle_dt(&gled);
    }
}

/* ---- reporter: periodically reads the shared data ---- */
static void reporter(void *p1, void *p2, void *p3)
{
    uint32_t rounds = 0;

    while (1) {
        k_msleep(500);

        struct stats snap;
        lock();
        snap = st;                   /* copy quickly, print after unlocking */
        unlock();

        if (snap.count[1] + snap.count[2] != snap.total) {
            printk("!! INCONSISTENT: %u + %u != %u\n",
                   snap.count[1], snap.count[2], snap.total);
        }
        if (++rounds % 10 == 0) {
            printk("--- sensor1: %u samples, avg %d | sensor2: %u samples, avg %d | total %u ---\n",
                   snap.count[1], snap.count[1] ? snap.sum[1] / (int32_t)snap.count[1] : 0,
                   snap.count[2], snap.count[2] ? snap.sum[2] / (int32_t)snap.count[2] : 0,
                   snap.total);
        }
    }
}

K_THREAD_DEFINE(soil_th, 2048, producer, &soil_cfg, NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(ldr_th,  2048, producer, &ldr_cfg,  NULL, NULL, 5, 0, 0);
K_THREAD_DEFINE(log_th,  2048, logger,   NULL, NULL, NULL, 4, 0, 0);
K_THREAD_DEFINE(rep_th,  2048, reporter, NULL, NULL, NULL, 3, 0, 0);  /* highest priority */

int main(void)
{
    printk("sensor logger started (USE_LOCK=%d)\n", USE_LOCK);
    return 0;
}