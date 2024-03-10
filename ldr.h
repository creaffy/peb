#pragma once
#include <string>
#include <windows.h>
#include <winternl.h>

namespace ldr {
    class iterator {
    public:
        iterator()
            : _Entry(0) {
        }

        iterator(LIST_ENTRY* Ptr)
            : _Entry(Ptr) {
        }

        static iterator begin() {
            return iterator(_GetTeb()->ProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList.Flink);
        }

        static iterator end() {
            return iterator(&_GetTeb()->ProcessEnvironmentBlock->Ldr->InMemoryOrderModuleList);
        }

        const LDR_DATA_TABLE_ENTRY* get() {
            return CONTAINING_RECORD(_Entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        }

        iterator& operator++() {
            _Entry = _Entry->Flink;
            return *this;
        }

        bool operator!=(const iterator& Iterator) {
            return _Entry != Iterator._Entry;
        }

        const LDR_DATA_TABLE_ENTRY& operator*() {
            return *CONTAINING_RECORD(_Entry, LDR_DATA_TABLE_ENTRY, InMemoryOrderLinks);
        }

        const LDR_DATA_TABLE_ENTRY* operator->() {
            return get();
        }

    private:
        LIST_ENTRY* _Entry;

        static inline TEB* _GetTeb() {
#ifdef _M_X64
            return reinterpret_cast<TEB*>(__readgsqword(offsetof(NT_TIB, Self))); // x64
#else
            return reinterpret_cast<TEB*>(__readfsdword(offsetof(NT_TIB, Self))); // x86
#endif
        }
    };

    const LDR_DATA_TABLE_ENTRY* get_module(const std::string_view ModuleName) {
        std::wstring _LcTargetName(ModuleName.begin(), ModuleName.end());
        for (auto& e : _LcTargetName) {
            e = ::tolower(e);
        };
        for (auto it = iterator::begin(); it != iterator::end(); ++it) {
            std::wstring _LcThisName(it->FullDllName.Buffer);
            for (auto& e : _LcThisName) {
                e = ::tolower(e);
            };
            std::wstring_view _LcThisFileName(_LcThisName.begin() + _LcThisName.find_last_of('\\') + 1, _LcThisName.end());
            if (_LcTargetName == _LcThisName || _LcTargetName == _LcThisFileName)
                return it.get();
        }
        return nullptr;
    }
}
