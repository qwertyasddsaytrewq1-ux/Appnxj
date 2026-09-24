#include "Thread/PlatformThread.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/syscall.h>

int zz::OSThread::GetCurrentProcessId() {
  return static_cast<int>(getpid());
}

int zz::OSThread::GetCurrentThreadId() {
#if defined(__APPLE__)
  return static_cast<int>(pthread_mach_thread_np(pthread_self()));
#elif defined(__ANDROID__)
  return static_cast<int>(gettid());
#elif defined(__linux__)
  return static_cast<int>(syscall(__NR_gettid));
#else
  return static_cast<int>(reinterpret_cast<intptr_t>(pthread_self()));
#endif
}

static zz::OSThread::LocalStorageKey PthreadKeyToLocalKey(pthread_key_t pthread_key) {
  return static_cast<zz::OSThread::LocalStorageKey>(pthread_key);
}

static pthread_key_t LocalKeyToPthreadKey(zz::OSThread::LocalStorageKey local_key) {
  return static_cast<pthread_key_t>(local_key);
}

zz::OSThread::LocalStorageKey zz::OSThread::CreateThreadLocalKey() {
  pthread_key_t key;
  int result = pthread_key_create(&key, nullptr);
  DCHECK_EQ(0, result);
  return PthreadKeyToLocalKey(key);
}

void zz::OSThread::DeleteThreadLocalKey(zz::OSThread::LocalStorageKey key) {
  pthread_key_t pthread_key = LocalKeyToPthreadKey(key);
  pthread_key_delete(pthread_key);
}

void *zz::OSThread::GetThreadLocal(zz::OSThread::LocalStorageKey key) {
  pthread_key_t pthread_key = LocalKeyToPthreadKey(key);
  return pthread_getspecific(pthread_key);
}

void zz::OSThread::SetThreadLocal(zz::OSThread::LocalStorageKey key, void *value) {
  pthread_key_t pthread_key = LocalKeyToPthreadKey(key);
  pthread_setspecific(pthread_key, value);
}
