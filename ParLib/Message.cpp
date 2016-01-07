#include "TCPSocket.h"
#include "Message.h"
#include <iostream>

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
  if (rc == headerSize)
  {
    Content* content = new Content(header->length);
    if (socket.Receive(reinterpret_cast<char*>(content->data), header->length) == header->length)
    {
      return std::make_shared<Message>(header, content);
    }
    delete content;
  }
  std::cout << "Failure to receive a message rc: " << rc << std::endl;
  delete header;
  return nullptr;
}

bool Message::Send(TCPSocket& socket)
{
  size_t headerSize = sizeof(*_header);
  int len = socket.Send(reinterpret_cast<const char*>(_header), headerSize);
  if (len != headerSize)
  {
    return false;
  }
  len = socket.Send(reinterpret_cast<const char*>(_content->data), _header->length);
  if (len == _header->length)
  {
    return true;
  }
  return false;
}