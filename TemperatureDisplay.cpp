#define I2C0DAT (*(volatile unsigned int *)0x4001C02C)
#define I2C0CONCLR (*(volatile unsigned int *) 0x4001C018)
#define I2C0CONSET (*(volatile unsigned int *) 0x4001C000)
#define I2C0SCLH (*(volatile unsigned int *)  0x4001C010)
#define I2C0SCLL (*(volatile unsigned int *) 0x4001C014)
#define PINSEL1 	(*(volatile unsigned int *)0x4002C004)

#ifdef _USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>
#include <stdbool.h>

void wait_ms(int ms) {
    long iterations = ms * 1000 / 0.25; // Convert ms to µs, then to cycles
    long number = 0;
    for (long i = 0; i < iterations; i++) {
        number++;
    }
}

void writeI2C(int data) {
    I2C0DAT = data;
    I2C0CONCLR = (1 << 3);
    while (((I2CCONSET >> 3) & 1) == 0) {
        // Wait for acknowledgment
    }
}

void startI2C() {
    I2C0CONSET = (1 << 3);
    I2C0CONSET = (1 << 5);
    I2C0CONCLR = (1 << 3);
    while (((I2C0CONSET >> 3) & 1) == 0) {
        // Wait for start condition
    }
    I2C0CONCLR = (1 << 5);
}

int readI2C(bool fin) {
    // Clear or set AA bit if it is the final read (fin == 1)
    if (fin == 1) {
        I2C0CONCLR = (1 << 2);
    } else {
        I2C0CONSET = (1 << 2);
    }
    I2C0CONCLR = (1 << 3);
    // Wait for SI to return 0
    while (((I2C0CONSET >> 3) & 1) == 0) {
        // Do nothing
    }
    return I2C0DAT;
}

void stopI2C() {
    I2C0CONSET = (1 << 4);
    I2C0CONCLR = (1 << 3);
    while (((I2C0CONSET >> 4) & 1) == 1) {
        // Wait for stop condition
    }
}

void initI2C() {
    PINSEL1 |= (1 << 22);
    wait_ms(1);
    PINSEL1 |= (1 << 24);
    I2C0SCLH = 0x55;
    I2C0SCLL = 0x55;
    I2C0CONSET = (1 << 6);
    wait_ms(1);
    I2C0CONSET = (1 << 6);
}

int sevenSegConverter(int input) {
    switch (input) {
        case -1: return 0b01111111;
        case 0: return 0b01111110;
        case 1: return 0b00110000;
        case 2: return 0b01101101;
        case 3: return 0b01111001;
        case 4: return 0b00110011;
        case 5: return 0b01011011;
        case 6: return 0b01011111;
        case 7: return 0b01110000;
        case 8: return 0b01111111;
        case 9: return 0b01111011;
        default: return 0b01111111;
    }
}

void writeLeft(int count) {
    startI2C();
    writeI2C(0b01000000);
    writeI2C(0x01);
    writeI2C(0x00);
    writeI2C(0x00);
    stopI2C();
    startI2C();
    writeI2C(0b01000000);
    writeI2C(0x13);
    writeI2C(0b01000000);
    writeI2C(sevenSegConverter(count));
    stopI2C();
}

void writeRight(int count) {
    startI2C();
    writeI2C(0b01000000);
    writeI2C(0x01);
    writeI2C(0x00);
    writeI2C(0x00);
    stopI2C();
    startI2C();
    writeI2C(0b01000000);
    writeI2C(0x12);
    writeI2C(0b01000000);
    writeI2C(sevenSegConverter(count));
    stopI2C();
}

bool readSW() {
    bool sw = 0;

    startI2C();
    writeI2C(0b01000000);
    writeI2C(0x81);
    writeI2C(0x80);
    stopI2C();

    startI2C();
    writeI2C(0b01000000);
    writeI2C(0x13);
    startI2C();
    writeI2C(0b01000001);
    sw = (readI2C(true) >> 7) & 1;
    stopI2C();

    return sw;
}

double convertF(double input) {
    double celsiusToFahrenheit = 0;
    celsiusToFahrenheit = (input * 9) / 5 + 32;
    return celsiusToFahrenheit;
}

double readTemp() {
    int MSByte = 0;
    int LSByte = 0;
    startI2C();
    writeI2C(0b01101111);
    MSByte = readI2C(false);
    LSByte = readI2C(true);
    MSByte = (MSByte << 3);
    LSByte = (LSByte >> 5);
    double total = (MSByte + LSByte) * 0.125;
    stopI2C();

    return total;
}

int main(void) {
    initI2C();
    bool fer = 1;
    // Temp Sensor address is 0110 111
    while (1) {
        float temp;
        int tens, ones;
        temp = readTemp();
        bool sw = readSW();

        if (sw == 1) {
            fer = !fer;
            wait_ms(10);
        }

        if (fer == 1) {
            temp = convertF(temp);
        }

        tens = temp / 10;
        int tempINT = temp;
        ones = tempINT % 10;

        writeLeft(tens);
        writeRight(ones);
    }

    return 0;
}
