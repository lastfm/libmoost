#include <windows.h>

namespace moost { namespace pdl {

class dynamic_library_impl : public dynamic_library_if
{
public:
   dynamic_library_impl(const std::string& library_name, bool resolve_symbols);

   ~dynamic_library_impl();

   void *get_symbol_by_name(const std::string& symbol_name) const;

   const std::string& library_path() const
   {
      return m_library_path;
   }

private:
   typedef HMODULE library_handle_t;

   static library_handle_t open_library(const std::string& library_name, bool resolve_symbols);
   DWORD get_last_error() const;
   std::string format_error(DWORD error) const;
   bool is_file_not_found_error(DWORD error) const;

   std::string m_library_path;
   library_handle_t m_library;
};

dynamic_library_impl::dynamic_library_impl(const std::string& library_name, bool resolve_symbols)
{
   std::string lib_path(library_name);
   DWORD error;

   m_library = open_library(lib_path, resolve_symbols);

   if (!m_library)
   {
      error = get_last_error();

      if (is_file_not_found_error(error))
      {
         lib_path += ".dll";

         m_library = open_library(lib_path, resolve_symbols);

         if (!m_library)
         {
            error = get_last_error();
         }
      }
   }

   if (!m_library)
   {
      std::string errstr = format_error(error);

      if (is_file_not_found_error(error))
      {
         throw library_not_found_error(errstr);
      }
      else
      {
         throw library_load_error(errstr);
      }
   }

   m_library_path = lib_path;
}

bool dynamic_library_impl::is_file_not_found_error(DWORD error) const
{
   return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND;
}

dynamic_library_impl::~dynamic_library_impl()
{
   try
   {
      ::FreeLibrary(m_library);
   }
   catch (...)
   {
      // there's nothing we can do here
   }
}

dynamic_library_impl::library_handle_t dynamic_library_impl::open_library(const std::string& library_name, bool resolve_symbols)
{
   return ::LoadLibraryExA(library_name.c_str(), NULL, resolve_symbols ? 0 : DONT_RESOLVE_DLL_REFERENCES);
}

void *dynamic_library_impl::get_symbol_by_name(const std::string& symbol_name) const
{
   return static_cast<void *>(::GetProcAddress(m_library, symbol_name.c_str()));
}

DWORD dynamic_library_impl::get_last_error() const
{
   return ::GetLastError();
}

std::string dynamic_library_impl::format_error(DWORD error) const
{
   try
   {
      LPSTR msg_buf = NULL;
      const DWORD res =
         ::FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                          NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                          reinterpret_cast<LPSTR>(&msg_buf), 0, NULL);
      std::string error;
      if (res != 0)
      {
         error.assign(msg_buf);
         ::LocalFree(msg_buf);
      }
      return error;
   }
   catch (...)
   {
      return std::string("unknown error");
   }
}

}}
