#ifndef DOBBY_SEMAPHORE_H
#define DOBBY_SEMAPHORE_H

#include <semaphore.h>
#include <stdint.h>
#include <time.h>

namespace v8 {
namespace base {

class Semaphore final {
public:
  explicit Semaphore(int count) { sem_init(&native_handle_, 0, count); }
  ~Semaphore() { sem_destroy(&native_handle_); }

  void Signal() { sem_post(&native_handle_); }
  void Wait() { sem_wait(&native_handle_); }
  
  bool WaitFor(long delta_ns) {
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += delta_ns;
    if (ts.tv_nsec >= 1000000000L) {
      ts.tv_sec += ts.tv_nsec / 1000000000L;
      ts.tv_nsec %= 1000000000L;
    }
    return sem_timedwait(&native_handle_, &ts) == 0;
  }

private:
  sem_t native_handle_;
};

} // namespace base
} // namespace v8

#endif
