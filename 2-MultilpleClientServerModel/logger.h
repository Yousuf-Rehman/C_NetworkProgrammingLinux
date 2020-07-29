
#ifndef LOGGER_H
#define LOGGER_H

// header contents
void loggerRemove();
void logger(const char *format, ...);
void logger_ConstFormat_Error(char *func, char *msg, int err);
void logger_ConstFormat_Debug(char *func, char *msg);
void logger_ConstFormat_Info(char *func, char *msg);

#endif