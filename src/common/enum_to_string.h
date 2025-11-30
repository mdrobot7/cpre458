#ifndef _ENUM_TO_STRING_H
#define _ENUM_TO_STRING_H

#include "common.h"

#include <string.h>

/*
 * ------------------------- Example Usage ----------------------------
 * some_enum.h:
 *
 *    #include "enum_to_string.h"
 *    #define SOME_ENUM(XX)
 *       XX(FIRST_VALUE, , "One")
 *       XX(SECOND_VALUE, , "Two")
 *       XX(SOME_OTHER_VALUE,= 50, "Other")
 *       XX(ONE_MORE_VALUE,= 100, "LAST")
 *    DECLARE_ENUM(SomeEnum, SOME_ENUM)
 *
 * some_enum.c:
 *
 *    #include "some_enum.h"
 *    DEFINE_ENUM(SomeEnum, SOME_ENUM)
 *
 * ------------------------- Expands to ----------------------------
 *
 * some_enum.h:
 *
 *    typedef enum {
 *        FIRST_VALUE  ,
 *        SECOND_VALUE  ,
 *        SOME_OTHER_VALUE  = 2,
 *        ONE_MORE_VALUE  = 3,
 *        SomeEnum_COUNT
 *    } SomeEnum;
 *    const char* SomeEnum_to_string( SomeEnum value );
 *
 * some_enum.c:
 *
 *    const char * SomeEnum_to_string(SomeEnum value)
 *    {
 *      switch(value)
 *      {
 *        case FIRST_VALUE: return "One";
 *        case SECOND_VALUE: return "Two";
 *        case SOME_OTHER_VALUE: return "Other";
 *        case ONE_MORE_VALUE: return "LAST";
 *        case some_enum_COUNT: return "_COUNT";
 *        default:
 *        {  // handle error
 *            ASSERT( false );
 *        }
 *      }
 *    }
 */

// expansion macro for enum value definition
#define ENUM_VALUE(name, value, string) name value,

// expansion macro for enum to string conversion
#define ENUM_TO_STRING_CASE(name, value, string) \
  case name:                                     \
    return string;

// expansion macro for string to enum conversion
#define ENUM_TO_ENUM_CASE(name, value, enumString)      \
  if (0 == strcasecmp(enumString, string_to_compare)) { \
    *output = name;                                     \
    return true;                                        \
  }

// expansion macro for enum to string conversion
#define ENUM_VALID_CASE(name, value, string) \
  case name:                                 \
    return true;


// declare enum type
#define ENUM_DECLARE(EnumType, ENUM_DEF)   \
  typedef enum {                           \
    ENUM_DEF(ENUM_VALUE) EnumType##_COUNT, \
  } EnumType;

// declare enum helper functions
#define ENUM_DECLARE_HELPERS(EnumType, ENUM_DEF)                                        \
  /**                                                                                   \
   * @brief Convert enum value to string.                                               \
   *                                                                                    \
   * @param value Enum value                                                            \
   * @return Converted string                                                           \
   */                                                                                   \
  const char * EnumType##_to_string(const EnumType value);                              \
                                                                                        \
  /**                                                                                   \
   * @brief Convert string to enum value.                                               \
   *                                                                                    \
   * @param string_to_compare String to convert                                         \
   * @param output Pointer to output value                                              \
   * @return True on success, false otherwise                                           \
   */                                                                                   \
  bool EnumType##_from_string(const char * string_to_compare, EnumType * const output); \
                                                                                        \
  /**                                                                                   \
   * @brief Check if an enum value is valid.                                            \
   *                                                                                    \
   * @param value Enum value                                                            \
   * @return True if value is valid, false otherwise                                    \
   */                                                                                   \
  bool EnumType##_is_valid(const int32_t value);                                        \
                                                                                        \
  /**                                                                                   \
   * @brief Convert an integer to an enum, checking validity.                           \
   *                                                                                    \
   * @param value Integer value to convert                                              \
   * @return Converted enum value on success, EnumType_COUNT on failure                 \
   */                                                                                   \
  EnumType EnumType##_to_enum(const int32_t value);

// define enum helper functions
#define ENUM_DEFINE_HELPERS(EnumType, ENUM_DEF)                                                \
  const char * EnumType##_to_string(const EnumType value) {                                    \
    switch (value) {                                                                           \
      ENUM_DEF(ENUM_TO_STRING_CASE)                                                            \
      case EnumType##_COUNT:                                                                   \
        return "_COUNT";                                                                       \
      default: { /* handle error */                                                            \
        return ("Invalid value for " #EnumType);                                               \
      }                                                                                        \
    }                                                                                          \
  }                                                                                            \
                                                                                               \
  bool EnumType##_from_string(const char * const string_to_compare, EnumType * const output) { \
    ENUM_DEF(ENUM_TO_ENUM_CASE)                                                                \
    return false;                                                                              \
  }                                                                                            \
                                                                                               \
  bool EnumType##_is_valid(const int32_t value) {                                              \
    switch (value) {                                                                           \
      ENUM_DEF(ENUM_VALID_CASE)                                                                \
                                                                                               \
      /* assumes _COUNT isn't valid */                                                         \
      default:                                                                                 \
        return false;                                                                          \
    }                                                                                          \
  }                                                                                            \
                                                                                               \
  EnumType EnumType##_to_enum(const int32_t value) {                                           \
    if (EnumType##_is_valid(value)) {                                                          \
      return (EnumType) value;                                                                 \
    } else {                                                                                   \
      return EnumType##_COUNT;                                                                 \
    }                                                                                          \
  }

#endif
