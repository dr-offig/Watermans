/***** serial.h *****/

#ifndef SERIAL_H
#define SERIAL_H


void readSerialPort();
int initSerial(const char *portname, int speed);

#endif