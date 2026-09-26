// tests/p2/test_p2.cpp
//
// YOUR test suite goes here. At least 12 assert-based test cases — see
// spec §5 for the required categories and the sample test for the
// expected level of rigor.
//
// This file is a stub so the project builds out of the box; replace the
// body of main() with your own tests.

#include "core/conversation.h"
#include "core/message.h"
#include "core/sentinel_scanner.h"
#include "harness/harness.h"
#include "model/replay_client.h"
#include "model/scripted_client.h"

#include <iostream>
#include <stdexcept>
#include <cassert>

int main() {

    //Empty Conversation Bounds: Handle Empty Conversations Without Out-Of-Bounds Access.
//-----------------------------------------------------------------------------------------------
    // 1. Test That Calling at() On An Empty Conversation Throws std::out_of_range.

    Conversation conv1;
    bool threw = false;

    try {
        conv1.at(0);
    }
    catch (const std::out_of_range&) {
        threw = true;
    }

    assert(threw);
    std::cout << "Test 1 passed: at() on empty Conversation throws std::out_of_range." << std::endl;

//-----------------------------------------------------------------------------------------------


    //System Message Ordering: Ensure System Messages Remain Pinned At The Front.
//-----------------------------------------------------------------------------------------------
    // 2. Test that Resizing the Array Never Unpins System Messages From The Front

    Conversation conv2;
    
    conv2.append(Message(Role::System, "System message")); // System Message Only Size = 1

    assert((conv2.at(0).role() == Role::System)); //System Message Role Check
    assert(conv2.at(0).content() == "System message"); //System Message Content Check

    conv2.append(Message(Role::User, "User message")); // Triggers Resize Capacity from 1->2

    assert((conv2.at(0).role() == Role::System)); //System Message Check
    assert(conv2.at(0).content() == "System message"); //System Message Content Check

    conv2.append(Message(Role::Assistant, "Assistant message")); // Triggers Resize Capacity from 2->4

    assert((conv2.at(0).role() == Role::System)); //System Message Check
    assert(conv2.at(0).content() == "System message"); //System Message Content Check

    std::cout << "Test 2 passed: System Messages Stay Pinned To Front." << std::endl;

//-----------------------------------------------------------------------------------------------


    //Rule of Five (Copy): Assert copy constructors allocate entirely different pointer addresses.
//-----------------------------------------------------------------------------------------------
    // 3. Check That Copy Constructor And Assignment Create Deep Copies Of Conversations.

    //Using Non-Empty Conversation From Test 2
    Conversation conv3(conv2);

    assert(conv3.size() == conv2.size());
    assert(conv3.begin() != conv2.begin());
    for (std::size_t i = 0; i < conv2.size(); ++i) 
    {
        assert(conv3.at(i).role() == conv2.at(i).role());
        assert(conv3.at(i).content() == conv2.at(i).content());
    }

    conv3.append(Message(Role::User, "Extra message"));
    conv3 = conv2;

    assert(conv3.size() == conv2.size());
    assert(conv3.begin() != conv2.begin());
    for (std::size_t i = 0; i < conv2.size(); ++i) 
    {
        assert(conv3.at(i).role() == conv2.at(i).role());
        assert(conv3.at(i).content() == conv2.at(i).content());
    }


    std::cout << "Test 3 passed: Both Copy Constructor And Assignment Create Deep Copies." << std::endl;

//-----------------------------------------------------------------------------------------------




}
