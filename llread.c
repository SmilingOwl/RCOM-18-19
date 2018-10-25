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

int id_trama=0;

volatile int STOP=FALSE;

int main(int argc, char** argv)
{
    int fd,c, res, i=0;
    struct termios oldtio,newtio;
    unsigned char buf[255];

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

	if(llopen(fd)==-1) { return -1; }
	
	save_data(fd);
	if(llclose(fd)==-1) { return -1; }

    tcsetattr(fd,TCSANOW,&oldtio);
    close(fd);
    return 0;
}

int llopen(int fd){

	unsigned char SET[5], buf;
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
		return -1;
	//estrutura a enviar UA
	unsigned char UA[5];
	UA[0] = 0x7E;
	UA[1] = 0x03;
	UA[2] = 0x07;
	UA[3] = 0x04;
	UA[4] = 0x7E;
  	res = write(fd, UA, sizeof(UA));
 	return 0;
}

int llread(int fd, char* buf2){
	
    unsigned char buf;
    unsigned char trama[512];
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
				if(buf != 0x7E){ i++; trama[i] = buf;}
				else{ i++; trama[i] = buf; STOP = TRUE;}
				break;
			}
    	}
  	}
printf("N: %d\n", (int) trama[5]);
	

	unsigned char message[5];
	message[0] = 0x7E;
	message[1] = 0x03;
	message[4] = 0x7E;

	if(trama[3]!= trama[1]^trama[2] && trama[2]!=0x00 && trama[2]!=0x40){

		if(trama[2]==0x00 && id_trama == 0){
			printf("REJ0\n");
			message[2] = REJ0;
			message[3] == message[1]^message[2];
			write(fd, message, 5);
			return -1;
		}
		if(trama[2]==0x00 && id_trama != 0){
			printf("RR1\n");
			message[2] = RR1;
			message[3] == message[1]^message[2];
			write(fd, message, 5);
			return -1;
		}
		if(trama[2]==0x40 && id_trama == 1){
			printf("REJ1\n");
			message[2] = REJ1;
			message[3] == message[1]^message[2];
			write(fd, message, 5);
			return -1;
		}
		if(trama[2]==0x40 && id_trama != 1){
			printf("RR0\n");
			message[2] = RR0;
			message[3] == message[1]^message[2];
			write(fd, message, 5);
			return -1;
		}
		return -1;
	}

	if(trama[2]==0x00 && id_trama != 0){
		printf("RR1 duplicate\n");
		message[2] = RR1;
		message[3] == message[1]^message[2];
		write(fd, message, 5);
		return -1;
	}
	if(trama[2]==0x40 && id_trama != 1){
		printf("RR0 duplicate\n");
		message[2] = RR0;
		message[3] == message[1]^message[2];
		write(fd, message, 5);
		return -1;
	}
	int n =0;
	unsigned char bcc2;
	int j = 4;
	while(j < i-1)
	{
    	if(trama[j] == 0x7D && trama[j+1] == 0x5E)	/*ciclo para apgar 0x5E e colocar 0x7D=0x7E */
    	{
			buf2[n] = 0x7E;
			j++;
    	}
    	else if(trama[j] == 0x7D && trama[j+1] == 0x5D) /*ciclo para apgar 0x5D*/
    	{
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
	
	if(bcc2 != trama[i-1] &&  id_trama == 0)
	{
			printf("REJ0 entered\n");
			message[2] = REJ0;
			message[3] == message[1]^message[2];
			write(fd, message, 5);
			return -1;
	}
	else if(bcc2 != trama[i-1] && id_trama == 1)
	{
			printf("REJ1 entered\n");
			message[2] = 0x81;
			message[3] == message[1]^message[2];
			write(fd, message, 5);
			return -1;
	}
	

	if(trama[2]==0x00 && id_trama == 0){
		printf("RR1\n");
		message[2] = RR1;
		message[3] == message[1]^message[2];
		write(fd, message, 5);
		return n-1;
	}
	if(trama[2]==0x40 && id_trama == 1){
		printf("RR0\n");
		message[2] = RR0;
		message[3] == message[1]^message[2];
		write(fd, message, 5);
		return n-1;
	}

}

int save_data(int fd){
	unsigned char buffer[512];
	int stop2 = 0, size, fd1;
	while(!stop2)
	{
		size = llread(fd, buffer);
	
		if(size <= 0) continue;

		if(id_trama == 0)
			id_trama=1;

		else if (id_trama == 1)
			id_trama = 0;

		if(buffer[0] == 0x02)
			fd1=analyze_start(buffer, size);
			
		else if(buffer[0]==0x01)
			analyze_data(buffer, size, fd1);
		
		else if(buffer[0]==0x03)
		{
			close(fd1);
			stop2=1;
		}
	}
	
	return 0;
}

int analyze_start(char* buffer, int size) {
	int l1, l2, i=1;
	if(buffer[1] == 0x00)
	{
		l1 =  (int) buffer[2];		
	
		if(buffer[3+l1]==0x01)
		{
			l2 = (int) buffer[4+l1]; //l2 = tamanho do nome do ficheiro

		}
	}
	unsigned char nome[l2+1];
	while(i <= l2)
	{
		nome[i-1]=buffer[4+l1+i];
		i++;
	}
	nome[l2] = '\0';



	int fd= open(nome, O_CREAT | O_WRONLY |O_APPEND);
	return fd;
}

int analyze_data(char * buffer, int size, int fd){

	int l1=(int) buffer[2];
	int l2=(int) buffer[3];

	int k = l1*256+l2; 
	unsigned char toWrite[k];
	int i=0;

	while(i < k)
	{
		toWrite[i] = buffer[i+4];
	
	i++;
	}
	write(fd, toWrite, k);
	return 0;
}

int llclose(int fd){
	char DISC[5], buf;
	DISC[0]  = 0x7E;
	int i=0, res;
	int state = 0; 
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
				if(buf != 0x7E) {state++; i++; DISC[i] = buf;}
				break;
			case 2:
				if(buf != 0x7E && i >= 4) STOP = TRUE;
				else if(buf != 0x7E){ i++; DISC[i] = buf;}
				else{ i++; DISC[i] = buf; STOP = TRUE;}
				break;
			}
     	}
  	}


	if(DISC[3]!= DISC[1]^DISC[2] && DISC[2]!=0x0B)
		return -1;
   	res = write(fd, DISC, 5);


	unsigned char UA[5];

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
				if(buf != 0x7E) {state++; i++; UA[i] = buf;}
				break;
			case 2:
				if(buf != 0x7E && i >= 4) STOP = TRUE;
				else if(buf != 0x7E){ i++; UA[i] = buf;}
				else{ i++; UA[i] = buf; STOP = TRUE;}
				break;
			}
    	}
  	}

	if(UA[3]!= UA[1]^UA[2] && UA[2]!=0x0B)
		return -1;
	return 0;
}
