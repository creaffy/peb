#pragma once
#include <string>
#include <algorithm>
#include <windows.h>
#include <winternl.h>

namespace peb {
    class modules_iterator {
    public:
        [[nodiscard]] constexpr inline modules_iterator() noexcept
            : entry(nullptr) {
        }

        [[nodiscard]] inline modules_iterator(LIST_ENTRY* Ptr) noexcept
            : entry(Ptr) {
        }

        [[nodiscard]] inline modules_iterator begin() const {
            return modules_iterator(get_teb()->ProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList.Flink);
        }

        [[nodiscard]] inline modules_iterator end() const {
            return modules_iterator(&get_teb()->ProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList);
        }

        [[nodiscard]] inline const LDR_DATA_TABLE_ENTRY* get() const {
            return CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        }

        inline modules_iterator& operator++() {
            entry = entry->Flink;
            return *this;
        }
        
        [[nodiscard]] inline bool operator==(const modules_iterator& Iterator) const noexcept {
            return entry == Iterator.entry;
        }

        [[nodiscard]] inline bool operator!=(const modules_iterator& Iterator) const noexcept {
            return entry != Iterator.entry;
        }

        [[nodiscard]] inline const LDR_DATA_TABLE_ENTRY& operator*() const {
            return *CONTAINING_RECORD(entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        }

        [[nodiscard]] inline const LDR_DATA_TABLE_ENTRY* operator->() const {
            return get();
        }

    private:
        LIST_ENTRY* entry;

        [[nodiscard]] static inline TEB* get_teb() {
#ifdef _WIN64
            return reinterpret_cast<TEB*>(__readgsqword(offsetof(NT_TIB, Self))); // x64
#else
            return reinterpret_cast<TEB*>(__readfsdword(offsetof(NT_TIB, Self))); // x86
#endif
        }
    };

    constexpr inline modules_iterator modules{};

    [[nodiscard]] inline void* find_module(std::string_view Name) {
        std::wstring name(Name.begin(), Name.end());
        std::transform(name.begin(), name.end(), name.begin(), tolower);

        auto iter = std::find_if(modules.begin(), modules.end(),
            [&name](const LDR_DATA_TABLE_ENTRY& entry) {
                std::wstring entry_name(entry.FullDllName.Buffer);
                std::transform(entry_name.begin(), entry_name.end(), entry_name.begin(), tolower);
                return entry_name == name || entry_name.ends_with(name);
            });

        return iter == modules.end() ? nullptr : iter->DllBase;
    }

    [[nodiscard]] inline void* find_export(void* Module, std::string_view Name) {
        if (!Module)
            return nullptr;

        auto module_base  = reinterpret_cast<std::uint8_t*>(Module);
        auto dos_header   = reinterpret_cast<IMAGE_DOS_HEADER*>(Module);
        auto nt_headers   = reinterpret_cast<IMAGE_NT_HEADERS*>(module_base + dos_header->e_lfanew);
        auto data_dir     = &nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT];
        auto export_table = reinterpret_cast<IMAGE_EXPORT_DIRECTORY*>(module_base + data_dir->VirtualAddress);
        auto names_table  = reinterpret_cast<std::uint32_t*>(module_base + export_table->AddressOfNames);
        auto funcs_table  = reinterpret_cast<std::uint32_t*>(module_base + export_table->AddressOfFunctions);

        for (std::uint32_t i = 0; i < export_table->NumberOfFunctions; ++i) {
            if (Name == reinterpret_cast<const char*>(module_base + names_table[i]))
                return reinterpret_cast<void*>(module_base + funcs_table[i]);
        }

        return nullptr;
    }
}