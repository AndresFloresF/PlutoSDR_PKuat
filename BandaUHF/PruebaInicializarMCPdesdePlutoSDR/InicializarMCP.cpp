#include <stdio.h>
#include "hidapi.h"


int main(){
    int opcion,registro,valor,config,confVal;
	hid_device *handle;

    printf("intento de conexion al MCP \n");

	if(hid_init()){return -1;}

	handle = hid_open(0x4d8, 0xde, NULL);
	if (!handle){
        printf("No esta conectado el MCP\n");
        return 1;
	}
	hid_set_nonblocking(handle, 1);

	printf("Conexión con dispositivo exitosa...\n");
}