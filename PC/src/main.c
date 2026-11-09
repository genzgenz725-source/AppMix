#include "../inc/main.h"
#include <string.h>

const char startpkg[15] = "0 210214301131";
char apps[3][10];
volatile uint8_t res = 0;
const uint32_t startSendingPkg = 0x067F0F67;
const char* LOGOPATH = ".logos/";

void write_log(const char* msg) {
    const char* HOMEDIR = getenv("HOME");
    char path_to_log[64];
    strcat(path_to_log, HOMEDIR); strcat(path_to_log, ".logs/AppMix.log");
    FILE* file = fopen(path_to_log, "wb");
    // rewrite
    printf("%s", msg);
}

_Bool init_apps() {
    char tmp_res = res;
    int count;

    const char* cmd = 
        "(pgrep -x \"Discord\" > /dev/null && echo -n \"1\" || echo -n \"0\"); "
        "(pgrep -x \"Zen\" > /dev/null && echo -n \"1\" || echo -n \"0\"); "
        "(pgrep -x \"Minecraft\" > /dev/null && echo -n \"1\" || echo -n \"0\"); "
        "(pgrep -x \"Chromium\" > /dev/null && echo -n \"1\" || echo -n \"0\"); "
        "(pgrep -x \"Telegram\" > /dev/null && echo -n \"1\" || echo -n \"0\"); "
        "echo \"\"";

    FILE* pipe = popen(cmd, "r");
    if (pipe) {
        char buffer[6];
        fgets(buffer, sizeof(buffer), pipe);
        for (int i = 0; i < 5; i++) {
            if (buffer[i] == '0') {
                continue;
            } else if (buffer[i] == '1') {
                res |= (1 << i);
            }
        }
        pclose(pipe);
    }
    if (tmp_res == res) {
        return 0;
    }
    return 1;
}

void send_logos(int* fd, const char* HOMEDIR) {     // sending logos to MC
    FILE* file; // fd for file
    int byte;   // byte which we take from 
    while (5) {
        char path_to_logo[64];
        strcat(path_to_logo, HOMEDIR); strcat(path_to_logo, "./logo/");
        file = fopen(path_to_logo, "wb");   // open fd to current file of logo
        byte = 0;   // cant use unsigned types cause of EOF = -1 = 255

        write(*fd, &startSendingPkg, 4);    // says that its gonna send logos to MC
        while ((byte = fgetc(file)) != EOF) {   // check for end of file
            write(*fd, &byte, 1);           // sending all logos to MC
        }
    }
    pclose(file);                           // close FD
}

_Bool main_cycle_monitoring(int* fd, char* buff, const char* HOMEDIR) { // main cycle
    while (*fd < 0) {
        // trying to open the port
        *fd = open("/dev/ttyUSB0", O_NOCTTY | O_RDONLY);
        
        if (*fd < 0) {
            // mini delay
            sleep(1);
        }
    }
    
    // MC was finnaly connected

    while(1) {
        memset(buff, 0, 64);
        read(*fd, buff, 64);    // reading msgs from MC
        if (!strcmp(buff, startpkg)) {  // if its not gonna be the start package we gonna close the MC FD and write log about that
            close(*fd);
            return 0;
        }
    }
    
    // first msg from MC equal the start package

    init_apps(); // filling res ptr by info about which apps is working right now

    while (1) {         // main cycle
        if (init_apps()) {   // every second we checking the apps running
            send_logos(fd, HOMEDIR);    // if some apps was chenged - we send pkgs to MC about it
        }

        sleep(1);   // delay on 1 sec
        
    }
}

int main(void) {
    const char* HOMEDIR = getenv("HOME");   // home dir for writing logs and 
    int fd;                                      // main FD for MC
    char buff[64];                               // buffer for msgs from MC

    if (!main_cycle_monitoring(&fd, buff, HOMEDIR)) {
        write_log("problem with verification");
        return 0;
    }

    return 0;
}