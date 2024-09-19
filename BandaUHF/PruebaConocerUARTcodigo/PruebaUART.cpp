
// C library headers
#include <iostream>
#include <string>
#include <bits/stdc++.h> 
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <termios.h>
#include <unistd.h>
#include<stdlib.h>
using namespace std;

int OpenPort(int* serial_port,char* portname);
int readFromSerialPort(int fd, char* buffer, size_t size);

int main() {
	int serial;
	int i;
	char buffer[100];
	int n;
	char Puerto1[]="/dev/tty";
	
	FILE* file_ptr = fopen("PruebaPuertosUSB.txt", "a");
        if (file_ptr == NULL)
        {
            printf("Error opening file\n");
            return 1;
        }
	fprintf(file_ptr, "Iniciando Prueba\n");
        fclose(file_ptr);
	OpenPort(&serial,Puerto1);
    	n = readFromSerialPort(serial, buffer, sizeof(buffer));
	file_ptr = fopen("PruebaPuertosUSB.txt", "a");
        if (n < 0) {
	    fprintf(file_ptr, "\nError reading from serial port 1: ");
	    fprintf(file_ptr, strerror(errno));
         }
        else {
	    fprintf(file_ptr, "\nRead from serial port 1: ");
	    string Informacion1=std::string(buffer, n);
	    char Info1[Informacion1.length() + 1];
	    strcpy(Info1, Informacion1.c_str()); 
	    fprintf(file_ptr, Info1);
        }
	fclose(file_ptr);
	close(serial);
	n=-1;

	char Puerto2[]="/dev/ttyPSO";
	OpenPort(&serial,Puerto2);
	n = readFromSerialPort(serial, buffer, sizeof(buffer));
	file_ptr = fopen("PruebaPuertosUSB.txt", "a");
        if (n < 0) {
            fprintf(file_ptr, "\nError reading from serial port 2: ");
	    fprintf(file_ptr, strerror(errno));
         }
        else {
            fprintf(file_ptr, "\nRead from serial port 2: ");
	    string Informacion2=std::string(buffer, n);
	    char Info2[Informacion2.length() + 1];
	    strcpy(Info2, Informacion2.c_str()); 
	    fprintf(file_ptr, Info2);
        }
	fclose(file_ptr);
	close(serial);
	n=-1;

	char Puerto3[]="/dev/ttyGS0";
	OpenPort(&serial,Puerto3);
	n = readFromSerialPort(serial, buffer, sizeof(buffer));
	file_ptr = fopen("PruebaPuertosUSB.txt", "a");
        if (n < 0) {
            fprintf(file_ptr, "\nError reading from serial port 3: ");
	    fprintf(file_ptr, strerror(errno));
         }
        else {
            fprintf(file_ptr, "\nRead from serial port 3: ");
	    string Informacion3=std::string(buffer, n);
	    char Info3[Informacion3.length() + 1];
	    strcpy(Info3, Informacion3.c_str()); 
	    fprintf(file_ptr, Info3);
        }
	fclose(file_ptr);
	close(serial);
	n=-1;

	for(i=0; i < 64;i++){
		string Cadena="/dev/tty"+std::to_string(i);
		char portname[Cadena.length() + 1];
		strcpy(portname, Cadena.c_str()); 
		OpenPort(&serial,portname);
		n = readFromSerialPort(serial, buffer, sizeof(buffer));
	        file_ptr = fopen("PruebaPuertosUSB.txt", "a");
		if (n < 0) {
	            string CadenaError="/Error reading from serial port "+std::to_string(i) +"\n";
	            char CadenaErrorA[CadenaError.length() + 1];
		    strcpy(CadenaErrorA, CadenaError.c_str());
		    fprintf(file_ptr, CadenaErrorA);
	    	    fprintf(file_ptr, strerror(errno));
		 }
		else {
		    string CadenaLectura="/Read from serial port "+std::to_string(i) +"\n";
	            char CadenaLecturaA[CadenaLectura.length() + 1];
		    strcpy(CadenaLecturaA, CadenaLectura.c_str());
		    fprintf(file_ptr, CadenaLecturaA);
                    string Informacionv=std::string(buffer, n);
	    	    char Infov[Informacionv.length() + 1];
	    	    strcpy(Infov, Informacionv.c_str()); 
	            fprintf(file_ptr, Infov);
		}
		fclose(file_ptr);
		close(serial);
		n=-1;
	}
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

// Function to read data from the serial port
int readFromSerialPort(int fd, char* buffer, size_t size)
{
    return read(fd, buffer, size);
}
