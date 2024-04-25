## Usage

### Iterating modules

```cpp
for (const LDR_DATA_TABLE_ENTRY& entry : peb::modules) {
    // ...
}
```

### Resolving modules / exports

```cpp
void* kernel32  = peb::find_module("kernel32.dll");
void* open_proc = peb::find_export(kernel32, "OpenProcess");
```
