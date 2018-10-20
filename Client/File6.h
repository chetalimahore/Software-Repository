///////////////////////////////////////////////////////////////////////////
// File6.h.1 - defines message structure used in communication channel //
// ver 1.0                                                               //
// Jim Fawcett, CSE687-OnLine Object Oriented Design, Fall 2017          //
///////////////////////////////////////////////////////////////////////////

#include <iostream>

using namespace MsgPassingCommunication;
using SUtils = Utilities::StringHelper;

//----< default constructor results in Message with no attributes >----

Message::Message() {}

//----< constructor accepting dst and src addresses >------------------

Message::Message(EndPoint to, EndPoint from)
{
  attributes_["to"] = to.toString();
  attributes_["from"] = from.toString();
}
//----< returns reference to Message attributes >----------------------

Message::Attributes& Message::attributes()
{
  return attributes_;
}
//----< adds or modifies an existing attribute >-----------------------

void Message::attribute(const Key& key, const Value& value)
{
  attributes_[key] = value;
}
//----< clears all attributes >----------------------------------------

void Message::clear()
{
  attributes_.clear();
}
//----< returns vector of attribute keys >-----------------------------

Message::Keys Message::keys()
{
  Keys keys;
  keys.reserve(attributes_.size());
  for (auto kv : attributes_)
  {
    keys.push_back(kv.first);
  }
  return keys;
}
//---< does this message have key? >-----------------------------------

bool Message::containsKey(const Key& key)
{
  if (attributes_.find(key) != attributes_.end())
    return true;
  return false;
}
//----< get to attribute >---------------------------------------------

EndPoint Message::to()
{
  if (containsKey("to"))
  {
    return EndPoint::fromString(attributes_["to"]);
  }
  return EndPoint();
}
//----< set to attribute >---------------------------------------------

void Message::to(EndPoint ep)
{
  attributes_["to"] = ep.toString();
}
//----< get from attribute >-------------------------------------------

EndPoint Message::from()
{
  if (containsKey("from"))
  {
    return EndPoint::fromString(attributes_["from"]);
  }
  return EndPoint();
}
//----< set from attribute >-------------------------------------------

void Message::from(EndPoint ep)
{
  attributes_["from"] = ep.toString();
}
//----< get name attribute >-------------------------------------------

std::string Message::name()
{
  if (containsKey("name"))
  {
    return attributes_["name"];
  }
  return "";
}
//----< set name attribute >-------------------------------------------

void Message::name(const std::string& nm)
{
  attributes_["name"] = nm;
}
//----< get command attribute >----------------------------------------

std::string Message::command()
{
  if (containsKey("command"))
  {
    return attributes_["command"];
  }
  return "";
}
//----< set command attribute >----------------------------------------

void Message::command(const std::string& cmd)
{
  attributes_["command"] = cmd;
}
