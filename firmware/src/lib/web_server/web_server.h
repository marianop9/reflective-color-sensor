#include "pico/util/queue.h"
#include "pico/sem.h"

typedef uint32_t cs_web_server_data_t;

int web_server_init(queue_t *pdata_queue, semaphore_t *ptrigger_sem);

void web_server_poll();