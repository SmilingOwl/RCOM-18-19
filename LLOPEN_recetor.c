/*LLOPEN Emissor Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define BAUDRATE B38400
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, i=0;
    struct termios oldtio,newtio;
    char buf[255];

    if ( (argc < 2) || 
  	     ((strcmp("/dev/ttyS0", argv[1])!=0) && 
  	      (strcmp("/dev/ttyS1", argv[1])!=0) )) {
      printf("Usage:\tnserial SerialPort\n\tex: nserial /dev/ttyS1\n");
      exit(1);
    }


  /*
    Open serial port device for reading and writing and not as controlling tty
    because we don't want to get killed if linenoise sends CTRL-C.
  */
  
    
    fd = open(argv[1], O_RDWR | O_NOCTTY );
    if (fd <0) {perror(argv[1]); exit(-1); }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
      perror("tcgetattr");
      exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 1;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 0;   /* blocking read until 5 chars received */



  /* 
    VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) próximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

	if(LLOPEN(fd)==-1){
	perror("Wrong set received");
	exit(-1);}




    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

int LLOPEN(int fd){

char SET[5], buf;
SET[0]  = 0x7E;
int i=0, res;
int state = 0; //fora


   while(!STOP)
   {
     res = read(fd, &buf,1);
     
     if(res!=0)
     {
	switch(state)
	{
	case 0:
		if(buf == 0x7E) state++;
		break;
	case 1:
		if(buf != 0x7E) {state++; i++; SET[i] = buf;}
		break;
	case 2:
		if(buf != 0x7E && i >= 4) STOP = TRUE;
		else if(buf != 0x7E){ i++; SET[i] = buf;}
		else{ i++; SET[i] = buf; STOP = TRUE;}
		break;
	}
     }
  }

//analise
if(SET[3]!= SET[1]^SET[2] && SET[2]!=0x03)
	LLOPEN(fd);
//estrutura a enviar UA
char UA[5];
UA[0] = 0x7E;
UA[1] = 0x03;
UA[2] = 0x07;
UA[3] = 0x04;
UA[4] = 0x7E;
   res = write(fd, UA, sizeof(UA));
 return 0;
}

int llread(){
char buf;
char trama[512];
trama[0] = 0x7E;
int i=0, res;
int state = 0; //fora


   while(!STOP)
   {
     res = read(fd, &buf,1);
     
     if(res!=0)
     {
	switch(state)
	{
	case 0:
		if(buf == 0x7E) state++;
		break;
	case 1:
		if(buf != 0x7E) {state++; i++; trama[i] = buf;}
		break;
	case 2:
		if(buf != 0x7E) STOP = TRUE;
		else if(buf != 0x7E){ i++; trama[i] = buf;}
		else{ i++; trama[i] = buf; STOP = TRUE;}
		break;
	}
     }
  }

if(trama[3]!= trama[1]^trama[2] && trama[2]!=0x00 && trama[2]!=0x40)
	llread(fd);

for(int j=4; j < i; j++)
{
    if(trama[j] == 0x7D && trama[j+1] == 0x5E)
    {/*ciclo para apgar 0x5E e colocar 0x7D=0x7E (i--)*/}
    else if(trama[j] == 0x7D && trama[j+1] == 0x5D)
    {/*ciclo para apgar 0x5D (i--)*/}

	
}
/*if(trama[4]==0x02)
 //START
else if(trama[4]==0x01)
 //F
else if(trama[4]==0x03)
//END*/
}

