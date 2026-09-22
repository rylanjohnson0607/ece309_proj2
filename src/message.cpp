#include "core/message.h"

Message::Message(Role role, std::string content) : role_(role), content_(content) {}

Message::Message() : role_(Role::System), content_("") {}

Role Message::role() const noexcept // Who sent this message.
{
	return role_;
}

const std::string& Message::content() const noexcept // The message text.
{
	return content_;
}