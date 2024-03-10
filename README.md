# Ldr Iterator

### Usage

```cpp
for (ldr::iterator it = ldr::iterator::begin(); it != ldr::iterator::end(); ++it) {
    // ...
}
```

```cpp
for (LDR_DATA_TABLE_ENTRY& mod : ldr::iterator()) {
    // ...
}
```

```cpp
// ldr::get_module does not care about lower/uppercase
auto k32_base = ldr::get_module("kernel32.dll")->DllBase;
std::println("kernel32.dll -> {}", k32_base);
std::println("OpenProcess -> {}", ldr::get_export(k32_base, "OpenProcess"));
```
