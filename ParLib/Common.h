#pragma once

#include <cstdio>
#include <cstdlib>

#define DEFAULT_PORT 1337

extern bool TerminationInProgress;

/**
* Just before the start. Print some info.
*/
void OnStart();

/**
 * Just before the exit cleanup sockets. Waiting for user input could also be useful.
 */
void OnEnd();

/**
* Upon an info shows given formatted message.
* Note: expanded (formatted) message can only grow by 1024 characters in length.
*/
void Info(const char* format, ...);

/**
 * Upon an error shows given formatted message.
 * Note: expanded (formatted) message can only grow by 1024 characters in length.
 */
void Error(const char* format, ...);

/**
 * Upon an error shows given formatted message and exits the program with error code 1.
 * Note: expanded (formatted) message can only grow by 1024 characters in length.
 */
void FatalError(const char* format, ...);

