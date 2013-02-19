#include <dlfcn.h>
#include <errno.h>
#include <cstring>

#include <boost/filesystem/path.hpp>

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
   typedef void * library_handle_t;

   static library_handle_t open_library(const std::string& library_name, bool resolve_symbols);
   static const char *default_suffix();
   std::string get_last_error() const;
   bool is_file_not_found_error(const std::string& error) const;

   std::string m_library_path;
   library_handle_t m_library;
};

dynamic_library_impl::dynamic_library_impl(const std::string& library_name, bool resolve_symbols)
{
   std::string lib_path(library_name);
   std::string error;

   m_library = open_library(lib_path, resolve_symbols);

   if (!m_library)
   {
      error = get_last_error();

      if (is_file_not_found_error(error))
      {
         lib_path += default_suffix();

         m_library = open_library(lib_path, resolve_symbols);

         if (!m_library)
         {
            error = get_last_error();

            if (is_file_not_found_error(error))
            {
               boost::filesystem::path path(lib_path);
               lib_path = (path.parent_path()/("lib" + path.filename())).string();

               m_library = open_library(lib_path, resolve_symbols);

               if (!m_library)
               {
                  error = get_last_error();
               }
            }
         }
      }
   }

   if (!m_library)
   {
      if (is_file_not_found_error(error))
      {
         throw library_not_found_error(error);
      }
      else
      {
         throw library_load_error(error);
      }
   }

   m_library_path = lib_path;
}

bool dynamic_library_impl::is_file_not_found_error(const std::string& error) const
{
   char buf[128] = {'\0'};
   const char *enoent = &buf[0];

#if (_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && ! _GNU_SOURCE
   strerror_r(ENOENT, buf, sizeof(buf));
#else
   enoent = strerror_r(ENOENT, buf, sizeof(buf));
#endif

   return error.rfind(enoent) != std::string::npos;
}

dynamic_library_impl::~dynamic_library_impl()
{
   try
   {
      dlclose(m_library);
   }
   catch (...)
   {
      // there's nothing we can do here
   }
}

dynamic_library_impl::library_handle_t dynamic_library_impl::open_library(const std::string& library_name, bool resolve_symbols)
{
   return dlopen(library_name.c_str(), RTLD_LOCAL | (resolve_symbols ? RTLD_NOW : RTLD_LAZY));
}

const char *dynamic_library_impl::default_suffix()
{
#if defined(hpux) || defined(_hpux) || defined(__hpux)
   return ".sl";
#else
   return ".so";
#endif
}

void *dynamic_library_impl::get_symbol_by_name(const std::string& symbol_name) const
{
   return dlsym(m_library, symbol_name.c_str());
}

std::string dynamic_library_impl::get_last_error() const
{
   try
   {
      return std::string(dlerror());
   }
   catch (...)
   {
      return std::string("unknown error");
   }
}

}}
