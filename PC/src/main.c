#include "../inc/main.h"
#include <unistd.h>

const char startpkg[15] = "0 210214301131";
char apps[3][10];
volatile uint16_t apps_working = 0;
const uint32_t startSendingPkg = 0x67F0F670;
uint16_t ADC_value = 0x0;
const char* LOGOPATH = ".logos/";
const char apps_arr[8][32] = {
    "Discord",
    "Zen",
    "Minecraft",
    "Chromium",
    "Telegram",
    "",
    "",
    ""
};

void log_error(const char* msg) {
    const char* HOMEDIR = getenv("HOME");
    char path_to_log[256];
    snprintf(path_to_log, sizeof(path_to_log), "%s/.logs/AppMix.log", HOMEDIR);

    FILE* file = fopen(path_to_log, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", msg);
        fclose(file);
    } else {
        render_error("Не удалось открыть лог-файл!\n");
    }
}

void render_error(const char* error) {
    // logic of out to DE notify
}

void init_and_send_apps(int* fd) {
    char tmp_apps = apps_working;

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
        memset(buffer, 0, 6);
        fgets(buffer, sizeof(buffer), pipe);
        for (int i = 0; i < 5; i++) {
            if (buffer[i] == '0') {
                apps_working &= ~(1 << i);
            } else if (buffer[i] == '1') {
                apps_working |= (1 << i);
            }
        }
        pclose(pipe);
    }
    if (tmp_apps == apps_working) {
        return;
    } else {
        tmp_apps = apps_working;
    }
    if (write(*fd, &tmp_apps, 1) != 1) {
        log_error("write error");
        render_error("write error from init and send apps");
        return;
    }
    return;
}

void build_pkg(uint32_t* dst, uint16_t ADC, uint8_t SCK) {
    *dst = 0;
    *dst |= SCK;
    *dst |= ((uint32_t)ADC << 8);
}

void take_pkg(uint32_t dst, uint16_t* ADC, uint8_t* SCK) { 
    *SCK = dst & 0xFF;
    *ADC = (dst >> 8) & 0x0FFF;
}

void Handler_for_processes(int* fd) {
    int pipe_data[2];
    if (pipe(pipe_data) < 0) {
        log_error("pipe failed");
        return;
    }
    pid_t pid1 = fork();
    if (pid1 < 0) {
        log_error("fork1 failed");
        close(pipe_data[0]);
        close(pipe_data[1]);
        return;
    }
    // set PW for our work

    if (pid1 == 0) { // child process
        close(pipe_data[0]);
        bool hardfault = 0;
        prctl(PR_SET_PDEATHSIG, SIGTERM); // rules linux like

        char tmp_buff[9];

        while (1) {
            memset(tmp_buff, 0, 9);

            uint8_t SCK_for_apps = 0;
            uint8_t readed = 0;
            char tmp_byte = 0x0;

            while (tmp_byte != '\\' && readed < 8) {    // taking all data
                int n = read(*fd, &tmp_byte, 1);

                if (n == 1) {
                    tmp_buff[readed++] = tmp_byte;
                } else if (n < 0 && errno == EINTR) {   // if read was interrupted by any other signal we try again
                    continue;
                } else {
                    hardfault = true;
                    break;
                }
            }

            if (hardfault) { break; }

            tmp_buff[readed] = '\0';

            if (readed > 4 && *tmp_buff == '1') {   // handle data if its ADC data
                uint32_t tmp_pkg = 0;
                uint16_t ADC_value = atoi(tmp_buff + 4);
                if (ADC_value != 0 && tmp_buff[5] != '0') {
                    build_pkg(&tmp_pkg, ADC_value, tmp_buff[2] - '0');
                }
                if (write(pipe_data[1], &tmp_pkg, 4) != 4) {
                    log_error("write error");
                    render_error("write error from init and send apps");
                    break;
                }
            }
            usleep(5000);
        }
        _exit(125);
    }

    pid_t pid2 = fork();
    if (pid2 < 0) {
        log_error("fork2 failed");
        kill(pid1, SIGTERM); waitpid(pid1, NULL, 0);
        return;
    }
    if (pid2 == 0) {
        close(pipe_data[1]);
        prctl(PR_SET_PDEATHSIG, SIGTERM);
        uint8_t total_read;
        uint32_t buff;
        bool hardfault = 0;
        uint16_t ADC_value;
        uint8_t SCK_value;

        while (1) {
            buff = 0;
            total_read = 0;
            while (total_read < 4) {    // waiting for all data
                ssize_t n = read(pipe_data[0], ((char*)&buff) + total_read, 4 - total_read); // waiting for 4 bytes but if we got 1 or 2 or 3 we gonna fill it up, and after make a << on readed * 8 bites
                if (n > 0) {
                    total_read += n;
                } else if (n == 0) {
                    hardfault = 1;
                    break;
                } else if (errno == EINTR) {
                    continue;
                } else {
                    break;
                }
            }

            if (hardfault) { break; }

            take_pkg(buff, &ADC_value, &SCK_value);

            
            // changing the vol
        }
        _exit(125);
    }

    close(pipe_data[0]); close(pipe_data[1]);
    while (1) {
        int status;
        pid_t r = waitpid(pid1, &status, WNOHANG);   // if child process is dead, we too
        if (r == pid1) {
            log_error("child1 died");
            break;
        }
        if (r == -1 && errno != EINTR) { log_error("waitpid error!!!!!"); break; }  // cant ping process

        pid_t r2 = waitpid(pid2, &status, WNOHANG);
        if (r2 == pid2) {
            log_error("child2 died");
            break;
        }
        if (r2 == -1 && errno != EINTR) { log_error("waitpid error!!!!!"); break; }  // cant ping process

        init_and_send_apps(fd);
        while (poll(NULL, 0, 1000) < 0 && errno == EINTR);
    }
    return;
}

void main_cycle_monitoring(int* fd, char* tmp_buff) { // main cycle
    *fd = open("/dev/ttyUSB0", O_NOCTTY | O_RDWR);
    while (*fd < 0) {
        poll(NULL, 0, 1000);
        // trying to open the port
        *fd = open("/dev/ttyUSB0", O_NOCTTY | O_RDWR);
    }

    // MC was finnaly connected

    memset(tmp_buff, 0, 15);
    uint8_t readed = 0;
    char tmp_byte = 0x0;

    while (readed < 14) {    // waiting for all bytes cause of UART
        int n = read(*fd, &tmp_byte, 1);

        if (n == 1) tmp_buff[readed++] = tmp_byte;
        else if (n < 0 && errno == EINTR) continue;
        else break;
    }
    tmp_buff[readed] = '\0';

    if (strcmp(tmp_buff, startpkg) != 0) {  // if its not gonna be the start package we gonna close the MC FD and write log about that
        close(*fd);
        return;
    }

    // first msg from MC equal the start package

    Handler_for_processes(fd);
    render_error("critical fault");
    close(*fd);
    return;
}

int main(void) {
    int fd = -1;                                  // main FD for MC
    char buff[15];                                // buffer for init_cfg msg from MC

    main_cycle_monitoring(&fd, buff);
    log_error("problem with verification");
    return 0;
}