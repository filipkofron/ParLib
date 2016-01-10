
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

std::shared_ptr<NetworkManager> GNetworkManager;
std::shared_ptr<Computation> GComputation;

#define JOB_PATH_LINUX "/mnt/job/test.txt"
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
  //while (GNetworkManager->GetClientCount() < 1)
  {
    std::cout << "Waiting for at least one client " << GNetworkManager->GetClientCount() << std::endl;
  }
  GComputation->StartComputation();

  while (!GComputation->HasFinished())
  {
    sleepMs(100);
  }
  GComputation->Terminate();

  GNetworkManager->Terminate();
  GNetworkManager = nullptr;

  OnEnd();
  return 0;
}
