#ifndef DB_SQLITEDB_H
#define DB_SQLITEDB_H

#include <sqlpp11/table.h>
#include <sqlpp11/data_types.h>
#include <sqlpp11/char_sequence.h>

namespace db
{
  namespace USERS_
  {
    struct ID
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "ID";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T ID;
            T& operator()() { return ID; }
            const T& operator()() const { return ID; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::integer, sqlpp::tag::require_insert>;
    };
    struct CITY
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "CITY";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T CITY;
            T& operator()() { return CITY; }
            const T& operator()() const { return CITY; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
    struct COUNTRY
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "COUNTRY";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T COUNTRY;
            T& operator()() { return COUNTRY; }
            const T& operator()() const { return COUNTRY; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
    struct LANG
    {
      struct _alias_t
      {
        static constexpr const char _literal[] =  "LANG";
        using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
        template<typename T>
        struct _member_t
          {
            T LANG;
            T& operator()() { return LANG; }
            const T& operator()() const { return LANG; }
          };
      };
      using _traits = sqlpp::make_traits<sqlpp::text, sqlpp::tag::require_insert>;
    };
  } // namespace USERS_

  struct USERS: sqlpp::table_t<USERS,
               USERS_::ID,
               USERS_::CITY,
               USERS_::COUNTRY,
               USERS_::LANG>
  {
    struct _alias_t
    {
      static constexpr const char _literal[] =  "USERS";
      using _name_t = sqlpp::make_char_sequence<sizeof(_literal), _literal>;
      template<typename T>
      struct _member_t
      {
        T USERS;
        T& operator()() { return USERS; }
        const T& operator()() const { return USERS; }
      };
    };
  };
} // namespace db
#endif
