#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <strings.h>
#include <ctype.h>
#include <fcntl.h>

#define SERVER_PORT 21
#define SERVER_ADDR "192.168.28.96"
#define MAX_SIZE 256

struct hostent *getip( char host[]);
int parseURL(char url[], char *user, char *password, char *host, char *url_path);
int sendCommand(int fd_socket, char *command);
int getCommand(char *command, int index, char *user, char *password, char *host, char *url_path);
int connection(int fd_socket, char *user, char *password, char *host, char *url_path);
int readAnswer(int fd_socket, unsigned char *answer);
void createNewFile(int fd_socket, char *file_name);
void parseFilename(char *path, char *filename);

char *file_name;

int main(int argc, char** argv){

	struct hostent *h;
	int sockfd;
  	struct sockaddr_in server_addr;

	char user[MAX_SIZE];
	char password[MAX_SIZE];
	char host[MAX_SIZE];
	char url_path[MAX_SIZE];
	char command[MAX_SIZE];
	char filename[MAX_SIZE];

	parseURL(argv[1], user, password, host, url_path);
	parseFilename(argv[1],filename);
	file_name = filename;

	h = getip(host);

	printf("User: %s\n",user);
	printf("Password: %s\n",password);
	printf("Host: %s\n",host);
	printf("Url path: %s\n",url_path);
	printf("Filename:%s\n",file_name);
	

/*server address handling*/
	bzero((char *)&server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr))); /*32 bit Internet address network byte ordered*/
	server_addr.sin_port = htons(SERVER_PORT);											/*server TCP port must be network byte ordered */

	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket()");
		exit(0);
	}
	/*connect to the server*/
	if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		perror("connect()");
		exit(0);
	}

  	/*send a string to the server*/
    sendCommand(sockfd, command);
 


    connection(sockfd, user, password, host, url_path);




  	close(sockfd);
  	exit(0);
 }


struct hostent *getip( char host[]){
    struct hostent *h;

    if ((h=gethostbyname(host)) == NULL) {
        herror("gethostbyname");
        exit(1);}

    return h;
}

//ftp:<user>:<password>@<host>/<url-path>.
int parseURL(char url[], char *user, char *password, char *host, char *url_path){

	int i=6, j=0;

	if(!(url[0]=='f' && url[1]=='t' && url[2]=='p' && url[3]==':' && url[4]=='/' && url[5]=='/')){
		perror("ERROR: url");
		return -1;
	}

	while(url[i]!=':'){
		user[j]=url[i];
		i++;
		j++;
	}

	user[j] = '\0';
	i++;
	j=0;
	while(url[i]!='@'){
		password[j]=url[i];
		i++;
		j++;
	}

	password[j] = '\0';
	i++;
	j=0;
	while(url[i]!='/'){
		host[j]=url[i];
		i++;
		j++;
	}
	host[j] = '\0';
	i++;
	j=0;
	
	while(url[i]!='\0'){
		url_path[j]=url[i];
		i++;
		j++;
	}

	url_path[j] = '\0';

	return 0;
}

void parseFilename(char *path, char *filename){
	int i=0;
	int j=0;
	int boolean=0;

while(path[i]!='\0'){
	if(path[i]=='-'){
		boolean=1;
		i++;
		continue;
	}
	if(boolean){
		filename[j]=path[i];
		j++;
	}
	i++;
}
}

int getCommand(char *command, int index, char *user, char *password, char *host, char *url_path){

  switch (index) {
    case 0:
      strcpy(command, "user ");
      strcat(command, user);
      break;
    case 1:
      strcpy(command, "pass ");
      strcat(command, password);
      break;
    case 2:
      strcpy(command, "pasv");
      break;
    case 3:
      strcpy(command, "retr");
      break;
    default:
      break;
  }

  printf("command: %s\n", command);
return 0;}

int sendCommand(int fd_socket, char *command){
  char answer[512];

  write(fd_socket, command, sizeof(command));
  readAnswer(fd_socket, answer);

printf("%s\n", answer);

  if(answer[0] == '1'){
	printf("entered 1");
   
    }else if(answer[0] == '2'){
	if(strcmp(command,"pasv") == 0){
	readSocketfd(fd_socket);
      		createNewFile(fd_socket, file_name);
    	}//}else  //readAnswer(fd_socket, answer);
printf("entered 2");
      return 0;
    }else if(answer[0] == '3'){
printf("entered 3");
      return 1;
    }else if(answer[0] == '4'){
printf("entered 4");
      sendCommand(fd_socket, command);
    }else if(answer[0] == '5'){
        close(fd_socket);
        exit(-1);
    }

  return 0;
}

int connection(int fd_socket, char *user, char *password, char *host, char *url_path){
  unsigned int i = 0;

  while(i <= 3){


    char command[512];

    getCommand(command, i, user, password, host, url_path);
    sendCommand(fd_socket, command);

    i++;
  }
  return 0;
}

int readAnswer(int fd_socket, unsigned char *answer){

    unsigned char response[3];
    unsigned char aux;
    unsigned int i = 0;


    while(read(fd_socket, &aux, 1) != 0){

      if(isdigit(aux)){

        response[i] = aux;
        i++;

      }else if(aux == '\n'){

          break;

      }else if(i == 3 && aux =='-'){
          i = 0;
      }

      //printf("%c\n", aux);
    }


    strcpy(answer, response);
    return 0;
  }

void createNewFile(int fd, char* name){

    fd = open(name,O_CREAT|O_APPEND|O_WRONLY);
    if(fd == -1){
      printf("Error creating a new file...\n");
      exit(-1);
    }
printf("open\n");
}

int readSocketfd(int sockfd) {
	unsigned char response[3];
    unsigned char aux2[4], aux;
    unsigned int i = 0, state = 0, first_byte, second_byte;


    while(read(sockfd, &aux, 1) != 0){

      if(isdigit(aux)){
	if(state > 0) {
		aux2[i] = aux;
	}

        response[i] = aux;
        i++;

      }else if(aux == '\n'){
          break;

      }else if(i == 3 && aux ==' '){
          i = 0;
      }else if(aux ==','){
          i = 0;
	first_byte = atoi(aux2);
	int j=0;	
	while(j < 4){
		aux2[j] = '\0';
		j++;
	}
      }else if(aux ==')'){
	second_byte = atoi(aux2);
      }
	
    }
printf("first_byte: %d\n", first_byte);
	printf("second_byte: %d\n", second_byte);
	return first_byte*256 + second_byte;

}

void savingFile(int fd){
	
}
