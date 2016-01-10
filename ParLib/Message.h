#pragma once
#include <cinttypes>
#include <cstring>
#define MSG_DEST_LEN 32
#include <string>

class TCPSocket;
class Message;

#include "Common.h"

#define MESSAGE_TYPE_INITIAL 0
#define MESSAGE_TYPE_ELECTION 1
#define MESSAGE_TYPE_ELECTED 2
#define MESSAGE_TYPE_KNOWN_LEADER 3
#define MESSAGE_TYPE_KEEP_ALIVE 4
#define MESSAGE_TYPE_KEEP_ALIVE_RESP 5
#define MESSAGE_TYPE_STACK_ASSIGNMENT 6
#define MESSAGE_TYPE_ASSIGNMENT_FINISHED 7

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
    char *data;
    Content(const char* inData, uint64_t length)
    {
      data = new char[length];
      memcpy(data, inData, length);
    }
    Content(uint64_t length)
    {
      data = new char[length];
      memset(data, 0, length);
    }
    ~Content()
    {
      delete[] data;
    }
    char* GetData() const
    {
      return data;
    }
  };
#pragma pack(pop)
  Header* _header;
  Content* _content;

public:
  Message(Header* header, Content* content);
  Message(int32_t type, const char* data, uint64_t length);
  virtual ~Message();
  static std::shared_ptr<Message> Receive(TCPSocket& socket);
  bool Send(TCPSocket& socket) const;
  std::string AsString(int from = 0, int to = -1);
  int32_t GetType() const { return _header ? _header->type : -1; }
  char* GetData() const { return _content ? _content->GetData() : NULL; }
  uint64_t GetLength() const { return _header ? _header->length : 0; }
};
