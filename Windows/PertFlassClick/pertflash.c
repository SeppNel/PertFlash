#include <stdio.h>
#include <windows.h> 
#include "libs/CSerialPort.h"

#define SERIAL_CLICK -1.0
#define SERIAL_READ_ERROR -69.0
#define AVG_UPPER_LIMIT 1000.0
#define AVG_LOWER_LIMIT 1.0

void click() {
    INPUT Inputs[3] = { 0 };

    Inputs[0].type = INPUT_MOUSE;
    Inputs[0].mi.dx = 32767; // Half width
    Inputs[0].mi.dy = 32767; // Half height
    Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;

    Inputs[1].type = INPUT_MOUSE;
    Inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    Inputs[2].type = INPUT_MOUSE;
    Inputs[2].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    SendInput(3, Inputs, sizeof(INPUT));
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Run with COM port argument\n");
        getc(stdin);
        return 1;
    }

    printf("Press any key when ready...");
    getc(stdin);
    Sleep(2000);

    int comIndex = atoi(argv[1]);

    //Initialize Serial
    auto serial = OpenPort(comIndex);
    SetPortBoudRate(serial, CP_BOUD_RATE_19200);
    SetPortDataBits(serial, CP_DATA_BITS_8);
    SetPortStopBits(serial, CP_STOP_BITS_ONE);
    SetPortParity(serial, CP_PARITY_NOPARITY);


    double sum = 0.0;
    int runs = 0;

    // Main loop
    while (1) {
        char buff[4];
        memset(buff, 0, 4);
        ReciveData(serial, buff, 4);
        float f;
        memcpy(&f, buff, 4);
        if(f == SERIAL_CLICK){
            click();
        }
        else if(f > 0.0){
            printf("Response Time: %f \n", f);
            if(f < AVG_UPPER_LIMIT && f > AVG_LOWER_LIMIT){
                sum += f;
                runs++;
            }

            if(runs % 50 == 0 && runs > 0){
                printf("Avg in %i runs = %f\n", runs, sum/runs);
            }
        }
        else if(f == SERIAL_READ_ERROR){
            printf("Error reading serial port\n");
            return 1;
        }
    }
}
