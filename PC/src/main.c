#include "../inc/main.h"

const char startpkg[15] = "0 210214301131";
char apps[3][10];
volatile uint8_t res = 0;
volatile uint16_t ADC_value = 0x0;
const uint32_t startSendingPkg = 0x67F0F670;
const char* LOGOPATH = ".logos/";

void log_error(const char* msg) {
    const char* HOMEDIR = getenv("HOME");
    char path_to_log[256];
    snprintf(path_to_log, sizeof(path_to_log), "%s/.logs/AppMix.log", HOMEDIR);
    
    FILE* file = fopen(path_to_log, "a");
    if (file != NULL) {
        fprintf(file, "%s\n", msg);
        fclose(file);
    } else {
        printf("Не удалось открыть лог-файл!\n");
    }
}

void render_error(const char* error) {
    // logic of out to DE notify
}

bool init_apps() {
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
                res &= ~(1 << i);
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

void send_id_logos(int* fd) {
    char pkg[8];
    memset(pkg, 0, 8);
    
    if (res & (1 << 7)) pkg[0] = '1'; else pkg[0] = '0';
    if (res & (1 << 6)) pkg[1] = '1'; else pkg[1] = '0';
    if (res & (1 << 5)) pkg[2] = '1'; else pkg[2] = '0';
    if (res & (1 << 4)) pkg[3] = '1'; else pkg[3] = '0';
    if (res & (1 << 3)) pkg[4] = '1'; else pkg[4] = '0';
    if (res & (1 << 2)) pkg[5] = '1'; else pkg[5] = '0';
    if (res & (0x1))    pkg[6] = '1'; else pkg[6] = '0';
    
    pkg[7] = '\\';

    write(*fd, pkg, 8);
}

void Handler_for_processes(int* fd) {
    pid_t pid = fork();

    if (pid == 0) {
        char tmp_buff[7];

        while (1) {
            memset(tmp_buff, 0, 7);
            
            uint8_t readed = 0;
            char tmp_byte = 0x0;

            while (tmp_byte != '\\' && readed < 7) {
                int n = read(*fd, &tmp_byte, 1);

                if (n == 1) {
                    tmp_buff[readed++] = tmp_byte;
                } else if (n < 0) {
                    break;
                }
            }
            
            tmp_buff[readed] = '\0';

            if (*tmp_buff == '1') {
                char* tmp_ptr = tmp_buff + 2;
                char tmp_int_buff[6]; memset(tmp_int_buff, 0, 6);
                uint8_t tmp_it = 0;
                
                while (*tmp_ptr != '\\' && *tmp_ptr != '\0') {
                    if (tmp_it >= 5)  {
                        log_error("MC is sending wrong ADC value!");
                        break;
                    }
                    tmp_int_buff[tmp_it++] = *tmp_ptr++;
                }
                tmp_int_buff[tmp_it] = '\0';
                
                ADC_value = atoi(tmp_int_buff);
            }

            usleep(5000);
        }
        return;
    } else {
        while (1) {
            if (init_apps()) {
                send_id_logos(fd);
            }
            sleep(1);
        }
        return;
    }
}

bool main_cycle_monitoring(int* fd, char* tmp_buff) { // main cycle
    *fd = open("/dev/ttyUSB0", O_NOCTTY | O_RDWR);
    while (*fd < 0) {
        sleep(1);
        // trying to open the port
        *fd = open("/dev/ttyUSB0", O_NOCTTY | O_RDWR);
    }
    
    // MC was finnaly connected

    memset(tmp_buff, 0, 64);
    uint8_t readed = 0;
    char tmp_byte = 0x0;

    while (tmp_byte != '\\' && readed < 15) {    // waiting for all bytes cause of UART
        int n = read(*fd, &tmp_byte, 1);
        
        if (n == 1) {
            tmp_buff[readed++] = tmp_byte;
        } else if (n < 0) {
            break;
        }
    }
    tmp_buff[readed] = '\0';

    if (strcmp(tmp_buff, startpkg) != 0) {  // if its not gonna be the start package we gonna close the MC FD and write log about that
        close(*fd);
        return 0;
    }

    // first msg from MC equal the start package

    Handler_for_processes(fd);
    render_error("critical fault");
}

int main(void) {
    int fd = -1;                                  // main FD for MC
    char buff[64];                               // buffer for msgs from MC

    if (!main_cycle_monitoring(&fd, buff)) {
        log_error("problem with verification");
        return 0;
    }

    return 0;
}