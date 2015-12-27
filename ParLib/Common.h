#pragma once

#include <cstdio>
#include <cstdlib>

/**
* Just before the start. Print some info.
*/
void OnStart();

/**
 * Just before the exit cleanup sockets. Waiting for user input could also be useful.
 */
void OnEnd();

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

