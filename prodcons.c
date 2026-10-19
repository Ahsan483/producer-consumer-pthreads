/*
 * prodcons.c - bounded-buffer producer/consumer with POSIX threads
 *
 * Two counting semaphores track free and filled slots; a mutex protects the
 * buffer itself. At the end every produced item must have been consumed once.
 *
 *   ./prodcons [producers] [consumers] [items_per_producer]
 */
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define SIZE 5

static int buf[SIZE], occupied[SIZE], in, out, count;
static sem_t empty_slots, full_slots;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static long produced_sum, consumed_sum;
static int per_producer, producers, consumers;

static void show(const char *who, int id, const char *what, int item)
{
    /* called with the lock held, so the snapshot is consistent */
    printf("%s %d %-8s %4d   buffer [", who, id, what, item);
    for (int i = 0; i < SIZE; i++) {
        if (occupied[i]) printf("%4d", buf[i]);
        else printf("   .");
    }
    printf(" ]  %d/%d\n", count, SIZE);
}

static void *producer(void *arg)
{
    int id = (int)(long)arg;
    for (int k = 0; k < per_producer; k++) {
        int item = id * 100 + k;
        usleep(rand() % 15000);          /* producers are faster, so the buffer fills up */
        sem_wait(&empty_slots);
        pthread_mutex_lock(&lock);
        buf[in] = item;
        occupied[in] = 1;
        in = (in + 1) % SIZE;
        count++;
        produced_sum += item;
        show("producer", id, "put", item);
        pthread_mutex_unlock(&lock);
        sem_post(&full_slots);
    }
    return NULL;
}

static void *consumer(void *arg)
{
    int id = (int)(long)arg, mine = 0;
    for (;;) {
        sem_wait(&full_slots);
        pthread_mutex_lock(&lock);
        if (buf[out] < 0) {               /* poison pill: no more work */
            pthread_mutex_unlock(&lock);
            break;
        }
        int item = buf[out];
        occupied[out] = 0;
        out = (out + 1) % SIZE;
        count--;
        consumed_sum += item;
        show("consumer", id, "took", item);
        pthread_mutex_unlock(&lock);
        sem_post(&empty_slots);
        mine++;
        usleep(rand() % 70000);
    }
    return (void *)(long)mine;
}

int main(int argc, char **argv)
{
    producers = argc > 1 ? atoi(argv[1]) : 2;
    consumers = argc > 2 ? atoi(argv[2]) : 2;
    per_producer = argc > 3 ? atoi(argv[3]) : 6;
    srand(42);

    sem_init(&empty_slots, 0, SIZE);
    sem_init(&full_slots, 0, 0);
    pthread_t pt[16], ct[16];

    for (long i = 0; i < producers; i++) pthread_create(&pt[i], NULL, producer, (void *)(i + 1));
    for (long i = 0; i < consumers; i++) pthread_create(&ct[i], NULL, consumer, (void *)(i + 1));
    for (int i = 0; i < producers; i++) pthread_join(pt[i], NULL);

    /* one poison pill per consumer, placed after all real items */
    for (int i = 0; i < consumers; i++) {
        sem_wait(&empty_slots);
        pthread_mutex_lock(&lock);
        buf[in] = -1;
        in = (in + 1) % SIZE;
        pthread_mutex_unlock(&lock);
        sem_post(&full_slots);
    }
    long handled[16];
    for (int i = 0; i < consumers; i++) {
        void *r;
        pthread_join(ct[i], &r);
        handled[i] = (long)r;
    }
    printf("\n");
    for (int i = 0; i < consumers; i++) printf("consumer %d handled %ld items\n", i + 1, handled[i]);
    printf("produced sum = %ld, consumed sum = %ld -> %s\n", produced_sum, consumed_sum,
           produced_sum == consumed_sum ? "OK, nothing lost or duplicated" : "MISMATCH");
    return produced_sum != consumed_sum;
}
