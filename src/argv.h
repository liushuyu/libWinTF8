/*
  Copyright (c) 2014 StarBrilliant <m13253@hotmail.com>
  All rights reserved.

  Redistribution and use in source and binary forms are permitted
  provided that the above copyright notice and this paragraph are
  duplicated in all such forms and that any documentation,
  advertising materials, and other materials related to such
  distribution and use acknowledge that the software was developed by
  StarBrilliant.
  The name of StarBrilliant may not be used to endorse or promote
  products derived from this software without specific prior written
  permission.

  THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
  IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
*/
#pragma once
#ifndef WTF8_ARGV_H_INCLUDED_
#define WTF8_ARGV_H_INCLUDED_

#ifdef __cplusplus
#include <vector>
#endif
#include "u8str.h"

#ifdef __cplusplus
/*!
 *  \addtogroup WTF8
 * libWinTF8 API for C++
 *  @{
 */
namespace WTF8 {

/*!\brief Get the command line arguments of this process
 *
 * \return
 *   The process name is in [0]
 *   Other arguments are followed
 *
 * \throws
 *   std::runtime_error
 */
std::vector<u8string> getargv();

}
#endif
/*! @} End of Doxygen Groups*/
/*!
 *  \addtogroup WTF8_C
 * libWinTF8 API extension for C
 *  @{
 */
#ifdef __cplusplus
extern "C" {
#endif

/*!\brief Get the command line arguments of this process
 *
 * \return
 *   An array of C strings
 *   The process name is in [0]
 *   Other arguments are followed
 *
 * \throws
 *   Upon error, returns NULL
 *
 * \remark
 *   The result must be released with `WTF8_freeargv`
 */
char **WTF8_getargv(int *argc);

/*!\brief Free the memory that was allocated with `WTF8_getargv`
 *
 * \result
 *   NULL
 */
char **WTF8_freeargv(char **argv);

#ifdef __cplusplus
}
#endif

#endif
/*! @} End of Doxygen Groups*/
