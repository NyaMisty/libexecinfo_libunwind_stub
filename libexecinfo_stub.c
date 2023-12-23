#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNW_LOCAL_ONLY
#include <libunwind.h>

int backtrace(void **buffer, int size) {
  // copy from unw_backtrace
  unw_cursor_t *cursors = malloc(sizeof(unw_cursor_t));
  if (!cursors)
    return 0;
  unw_word_t ip;
  int n = 0;

  unw_cursor_t cursor;
  unw_context_t context;
  unw_getcontext(&context);
  if (unw_init_local (&cursor, &context) < 0)
    return 0;

  while (unw_step (&cursor) > 0)
    {
      memcpy(&cursors[n], &cursor, sizeof(unw_cursor_t));
      if (n >= size - 1)
        break;
      if (unw_get_reg (&cursors[n], UNW_REG_IP, &ip) < 0)
        break;
      cursors = realloc(cursors, sizeof(unw_cursor_t) * (n + 2));
      if (!cursors) return 0;
      buffer[n++] = (void *) (uintptr_t) ip;
    }
  buffer[n] = cursors;
  return n;
}

char **backtrace_symbols(void *const *buffer, int size) {
    char **result = malloc(size * sizeof(char *) + size * sizeof(unw_cursor_t));
    if (result == NULL) {
        return NULL;
    }
	unw_cursor_t *cursors = (unw_cursor_t *)&result[size];
	memcpy(cursors, buffer[size], size * sizeof(unw_cursor_t));
	free(buffer[size]);

    for (int i = 0; i < size; i++) {
        char func_name[256] = {0};
        unw_word_t offset = 0;

        unw_get_proc_name(&cursors[i], func_name, sizeof(func_name), &offset);
        asprintf(&result[i], "%p : (%s+0x%lx)", buffer[i], func_name, offset);
    }

    return result;
}

// 类似于 backtrace_symbols_fd 的函数
void backtrace_symbols_fd(void *const *buffer, int size, int fd) {
    unw_cursor_t *cursors = (unw_cursor_t *)buffer[size];
    for (int i = 0; i < size; i++) {
        char func_name[256] = {0};
        unw_word_t offset = 0;

        unw_get_proc_name(&cursors[i], func_name, sizeof(func_name), &offset);
        dprintf(fd, "%p : (%s+0x%lx)\n", buffer[i], func_name, offset);
    }
}

/*
// 测试函数
void function_to_trace() {
    const int size = 128;
    void *buffer[size];
    int num_entries = backtrace(buffer, size);
    char **symbols = backtrace_symbols(buffer, num_entries);

    for (int i = 0; i < num_entries; i++) {
        printf("%s\n", symbols[i]);
        free(symbols[i]);
    }

    free(symbols);

    // 或者直接输出到文件描述符（例如标准输出）
    // my_backtrace_symbols_fd(buffer, num_entries, STDOUT_FILENO);
}

int main() {
    function_to_trace();
    return 0;
}
*/
