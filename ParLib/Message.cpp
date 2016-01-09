#include "TCPSocket.h"
#include "Message.h"
#include <iostream>
#include <sstream>

Message::Message(Header* header, Content* content)
  : _header(header), _content(content)
{
}

Message::Message(int32_t type, const char* data, uint64_t length)
{
  _header = new Header(type, length);
  _content = new Content(data, length);
}

Message::~Message()
{
  delete _header;
  delete _content;
}

std::shared_ptr<Message> Message::Receive(TCPSocket& socket)
{
  Header* header = new Header;
  size_t headerSize = sizeof(*header);
  int rc = socket.Receive(reinterpret_cast<char*>(header), headerSize);
  if (!socket.IsOk()) FatalError("RECEIVE HEADER ERROR!");
  if (rc == headerSize)
  {
    Content* content = new Content(header->length);
    if (socket.Receive(reinterpret_cast<char*>(content->data), header->length) == header->length)
    {
      FatalError("RECEIVE CONTENT ERROR!");
      return std::make_shared<Message>(header, content);
    }
    FatalError("RECEIVE CONTENT ERROR!");
    delete content;
  }
  if (!socket.IsOk()) FatalError("RECEIVE HEADER ERROR!");
  std::cout << "Failure to receive a message rc: " << rc << std::endl;
  delete header;
  return nullptr;
}

bool Message::Send(TCPSocket& socket) const
{
  size_t headerSize = sizeof(*_header);
  int len = socket.Send(reinterpret_cast<const char*>(_header), headerSize);
  if (!socket.IsOk()) FatalError("SEND HEADER ERROR!");
  if (len != headerSize)
  {
    FatalError("SEND HEADER ERROR!");
    return false;
  }
  len = socket.Send(reinterpret_cast<const char*>(_content->data), _header->length);
  if (!socket.IsOk()) FatalError("SEND CONTENT ERROR!");
  if (len == _header->length)
  {
    return true;
  }
  FatalError("SEND CONTENT ERROR!");
  return false;
}

std::string Message::AsString(int from, int to)
{
  std::string str;

  if (to == -1)
  {
    to = _header->length;
  }

  for (int i = from; i < to; i++)
    str.push_back(static_cast<char>(_content->data[i]));

  return str;
}