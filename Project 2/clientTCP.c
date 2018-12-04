/*      (C)2000 FEUP  */

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

#define SERVER_PORT 6000
#define SERVER_ADDR "192.168.28.96"
#define MAX_SIZE 256


struct hostent *getip( char host[]);
int parseURL(char url[], char *user, char *password, char *host, char *url_path);
int sendCommand(int fd_socket, char *command);
int getCommand(char *command, int index, char *user, char *password, char *host, char *url_path);

int main(int argc, char** argv){

struct hostent *h;

char user[MAX_SIZE];
char password[MAX_SIZE];
char host[MAX_SIZE];
char url_path[MAX_SIZE];
char command[MAX_SIZE];

parseURL(argv[1], user, password, host, url_path);

getCommand(command, 0, user, password, host, url_path);
getCommand(command, 1, user, password, host, url_path);
getCommand(command, 2, user, password, host, url_path);
getCommand(command, 3, user, password, host, url_path);
getCommand(command, 4, user, password, host, url_path);
getCommand(command, 5, user, password, host, url_path);


printf("User: %s\n",user);
printf("Password: %s\n",password);
printf("Host: %s\n",host);
printf("Url path: %s\n",url_path);
//
//   	int	sockfd;
//   	struct	sockaddr_in server_addr;
//   	char	buf[] = "Mensagem de teste na travessia da pilha TCP/IP\n";
//   	int	bytes;
//
//   	/*server address handling*/
//   	bzero((char*)&server_addr,sizeof(server_addr));
//   	server_addr.sin_family = AF_INET;
//   	server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);	/*32 bit Internet address network byte ordered*/
//   	server_addr.sin_port = htons(SERVER_PORT);		/*server TCP port must be network byte ordered */
//
//   	/*open an TCP socket*/
//   	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
//       		perror("socket()");
//           	exit(0);
//       	}
//   	/*connect to the server*/
//       	if(connect(sockfd,
//   	           (struct sockaddr *)&server_addr,
//   		   sizeof(server_addr)) < 0){
//           	perror("connect()");
//   		exit(0);
//   	}
//
//   	/*send a string to the server*/
//   	bytes = write(sockfd, buf, strlen(buf));
//   	printf("Bytes escritos %d\n", bytes);
//
//
// 		//h = getip(host);
//
//
//   	close(sockfd);
  	exit(0);
 }


   struct hostent *getip( char host[]){
          struct hostent *h;

         if ((h=gethostbyname(host)) == NULL) {
             herror("gethostbyname");
             exit(1);
        }

        printf("Host name  : %s\n", h->h_name);
        printf("IP Address : %s\n",inet_ntoa(*((struct in_addr *)h->h_addr)));

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

int getCommand(char *command, int index, char *user, char *password, char *host, char *url_path){

  switch (index) {
    case 0:
      strcpy(command, "telnet ");
      strcat(command, url_path);
      strcat(command, " 110");
      break;
    case 1:
      strcpy(command, "user ");
      strcat(command, user);
      break;
    case 2:
      strcpy(command, "pass ");
      strcat(command, password);
      break;
    case 3:
      strcpy(command, "stat");
      break;
    case 4:
      strcpy(command, "retr ");
      //strcat(command, )
      break;
    case 5:
      strcpy(command, "quit");
      break;
    default:
    break;

  }

  printf("command: %s\n", command);


return 0;

}

int sendCommand(int fd_socket, char *command){

	write(fd_socket, command, sizeof(command));



}
