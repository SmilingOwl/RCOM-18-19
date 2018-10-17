/*Non-Canonical Input Processing*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <math.h>
#include "alarme.c"
#define BAUDRATE B38400
#define MODEMDEVICE "/dev/ttyS1"
#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1
#define MAX 3
#define DATA 0x01
#define FLAG 0x7E
#define RR0 0x05
#define RR1 0x85
#define REJ0 0x01
#define REJ1 0x81
#define START 0x02
#define END 0x03
#define FILE_LENGTH 0x00
#define FILE_NAME 0x01
int received=0;
int llopen(int fd);
int create_start(int file_length,char *fileName,int fd);
int create_data(int fd);
int llwrite(int fd, char* set,int size);


volatile int STOP=FALSE;
void atende();
char set[5];
char trama=0x00;

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

char buffer[4];
  buffer[0]=0x01;
  buffer[1]=0x10;
  buffer[2]=0x11;
  buffer[3]=0x7E;
  printf("llopen\n");

/*	int file = open ("test.txt",O_RDONLY);
  int file_length = lseek(file,0, SEEK_END);
  close(file);
  lseek(file,0,SEEK_SET);
*/
   llopen(fd);
    create_start(2, "test.txt",fd);
   //llwrite(fd,buffer,sizeof(buffer));
    printf("before create_data\n");
    create_data(fd);
    create_end(2,"test.txt",fd);
    //close(fd);

  return 0;
}
int create_start(int file_length,char *fileName,int fd){

  char pack_start[256];
  pack_start[0]=START;
  int i = 0;
  char v1[256];
  do {
        v1[0]=(int)(file_length/pow(256.0,(double)i)) % 256;
        i++;
  }while(v1[i]!=0);

  pack_start[1]= FILE_LENGTH;
  pack_start[2]=i;

  int j;
  for (j=3;j<i;j++){
    pack_start[j]=v1[j-3];
  }
  i+=4;
  if (fileName != NULL){
    pack_start[i]=FILE_NAME;
    int str_length=strlen(fileName);
    i++;
    pack_start[i]=str_length;
    i++;
    for (j = 0;j<str_length;j++){
      pack_start[i]=fileName[j];
      i++;
    }
  }
      
    int res = llwrite(fd,pack_start,i+1);
    trama=0x01;

}


int create_data(int fd){
	char pack_data[260];
	int i =0;
 
 
  char dest[260];
	int n = 0;
  char filename[20];
  strcpy (filename,"test.txt");
  int open_file = open(filename,O_RDONLY);
  printf("AUX::%d\n",aux);
  while(read(open_file, dest,260)>0){
   printf("read\n");
  pack_data[i]=DATA;
  i++;
  pack_data[i] = (int) n;
  i++;
  pack_data[i] = (int) 1;
  i++;
  pack_data[i] = (int) 4;
  i++;  
  pack_data[i] = dest;
  i++;
  int res = llwrite(fd,pack_data,i+1); 
  n++;
  i=0;
  if ((n%2)==0)
    trama=0x40;
  else
    trama =0x00;
  aux =read(open_file, dest,260);
  }
}
int create_end(int file_length,char *fileName,int fd){
  char pack_end[256];
  pack_end[0]=END;
  int i = 0;
  char v1[256];
  do {
        v1[0]=(int)(file_length/pow(256.0,(double)i)) % 256;
        i++;
  }while(v1[i]!=0);

  pack_end[1]= FILE_LENGTH;
  pack_end[2]=i;

  int j;
  for (j=3;j<i;j++){
    pack_end[j]=v1[j-3];
  }
  i+=4;
  if (fileName != NULL){
    pack_end[i]=FILE_NAME;
    int str_length=strlen(fileName);
    i++;
    pack_end[i]=str_length;
    i++;
    for (j = 0;j<str_length;j++){
      pack_end[i]=fileName[j];
      i++;
    }
  }

    int res = llwrite(fd,pack_end,i+1);

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

int llwrite(int fd, char* set,int size){
    printf("SET: 0x%x\n", set);

	char buf1[255];
  char buf2[255];
  char buf3[255];
	char bcc2;
char confirmation;
  int res=1;
	int i,j;
  i=0;
  j=0;
flag=1;

 	for(; i<size;i++){
		buf1[i]=set[i];
      printf("buf1:0x%x\n",buf1[i]);
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

	}
  buf2[j]=bcc2;
  buf3[0]=FLAG;
  buf3[1]=0x03;
  buf3[2]=trama;
  buf3[3]=buf3[1]^buf3[2];
  i=4;
if (trama ==0x00){
		confirmation=RR1;

}
else
	confirmation=RR0;
  while(i<j+4){

    buf3[i]=buf2[i-4];
    i++;
  }

  buf3[i]=FLAG;

  res=write(fd,buf3,i+1);
  i=0;
  char buf[255];
  alarm(3);
  conta_zero();
	flag=0;
	received=0;

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
    if (buf[2] == confirmation && buf[3]==buf[1]^buf[2]) {
      received=1;
      desativa_alarme();

      }
    }
  }


}




}
