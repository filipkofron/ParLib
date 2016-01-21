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
  int rc = static_cast<int>(socket.Receive(reinterpret_cast<char*>(header), static_cast<int>(headerSize)));
  if (rc == headerSize)
  {
    Content* content = new Content(header->length);
    if (socket.Receive(reinterpret_cast<char*>(content->data), static_cast<int>(header->length)) == header->length)
    {
      return std::make_shared<Message>(header, content);
    }
    delete content;
  }
  if (DEBUGVerbose) Err << "Failure to receive a message rc: " << rc << std::endl;
  delete header;
  return nullptr;
}

bool Message::Send(TCPSocket& socket) const
{
  size_t headerSize = sizeof(*_header);
  int len = static_cast<int>(socket.Send(reinterpret_cast<const char*>(_header), static_cast<int>(headerSize)));
  if (len != headerSize)
  {
    return false;
  }
  len = static_cast<int>(socket.Send(reinterpret_cast<const char*>(_content->data), static_cast<int>(_header->length)));
  if (len == _header->length)
  {
    return true;
  }
  return false;
}

std::string Message::AsString(int from, int to)
{
  std::string str;

  if (to == -1)
  {
    to = static_cast<int>(_header->length);
  }

  for (int i = from; i < to; i++)
    str.push_back(static_cast<char>(_content->data[i]));

  return str;
}