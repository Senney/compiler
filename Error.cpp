/* Sean Heintz - 10053525 */
#include "Error.h"

extern bool Verbose;

/* Prints a message with the FATAL indication. */
/* Expects a character array */
void panic(const char* msg, ...) {
	va_list argptr;
	va_start(argptr, msg);

	char pref[] = "[FATAL]\t";
	char panicmsg[ERROR_OUT_BUFFER];
	
	strcpy(panicmsg, pref);
	strcat(panicmsg, msg);
	vsprintf(panicmsg, panicmsg, argptr);

	message(stderr, panicmsg);
}

/* Prints a message with the WARN indication */
void warning(const char* msg, ...) {
	va_list argptr;
	va_start(argptr, msg);
	
	char pref[] = "[WARN]\t";
	char panicmsg[ERROR_OUT_BUFFER];
	
	strcpy(panicmsg, pref);
	strcat(panicmsg, msg);
	vsprintf(panicmsg, panicmsg, argptr);
	
	message(stderr, panicmsg);
}

/* Prints a message with the INFO indication */
void info(const char* msg, ...) {
	if (!Verbose) return;

	va_list argptr;
	va_start(argptr, msg);
	
	char pref[] = "[INFO]\t";
	char panicmsg[ERROR_OUT_BUFFER];
	
	strcpy(panicmsg, pref);
	strcat(panicmsg, msg);
	vsprintf(panicmsg, panicmsg, argptr);
	
	message(stderr, panicmsg);
}

/* Prints a line with error information attached. For compiler errors. */
void error(const char* filename, int line, const char* msg, ... ) {
        va_list argptr;
        va_start(argptr, msg);
        
        char pref[] = "[ERROR] [%s:%d]\t";
        fprintf(stderr, pref, filename, line);
        
        char errormsg[ERROR_OUT_BUFFER];
        strcpy(errormsg, msg);
        //vsprintf(errormsg, errormsg, argptr);
        
        //message(stderr, errormsg);
}


void message(_IO_FILE* out, const char* msg) {
	fprintf(out, msg, NULL);
}

