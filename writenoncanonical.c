/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "alarme.c"
#include <signal.h>
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX 3
#define FLAG 0x7E
int received=0;
int llopen(int fd);

volatile int STOP=FALSE;
void atende();
char set[5];

int main(int argc, char** argv)
{
    int fd,c, res;
    struct termios oldtio,newtio;
	(void) signal(SIGALRM,atende);
	set[0]= FLAG;
	set[1]= 0x03; //A
	set[2]= 0x03;//C, define a trama que se esta a usar, 0x03 - transmicao emissor -> recetor
	set[3]= set[1]^set[2];
	set[4]=FLAG;

    char buf[255];
    int i, sum = 0, speed = 0;

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
    leitura do(s) pr�ximo(s) caracter(es)
  */



    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    printf("New termios structure set\n");



/*    printf ("Insert a sentence\n");
    gets(&buf);

    res = write(fd,buf,sizeof(buf));
    printf("%d bytes written\n", res);


  /*
    O ciclo FOR e as instru��es seguintes devem ser alterados de modo a respeitar
    o indicado no gui�o
  */
/*
  i =0;
    while(STOP == FALSE){
        res = read(fd,buf+i,1);

        if (buf[i]=='\0'){
          STOP=TRUE;
        }
      else  i++;
    }
      printf(":%s\n", buf);


    if ( tcsetattr(fd,TCSANOW,&oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }
*/
char test[4];
  test[0]=0x01;
  test[1]=0x10;
  test[2]=0x11;
  test[3]=0x7E;
	 llopen(fd);
   //llwrite(fd,test,0x40);

    close(fd);

  return 0;
}

int llopen(int fd){
	int res;
	int i=0;
	char buf[5];

	while(conta<MAX && !received){
		desativa_alarme();

		res=write(fd,set,sizeof(set));

		alarm(3);
		conta_zero();
		while(!flag && !received){

			res=read(fd,buf+i,1);

		if (res!=0)
			if(i<5 && res!=0) {
			switch(buf[i]){
				case 0x7E:
				if (i==0 || i==4)
					i++;
				break;
				default:
				if (i!=0 && i!=4)
					i++;

			}
}
			if (i==5){
			if (buf[2]==0x07 && buf[3]==buf[1]^buf[2]) {
				received=1;
				desativa_alarme();

				}
			}
		}
}
}

int llwrite(int set,int fd,int trama){

	char buf1[255];
  char buf2[255];
  char buf3[255];
	char bcc2;
  int res;
	int i,j;
  i=0;
  j=0;
  while(res!=0){

			res=read(set,buf1+i,1);
      if (buf1[i]==0x7E){
        buf2[j]=0x7D;
        buf2[j+1]=0x5E;
        j++;

      }
      else if(buf1[i]==0x7D){
        buf2[j]=0x7D;
        buf2[j+1]=0x5D;
        j++;
      }
      else
        {
            buf2[j]=buf1[i];
        }
        j++;
			bcc2=bcc2^buf1[i];
			i++;
	}
  buf2[j]=bcc2;
  buf3[0]=FLAG;
  buf3[1]=0x03;
  buf3[2]=trama;
  buf3[3]=buf3[1]^buf3[2];
  i=4;
  while(i<j+4){
    buf3[i]=buf2[j-4];
    i++;
  }
  buf3[i]=FLAG;
  i=0;
  res=write(fd,buf3,sizeof(buf3));
  char buf[255];
  alarm(3);
  conta_zero();
  while(!flag && !received){

    res=read(fd,buf+i,1);

    if(res!=0) {
    switch(buf[i]){
      case 0x7E:
      if (i==0 || i==4)
        i++;
      break;
      default:
      if (i!=0 && i!=4)
        i++;

}
    if (i==5){
    if (buf[2]==0x07 && buf[3]==buf[1]^buf[2]) {
      received=1;
      desativa_alarme();

      }
    }
  }


}




}
