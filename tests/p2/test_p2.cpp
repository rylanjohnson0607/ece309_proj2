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
#include <fstream>
#include <cstdio>

int main()
{
    // Empty Conversation Bounds: Handle Empty Conversations Without Out-Of-Bounds Access.
    //-----------------------------------------------------------------------------------------------
    // 1. Test That Calling at() On An Empty Conversation Throws std::out_of_range.

    Conversation conv1;
    bool threw = false;

    try
    {
        conv1.at(0);
    }
    catch (const std::out_of_range &)
    {
        threw = true;
    }

    assert(threw);
    std::cout << "Test 1 passed: at() on empty Conversation throws std::out_of_range." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // System Message Ordering: Ensure System Messages Remain Pinned At The Front.
    //-----------------------------------------------------------------------------------------------
    // 2. Test that Resizing the Array Never Unpins System Messages From The Front

    Conversation conv2;

    conv2.append(Message(Role::System, "System message")); // System Message Only Size = 1

    assert((conv2.at(0).role() == Role::System));      // System Message Role Check
    assert(conv2.at(0).content() == "System message"); // System Message Content Check

    conv2.append(Message(Role::User, "User message")); // Triggers Resize Capacity from 1->2

    assert((conv2.at(0).role() == Role::System));      // System Message Check
    assert(conv2.at(0).content() == "System message"); // System Message Content Check

    conv2.append(Message(Role::Assistant, "Assistant message")); // Triggers Resize Capacity from 2->4

    assert((conv2.at(0).role() == Role::System));      // System Message Check
    assert(conv2.at(0).content() == "System message"); // System Message Content Check

    std::cout << "Test 2 passed: System Messages Stay Pinned To Front." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // Rule of Five (Copy): Assert copy constructors allocate entirely different pointer addresses.
    //-----------------------------------------------------------------------------------------------
    // 3. Check That Copy Constructor And Assignment Create Deep Copies Of Conversations.

    // Using Non-Empty Conversation From Test 2
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

    // Rule of Five (Move): Assert move constructors steal the data pointer and zero the source.
    //-----------------------------------------------------------------------------------------------
    // 4. Verify Move Constructor And Move Assignment Steal The Original Buffer And Leave The Source Empty.

    Conversation source;
    source.append(Message(Role::User, "Hello"));
    const Message *original = source.begin();

    // Move constructor
    Conversation moved(std::move(source));
    assert(moved.begin() == original);
    assert(moved.size() == 1);
    assert(source.begin() == nullptr && source.size() == 0);

    // Move assignment
    Conversation destination;
    destination.append(Message(Role::User, "Old"));
    destination = std::move(moved);
    assert(destination.begin() == original);
    assert(destination.size() == 1);
    assert(destination.at(0).content() == "Hello");
    assert(moved.begin() == nullptr && moved.size() == 0);

    std::cout << "Test 4 passed: Move operations transfer ownership and leave sources empty and reusable." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // Growth behavior: Assert capacity grows per your documented growth factor and size()/at() stay correct across reallocation.
    //-----------------------------------------------------------------------------------------------
    // 5. Verify Appending Messages Preserves Their Order And Contents As The Array Grows.

    Conversation conv5;

    for (std::size_t i = 0; i < 100; ++i)
    {
        conv5.append(Message(Role::User, std::to_string(i)));
        assert(conv5.size() == i + 1);

        for (std::size_t j = 0; j <= i; ++j)
        {
            assert(conv5.at(j).role() == Role::User);
            assert(conv5.at(j).content() == std::to_string(j));
        }
    }

    std::cout << "Test 5 passed: Growth preserves size, message order, and contents." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // Scanner (Clean Text): Verify the scanner processes strings with no sentinel correctly.
    //-----------------------------------------------------------------------------------------------
    // 6. Verify Clean Chunks Return Their Text Without Detecting A Sentinel.

    SentinelScanner scanner6("<|end_conversation|>");

    auto result6 = scanner6.feed("Hello ");
    assert(result6.safe_text == "Hello ");
    assert(!result6.sentinel_found);

    result6 = scanner6.feed("world!");
    assert(result6.safe_text == "world!");
    assert(!result6.sentinel_found);

    result6 = scanner6.flush();
    assert(result6.safe_text.empty());
    assert(!result6.sentinel_found);

    std::cout << "Test 6 passed: Clean text passes through without detecting a sentinel." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // 7. Scanner (Split Sentinel): Prove the scanner catches the sentinel split across every possible boundary
    //(loop over all split points programmatically).
    //-----------------------------------------------------------------------------------------------
    // 7. Verify Sentinel Detection At Every Split Point Between Two Chunks.

    const std::string sentinel7 = "<|end_conversation|>";

    for (std::size_t split = 1; split < sentinel7.size(); ++split)
    {
        SentinelScanner scanner7(sentinel7);

        auto result7 = scanner7.feed("Hello " + sentinel7.substr(0, split));
        assert(result7.safe_text == "Hello ");
        assert(!result7.sentinel_found);

        result7 = scanner7.feed(sentinel7.substr(split) + "Ignored text");
        assert(result7.safe_text.empty());
        assert(result7.sentinel_found);
    }

    std::cout << "Test 7 passed: Sentinel detected across every split boundary." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // 8. Scanner (False Alarms): Ensure the scanner doesn't trigger on partial matches (e.g., <|end_world|>).
    //-----------------------------------------------------------------------------------------------
    // 8. Verify Partial Matches Are Returned As Text Without Triggering Sentinel Detection.

    SentinelScanner scanner8("<|end_conversation|>");

    auto result8 = scanner8.feed("<|end_");
    assert(result8.safe_text.empty());
    assert(!result8.sentinel_found);

    result8 = scanner8.feed("world|>");
    assert(result8.safe_text == "<|end_world|>");
    assert(!result8.sentinel_found);

    result8 = scanner8.feed("<|end_con");
    assert(result8.safe_text.empty());
    assert(!result8.sentinel_found);

    result8 = scanner8.flush();
    assert(result8.safe_text == "<|end_con");
    assert(!result8.sentinel_found);

    std::cout << "Test 8 passed: Partial matches do not trigger sentinel detection." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // 9. Scanner (Bounded Memory): Assert pending_ never exceeds sentinel.size() - 1 while feeding a large adversarial stream.
    //-----------------------------------------------------------------------------------------------
    // 9. Verify Pending Storage Stays Within The Limit During Repeated Incomplete Sentinel Matches.

    const std::string sentinel9 = "<|end_conversation|>";
    SentinelScanner scanner9(sentinel9);
    const std::string partial9 = sentinel9.substr(0, sentinel9.size() - 1);

    for (std::size_t i = 0; i < 10000; ++i)
    {
        auto result9 = scanner9.feed(partial9);
        assert(!result9.sentinel_found);
        assert(scanner9.pending_size() <= sentinel9.size() - 1);
    }

    scanner9.flush();
    assert(scanner9.pending_size() == 0);

    std::cout << "Test 9 passed: Pending storage remains bounded during a large adversarial stream." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // 10. Harness (Turn Limit): Confirm the provided loop stops with TurnLimit when your Conversation is used underneath it.
    //-----------------------------------------------------------------------------------------------
    // 10. Verify The Harness Stops After Two Turns Using The Provided ScriptedModelClient.

    struct Input10 : InputSource // Override InputSource To Reply "Hello"
    {
        int reads = 0;
        std::string read_line() override
        {
            ++reads;
            return "Hello";
        }
        bool is_eof() const override { return reads > 2; } // Just Safety Incase Turn Limit Doesn't Stop The Conv
    } input10;

    struct Output10 : OutputSink // Override Output Source And Don't Print To Avoid Cluttering Output Window
    {
        void write(std::string_view) override {}
    } output10;

    HarnessConfig config10;
    config10.max_turns = 2; // Set Max Turns = 2

    Harness harness10(
        std::make_unique<ScriptedModelClient>("scripts/greeting.script"),
        config10);

    auto reason10 = harness10.run(input10, output10);
    assert(reason10.kind == StopReason::Kind::TurnLimit); // Check Stop Reason Caused By Max Turns
    assert(harness10.conversation().size() == 4);         // 2 Model Replies, 2 Scripted User Inputs

    std::cout << "Test 10 passed: Harness stops at the turn limit." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // 11. Harness (Sentinel Halt): Confirm the provided loop halts exactly when your SentinelScanner reports the sentinel found.
    //-----------------------------------------------------------------------------------------------
    // 11. Verify The Third Scripted Reply Stops The Harness Before The Turn Limit.

    struct Input11 : InputSource
    {
        int reads = 0;
        std::string read_line() override // Override InputSource To Reply "Hello"
        {
            ++reads;
            return "Hello";
        }
        bool is_eof() const override { return reads > 3; } // Just Safety Incase Sentinel Doesn't Stop The Conv
    } input11;

    struct Output11 : OutputSink
    {
        std::string text;
        void write(std::string_view chunk) override { text += chunk; }
    } output11;

    HarnessConfig config11;
    config11.max_turns = 10;

    Harness harness11(
        std::make_unique<ScriptedModelClient>("scripts/greeting.script"),
        config11);

    auto reason11 = harness11.run(input11, output11);

    assert(reason11.kind == StopReason::Kind::Sentinel);
    assert(input11.reads == 3);
    assert(harness11.conversation().size() == 6);
    assert(harness11.conversation().at(5).content() ==
           "Goodbye!<|end_conversation|>");
    assert(output11.text.find("Goodbye!") != std::string::npos);
    assert(output11.text.find("<|end_conversation|>") == std::string::npos);

    std::cout << "Test 11 passed: Harness stops at the sentinel without printing it." << std::endl;

    //-----------------------------------------------------------------------------------------------

    // 12. Transcript Round-Trip: Save a mock conversation, load it via the provided ReplayModelClient, assert identical playback.
    //-----------------------------------------------------------------------------------------------
    // 12. Verify Saved Assistant Replies Replay With Identical Roles, Text, And Order.

    Conversation conv12;
    conv12.append(Message(Role::System, "Be concise."));
    conv12.append(Message(Role::User, "Hello"));
    conv12.append(Message(Role::Assistant, "Hi!\nHow are you?"));
    conv12.append(Message(Role::User, "Bye"));
    conv12.append(Message(Role::Assistant, "Goodbye!<|end_conversation|>"));

    const char *path12 = "test12_transcript.tmp";
    std::ofstream file12(path12);
    assert(file12.is_open());

    for (std::size_t i = 0; i < conv12.size(); ++i)
    {
        const Message &message = conv12.at(i);
        const char *role = message.role() == Role::System ? "system" : message.role() == Role::User ? "user": "assistant";

        if (i != 0)
        {
            file12 << "---\n";
        }
        file12 << "role: " << role << "\n"  << message.content() << "\n";
    }

    file12.close();
    assert(!file12.fail());

    ReplayModelClient replay12(path12);
    assert(replay12.system_message() == conv12.at(0).content());

    for (std::size_t i = 2; i < conv12.size(); i += 2)
    {
        Message reply = replay12.generate(conv12);
        assert(reply.role() == conv12.at(i).role());
        assert(reply.content() == conv12.at(i).content());
    }

    std::remove(path12);

    std::cout << "Test 12 passed: Saved transcript replays identical assistant replies." << std::endl;

    //-----------------------------------------------------------------------------------------------
}
