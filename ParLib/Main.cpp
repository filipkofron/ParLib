
#include <iostream>
#include <fstream>
#include <cstring>
#include "TCPSocket.h"
#include "Common.h"
#include "NetworkManager.h"
#include "AddrIterator.h"
#include "Matrix.h"
#include "MaximumCut.h"
#include "Computation.h"

#ifndef _WIN32
#include <signal.h>
#endif // _WIN32

std::shared_ptr<NetworkManager> GNetworkManager;
std::shared_ptr<Computation> GComputation;
std::vector<std::shared_ptr<std::thread> > GTerminatingThreads;

#define JOB_PATH_LINUX "/mnt/jobs/test.txt"
#define JOB_PATH_WINDOWS "\\\\192.168.56.101\\jobs\\test.txt"

void ReadJob()
{
  const char* path;
#ifdef _WIN32
  path = JOB_PATH_WINDOWS;
#else
  path = JOB_PATH_LINUX;
#endif
  std::ifstream ifs;
  ifs.open(path, std::ios::binary);
  if (ifs.fail())
  {
    FatalError("Cannot load the job file: '%s'!!!", path);
  }
  std::cout << "Reading job from '" << path << "'." << std::endl;
  std::shared_ptr<Matrix> matrix = std::make_shared<Matrix>();
  matrix->fromStream(ifs);
  std::cout << "Loaded matrix size: '" << matrix->getSize() << "'." << std::endl;
  GComputation->SetMaximumCut(std::make_shared<MaximumCut>(matrix));
}

int main(int argc, const char* args[])
{
#ifndef _WIN32
  signal(SIGPIPE, SIG_IGN);
#endif // _WIN32
  srand(time(nullptr));
  OnStart();
  GComputation = std::make_shared<Computation>();
  ReadJob();
  //std::string ip = "147.32.76.241";
  std::string ip = "192.168.1.1";
  GNetworkManager = std::make_shared<NetworkManager>(ip, 8);
  sleepMs(500);
  GNetworkManager->DiscoverAll();
  sleepMs(1000);
  //DEBUGVerbose = true;
  while (GNetworkManager->GetClientCount() < 1)
  {
    std::cout << "Waiting for at least 1 other clients, currently: " << GNetworkManager->GetClientCount() << std::endl;
    sleepMs(500);
  }
  GComputation->StartComputation();

  while (!GComputation->HasFinished())
  {
    sleepMs(100);
  }
  GComputation->Terminate();

  GNetworkManager->Terminate();
  std::cout << "Waiting for all threads to stop." << std::endl;
  sleepMs(300);
  GNetworkManager = nullptr;

  for (auto& t : GTerminatingThreads)
  {
    if (t)
      t->join();
  }
  GTerminatingThreads.clear();

  OnEnd(false);
  return 0;
}
