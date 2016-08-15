#pragma once

#include <headers.hpp>

namespace resurgence
{
    namespace system
    {
        class process;

        class process_memory
        {
        public:
            process_memory(process* proc);

            void                                read_bytes(std::uint8_t* address, std::uint8_t* buffer, std::size_t size);
            void                                write_bytes(std::uint8_t* address, std::uint8_t* buffer, std::size_t size);
            template<typename _Ty> _Ty          read(std::uint8_t* address);
            template<typename _Ty> void         write(std::uint8_t* address, const _Ty& buffer, std::size_t size = sizeof(_Ty));
            template<typename _Ty> std::string  read_string(_Ty address, std::size_t length);
            template<typename _Ty> std::wstring read_unicode_string(_Ty address, std::size_t length);

        private:
            friend class process;
            process_memory();

            process* _process;
        };

        template<typename _Ty> _Ty process_memory::read(std::uint8_t* address)
        {
            _Ty buffer;
            read_bytes(address, (std::uint8_t*)&buffer, sizeof(_Ty));
            return buffer;
        }
        template<typename _Ty> void process_memory::write(std::uint8_t* address, const _Ty& buffer, std::size_t size /*= sizeof(_Ty)*/)
        {
            write_bytes(address, (std::uint8_t*)&buffer, size);
        }
        template<typename _Ty> std::string process_memory::read_string(_Ty address, std::size_t length)
        {
            std::string str;
            char* buffer = new char[length + 1]();
            read_bytes((std::uint8_t*)address, (std::uint8_t*)buffer, length);
            str = std::string(buffer);
            delete[] buffer;
            return str;
        }
        template<typename _Ty> std::wstring process_memory::read_unicode_string(_Ty address, std::size_t length)
        {
            std::wstring str;
            wchar_t* buffer = new wchar_t[length+1]();
            read_bytes((std::uint8_t*)address, (std::uint8_t*)buffer, length * sizeof(WCHAR));
            str = std::wstring(buffer);
            delete[] buffer;
            return str;
        }
    }
}