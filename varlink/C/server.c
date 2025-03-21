/**
 * top.jouyouyun.calculator 是一个简单的计算器服务
 **/
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <varlink.h>

#define _IFACE_ADDRESS "unix:/run/user/1000/top.jouyouyun.calculator"

// TODO: handle client call closed

// handle client method call
long handle_method_call(VarlinkService *srv, VarlinkCall *call,
                        VarlinkObject *params, uint64_t flags, void *userdata) {
  long rc = 0;
  int64_t i = 0, j = 0;
  VarlinkObject *reply = NULL;
  const char *method = varlink_call_get_method(call);

  // Read parameters with validation
  rc = varlink_object_get_int(params, "i", &i);
  if (rc < 0) {
    fprintf(stderr, "varlink object get 'i': %s\n", varlink_error_string(-rc));
    varlink_call_reply_invalid_parameter(call, "i");
    return -1;
  }

  rc = varlink_object_get_int(params, "j", &j);
  if (rc < 0) {
    fprintf(stderr, "varlink object get 'j': %s\n", varlink_error_string(-rc));
    varlink_call_reply_invalid_parameter(call, "j");
    return -1;
  }

  // debug logging
  fprintf(stderr, "Method call: %s, i=%ld, j=%ld\n", method, i, j);

  rc = varlink_object_new(&reply);
  if (rc < 0) {
    fprintf(stderr, "varlink object new reply: %s\n",
            varlink_error_string(-rc));
    return rc;
  }

  if (strcmp(method, "Add") == 0) {
    rc = varlink_object_set_int(reply, "rc", i + j);
  } else if (strcmp(method, "Sub") == 0) {
    rc = varlink_object_set_int(reply, "rc", i - j);
  } else if (strcmp(method, "Multiple") == 0) {
    rc = varlink_object_set_int(reply, "rc", i * j);
  } else if (strcmp(method, "Division") == 0) {
    if (j == 0) {
      varlink_call_reply_invalid_parameter(call, "j");
      return -1;
    }
    rc = varlink_object_set_int(reply, "rc", i / j);
  } else {
    // Unknown method
    char buf[1024] = {0};
    sprintf(buf, "unknown method: %s", method);
    varlink_call_reply_error(call, buf, NULL);
    return -1;
  }

  if (rc < 0) {
    fprintf(stderr, "varlink object set reply: %s\n",
            varlink_error_string(-rc));
    return rc;
  }

  varlink_call_reply(call, reply, 0);
  return 0;
}

uint64_t read_file(const char *filename, char **contents) {
  FILE *file = fopen(filename, "rb");
  if (!file) {
    // Set errno to indicate failure
    return 0;
  }

  // Get file size
  fseek(file, 0, SEEK_END);
  uint64_t size = ftell(file);
  rewind(file);

  if (size == 0) {
    fclose(file);
    return 0;
  }

  // Allocate memory for contents + null terminator
  *contents = (char *)calloc(size + 1, sizeof(char));
  if (!*contents) {
    fclose(file);
    return 0;
  }

  // Read file contents
  if (fread(*contents, 1, size, file) != size) {
    free(*contents);
    fclose(file);
    return 0;
  }

  fclose(file);
  return size;
}

int main(int argc, char *argv[]) {
  long rc = 0;
  uint64_t size = 0;
  char *contents = NULL;
  VarlinkService *srv = NULL;

  int epoll_fd = 0;
  struct epoll_event event;

  // Read varlink interface file
  size = read_file(argv[1], &contents);
  if (size == 0) {
    fprintf(stderr, "read file failure\n");
    return -1;
  }

  rc = varlink_service_new(&srv, "Jouyouyun", "Calculator Service", "1",
                           "https://jouyouyun.github.io", _IFACE_ADDRESS, -1);
  if (rc < 0) {
    fprintf(stderr, "new varlink service failed: %s\n",
            varlink_error_string(-rc));
    free(contents);
    return -1;
  }

  rc = varlink_service_add_interface(
      srv, contents, "Add", handle_method_call, NULL, "Sub", handle_method_call,
      NULL, "Multiple", handle_method_call, NULL, "Division",
      handle_method_call, NULL, NULL);
  free(contents);
  if (rc < 0) {
    fprintf(stderr, "add varlink interface failed: %s\n",
            varlink_error_string(-rc));
    return -1;
  }

  epoll_fd = epoll_create1(EPOLL_CLOEXEC);
  if (epoll_fd < 0) {
    fprintf(stderr, "epoll create failed: %s\n", strerror(-epoll_fd));
    goto out;
  }

  memset(&event, 0, sizeof(event));
  event.events = EPOLLIN;
  event.data.fd = varlink_service_get_fd(srv);
  rc = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, varlink_service_get_fd(srv), &event);
  if (rc < 0) {
    fprintf(stderr, "epoll add failed: %s\n", strerror(-rc));
    goto out;
  }

  for (;;) {
    struct epoll_event ev;

    rc = epoll_wait(epoll_fd, &ev, 1, -1);
    if (rc < 0) {
      fprintf(stderr, "epoll wait failed: %s\n", strerror(-rc));
      break;
    } else if (rc == 0)
      continue;

    if (ev.data.fd == varlink_service_get_fd(srv)) {
      rc = varlink_service_process_events(srv);
      if (rc < 0) {
        fprintf(stderr, "process varlink events failed: %s\n",
                varlink_error_string(-rc));
        break;
      }
    }
  }

out:
  if (epoll_fd > 0)
    close(epoll_fd);
  varlink_service_free(srv);

  return 0;
}
