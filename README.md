# zmq-hwm-test

## Dependencies

Docker is the only dependency of this repo. A working Docker installation looks
like this

```bash
$ docker version
Client:
 Version:      1.11.2
 API version:  1.23
 Go version:   go1.5.4
 Git commit:   b9f10c9
 Built:        Wed Jun  1 21:20:08 2016
 OS/Arch:      darwin/amd64

Server:
 Version:      1.11.2
 API version:  1.23
 Go version:   go1.5.4
 Git commit:   b9f10c9
 Built:        Wed Jun  1 21:20:08 2016
 OS/Arch:      linux/amd64
```

## Repro steps

Open two terminal windows (T1 and T2)

### In T1

Run `make run_container`

wait for the container to build and a prompt that looks like `root@be8621b91d28:/host/build#`

Run `./run.sh broker 1 1` which sets the HWM to 1 (first param), and per-socket
options instead of global (param 2)

You should see output like this

```
-- Configuring done
-- Generating done
-- Build files have been written to: /host/build
[100%] Built target hwm_test
Starting broker w/ HWM 1 and per-socket options
```

### In T2

`make share_container` to enter the running container started in T1

`./run.sh client 1 1`

The first message will be `Starting client w/ HWM 1 and per-socket options`

100000 messages are sent. When the last one is sent you can Ctrl+C to terminate
the process.

Now run `./run.sh server 1 1` to consume the messages buffered in the broker.

## Actual result

~1000 messages sent by the client (DEALER) were buffered in the broker even
though each of its socket's snd/rcv HWM was 1.

Once the client is shut down and the server starts, ~1000 messages are
received by the server (REP).

Values above 1000 (i.e., `./run.sh broker 2000 1`) appear to work as expected.

Setting global, instead of per-socket, HWMs appear to be ignored.
`./run.sh broker 2000 0` buffers ~1000 messages.

## Expected result

The HWM of each socket in this test was set to 1. According to the [docs](http://api.zeromq.org/4-0:zmq-socket)
99999 messages should have been dropped.

> When a ZMQ_ROUTER socket enters the mute state due to having reached the high
> water mark for all peers, then any messages sent to the socket shall be
> dropped until the mute state ends. Likewise, any messages routed to a peer for
> which the individual high water mark has been reached shall also be dropped.

Or at most, the number of messages that should be buffered in the broker are the
sum of its sockets' snd/rcv HWMs. For this test it was 4
(DEALER SND = 1, DEALER RCV = 1, ROUTER SND = 1, ROUTER RCV = 1)

I expected the DEALER side to block and the ROUTER side to drop messages.
