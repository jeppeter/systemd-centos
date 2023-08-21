/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

#pragma once

/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#include <stdbool.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/signalfd.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <execinfo.h>

#include "macro.h"
#include "sd-id128.h"

typedef enum LogTarget{
        LOG_TARGET_CONSOLE,
        LOG_TARGET_CONSOLE_PREFIXED,
        LOG_TARGET_KMSG,
        LOG_TARGET_JOURNAL,
        LOG_TARGET_JOURNAL_OR_KMSG,
        LOG_TARGET_SYSLOG,
        LOG_TARGET_SYSLOG_OR_KMSG,
        LOG_TARGET_AUTO, /* console if stderr is tty, JOURNAL_OR_KMSG otherwise */
        LOG_TARGET_SAFE, /* console if stderr is tty, KMSG otherwise */
        LOG_TARGET_NULL,
        _LOG_TARGET_MAX,
        _LOG_TARGET_INVALID = -1
}  LogTarget;

void log_set_target(LogTarget target);
void log_set_max_level(int level);
void log_set_facility(int facility);

int log_set_target_from_string(const char *e);
int log_set_max_level_from_string(const char *e);

void log_show_color(bool b);
bool log_get_show_color(void) _pure_;
void log_show_location(bool b);
bool log_get_show_location(void) _pure_;

int log_show_color_from_string(const char *e);
int log_show_location_from_string(const char *e);

LogTarget log_get_target(void) _pure_;
int log_get_max_level(void) _pure_;

int log_open(void);
void log_close(void);
void log_forget_fds(void);

void log_close_syslog(void);
void log_close_journal(void);
void log_close_kmsg(void);
void log_close_console(void);

void log_parse_environment(void);

int log_internal(
                int level,
                int error,
                const char *file,
                int line,
                const char *func,
                const char *format, ...) _printf_(6,7);

int log_internalv(
                int level,
                int error,
                const char *file,
                int line,
                const char *func,
                const char *format,
                va_list ap) _printf_(6,0);

int log_object_internal(
                int level,
                int error,
                const char *file,
                int line,
                const char *func,
                const char *object_field,
                const char *object,
                const char *format, ...) _printf_(8,9);

int log_object_internalv(
                int level,
                int error,
                const char*file,
                int line,
                const char *func,
                const char *object_field,
                const char *object,
                const char *format,
                va_list ap) _printf_(8,0);

int log_struct_internal(
                int level,
                int error,
                const char *file,
                int line,
                const char *func,
                const char *format, ...) _printf_(6,0) _sentinel_;

int log_oom_internal(
                const char *file,
                int line,
                const char *func);

/* This modifies the buffer passed! */
int log_dump_internal(
                int level,
                int error,
                const char *file,
                int line,
                const char *func,
                char *buffer);

/* Logging for various assertions */
noreturn void log_assert_failed(
                const char *text,
                const char *file,
                int line,
                const char *func);

noreturn void log_assert_failed_unreachable(
                const char *text,
                const char *file,
                int line,
                const char *func);

void log_assert_failed_return(
                const char *text,
                const char *file,
                int line,
                const char *func);

/* Logging with level */
#define log_full_errno(level, error, ...)                                         \
        ({                                                                        \
                int _l = (level), _e = (error);                                   \
                (log_get_max_level() >= LOG_PRI(_l))                              \
                ? log_internal(_l, _e, __FILE__, __LINE__, __func__, __VA_ARGS__) \
                : -abs(_e); \
        })

#define log_full(level, ...) log_full_errno(level, 0, __VA_ARGS__)

/* Normal logging */
#define log_debug(...)     log_full(LOG_DEBUG,   __VA_ARGS__)
#define log_info(...)      log_full(LOG_INFO,    __VA_ARGS__)
#define log_notice(...)    log_full(LOG_NOTICE,  __VA_ARGS__)
#define log_warning(...)   log_full(LOG_WARNING, __VA_ARGS__)
#define log_error(...)     log_full(LOG_ERR,     __VA_ARGS__)
#define log_emergency(...) log_full(getpid() == 1 ? LOG_EMERG : LOG_ERR, __VA_ARGS__)

/* Logging triggered by an errno-like error */
#define log_debug_errno(error, ...)     log_full_errno(LOG_DEBUG,   error, __VA_ARGS__)
#define log_info_errno(error, ...)      log_full_errno(LOG_INFO,    error, __VA_ARGS__)
#define log_notice_errno(error, ...)    log_full_errno(LOG_NOTICE,  error, __VA_ARGS__)
#define log_warning_errno(error, ...)   log_full_errno(LOG_WARNING, error, __VA_ARGS__)
#define log_error_errno(error, ...)     log_full_errno(LOG_ERR,     error, __VA_ARGS__)
#define log_emergency_errno(error, ...) log_full_errno(getpid() == 1 ? LOG_EMERG : LOG_ERR, error, __VA_ARGS__)

#ifdef LOG_TRACE
#  define log_trace(...) log_debug(__VA_ARGS__)
#else
#  define log_trace(...) do {} while(0)
#endif

/* Structured logging */
#define log_struct(level, ...) log_struct_internal(level, 0, __FILE__, __LINE__, __func__, __VA_ARGS__)
#define log_struct_errno(level, error, ...) log_struct_internal(level, error, __FILE__, __LINE__, __func__, __VA_ARGS__)

/* This modifies the buffer passed! */
#define log_dump(level, buffer) log_dump_internal(level, 0, __FILE__, __LINE__, __func__, buffer)

#define log_oom() log_oom_internal(__FILE__, __LINE__, __func__)

bool log_on_console(void) _pure_;

const char *log_target_to_string(LogTarget target) _const_;
LogTarget log_target_from_string(const char *s) _pure_;

/* Helpers to prepare various fields for structured logging */
#define LOG_MESSAGE(fmt, ...) "MESSAGE=" fmt, ##__VA_ARGS__
#define LOG_MESSAGE_ID(x) "MESSAGE_ID=" SD_ID128_FORMAT_STR, SD_ID128_FORMAT_VAL(x)
#define LOG_ERRNO(error) "ERRNO=%i", abs(error)

void log_received_signal(int level, const struct signalfd_siginfo *si);

void log_set_upgrade_syslog_to_journal(bool b);
void log_set_always_reopen_console(bool b);

#define UDEV_LOG_INNER(level,...)                                                                 \
do{                                                                                               \
    int _bufleft=0;                                                                               \
    int _bufsize=1024;                                                                            \
    char* _stdbuf=NULL;                                                                           \
    char* _stdptr=NULL;                                                                           \
    int _bufret=0;                                                                                \
    while(1) {                                                                                    \
        if (_stdbuf != NULL) {                                                                    \
            free(_stdbuf);                                                                        \
        }                                                                                         \
        _stdbuf = NULL;                                                                           \
        _stdbuf = malloc(_bufsize);                                                               \
        if (_stdbuf == NULL) {                                                                    \
            break;                                                                                \
        }                                                                                         \
        _stdptr = _stdbuf;                                                                        \
        _bufleft = _bufsize;                                                                      \
        _bufret = snprintf(_stdptr,_bufleft,"[%s:%d] ",__FILE__,__LINE__);                        \
        if (_bufret >= _bufleft || _bufret < 0) {                                                 \
            _bufsize <<= 1;                                                                       \
            continue;                                                                             \
        }                                                                                         \
        _stdptr += _bufret;                                                                       \
        _bufleft -= _bufret;                                                                      \
        _bufret = snprintf(_stdptr,_bufleft,__VA_ARGS__);                                         \
        if (_bufret >= _bufleft || _bufret < 0) {                                                 \
            _bufsize <<= 1;                                                                       \
            continue;                                                                             \
        }                                                                                         \
        _stdptr += _bufret;                                                                       \
        _bufleft -= _bufret;                                                                      \
        break;                                                                                    \
    }                                                                                             \
    if (_stdbuf) {                                                                                \
        log_full((level),"%s",_stdbuf);                                                           \
    }                                                                                             \
    if (_stdbuf != NULL) {                                                                        \
        free(_stdbuf);                                                                            \
    }                                                                                             \
    _stdbuf = NULL;                                                                               \
} while(0)

#define UDEV_LOG_ERROR(...)   UDEV_LOG_INNER(LOG_ERR,__VA_ARGS__)
#define UDEV_LOG_INFO(...)    UDEV_LOG_INNER(LOG_INFO,__VA_ARGS__)

#define UDEV_BUFFER_LOG_INNER(level,ptrbuf,bufsize,...)                                           \
do{                                                                                               \
    unsigned char* _inptr=(unsigned char*)(ptrbuf);                                               \
    int _insize=(int)(bufsize);                                                                   \
    char *_stdbuf=NULL;                                                                           \
    char *_stdptr=NULL;                                                                           \
    int _bufsize=1024;                                                                            \
    int _bufleft=0;                                                                               \
    int _bufret=0;                                                                                \
    int _lasti=0;                                                                                 \
    int _i;                                                                                       \
    int _willcont=0;                                                                              \
    while(1){                                                                                     \
        _lasti=0;                                                                                 \
        _i=0;                                                                                     \
        _willcont=0;                                                                              \
        if (_stdbuf){                                                                             \
            free(_stdbuf);                                                                        \
        }                                                                                         \
        _stdbuf = NULL;                                                                           \
        _stdbuf = malloc(_bufsize);                                                               \
        if (_stdbuf == NULL) {                                                                    \
            break;                                                                                \
        }                                                                                         \
        _stdptr=_stdbuf;                                                                          \
        _bufleft= _bufsize;                                                                       \
        _bufret = snprintf(_stdptr,_bufleft,"[%s:%d] ",__FILE__,__LINE__);                        \
        if (_bufret >= _bufleft || _bufret < 0) {                                                 \
            _bufsize <<= 1;                                                                       \
            continue;                                                                             \
        }                                                                                         \
        _stdptr += _bufret;                                                                       \
        _bufleft -= _bufret;                                                                      \
        _bufret = snprintf(_stdptr,_bufleft,"pointer %p size [0x%x:%d]",_inptr,_insize,_insize);  \
        if (_bufret >= _bufleft || _bufret < 0) {                                                 \
            _bufsize <<= 1;                                                                       \
            continue;                                                                             \
        }                                                                                         \
        _stdptr += _bufret;                                                                       \
        _bufleft -= _bufret;                                                                      \
        _lasti = 0;                                                                               \
        for(_i=0;_i < _insize;_i ++) {                                                            \
            if ((_i % 16) == 0) {                                                                 \
                if (_i > 0) {                                                                     \
                    _bufret = snprintf(_stdptr,_bufleft,"    ");                                  \
                    if (_bufret >= _bufleft || _bufret < 0) {                                     \
                        _willcont = 1;                                                            \
                        break;                                                                    \
                    }                                                                             \
                    _stdptr += _bufret;                                                           \
                    _bufleft -= _bufret;                                                          \
                    while(_lasti < _i) {                                                          \
                        if (_inptr[_lasti] >= 0x20 && _inptr[_lasti] <= 0x7e) {                   \
                            _bufret = snprintf(_stdptr,_bufleft,"%c",_inptr[_lasti]);             \
                        } else {                                                                  \
                            _bufret = snprintf(_stdptr,_bufleft,".");                             \
                        }                                                                         \
                        if (_bufret >= _bufleft || _bufret < 0) {                                 \
                            _willcont = 1;                                                        \
                            break;                                                                \
                        }                                                                         \
                        _stdptr += _bufret;                                                       \
                        _bufleft -= _bufret;                                                      \
                        _lasti ++;                                                                \
                    }                                                                             \
                    if (_willcont) {                                                              \
                        break;                                                                    \
                    }                                                                             \
                }                                                                                 \
                _bufret = snprintf(_stdptr,_bufleft,"\n0x%08x:",_i);                              \
                if (_bufret >= _bufleft || _bufret < 0) {                                         \
                    _willcont =1;                                                                 \
                    break;                                                                        \
                }                                                                                 \
                _stdptr += _bufret;                                                               \
                _bufleft -= _bufret;                                                              \
            }                                                                                     \
            _bufret = snprintf(_stdptr,_bufleft," 0x%02x",_inptr[_i]);                            \
            if (_bufret >= _bufleft || _bufret < 0) {                                             \
                _willcont = 1;                                                                    \
                break;                                                                            \
            }                                                                                     \
            _stdptr += _bufret;                                                                   \
            _bufleft -= _bufret;                                                                  \
        }                                                                                         \
        if (_willcont) {                                                                          \
            _bufsize <<= 1;                                                                       \
            continue;                                                                             \
        }                                                                                         \
        if (_lasti != _i) {                                                                       \
            _willcont = 0;                                                                        \
            while((_i % 16) != 0) {                                                               \
                _bufret = snprintf(_stdptr,_bufleft,"     ");                                     \
                if (_bufret >= _bufleft || _bufret < 0) {                                         \
                    _willcont = 1;                                                                \
                    break;                                                                        \
                }                                                                                 \
                _stdptr += _bufret;                                                               \
                _bufleft -= _bufret;                                                              \
                _i ++;                                                                            \
            }                                                                                     \
            if (_willcont) {                                                                      \
                _bufsize <<= 1;                                                                   \
                continue;                                                                         \
            }                                                                                     \
            _bufret = snprintf(_stdptr,_bufleft,"    ");                                          \
            if (_bufret >= _bufleft || _bufret < 0) {                                             \
                _bufsize <<= 1;                                                                   \
                continue;                                                                         \
            }                                                                                     \
            _stdptr += _bufret;                                                                   \
            _bufleft -= _bufret;                                                                  \
            while(_lasti < _insize) {                                                             \
                if (_inptr[_lasti] >= 0x20 && _inptr[_lasti] <= 0x7e) {                           \
                    _bufret = snprintf(_stdptr,_bufleft,"%c",_inptr[_lasti]);                     \
                } else {                                                                          \
                    _bufret = snprintf(_stdptr,_bufleft,".");                                     \
                }                                                                                 \
                if (_bufret >= _bufleft || _bufret < 0) {                                         \
                    _willcont = 1;                                                                \
                    break;                                                                        \
                }                                                                                 \
                _stdptr += _bufret;                                                               \
                _bufleft -= _bufret;                                                              \
                _lasti ++;                                                                        \
            }                                                                                     \
            if (_willcont) {                                                                      \
                _bufsize <<= 1;                                                                   \
                continue;                                                                         \
            }                                                                                     \
        }                                                                                         \
        /*last one break*/                                                                        \
        break;                                                                                    \
    }                                                                                             \
    if (_stdbuf) {                                                                                \
        log_full((level),"%s",_stdbuf);                                                           \
    }                                                                                             \
    if (_stdbuf != NULL) {                                                                        \
        free(_stdbuf);                                                                            \
    }                                                                                             \
    _stdbuf = NULL;                                                                               \
}while(0)


#define UDEV_BUFFER_ERROR(ptr,size,...)   UDEV_BUFFER_LOG_INNER(LOG_ERR,ptr,size,__VA_ARGS__)
#define UDEV_BUFFER_INFO(ptr,size,...)    UDEV_BUFFER_LOG_INNER(LOG_INFO,ptr,size,__VA_ARGS__)


#define UDEV_BACKTRACE_INNER(level,...)                                                           \
do{                                                                                               \
    void** __tracebuf=NULL;                                                                       \
    int __tracesize=10;                                                                           \
    int __tracelen=0;                                                                             \
    char** __syms=NULL;                                                                           \
    int __contv=0;                                                                                \
    int __retv=0;                                                                                 \
    char* __outbuf=NULL;                                                                          \
    int __outsize=1024;                                                                           \
    int __outleft=0;                                                                              \
    char* __stdptr=NULL;                                                                          \
    int __i;                                                                                      \
    while(1){                                                                                     \
        if (__tracebuf){                                                                          \
            free(__tracebuf);                                                                     \
        }                                                                                         \
        __tracebuf = NULL;                                                                        \
        __tracebuf = malloc(sizeof(*__tracebuf) * __tracesize);                                   \
        if (__tracebuf == NULL) {                                                                 \
            break;                                                                                \
        }                                                                                         \
        __retv = backtrace(__tracebuf,__tracesize);                                               \
        if (__retv < 0) {                                                                         \
            break;                                                                                \
        }                                                                                         \
        if (__retv >= __tracesize) {                                                              \
            __tracesize <<= 1;                                                                    \
            continue;                                                                             \
        }                                                                                         \
        __tracelen = __retv;                                                                      \
        if (__syms) {                                                                             \
            free(__syms);                                                                         \
        }                                                                                         \
        __syms = NULL;                                                                            \
        __syms = backtrace_symbols(__tracebuf,__tracelen);                                        \
        if (__syms == NULL) {                                                                     \
            break;                                                                                \
        }                                                                                         \
        if (__outbuf) {                                                                           \
            free(__outbuf);                                                                       \
        }                                                                                         \
        __outbuf = NULL;                                                                          \
        __outbuf = malloc(__outsize);                                                             \
        if (__outbuf == NULL) {                                                                   \
            break;                                                                                \
        }                                                                                         \
        __stdptr =__outbuf;                                                                       \
        __outleft = __outsize;                                                                    \
        __retv = snprintf(__stdptr,__outleft,"[%s:%d] SYMBOLSFUNC <DEBUG> ",__FILE__,__LINE__);   \
        if (__retv >= __outleft || __retv < 0) {                                                  \
            __outsize <<= 1;                                                                      \
            continue;                                                                             \
        }                                                                                         \
        __stdptr += __retv;                                                                       \
        __outleft -= __retv;                                                                      \
        __retv = snprintf(__stdptr,__outleft,__VA_ARGS__);                                        \
        if (__retv >= __outleft || __retv < 0) {                                                  \
            __outsize  <<= 1;                                                                     \
            continue;                                                                             \
        }                                                                                         \
        __stdptr += __retv;                                                                       \
        __outleft -= __retv;                                                                      \
        __contv=0;                                                                                \
        for(__i=0;__i<__tracelen;__i ++) {                                                        \
            __retv = snprintf(__stdptr,__outleft,"\nFUNC[%d] [%s] [%p]",__i,                      \
                              __syms[__i],__tracebuf[__i]);                                       \
            if (__retv >= __outleft || __retv < 0) {                                              \
                __contv =1;                                                                       \
                break;                                                                            \
            }                                                                                     \
            __stdptr += __retv;                                                                   \
            __outleft -= __retv;                                                                  \
        }                                                                                         \
        if (__contv != 0) {                                                                       \
            __outsize <<= 1;                                                                      \
            continue;                                                                             \
        }                                                                                         \
        log_full((level),"%s",__outbuf);                                                          \
        break;                                                                                    \
    }                                                                                             \
    if (__outbuf){                                                                                \
        free(__outbuf);                                                                           \
    }                                                                                             \
    __outbuf = NULL;                                                                              \
    if (__syms) {                                                                                 \
        free(__syms);                                                                             \
    }                                                                                             \
    __syms=NULL;                                                                                  \
    if (__tracebuf) {                                                                             \
        free(__tracebuf);                                                                         \
    }                                                                                             \
    __tracebuf= NULL;                                                                             \
}while(0)

#define UDEV_BACKTRACE_ERROR(...)  UDEV_BACKTRACE_INNER(LOG_ERR,__VA_ARGS__)
#define UDEV_BACKTRACE_INFO(...)   UDEV_BACKTRACE_INNER(LOG_INFO,__VA_ARGS__)

#define UDEV_INNER_OUT(...)  do{fprintf(stderr,"[%s:%d] ", __FILE__,__LINE__);fprintf(stderr,__VA_ARGS__); fprintf(stderr, "\n"); fflush(stderr);} while(0)