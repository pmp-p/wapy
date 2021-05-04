#ifndef WAPY_DEBUG_H

#include <stdio.h>
#include <stdarg.h>

extern FILE *fd_logger;
extern int VMFLAGS_IF;
extern int show_os_loop(int state);

#ifndef PSTR
#define PSTR(X) X
#endif

#if defined(NO_NLR) && !defined(MPYCROSS)
#define cdbg(fmt, ...) if (show_os_loop(-1)){ fprintf(fd_logger,fmt "\n", ##__VA_ARGS__ ); }
#else
#define cdbg(fmt, ...) if (1){ fprintf(fd_logger, PSTR(__FILE__ ":" TOSTRING(__LINE__) " " fmt), ##__VA_ARGS__ );fprintf(fd_logger, "\n"); }
#endif
//#define clog(...) if (show_os_loop(-1)){ fprintf(fd_logger, __VA_ARGS__ );fprintf(fd_logger, "\n"); }


#define WAPY_DEBUG_H
#endif

