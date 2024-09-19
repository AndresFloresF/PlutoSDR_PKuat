#include <iostream>
using namespace std;
// C library headers
#include <stdio.h>
#include <string.h>
#include <math.h>

// Linux headers
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h> // write(), read(), close()

void SendGPS(int* serial);
void sendData(int* serial, unsigned char* data, int size);
void Sleep(int ms);
int OpenPort(int* serial_port,char* portname);

int main(int argc,char** argv) {
    //Pass to main function number of cicles and time step [ms]
	float B[3]{}, B_1[3]{}, V[3]{};
    int serial;
    int cicles=1,dt=1000;
    char portname[]="/dev/ttyUSB0";
    //Abrir puerto serial
	OpenPort(&serial,portname);
    if (argc>1){
        cicles=atol(argv[1]);
    }
    printf("cicles: %d\n",cicles);
    if (argc>2){
        dt=atol(argv[2]);
    }
    printf("time step: %d\n",dt);
    if (argc>3){
        strcpy(portname,argv[3]);
    }
    printf("port: %s\n",portname);
    for(int i=0;i<cicles;i++){
        //SendCMD(&serial,'d');
        SendGPS(&serial);
        Sleep(1000);
    }
	close(serial);
}

void SendGPS(int* serial){
    unsigned char GPS[200];
    int size=0;
    size=sprintf((char*)GPS,"$GPGSV,3,1,09,05,03,195,,06,71,009,35,12,15,322,29,14,33,117,09*72\r\n");
    sendData(serial,GPS,size);
    size=sprintf((char*)GPS,"$GPGSV,3,2,09,17,25,050,34,19,38,026,35,20,03,200,,24,35,282,31*7A\r\n");
    sendData(serial,GPS,size);
    size=sprintf((char*)GPS,"$GPGSV,3,3,09,28,49,102,12*47\r\n");
    sendData(serial,GPS,size);
    size=sprintf((char*)GPS,"$GPGLL,2042.29880,N,10027.05018,W,180548.00,A,A*7C\r\n");
    sendData(serial,GPS,size);
    size=sprintf((char*)GPS,"$GPGGA,180549.00,2042.29894,N,10027.05023,W,1,05,3.08,1964.2,M,-9.3,M,,*64\r\n");
    sendData(serial,GPS,size);
}

void sendData(int* serial, unsigned char* data, int size){
    write(*serial, data, size);
}

void Sleep(int ms){
    usleep(ms*1000);
}

int OpenPort(int* serial_port,char* portname){
    // Open the serial port. Change device path as needed (currently set to an standard FTDI USB-UART cable type device)
  *serial_port = open(portname, O_RDWR);

  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if(tcgetattr(*serial_port, &tty) != 0) {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ECHOE; // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B9600);
  cfsetospeed(&tty, B9600);

  // Save tty settings, also checking for error
  if (tcsetattr(*serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }
  return 0;
}
