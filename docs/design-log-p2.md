# Project 2 Design Log

## Growth Factor Decision and Conversation Class Design

When designing my `Conversation` class, I thought the best way to handle the growing conversation would be a dynamically allocated array. It is easy to index and lets me allocate and delete storage as needed. I use the provided `size_` and `capacity_` variables to track the number of messages and the total number of slots. When size equals capacity, I know it is time to resize. I allocate a new array with double the capacity, copy the data over, and delete the old, smaller array after all the data is copied. If capacity is zero, I start with one slot.

## Append Function Time Complexity

For normal appending operations where size does not equal capacity, the operation is just `data_[size_] = m` and `++size_`. Whether the conversation has 100 messages or 100,000 messages, it still accesses one slot and increments the size. This is constant work relative to the number of messages, although copying the string also depends on its length.

On the other hand, when appending requires resizing, that individual operation is O(n), since it copies every existing message into the new array. Although this append is O(n), the amortized runtime is still O(1).

Across nine appends, resizing copies 1 + 2 + 4 + 8 = 15 existing messages, which is less than 2(9). This can be proved for any number of appends. Let k be the number of messages copied during the most recent resize:

```text
S  = 1 + 2 + 4 + ... + k
2S = 2 + 4 + 8 + ... + 2k
```

Subtracting the first equation from the second cancels the middle terms:

```text
S = 2k - 1
```

That resize copied k messages to make room for message k + 1. After n appends, k <= n - 1, so:

```text
S = 2k - 1 <= 2n - 3 < 2n
```

Then, accounting for inserting each new message:

```text
Total copies and insertions = S + n < 2n + n = 3n
```

If no messages have been copied yet, the copy count is zero. Array initialization and destruction also add O(n) total work because the capacities grow geometrically. Dividing the total O(n) work by n gives O(1) amortized work per append, although the worst case for one append is O(n).

## Rule of Five

My code implements the Rule of Five by copying messages into separate arrays, deleting old arrays, and safely moving ownership from one object to another.

My default constructor does nothing because the header already initializes the pointer to null and the counters to zero. There is no reason to allocate storage yet. My destructor simply deletes the current array by calling `delete[] data_`.

My copy constructor allocates a new array using the other object's capacity and copies its messages over. If the other object's capacity is zero, it leaves the new object empty.

My copy assignment does mostly the same thing, but it also checks for self-assignment and deletes its old array after the new copy succeeds. In both copy operations, if copying throws, the catch block deletes the temporary array and rethrows the exception. This prevents leaking that allocation.

My move constructor sets `data_`, `size_`, and `capacity_` to the other object's corresponding values. It then sets the other object's pointer to null and its counters to zero. My move assignment does the same thing, except it checks for self-assignment and deletes its old array before taking ownership.

## Scanner Memory Bound

Looking at my `feed()` function, `pending_` only keeps a partial match when the full sentinel is not found in the combined text. It keeps the longest ending substring that matches the beginning of the sentinel.

Let L be the nonempty sentinel's length. If the combined text has at least L characters, my loop only checks lengths up to L - 1. Otherwise, it checks up to the combined text's length, which is already less than L. Therefore, the substring assigned to `pending_` can never exceed L - 1 characters. If no partial match exists, it is cleared. Finding the sentinel or calling `flush()` also clears it. Temporary combined text still depends on the chunk size.

## Hindsight and Reflection

Looking back, I like how I designed my classes. Outside this project's restrictions, I would probably use `std::vector<Message>` instead of managing a raw array. It would require less effort to handle memory and copying safely. It would reduce the risk of leaks, although bounds checking would still require care. Implementing the array myself helped me understand what `std::vector` normally does for me.
