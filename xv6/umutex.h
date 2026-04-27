#ifndef UMUTEX_H
#define UMUTEX_H

typedef struct {
  volatile int locked;
} umutex_t;

void mutex_init(umutex_t *m);
void mutex_lock(umutex_t *m);
void mutex_unlock(umutex_t *m);

#endif
