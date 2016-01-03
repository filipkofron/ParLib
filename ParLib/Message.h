#pragma once
#include <cinttypes>
#include <cstring>
#define MSG_DEST_LEN 32
#include <string>
#include "Common.h"

class TCPSocket;

#define MESSAGE_TYPE_INITIAL 0
#define MESSAGE_TYPE_ELECTION 1
#define MESSAGE_TYPE_ELECTED 2
#define MESSAGE_TYPE_KEEP_ALIVE 3
#define MESSAGE_TYPE_KEEP_DATA 4

class Message
{
private:
#pragma pack(push,1)
  struct Header
  {
    int32_t type;
    uint64_t length;
    Header()
      : type(-1), length(0)
    {
    }
    Header(int type, uint64_t length)
      : type(type), length(length)
    {
    }
  };
  struct Content
  {
    uint8_t *data;
    Content(const uint8_t* inData, uint64_t length)
    {
      data = new uint8_t[length];
      memcpy(data, inData, length);
    }
    Content(uint64_t length)
    {
      data = new uint8_t[length];
      memset(data, 0, length);
    }
    ~Content()
    {
      delete[] data;
    }
    uint8_t* GetData() const
    {
      return data;
    }
  };
#pragma pack(pop)
  Header *_header;
  Content *_content;

public:
  static std::shared_ptr<Message> Receive(TCPSocket& socket);
  bool Send(TCPSocket& socket);
};
