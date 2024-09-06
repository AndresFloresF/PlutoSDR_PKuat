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

void ReadReg(int* serial, unsigned char reg, unsigned char* data, int size);
void SetReg(int* serial, unsigned char reg, unsigned char* data, int size);
void SendCMD(int* serial, unsigned char cmd);
void ReadMagnetometer(int* serial, float B[3], float B_1[3]);
void float2uchar(float* fdata, unsigned char* cdata);
void uchar2float(unsigned char* cdata, float* fdata, int size);
void BDot(float B[3], float B_1[3], float V[3], float dt);
void SetMagnetorquers(int* serial, float V[3]);
void MagnetorquersOff(int* serial);
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
        //Lectura de sensores
        ReadMagnetometer(&serial,B, B_1);
        //Algoritmo de control
        BDot(B,B_1,V,dt);
        //Visualizacion datos
        cout << "B(-1): " << B_1[0] << " " << B_1[1] << " " << B_1[2] << " T" << endl;
        cout << "B:     " << B[0] << " " << B[1] << " " << B[2] << " T" << endl;
        cout << "V:     " << V[0] << " " << V[1] << " " << V[2] << endl;
        //Encender magnetorcas
        SetMagnetorquers(&serial,V);
        //Tiempo de activación magnetorcas
        Sleep(dt);
        //Apagar magnetorcas
        MagnetorquersOff(&serial);
        //Enviar comando
        SendCMD(&serial,'1');
    }
	close(serial);
}

void float2uchar(float* fdata, unsigned char* cdata) {
	//convierte float a 4 bytes en char
	for (int i = 0; i < 4;i++) {
		cdata[i] = *((unsigned char*)fdata + i);
	}
}

void uchar2float(unsigned char* cdata, float* fdata, int size) {
	for (int i = 0; i < size;i++) {
		fdata[i] = *((float*)(&cdata[i * 4]));
	}
}

void ReadReg(int* serial, unsigned char reg, unsigned char* data, int size) {
	// Allocate memory for read buffer, set size according to your needs
    char read_buf='\0';
    int n=0;

    memset(&read_buf, '\0', sizeof(read_buf));

	//WriteFile(*serial,&reg,1,&written,NULL);
    write(*serial, &reg, 1);
	//ReadFile(*serial, data, size, &read, NULL);
    for(int i=0;i<size;i++){
        n = read(*serial, &read_buf, 1);
        data[i]=(unsigned char)read_buf;
    }
}

void SetReg(int* serial, unsigned char reg, unsigned char* data, int size) {

	//WriteFile(*serial, &reg, 1, &written, NULL);
    write(*serial, &reg, 1);
	//WriteFile(*serial, data, 6, &written, NULL);
    write(*serial, data, 6);
}

void SendCMD(int* serial, unsigned char cmd){
    unsigned char Header[]="$CMD,1\r\n";
    write(*serial, Header, 6);
    //write(*serial, &cmd, 1);
}

double CA2_32(uint8_t msb, uint8_t lsb1, uint8_t lsb2) {
	double n = 0;
	if ((msb & 0x80) == 0x80) {
		uint8_t n1 = ~msb;
		uint8_t n2 = ~lsb1;
		uint8_t n3 = ~lsb2;
		n = (uint32_t)(n1 << 16 | n2 << 8 | n3) + 1;
		n = (-1) * n;
	}
	else
		n = msb << 16 | lsb1 << 8 | lsb2;
	return n;
}

double norm(float* v) {
	double n = 0;
	for (int i = 0;i < 3;i++)
		n += v[i] * v[i];
	n = sqrtf(n);
	return n;
}

void ReadMagnetometer(int* serial, float B[3], float B_1[3]) {
	
	//Registro de medicion en crudo magnetometro
	unsigned char reg = 0x15;
	unsigned char data[12]{};
	//Comuncación con ADCS, se reciben 9 bytes
	ReadReg(serial,reg, data, 12);
	uchar2float(data, B_1, 3);
	reg = 0x09;
	ReadReg(serial, reg, data, 12);
	uchar2float(data, B, 3);
}

void BDot(float B[3], float B_1[3], float V[3], float dt) {

	//Variables=====================================
	//Sub-optimal gain
	float kb = 8.6593E-06;
	//Magnetorquer NRounds*Area
	float AN = 0.39861;
	//Resistencia de magnetorca (Ohms)
	float r = 4.7;
	//vector de campo magnetico actual unitario
	float Bu[3]{}, Bu_1[3]{};
	//magnitud de vector de campo magnético
	float magb,magb_1;
	//Bdot
	float bdot[3]{};
	//Momento magnético
	float m[3];
	//Paso de tiempo
	//float dt = 0.02;
	//===============================================

	//===============================================
	//Cálculo de magnitudes de vector de campo magnético
	magb = norm(B);
	magb_1 = norm(B_1);
	//===============================================
	//Obtención de vector de campo magnético unitario
	for (int i = 0; i < 3; i++)
		Bu[i] = B[i] / magb;
	for (int i = 0; i < 3; i++)
		Bu_1[i] = B_1[i] / magb;
	//===============================================
	//bdot = (B unitario(actual)-B unitario(anterior))/dt
	for (int i = 0; i < 3; i++)
		bdot[i] = (B[i]/magb - B_1[i]/magb_1) / (dt/1000);
	//===============================================
	//m = -k* bdot /magb
	for (int i = 0; i < 3; i++)
		m[i] = (-1) * kb * bdot[i] / magb;
	//===============================================
	//Calculo de voltajes
	for (int i = 0; i < 3; i++) {
		V[i] = m[i] * r / AN;
		if (V[i] > 5)
			V[i] = 5;
		if (V[i] < -5)
			V[i] = -5;
	}
	//===============================================
	//Estimación de Torque entregado
	//Pcruz(m,bnuev,Torq);
	//===============================================
}

void SetMagnetorquers(int* serial, float V[3]) {
	//Registro de magnetorcas
	unsigned char reg = 0x01;
	unsigned char data[6]{};
	for (int i = 0; i < 3; i++) {
		if (V[i] < 0) {
			data[i * 2] = 0xC0;
			V[i] = -V[i];
		}
		else
			data[i * 2] = 0x80;
		data[i * 2] = data[i * 2] | (((int)(V[i] * 200))>>4);
		data[i * 2 + 1] = 0xF0 & ((int)(V[i] * 200)) << 4;
	}
	for (int i = 0; i < 3;i++)
		printf("%x%x ", data[i*2],data[i*2+1]);
	cout << endl;
	SetReg(serial, reg, data, 6);
}

void MagnetorquersOff(int* serial) {
	//Registro de magnetorcas
	unsigned char reg = 0x01;
	unsigned char data[6]{};
	SetReg(serial, reg, data, 6);
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
