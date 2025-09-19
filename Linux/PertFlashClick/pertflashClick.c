#include "serial.h"
#include <signal.h>
#include <stdio.h>

#define SERIAL_CLICK -1.0
#define SERIAL_READ_ERROR -69.0
#define AVG_UPPER_LIMIT 1000.0
#define AVG_LOWER_LIMIT 1.0
#define YDOTOOL_LATENCY 10

int mainRunning = 1;

void intHandler(int dummy) {
    mainRunning = 0;
    system("pkill -9 ydotoold");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Run with COM port argument\n");
        return 1;
    }

    int comIndex = atoi(argv[1]);

    // Initialize Serial
    while (!serial_setup(comIndex)) {
        sleep(1);
    }

    signal(SIGINT, intHandler);
    system("ydotoold &");

    double sum = 0.0;
    int runs = 0;

    // Main loop
    while (mainRunning) {
        float f = serial_read();
        if (f == SERIAL_CLICK) {
            system("ydotool click 0x40 >> /dev/null");
            system("ydotool click 0x80 >> /dev/null");
        } else if (f > 0.0) {
            f -= YDOTOOL_LATENCY;
            printf("Response Time: %f \n", f);
            if (f < AVG_UPPER_LIMIT && f > AVG_LOWER_LIMIT) {
                sum += f;
                runs++;
            }

            if (runs % 50 == 0) {
                printf("Avg in %i runs = %f\n", runs, sum / runs);
            }
        } else if (f == SERIAL_READ_ERROR) {
            printf("Error reading serial port\n");
            return 1;
        }
    }

    system("pkill -9 ydotoold");

    return 0;
}
