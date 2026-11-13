#ifndef MAINHPP
#define MAINHPP

#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <sys/prctl.h>
#include <signal.h>
#include <wait.h>
#include <errno.h>
#include <poll.h>

#define TYPE_DISCORD 0x1


void write_log(const char* msg);
void render_error(const char* error);
void init_and_send_apps(int* fd);
void send_id_logos(int* fd);
void Handler_for_processes(int* fd);
void main_cycle_monitoring(int* fd, char* tmp_buff);
int main(void);

#endif