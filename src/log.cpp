/*!
 * @file       log.cpp
 * @brief      Defines the Fastcgipp debugging/logging facilities
 * @author     Eddie Carle &lt;eddie@isatec.ca&gt;
 * @date       May 18, 2016
 * @copyright  Copyright &copy; 2016 Eddie Carle. This project is released under
 *             the GNU Lesser General Public License Version 3.
 */

/*******************************************************************************
* Copyright (C) 2016 Eddie Carle [eddie@isatec.ca]                             *
*                                                                              *
* This file is part of fastcgi++.                                              *
*                                                                              *
* fastcgi++ is free software: you can redistribute it and/or modify it under   *
* the terms of the GNU Lesser General Public License as  published by the Free *
* Software Foundation, either version 3 of the License, or (at your option)    *
* any later version.                                                           *
*                                                                              *
* fastcgi++ is distributed in the hope that it will be useful, but WITHOUT ANY *
* WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS    *
* FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for     *
* more details.                                                                *
*                                                                              *
* You should have received a copy of the GNU Lesser General Public License     *
* along with fastcgi++.  If not, see <http://www.gnu.org/licenses/>.           *
*******************************************************************************/

#include "fastcgi++/log.hpp"

#include <iomanip>
#include <iostream>
#include <ctime>
#include <codecvt>
#include <cstring>
#include <array>
#include <sstream>

#include <unistd.h>
#include <limits.h>
#include <sys/types.h>

//! Topmost namespace for the fastcgi++ library
namespace Fastcgipp
{
    //! Contains the Fastcgipp debugging/logging mechanism
    namespace Logging
    {
        std::wstring getHostname()
        {
            char buffer[HOST_NAME_MAX+2];
            gethostname(buffer, sizeof(buffer));
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            try
            {
                return(converter.from_bytes(
                            buffer,
                            buffer+std::strlen(buffer)));
            }
            catch(const std::range_error& e)
            {
                WARNING_LOG("Error in hostname code conversion from utf8")
                return std::wstring(L"localhost");
            }

        }

        std::wstring getProgram()
        {
            std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
            std::wostringstream ss;
            try
            {
                ss << converter.from_bytes(
                        program_invocation_name,
                        program_invocation_name
                            +std::strlen(program_invocation_name));
            }
            catch(const std::range_error& e)
            {
                WARNING_LOG("Error in program name code conversion from utf8")
                ss << "unknown";
            }

            ss << '[' << getpid() << ']';
            return ss.str();
        }

        std::array<std::wstring, 6> levels
        {{
            L"[info]: ",
            L"[fail]: ",
            L"[error]: ",
            L"[warning]: ",
            L"[debug]: ",
            L"[diagnostic]: "
        }};
    }
}

std::wostream* Fastcgipp::Logging::logstream(&std::wcerr);
std::mutex Fastcgipp::Logging::mutex;
bool Fastcgipp::Logging::suppress(false);
std::wstring Fastcgipp::Logging::hostname(Fastcgipp::Logging::getHostname());
std::wstring Fastcgipp::Logging::program(Fastcgipp::Logging::getProgram());

void Fastcgipp::Logging::header(Level level)
{
    const std::time_t now = std::time(nullptr);
    *logstream
        << std::put_time(std::localtime(&now), L"%b %d %H:%M:%S ")
        << hostname << ' ' << program << ' ' << levels[level];
}

#include <stdio.h>
#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

void print_backtrace() {
  dprintf(STDERR_FILENO, "=====> backtrace:\n");
  void *array[30];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 30);

  // print out all the frames to stderr
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  dprintf(STDERR_FILENO, "\n");
}

void faultHandler(int sig) {
  dprintf(STDERR_FILENO, "Error: signal %d:\n", sig);
  print_backtrace();
  exit(1);
}

void terminateHandler() {
    faultHandler();
}

std::string wstring2string(const std::wstring &toConvert) {
    //std::wstring string_to_convert;

    //setup converter
    using convert_type = std::codecvt_utf8<wchar_t>;
    std::wstring_convert<convert_type, wchar_t> converter;

    //use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
    std::string converted_str = converter.to_bytes( toConvert );
    return converted_str;
}

