#include <unistd.h>
#include <signal.h>
#include <stdio.h>

int flag=1, conta=1;

void atende()                   // atende alarme
{
	printf("alarme # %d\n", conta+1);
	flag=1;
	conta++;
}


void desativa_alarme(){
	flag=0;
	alarm(0);

}
void conta_zero(){
conta=0;
}
