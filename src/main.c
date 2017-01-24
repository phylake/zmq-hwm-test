#include <czmq.h>

int
client (int hwm, int perSocket)
{
  void *context = zmq_ctx_new ();

  int rc;

  void *client = zmq_socket (context, ZMQ_DEALER);

  if (perSocket)
  {
    rc = zmq_setsockopt (client, ZMQ_SNDHWM, &hwm, sizeof (hwm));
    assert (rc == 0);

    rc = zmq_setsockopt (client, ZMQ_RCVHWM, &hwm, sizeof (hwm));
    assert (rc == 0);
  }

  rc = zmq_connect (client, "tcp://localhost:4444");
  assert (rc == 0);

  for (int i = 0; i < 100000; ++i)
  {
    printf ("Sending %d\n", i);

    zstr_sendm (client, "");
    zstr_sendf (client, "%d", i);
  }

  zpoller_t *poller = zpoller_new (client);

  while (1)
  {
    zpoller_wait (poller, -1);
    if (zpoller_expired (poller) || zpoller_terminated (poller))
      break;

    char *msg = zstr_recv (client);
    zstr_free (&msg);
  }

  zmq_close (client);
  zmq_ctx_destroy (context);
  return 0;
}

int
server (int hwm, int perSocket)
{
  void *context = zmq_ctx_new ();

  int rc;

  void *server = zmq_socket (context, ZMQ_REP);

  if (perSocket)
  {
    rc = zmq_setsockopt (server, ZMQ_RCVHWM, &hwm, sizeof (hwm));
    assert (rc == 0);
  }

  rc = zmq_connect (server, "tcp://localhost:5555");
  assert (rc == 0);

  zpoller_t *poller = zpoller_new (server);

  while (1)
  {
    zpoller_wait (poller, -1);
    if (zpoller_expired (poller) || zpoller_terminated (poller))
      break;

    char *msg = zstr_recv (server);
    printf ("Received %s\n", msg);
    zstr_free (&msg);

    rc = zstr_send (server, "ack");
    assert (rc == 0);
  }

  zmq_close (server);
  zmq_ctx_destroy (context);
  return 0;
}

int
broker (int hwm, int perSocket)
{
  void *context = zmq_ctx_new ();

  int rc;

  //  Socket facing clients (DEALERs)
  void *frontend = zmq_socket (context, ZMQ_ROUTER);

  if (perSocket)
  {
    rc = zmq_setsockopt (frontend, ZMQ_SNDHWM, &hwm, sizeof (hwm));
    assert (rc == 0);

    rc = zmq_setsockopt (frontend, ZMQ_RCVHWM, &hwm, sizeof (hwm));
    assert (rc == 0);
  }

  rc = zmq_bind (frontend, "tcp://*:4444");
  assert (rc == 0);

  //  Socket facing servers (REPs)
  void *backend = zmq_socket (context, ZMQ_DEALER);

  if (perSocket)
  {
    rc = zmq_setsockopt (backend, ZMQ_SNDHWM, &hwm, sizeof (hwm));
    assert (rc == 0);

    rc = zmq_setsockopt (backend, ZMQ_RCVHWM, &hwm, sizeof (hwm));
    assert (rc == 0);
  }

  rc = zmq_bind (backend, "tcp://*:5555");
  assert (rc == 0);

  //  Start the proxy
  zmq_proxy (frontend, backend, NULL);

  //  We never get here…
  zmq_close (frontend);
  zmq_close (backend);
  zmq_ctx_destroy (context);
  return 0;
}

int
main (int argc, char const *argv[])
{
  if (argc != 4)
  {
    printf ("[broker|client|server] <HWM> [1|0]\n");
    return 2;
  }

  int hwm = atoi (argv[2]);
  int perSocket = atoi (argv[3]);

  if (!perSocket)
  {
    zsys_set_sndhwm (hwm);
    zsys_set_rcvhwm (hwm);
  }

  if (perSocket)
    printf ("Starting %s w/ HWM %d and per-socket options\n", argv[1], hwm);
  else
    printf ("Starting %s w/ HWM %d and global socket options\n", argv[1],
            hwm);

  if (!strcmp (argv[1], "broker"))
  {
    return broker (hwm, perSocket);
  }
  else if (!strcmp (argv[1], "client"))
  {
    return client (hwm, perSocket);
  }
  else if (!strcmp (argv[1], "server"))
  {
    return server (hwm, perSocket);
  }
  else
  {
    return 2;
  }
}
