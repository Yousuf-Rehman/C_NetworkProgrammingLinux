#include <stdio.h> //IO.. printf, scanf, etc
#include <time.h>
#include <stdarg.h>
#include "msgToPrint.h"
#include "logger.h"

void loggerRemove(char *FileName)
{
  remove(FileName);
}

void logger(const char *format, ...)
{
  FILE *fptr = fopen(CLIENT_LOG_FILE, "a"); //append data
  if (fptr == NULL)
  {
    printf("logger(): Could not open file");
    return;
  }

  time_t now;
  char timebuffer[20]; //time
  va_list args;

  va_start(args, format);
  time(&now);
  strftime(timebuffer, sizeof(timebuffer), "%Y-%m-%d %H:%M:%S", gmtime(&now));
  fprintf(fptr, "[%s]  ", timebuffer); //Time format is seperate
  vfprintf(fptr, format, args);
  fputc('\n', fptr);
  va_end(args);
  fclose(fptr);
}

void logger_ConstFormat_Error(char *func, char *msg, int err)
{
  logger("[%s]  %s: %s, error value: %d\n", "ERROR", func, msg, err);
}

void logger_ConstFormat_Debug(char *func, char *msg)
{
  logger("[%s]  %s: %s\n", "DEBUG", func, msg);
}

void logger_ConstFormat_Info(char *func, char *msg)
{
  logger("[%s]  %s: %s\n", "INFO", func, msg);
}