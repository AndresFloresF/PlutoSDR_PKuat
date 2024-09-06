#include <string.h>
#include <fcntl.h> // Contains file controls like O_RDWR
#include <errno.h> // Error integer and strerror() function
#include <termios.h> // Contains POSIX terminal control definitions
#include <unistd.h>
#include <iostream>
using namespace std;

int main(){

int serial_port = open("/dev/ttyUSB0", O_RDWR);

if (serial_port < 0) {
    printf("Error %i from open: %s\n", errno, strerror(errno));
}

struct termios tty;
if(tcgetattr(serial_port, &tty) != 0) {
    printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
}

struct termios {
	tcflag_t c_iflag;		/* input mode flags */
	tcflag_t c_oflag;		/* output mode flags */
	tcflag_t c_cflag;		/* control mode flags */
	tcflag_t c_lflag;		/* local mode flags */
	cc_t c_line;			/* line discipline */
	cc_t c_cc[NCCS];		/* control characters */
};

tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
tty.c_cflag |= CS8;
tty.c_cflag &= ~CRTSCTS;
tty.c_cflag |= CREAD | CLOCAL;
tty.c_lflag &= ~ICANON;
tty.c_lflag &= ~ECHO; // Disable echo
tty.c_lflag &= ~ECHOE; // Disable erasure
tty.c_lflag &= ~ECHONL;
tty.c_lflag &= ~ISIG;
tty.c_iflag &= ~(IXON | IXOFF | IXANY);
tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL);
tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
tty.c_oflag &= ~ONLCR;

cfsetispeed(&tty, B9600);
cfsetospeed(&tty, B9600);
unsigned char cmd[1]={0x09};
write(serial_port,cmd,sizeof(cmd));
char read_buf[256];
int n=read(serial_port,&read_buf,sizeof(read_buf));
close(serial_port);

printf("%s",read_buf);
return 0;

}
