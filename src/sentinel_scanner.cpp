#include "core/sentinel_scanner.h"
#include <sstream>

SentinelScanner::SentinelScanner(std::string sentinel) : sentinel_(sentinel) {}

// Feed the next chunk. Returns text guaranteed NOT to be part of
// the sentinel (safe to print immediately) and whether the
// sentinel has now been fully seen.
SentinelScanner::Out SentinelScanner::feed(std::string_view chunk) {
	std::string safe_text = "";
	std::string combined = pending_ + std::string(chunk);

	std::size_t position = combined.find(sentinel_);
	if (position != std::string::npos)
	{
		// Sentinel found
		safe_text = combined.substr(0, position);
		pending_.clear();
		return { safe_text, true };
	}

	int highestFind = -1;
	if (combined.size() >= sentinel_.size()) {
		for (size_t i = 1; i <= sentinel_.size() - 1; ++i) {
			std::string endChunk = combined.substr(combined.size() - i);
			if (sentinel_.substr(0, i) == endChunk) {
				highestFind = i;
				safe_text = combined.substr(0, combined.size() - i);
			}
		}
	}
	else{
		for (size_t i = 1; i <= combined.size(); ++i) {
			std::string endChunk = combined.substr(combined.size() - i);
			if (sentinel_.substr(0, i) == endChunk) {
				highestFind = i;
				safe_text = combined.substr(0, combined.size() - i);
			}
		}
	}

	if (highestFind == -1){
		pending_.clear();
		safe_text = combined;
	}
	else{
		pending_ = sentinel_.substr(0, highestFind);
	}

	return { safe_text, false };
}


SentinelScanner::Out SentinelScanner::flush() {
	Out result;
	result.safe_text = pending_;
	result.sentinel_found = false;
	pending_.clear();
	return result;
}

std::size_t SentinelScanner::pending_size() const noexcept 
{ 
	return pending_.size(); 
}