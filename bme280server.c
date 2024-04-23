//********************//
//    ___   ___  ____ //
//   / _ | / _ \/ __/ //
//  / __ |/ // / _/   //
// /_/ |_/____/___/   //
//********************//
//!
//! \file: udp-server.c
//! \brief: UDP server sample
//! \author: Jose Luis Unibaso

/********************** Include Files **************************/
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include "bme280.h"


/********************* Type Definitions ************************/
/******************* Constant Definitions **********************/
#define SERVER_PORT 5000
#define BUFFSIZE 1024
// delay between samples in microseconds
#define DELAY 1000000

/*********** Macros (Inline Functions) Definitions *************/
/******************* Variable Definitions **********************/
/******************** Function Prototypes **********************/

//
// Start of MAIN program
//

int main(void) {
    int sock, num;
    socklen_t len;
    char buffer[BUFFSIZE];
    char str[INET_ADDRSTRLEN];
    char respuesta[BUFFSIZE];
    struct sockaddr_in serveraddr, clientaddr;
    int i;
	int T, P, H; // calibrated values

    // Configure the GPIOs
    //configGPIO();

    // Create UDP socket:
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("Error while creating socket\n");
        return 1;
    }
    
    //Conectarse al sensor
    i = bme280Init(1, 0x76);
	if (i != 0)
	{
		return -1; // problem - quit
	}

    len = sizeof(clientaddr);

    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = INADDR_ANY;
    serveraddr.sin_port = htons(SERVER_PORT);

    if (bind(sock, (struct sockaddr *)&serveraddr, sizeof(serveraddr))) {
        printf("Couldn't bind socket\n");
        return 1;
    }
    printf("Socket Binding OK\n");
    printf("Listening for incoming messages...\n\n");

    // Start listening:
    while (1) {
        memset(buffer,'\0',sizeof(buffer));
        //Recibe mensaje
        num = recvfrom(sock, buffer, BUFFSIZE, MSG_WAITALL,
                       (struct sockaddr *)&clientaddr, &len);
        
        //Lee sensor
        bme280ReadValues(&T, &P, &H);
        
        //Analiza mensaje recibido
        if (strcmp(buffer,"temp")==0) {
            //Construye String con valor de temperatura calibrado
            sprintf(respuesta,"La temperatura es de %.2fºC\n",(float)T/100.0);
            //Manda valor temperatura
            sendto(sock, &respuesta, strlen(respuesta), MSG_CONFIRM,
               (struct sockaddr *)&clientaddr, len);
        }
        else if (strcmp(buffer,"hum")==0) {
            sprintf(respuesta,"La humedad relativa es del %.6f%\n",(float)H/1024.0);
            sendto(sock, &respuesta, strlen(respuesta), MSG_CONFIRM,
               (struct sockaddr *)&clientaddr, len);
        }
        else if (strcmp(buffer,"press")==0) {
            sprintf(respuesta,"La presión atmósferica es de %.6fhPa\n",(float)P/256.0);
            sendto(sock, &respuesta, strlen(respuesta), MSG_CONFIRM,
               (struct sockaddr *)&clientaddr, len);
        }
        else if (strcmp(buffer,"close")==0) {
            break;
        }

        inet_ntop(AF_INET, &(clientaddr.sin_addr), str, INET_ADDRSTRLEN);

        printf("New message from %s:%d -- %s\n", str, ntohs(clientaddr.sin_port), buffer);
                // Send echo back:
        sendto(sock, &buffer, strlen(buffer), MSG_CONFIRM,
               (struct sockaddr *)&clientaddr, len);
    }

    close(sock);
}

