#pragma once
int watch_file(const char *filename, unsigned int init_flags,
	       unsigned int mark_flags, unsigned int event_flags);
int watch_remove(int fd);
int poll_event();
