#ifndef FAKE_EXECINFO_H
#define FAKE_EXECINFO_H
inline int backtrace(void **array, int size) { return 0; }
inline char **backtrace_symbols(void *const *array, int size) { return 0; }
inline void backtrace_symbols_fd (void *const *array, int size, int fd) {}
#endif