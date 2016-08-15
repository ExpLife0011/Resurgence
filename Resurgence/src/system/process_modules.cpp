#include <system/process_modules.hpp>
#include <system/process.hpp>
#include <misc/exceptions.hpp>
#include <misc/winnt.hpp>

namespace resurgence
{
    namespace system
    {
        auto startsWith = [](const std::wstring& str1, const std::wstring& str2, bool ignoreCasing) {
            if(ignoreCasing) {
                std::wstring copy1 = str1, copy2 = str2;
                std::transform(copy1.begin(), copy1.end(), copy1.begin(), ::tolower);
                std::transform(copy2.begin(), copy2.end(), copy2.begin(), ::tolower);
                return copy1.compare(0, copy2.size(), copy2) == 0;
            } else {
                return str1.compare(0, str2.size(), str2) == 0;
            }
        };

        module::module()
        {
        }
        module::module(process* proc, PLDR_DATA_TABLE_ENTRY entry)
        {
            if(proc->is_current_process()) {
                _base = (std::uint8_t*)entry->DllBase;
                _size = (std::size_t)entry->SizeOfImage;
                _name = entry->BaseDllName.Buffer;
                _path = entry->FullDllName.Buffer;
            } else {
                _base = (std::uint8_t*)entry->DllBase;
                _size = (std::size_t)entry->SizeOfImage;
                _name = proc->memory()->read_unicode_string(entry->BaseDllName.Buffer, entry->BaseDllName.Length / sizeof(WCHAR));
                _path = proc->memory()->read_unicode_string(entry->FullDllName.Buffer, entry->FullDllName.Length / sizeof(WCHAR));
            }
        }
        module::module(process* proc, PLDR_DATA_TABLE_ENTRY32 entry)
        {
            _base = (std::uint8_t*)entry->DllBase;
            _size = (std::size_t)entry->SizeOfImage;
            _name = proc->memory()->read_unicode_string(entry->BaseDllName.Buffer, entry->BaseDllName.Length / sizeof(WCHAR));
            _path = proc->memory()->read_unicode_string(entry->FullDllName.Buffer, entry->FullDllName.Length / sizeof(WCHAR));

            auto system32 = std::wstring(USER_SHARED_DATA->NtSystemRoot) + L"\\System32";
            auto syswow64 = std::wstring(USER_SHARED_DATA->NtSystemRoot) + L"\\SysWOW64";
            if(startsWith(_path, std::wstring(USER_SHARED_DATA->NtSystemRoot) + L"\\System32", TRUE)) {
                _path = _path.replace(0, system32.size(), syswow64);
            }
        }
        module::module(PRTL_PROCESS_MODULE_INFORMATION entry)
        {
            WCHAR path[MAX_PATH] = {NULL};

            _base = (std::uint8_t*)entry->ImageBase;
            _size = (std::size_t)entry->ImageSize;
            MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, (const char*)entry->FullPathName, 256, path, MAX_PATH);
            _name = (path + entry->OffsetToFileName);
            _path = misc::winnt::get_dos_path(path);
        }

        process_modules::process_modules(process* proc)
            : _process(proc)
        {
        }
        process_modules::process_modules()
            : _process(nullptr)
        {

        }
        std::vector<module> process_modules::get_all_modules()
        {
            using namespace misc;
            assert(_process != nullptr);

            std::vector<module> modules;

            auto id = _process->get_pid();
            auto handle = _process->get_handle();

            if(id != SYSTEM_IDLE_PROCESS_ID) {
                if(id == SYSTEM_PROCESS_ID) {
                    winnt::enumerate_system_modules([&](PRTL_PROCESS_MODULE_INFORMATION info) {
                        modules.emplace_back(info);
                        return STATUS_NOT_FOUND;
                    });
                } else if(handle.is_valid()) {
                    winnt::enumerate_process_modules(handle.get(), [&](PLDR_DATA_TABLE_ENTRY entry) {
                        modules.emplace_back(_process, entry);
                        return STATUS_NOT_FOUND;
                    });
                    if(_process->get_platform() == platform_x86) {
                        std::vector<module> modules32;
                        winnt::enumerate_process_modules32(handle.get(), [&](PLDR_DATA_TABLE_ENTRY32 entry) {
                            modules32.emplace_back(_process, entry);
                            return STATUS_NOT_FOUND;
                        });
                        if(modules32.size() > 1) {
                            auto begin = ++std::begin(modules32);
                            auto end = std::end(modules32);
                            modules.insert(std::end(modules), begin, end);
                        }
                    }
                }
            }
            return modules;
        }
        module process_modules::get_module_by_name(const std::wstring& name)
        {
            throw;
        }
        module process_modules::get_module_by_handle(HANDLE handle)
        {
            throw;
        }
        module process_modules::get_module_by_load_order(int i)
        {
            throw;
        }
    }
}