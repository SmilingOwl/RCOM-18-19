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
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81

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

	llread(0,fd);




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

int llread(int id_trama, int fd){
    char buf;
    char trama[512];
    trama[0] = 0x7E;
    int i=0, res;
    int state = 0; //fora
    STOP = FALSE;

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
	int a = 0;
  while(a <= i-1)
  {
  	printf("i=%d, trama: 0x%x\n", a, trama[a]);
	a++;
  }

char message[5];
message[0] = 0x7E;
message[1] = 0x03;
message[4] = 0x7E;

if(trama[3]!= trama[1]^trama[2] && trama[2]!=0x00 && trama[2]!=0x40){
	printf("error\n");
	return -1;
}
if(trama[2]==0x00 && id_trama != 0){
	printf("REJ0\n");
	message[2] = REJ0;
	message[3] == message[1]^message[2];
	write(fd, message, 5);
	return -1;
}
if(trama[2]==0x40 && id_trama != 1){
	printf("REJ1\n");
	message[2] = REJ1;
	message[3] == message[1]^message[2];
	write(fd, message, 5);
	return -1;
}
char buf2[512];
int n =0;
char bcc2;
int j = 4;
while(j < i-1)
{
    if(trama[j] == 0x7D && trama[j+1] == 0x5E)
    {/*ciclo para apgar 0x5E e colocar 0x7D=0x7E (i--)*/
	buf2[n] = 0x7E;
	j++;
    }
    else if(trama[j] == 0x7D && trama[j+1] == 0x5D)
    {/*ciclo para apgar 0x5D (i--)*/
	buf2[n] = 0x7D;
	j++;
    }
    else {
	buf2[n] = trama[j];
    }
    if(n==0) 
	bcc2=buf2[n];
    else 
	bcc2=bcc2^buf2[n];
    n++;
    j++;	
}

  a=0;
  while(a <= n-1)
    {
  	printf("i=%d, buf2: 0x%x\n", a, buf2[a]);
	a++;
    }

if(trama[2]==0x00 && id_trama == 0){
	printf("RR1\n");
	message[2] = RR1;
	message[3] == message[1]^message[2];
	write(fd, message, 5);
	return 0;
}
if(trama[2]==0x40 && id_trama == 1){
	printf("RR0\n");
	message[2] = RR0;
	message[3] == message[1]^message[2];
	write(fd, message, 5);
	return 0;
}

}

