
/***************************************************************************
 * logging facility for libdc1394
 *
 * These functions provide the logging of error, warning and debug messages
 * They allow registering of custom logging functions or the use
 * of the builtin loggers which redirect the output to stderr.
 * Three log levels are supported:
 * error:   Indicates that an error has been detected which mandates
 *          shutdown of the program as soon as feasible
 * warning: Indicates that something happened which prevents libdc1394
 *          from working but which could possibly be resolved by the
 *          application or the user: plugging in a camera, resetting the
 *          firewire bus, ....
 * debug:   A sort of way point for the library. This log level is supposed
 *          to report that a specific function has been entered or has 
 *          passed a certain stage. This log level is turned off by default
 *          and may produce a lot of output during regular operation.
 *          The main purpose for this log level is for debugging libdc1394
 *          and for generating meaningful problem reports.
 ***************************************************************************/

#include "control.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>


#ifndef __DC1394_LOG_H__
#define __DC1394_LOG_H__

extern void(*system_errorlog_handler)(dc1394log_t type, const char *message, const char *file, int line, const char *function, void* user);
extern void(*system_warninglog_handler)(dc1394log_t type, const char *message, const char *file, int line, const char *function, void* user);
extern void(*system_debuglog_handler)(dc1394log_t type, const char *message, const char *file, int line, const char *function, void* user);

#if ! defined (_MSC_VER)
/* Error logging/checking macros. Logs an error string on stderr and exit current function
   if error is positive. Neg errors are messages and are thus ignored */

/**
 * dc1394_log_error: logs a fatal error condition to the registered facility
 * This function shall be invoked if a fatal error condition is encountered.
 * The message passed as argument is delivered to the registered error reporting
 * function registered before.
 * @param [in] message: error message to be logged
 */
//void dc1394_log_error(const char *message);

#define dc1394_log_error(message,user)            \
  do {                                            \
    if (system_errorlog_handler != NULL) {        \
      system_errorlog_handler(                    \
         DC1394_LOG_ERROR, message,               \
	 __FILE__, __LINE__, __FUNCTION__,user);  \
    }                                             \
  } while (0);
 
/**
 * dc1394_log_warning: logs a nonfatal error condition to the registered facility
 * This function shall be invoked if a nonfatal error condition is encountered.
 * The message passed as argument is delivered to the registered warning reporting
 * function registered before.
 * @param [in] message: error message to be logged
 */
//void dc1394_log_warning(const char *message);

#define dc1394_log_warning(message,user)          \
  do {                                            \
    if (system_warninglog_handler != NULL) {      \
      system_warninglog_handler(                  \
         DC1394_LOG_WARNING, message,             \
	 __FILE__, __LINE__, __FUNCTION__,user);  \
    }                                             \
  } while (0);


/**
 * dc1394_log_debug: logs a debug statement to the registered facility
 * This function shall be invoked if a debug statement is to be logged.
 * The message passed as argument is delivered to the registered debug reporting
 * function registered before.
 * @param [in] message: debug statement to be logged
 */
//void dc1394_log_debug(const char *message);

#define dc1394_log_debug(message,user)            \
  do {                                            \
    if (system_debuglog_handler != NULL) {        \
      system_debuglog_handler(                    \
         DC1394_LOG_DEBUG, message,               \
	 __FILE__, __LINE__, __FUNCTION__,user);  \
    }                                             \
  } while (0);

/* Some macros to log errors, etc... conditionally */
#define DC1394_WRN(err,message)                           \
  do {                                                    \
    if ((err>0)||(err<=-DC1394_ERROR_NUM))                \
      err=DC1394_INVALID_ERROR_CODE;                      \
                                                          \
    if (err!=DC1394_SUCCESS) {                            \
      const char *macro_error_string=dc1394_error_get_string(err);    \
      dc1394_log_warning(macro_error_string,NULL);        \
    }                                                     \
  } while (0);

#define DC1394_ERR(err,message)                           \
  do {                                                    \
    if ((err>0)||(err<=-DC1394_ERROR_NUM))                \
      err=DC1394_INVALID_ERROR_CODE;                      \
                                                          \
    if (err!=DC1394_SUCCESS) {                            \
      const char *macro_error_string=dc1394_error_get_string(err);    \
      dc1394_log_error(macro_error_string,NULL);          \
      return;                                             \
    }                                                     \
  } while (0);

#define DC1394_ERR_RTN(err,message)                       \
  do {                                                    \
    if ((err>0)||(err<=-DC1394_ERROR_NUM))                \
      err=DC1394_INVALID_ERROR_CODE;                      \
                                                          \
    if (err!=DC1394_SUCCESS) {                            \
      const char *macro_error_string=dc1394_error_get_string(err);    \
      dc1394_log_error(macro_error_string,NULL);          \
      return err;                                         \
    }                                                     \
  } while (0);

#define DC1394_ERR_CLN(err,cleanup,message)               \
  do {                                                    \
    if ((err>0)||(err<=-DC1394_ERROR_NUM))                \
      err=DC1394_INVALID_ERROR_CODE;                      \
                                                          \
    if (err!=DC1394_SUCCESS) {                            \
      const char *macro_error_string=dc1394_error_get_string(err);    \
      dc1394_log_error(macro_error_string,NULL);          \
      cleanup;                                            \
      return;                                             \
    }                                                     \
  } while (0);

#define DC1394_ERR_CLN_RTN(err,cleanup,message)           \
  do {                                                    \
    if ((err>0)||(err<=-DC1394_ERROR_NUM))                \
      err=DC1394_INVALID_ERROR_CODE;                      \
                                                          \
    if (err!=DC1394_SUCCESS) {                            \
      const char *macro_error_string=dc1394_error_get_string(err);    \
      dc1394_log_error(macro_error_string,NULL);          \
      cleanup;                                            \
      return err;                                         \
    }                                                     \
  } while (0);


#endif /* _MSC_VER */

/**
 * dc1394_log_register_handler: register log handler for reporting error, warning or debug statements
 * Passing NULL as argument turns off this log level.
 * @param [in] log_handler: pointer to a function which takes a character string as argument
 *             type: the type of log
 */
dc1394error_t dc1394_log_register_handler(dc1394log_t type, void(*log_handler)(dc1394log_t type,
		      const char *message, const char *file, int line, const char *function, void* user));

/**
 * dc1394_log_set_default_handler: set the log handler to the default handler
 * At boot time, debug logging is OFF (handler is NULL). Using this function for the debug statements
 * will start logging of debug statements usng the default handler.
 */
dc1394error_t dc1394_log_set_default_handler(dc1394log_t type);

#endif