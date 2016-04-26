/* Sean Heintz - 10053525 */

#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#define ERROR_OUT_BUFFER 1024

void panic(const char* msg, ...);
void warning(const char* msg, ...);
void info(const char* msg, ...);

void error(const char* filename, int line, const char* msg, ...);

void message(_IO_FILE* out, const char* msg);

#endif 
