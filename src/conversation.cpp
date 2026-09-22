#include "core/conversation.h"
#include <stdexcept>

// Empty conversation: size() == 0, no allocation yet.
Conversation::Conversation() {}

// Releases all owned Message storage. No effect if already empty
// (e.g. moved-from).
Conversation::~Conversation() {
    delete[] data_;
}

// Deep copy: allocates its own buffer and copies every Message.
// this->begin() must differ from other.begin() afterward.
Conversation::Conversation(const Conversation& other) {
	if (other.capacity_ == 0)
	{
		data_ = nullptr;
		size_ = 0;
		capacity_ = 0;
		return;
	}

	Message* new_data_ = new Message[other.capacity_];
	try {
		for (std::size_t i = 0; i < other.size_; ++i) {
			new_data_[i] = other.data_[i];
		}
	}
	catch (...) {
		delete[] new_data_;
		throw;
	}

	data_ = new_data_;
	size_ = other.size_;
	capacity_ = other.capacity_;
}

// Deep copy: allocates its own buffer and copies every Message.
// this->begin() must differ from other.begin() afterward.
Conversation& Conversation::operator=(const Conversation& other) {
	if (this == &other)
	{
		return *this;
	}

	if (other.capacity_ == 0)
	{
		delete[] data_;
		data_ = nullptr;
		size_ = 0;
		capacity_ = 0;
		return *this;
	}

	Message* new_data_ = new Message[other.capacity_];
	try {
		for (std::size_t i = 0; i < other.size_; ++i) {
			new_data_[i] = other.data_[i];
		}
	}
	catch (...) {
		delete[] new_data_;
		throw;
	}

	
	delete[] data_;
	data_ = new_data_;
	size_ = other.size_;
	capacity_ = other.capacity_;

	return *this;
}

// Steals other's buffer — no per-element copying. Afterward, other
// must be left valid and empty (safe to destroy or reassign).
Conversation::Conversation(Conversation&& other) noexcept {
	data_ = other.data_;
	size_ = other.size_;
	capacity_ = other.capacity_;

	other.data_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;
}

// Steals other's buffer — no per-element copying. Afterward, other
// must be left valid and empty (safe to destroy or reassign).
Conversation& Conversation::operator=(Conversation&& other) noexcept {
	if (this == &other)
	{
		return *this;
	}

	delete[] data_;
	data_ = other.data_;
	size_ = other.size_;
	capacity_ = other.capacity_;

	other.data_ = nullptr;
	other.size_ = 0;
	other.capacity_ = 0;

	return *this;
}

// Appends m, growing the backing array if needed. Amortized O(1) —
// document and justify your growth strategy in the design log
// (see Appendix C if you want a refresher first).
void Conversation::append(Message m) {
	std::size_t newCap = 0;
	if (size_ == capacity_)
	{
		if (capacity_ == 0)
		{
			newCap = 1;
		}
		else
		{
			newCap = capacity_ * 2;
		}
		Message* new_data_ = new Message[newCap];
		try {
			for (std::size_t i = 0; i < size_; ++i) {
				new_data_[i] = data_[i];
			}
		}
		catch (...) {
			delete[] new_data_;
			throw;
		}
		
		delete[] data_;
		data_ = new_data_;
		capacity_ = newCap;
	}

	data_[size_] = m;
	++size_;
}

// Number of messages currently stored.
std::size_t Conversation::size() const noexcept {
	return size_;
}

// Bounds-checked access. Decide what happens on i >= size() (throw,
// assert, whatever you pick) and test that behavior explicitly.
const Message& Conversation::at(std::size_t i) const {
	if (i >= size_) {
		throw std::out_of_range("Index out of range");
	}
	return data_[i];
}

// Range-for iteration, oldest message first. begin() == end() when
// size() == 0.
const Message* Conversation::begin() const noexcept {
	return data_;
}

const Message* Conversation::end() const noexcept {
	return data_ + size_;
}