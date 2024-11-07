#include "web_server.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/sem.h"
#include "pico/util/queue.h"

#include <stdio.h>

#define FLAG 99

// replaced with semaphore
// volatile bool triggered = false;
semaphore_t trigger_sem;

queue_t q;

void core1_main() {
    web_server_init(&q, &trigger_sem);

    // envia un valor arbitrario al nucleo0 indicando que el servidor inicio
    multicore_fifo_push_blocking(FLAG);

    web_server_poll();
}

int main() {
    stdio_init_all();
    sleep_ms(2000);

    // envia el ultimo resultado de la medicion, es recibida por el servidor
    uint32_t value = 0xff;

    multicore_launch_core1(core1_main);

    // sincroniza el nucleo0 con el nucleo1. Espera que el 1 levante el servidor
    printf("core0 is waiting...\n");
    if (multicore_fifo_pop_blocking() != FLAG) {
        printf("sync mismatch, exiting...\n");
        return 1;
    }
    printf("core1 server is up, core0 starts running...\n");

    while (1) {
        sem_acquire_blocking(&trigger_sem);

        // cuando se pide una medicion se libera el semaforo y se ejecuta una
        // medicion - la respuesta al pedido solo indica que se ejecuta la
        // medicion

        // la pagina vuelve a hacer otro pedido con el resultado de la medicion
        // luego de unos segundos

        // simulacion de la medicion
        // se envia un valor a la cola y se asigna un nuevo valor
        if (queue_try_add(&q, &value)) {
            printf("\tpushed %08X\n", value);

            if (value < 0xff000000) {
                value = value << 8;
            } else {
                value = 0xff;
            }
        }
    }

    printf("exiting...\n");

    return 0;
}