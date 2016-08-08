#include <libmuta.h>
#include <libpomp.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/socket.h>

#define ULOG_TAG muta
#include <ulog.h>
ULOG_DECLARE_TAG(muta);

struct app {
  struct pomp_ctx *ctx;
  struct pomp_loop *loop;
  void *priv;
  int stop;
  libmuta_processing_callback process;
};

enum
{
  SEND_FD,
  BUFFER_PROCESSING_DONE,
};


static struct app s_app = {NULL, NULL, NULL, 0, NULL};

static int client_start(const struct sockaddr *addr, uint32_t addrlen)
{
  int res;
  res = pomp_ctx_connect(s_app.ctx, addr, addrlen);
  if (res < 0)
    ULOGE("pomp_ctx_connect: err=%d(%s)", res, strerror(-res));
  return res;
}

void muta_stop(void)
{
  s_app.stop = 1;
}

static void client_event_cb(struct pomp_ctx *ctx, enum pomp_event event,
                struct pomp_conn *conn, const struct pomp_msg *msg,
                void *userdata)
{
  int fd, msgid;
  unsigned int bufsize;
  void *video_buffer;
  GstVideoFormat videoformat;
  unsigned int width, height;

  switch (event) {
    case POMP_EVENT_CONNECTED:
      ULOGI("connected to pimp user filter");
      break;
    case POMP_EVENT_DISCONNECTED:
      ULOGI("disconnected from pimp user filter");
      break;
    case POMP_EVENT_MSG:
      switch (msgid = pomp_msg_get_id(msg)) {
        case SEND_FD:
          pomp_msg_read(msg, "%x %u %u %u %u", &fd, &bufsize, &videoformat, &width, &height);
          ULOGI("received a FD from pimp: %d", fd);
          video_buffer = mmap(NULL, bufsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
          s_app.process(video_buffer, bufsize, videoformat, width, height, s_app.priv);
          pomp_msg_send();
          break;
        default:
          ULOGW("received unknown message id from pimp : %d", msgid);
          break;
      }
      break;
    default:
      ULOGE("Unknown event: %d", event);
      break;
  }
}

void muta_connect(libmuta_processing_callback f, void *priv)
{
  struct sockaddr_storage addr_storage;
  struct sockaddr *addr = NULL;
  uint32_t addrlen = 0;

  memset(&addr_storage, 0, sizeof(addr_storage));
  addr = (struct sockaddr *)&addr_storage;
  addrlen = sizeof(addr_storage); 

  pomp_addr_parse("unix:/var/snap/facedetect/common/lol.sock", addr, &addrlen);

  s_app.ctx = pomp_ctx_new(client_event_cb, f);
  s_app.loop = pomp_ctx_get_loop(s_app.ctx);

  client_start(addr, addrlen);
  s_app.stop = 0;
  s_app.priv = priv;
  s_app.process = f;

  while (!s_app.stop) {
    pomp_loop_wait_and_process(s_app.loop, -1);
  }

  if (s_app.ctx != NULL) {
    pomp_ctx_stop(s_app.ctx);
    pomp_ctx_destroy(s_app.ctx);
  }
}
