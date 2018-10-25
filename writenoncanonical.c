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
#define DISC 0x0B
#define FILE_LENGTH 0x00
#define FILE_NAME 0x01
int received=0;
int llopen(int fd);
int create_start(int file_length,char *fileName,int fd);
int create_data(int fd);
int llwrite(int fd, char* set1,int size);
int llclose(int fd);
int create_end(int file_length,char *fileName,int fd);

volatile int STOP=FALSE;
void atende();
char set[5];
char trama=0x00;
char file[200];
int n = 1;
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


  llopen(fd);

	printf("Please enter the name of the file: ");
	scanf("%s",file);

	struct stat st;
	int file_size;
	stat(file, &st);
	file_size = st.st_size;

    create_start(file_size, file,fd);
	  create_data(fd);
    create_end(file_size,file,fd);
    llclose(fd);
    close(fd);

  return 0;
}
int create_start(int file_length,char *fileName,int fd){

  char pack_start[256];
  pack_start[0]=START;
  int i = 0;
  unsigned char v1[256];
  do {
        v1[0]=(char)(file_length/pow(256.0,(double)i)) % 256;
        i++;
  }while(v1[i]!=0);

  pack_start[1]= FILE_LENGTH; //t1
  pack_start[2]=i; //l1
  //printf("l1: %d \n",i);

  int j;
  for (j=3;j<i+3;j++){
    pack_start[j]=v1[j-3];
	//printf("v[i] = 0x%x \n", pack_start[j]);
  }
  i+=3;
  if (fileName != NULL){
    pack_start[i]=FILE_NAME; //t2
	 	 int str_length=strlen(fileName);

    i++;
    pack_start[i]=str_length; //l2
    i++;

    for (j = 0;j<str_length;j++){
      pack_start[i]=fileName[j]; //v2


      i++;
    }
  }

    int res = llwrite(fd,pack_start,i+1);
    trama=0x40;

	while(i>0){
	//printf("pack_start: 0x%x \n", pack_start[i]);
	i--;

	}
}


int create_data(int fd){
	char pack_data[260];
	int i =0;


  char dest[260];
  char filename[20];
  strcpy (filename,file);
  int open_file = open(filename,O_RDONLY);
  int res;


 while((res=read(open_file, dest,260))>0){

   //printf("read\n");
  pack_data[i]=DATA;
//  printf("buf1: %x\n",pack_data[0]);
  i++;
  pack_data[i] = (int) n;
  i++;
  pack_data[i] = (int) res / 256; //l2
  i++;
  pack_data[i] = (int) res % 256; //l1
  i++;

	int j=0;

	for(j;j<res;j++){
	  pack_data[i] = dest[j];
  //  printf("packdata: %x\n", pack_data[i]);
		i++;
	}


  int res = llwrite(fd,pack_data,i+1);

  n++;
  printf("N: %d\n",n );
  i=0;
  if ((n%2)==0)
    trama=0x00;
  else
    trama =0x40;

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
	conta_zero();
	while(conta<MAX && !received){
		desativa_alarme();

		res=write(fd,set,sizeof(set));
		//printf("res: %d \n", res);


		alarm(3);
		while(!flag && !received){

			res=read(fd,buf+i,1);
			//printf("res: %d \n", res);

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
        printf("Llopen successful!\n");
				}
			}
		}
}
}


int llwrite(int fd, char* set1,int size){

	unsigned char buf1[512];
  unsigned char buf2[512];
  unsigned char buf3[512];
	unsigned char bcc2;
  unsigned char confirmation;
  int res=1;
	int i,j;
  i=0;
  j=0;
flag=1;

 	for(; i<size;i++){
		buf1[i]=set1[i];


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
        if (i==0){
          bcc2=buf1[i];
        }else
			bcc2=bcc2^buf1[i];


	}
  int aux2=i;
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


  while(i<=j+4){

    buf3[i]=buf2[i-4];
    i++;
  }
  buf3[i]=FLAG;

  conta_zero();
  received=0;
  int aux = i+1;
  while(conta <= MAX && !received){

  desativa_alarme();
  //sleep(0.7);
  res=write(fd,buf3,aux);
  printf("numero %d\n", res);

  i=0;
  unsigned char buf[512];
  alarm(3);
	flag=0;
//  printf("flag %d\n",flag );
//  printf("received %d\n",received );
//  printf("conta2 %d\n",conta );
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


  //printf("conta3 %d\n",conta );
    if (i==5){
       printf("buf[2]== REJ0: %d\n",buf[2]== REJ0);
       printf("buf[2]== REJ1: %d\n",buf[2]== REJ1);
       printf("buf2 0x%x\n", buf[2]);
       printf("trama 0x%x\n", trama);

    if (buf[2] == confirmation && buf[3]==buf[1]^buf[2]) {
      received=1;
      //stop=1;
      //printf("Received confirmation - deactivating the alarm...\n");
      desativa_alarme();

      }


      else if(buf[2]== REJ0 && trama == 0x00){
       printf("Restransmiting - received REJ0\n");
       return llwrite(fd,set1,size);
      }else if(buf[2]== REJ1 && trama == 0x40){
       printf("Restransmiting - received REJ1\n");
      return llwrite(fd,set1,size);
      }
  }
}

}

}
printf("received final %d\n", received);
if(received == 0){
  printf("timeout!\n");
  exit(0);
}

return 0;
}

int llclose(int fd){
	int res;
	int i=0;
	char buf[5];
	conta_zero();
  set[2]=DISC;
  received = 0;
	while(conta<MAX && !received){
		desativa_alarme();

		res=write(fd,set,sizeof(set));



		alarm(3);
		while(!flag && !received){

			res=read(fd,buf+i,1);
			//printf("res: %d \n", res);

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
			if (buf[2]==DISC && buf[3]==buf[1]^buf[2]) {
				received=1;
        set[2]=0x07;
        res=write(fd,set,sizeof(set));
				desativa_alarme();

				}
			}
		}


printf("Llclose successful!\n");
}
}
