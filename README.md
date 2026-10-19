# Producer-Consumer with POSIX threads (C)

The bounded-buffer problem solved with two counting semaphores and a mutex:

- `empty_slots` counts free slots, so producers block when the buffer is full
- `full_slots` counts filled slots, so consumers block when it is empty
- `lock` protects the buffer indexes

Each line shows the buffer at that moment. Consumers stop cleanly using one
"poison pill" per consumer, and the program checks at the end that every item
was consumed exactly once.

## Build and run

```sh
make
./prodcons            # 2 producers, 2 consumers, 6 items each
./prodcons 3 2 10     # producers consumers items_per_producer
```

## Example run

```
$ ./prodcons
producer 2 put       200   buffer [ 200   .   .   .   . ]  1/5
consumer 1 took      200   buffer [   .   .   .   .   . ]  0/5
producer 1 put       100   buffer [   . 100   .   .   . ]  1/5
consumer 2 took      100   buffer [   .   .   .   .   . ]  0/5
producer 2 put       201   buffer [   .   . 201   .   . ]  1/5
producer 1 put       101   buffer [   .   . 201 101   . ]  2/5
producer 2 put       202   buffer [   .   . 201 101 202 ]  3/5
consumer 1 took      201   buffer [   .   .   . 101 202 ]  2/5
producer 2 put       203   buffer [ 203   .   . 101 202 ]  3/5
producer 1 put       102   buffer [ 203 102   . 101 202 ]  4/5
producer 1 put       103   buffer [ 203 102 103 101 202 ]  5/5
consumer 2 took      101   buffer [ 203 102 103   . 202 ]  4/5
producer 2 put       204   buffer [ 203 102 103 204 202 ]  5/5
consumer 1 took      202   buffer [ 203 102 103 204   . ]  4/5
producer 1 put       104   buffer [ 203 102 103 204 104 ]  5/5
consumer 1 took      203   buffer [   . 102 103 204 104 ]  4/5
producer 2 put       205   buffer [ 205 102 103 204 104 ]  5/5
consumer 1 took      102   buffer [ 205   . 103 204 104 ]  4/5
producer 1 put       105   buffer [ 205 105 103 204 104 ]  5/5
consumer 2 took      103   buffer [ 205 105   . 204 104 ]  4/5
consumer 2 took      204   buffer [ 205 105   .   . 104 ]  3/5
consumer 1 took      104   buffer [ 205 105   .   .   . ]  2/5
consumer 2 took      205   buffer [   . 105   .   .   . ]  1/5
consumer 2 took      105   buffer [   .   .   .   .   . ]  0/5

consumer 1 handled 6 items
consumer 2 handled 6 items
produced sum = 1830, consumed sum = 1830 -> OK, nothing lost or duplicated
```

Thread scheduling is non-deterministic, so the order changes on every run. The final check never does.

## About

Written by Ahsan (Code Projects on YouTube).
I build and debug operating-systems, graphics and desktop projects. Issues and pull requests are welcome.

Licensed under the MIT License.
