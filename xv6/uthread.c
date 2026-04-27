#include "types.h"
#include "stat.h"
#include "user.h"
#include "uthread.h"

#define MAX_THREADS 8
#define STACK_SIZE  4096

enum {
  T_UNUSED,
  T_RUNNABLE,
  T_RUNNING,
  T_ZOMBIE
};

struct context {
  uint edi;
  uint esi;
  uint ebx;
  uint ebp;
  uint eip;
};

struct thread {
  int state;
  struct context *context;
  char *stack;
  void (*fn)(void*);
  void *arg;
};

static struct thread threads[MAX_THREADS];
static int current;
static int initialized;

static void thread_stub(void);

void
thread_init(void)
{
  int i;

  for(i = 0; i < MAX_THREADS; i++){
    threads[i].state = T_UNUSED;
    threads[i].context = 0;
    threads[i].stack = 0;
    threads[i].fn = 0;
    threads[i].arg = 0;
  }

  current = 0;
  threads[0].state = T_RUNNING;
  initialized = 1;
}

tid_t
thread_create(void (*fn)(void*), void *arg)
{
  int i;
  uint sp;
  struct context *ctx;

  if(!initialized)
    thread_init();

  for(i = 1; i < MAX_THREADS; i++){
    if(threads[i].state == T_UNUSED)
      break;
  }
  if(i == MAX_THREADS)
    return -1;

  threads[i].stack = malloc(STACK_SIZE);
  if(threads[i].stack == 0)
    return -1;

  sp = (uint)(threads[i].stack + STACK_SIZE);
  sp &= ~3;
  sp -= sizeof(*ctx);
  ctx = (struct context*)sp;
  memset(ctx, 0, sizeof(*ctx));
  ctx->eip = (uint)thread_stub;

  threads[i].context = ctx;
  threads[i].fn = fn;
  threads[i].arg = arg;
  threads[i].state = T_RUNNABLE;

  return i;
}

void
thread_yield(void)
{
  int i;
  int candidate;
  int next;
  int old;

  if(!initialized)
    thread_init();

  next = -1;
  for(i = 1; i <= MAX_THREADS; i++){
    candidate = (current + i) % MAX_THREADS;
    if(threads[candidate].state == T_RUNNABLE){
      next = candidate;
      break;
    }
  }

  if(next < 0)
    return;

  old = current;
  if(threads[old].state == T_RUNNING)
    threads[old].state = T_RUNNABLE;
  threads[next].state = T_RUNNING;
  current = next;

  uswtch(&threads[old].context, threads[next].context);
}

int
thread_join(tid_t tid)
{
  if(!initialized)
    thread_init();

  if(tid <= 0 || tid >= MAX_THREADS || tid == current)
    return -1;

  while(threads[tid].state != T_ZOMBIE){
    if(threads[tid].state == T_UNUSED)
      return -1;
    thread_yield();
  }

  free(threads[tid].stack);
  threads[tid].stack = 0;
  threads[tid].context = 0;
  threads[tid].fn = 0;
  threads[tid].arg = 0;
  threads[tid].state = T_UNUSED;
  return 0;
}

static void
thread_stub(void)
{
  threads[current].fn(threads[current].arg);
  threads[current].state = T_ZOMBIE;
  thread_yield();

  exit();
}
