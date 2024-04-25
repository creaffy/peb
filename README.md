# Ldr Iterator

### Usage

```cpp
for (const LDR_DATA_TABLE_ENTRY& entry : peb::modules) {
    // ...
}
```

```cpp
void* kernel32 = peb::find_module("kernel32.dll");
std::println("{}", peb::find_export(kernel32, "OpenProcess"));
```
