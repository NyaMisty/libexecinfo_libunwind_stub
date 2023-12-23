# `libexecinfo` Replacement Based on `libunwind`

libexecinfo is causing various segfaults in alpine world.

This library fixes it by re-implementing `backtrace()` & `backtrace_symbols()` & `backtrace_symbols_fd()` fully using libunwind.

## Usage

```
gcc libexecinfo_stub.c -o libexecinfo_stub.so -fPIC -shared -lunwind
```
You can then move the `libexecinfo_stub.so` into places like `/usr/lib/libexecinfo.so.1`

## Caveats

Due to difference between interface of `libunwind` and `libexecinfo`, a small buffer will be allocated when calling backtrace().

Currently this implementation can support these usage patterns:

1. Only uses backtrace(): you will need to manually free a hidden context
```
void *buffer[100];
int size = backtrace(buffer, 100);

// you will need to manually fix the additional buffer, or memory will leak
if (size > 0) free(buffer[size]);
``` 

2. Uses backtrace() + backtrace_symbols(): no additional free() needed, because the hidden context will be consolidated into returned buffer.
```
void *buffer[100];
int size = backtrace(buffer, 100);
char **syms = backtrace_symbols(buffer, size);
// directly free syms buffer, everything done
free(syms);
```

3. Uses backtrace() + backtrace_symbols_fd(): (*Possible Crash*) no additional free() needed, the hidden context will be automatically freed in `backtrace_symbols_fd`. However, you cannot call `backtrace_symbols_fd` multiple times on the same buffer.
```
void *buffer[100];
int size = backtrace(buffer, 100);
backtrace_symbols_fd(buffer, size, stderr);

// no free() needed

// but also never call backtrace_symbols_fd on same buffer multiple times!
// backtrace_symbols_fd(buffer, size, stderr);
```
