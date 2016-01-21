#pragma once

#define DEFAULT_PORT 1337
#include <vector>
#include <memory>

#include <cstdio>
#include <cstdlib>

#define Log WritePreLog(std::cout)
#define Err WritePreLog(std::cerr)

std::ostream& WritePreLog(std::ostream& os);

#include "NetworkManager.h"
#include "Computation.h"
#include "MaximumCut.h"

#define KILL_TO_DEBUG *((int*) nullptr) = 0

extern bool TerminationInProgress;

class Computation;
extern std::shared_ptr<NetworkManager> GNetworkManager;
extern std::shared_ptr<Computation> GComputation;

extern int DEBUGVerbose;

extern std::vector<std::shared_ptr<std::thread> > GTerminatingThreads;

/**
* Just before the start. Print some info.
*/
void OnStart();

/**
 * Just before the exit cleanup sockets. Waiting for user input could also be useful.
 */
void OnEnd(bool waitForUserInput);

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

void DebugFile(const char* format, ...);

std::vector<std::string> SplitIPV4Addr(const std::string& addrStr);
uint32_t ParseIPV4Addr(const std::string& addrStr);

void sleepMs(unsigned long milis);

int64_t millis();

