//Michal Wojtowicz 

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <errno.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <ctype.h>

#define SIZE 128



struct msgbuf	//obsluga komunikatow
{
    long mtype;
    char mtext[SIZE];
    int rozkaz; //1 - stop pobierania, 2 - stop przetwarzania, 3 - stop wyprowadzania, 4 - start pobierania, 5 - start przetwarzania, 6- start wyprowadzania
    			//7 - usun pobieranie 8 - usun przetwarzanie, 9 -usun wyswietlanie
};

int shmid;

int PID_A, PID_B, PID_C;
int *tab;

short ok = 1;
short zawies_czytelnik = 0;
short zawies_sprawdz = 0;
short zawies_pisarz = 0;

int msqid;
int msgflg;
key_t key;

char bufor[SIZE];

int fd_cp[2]; //z czytania do sprawdzenia
int fd_pp[2]; //ze sprawdzenia do wypisania


void pobierz()	//pobieranie danych od uzytkownika
{
    int tmp;
    close(fd_cp[0]);
    
   
    while (1)
    {
	    	tmp = read(STDIN_FILENO, bufor, SIZE);
		if (zawies_czytelnik==0)
		{

	    	if(tmp > 1  )
	    	{
				write(fd_cp[1], bufor, tmp);	//przekazywanie pobranego tekstu do drugiego procesu
	    	}

		}

    }
    
}

void sprawdz()	//sprawdzenie poprawnosci wprowadzonych danych
{
    int tmp, i;
    int n;
    int plus = 0;
    close(fd_cp[1]);
    close(fd_pp[0]);
    while (ok)
    {
		if((tmp = read(fd_cp[0], bufor, SIZE)) > 0 && zawies_sprawdz==0)	//odebranie danych z procesu 1
		{
	   		bufor[tmp-1] = 0;
	   		n = strlen(bufor);
	   		if(bufor[0]!='+' && bufor[n-1]!='+')		
	   		{					// nie zaczyna sie i nie konczy '+'
				for(i=0;i<n; i++)
				{
		
		   			if(!isdigit(bufor[i]))   		// nie jest cyfra
		   			{
		     			if(bufor[i]!='+')
		     			{
		     				printf("\nNiepoprawne dane.\n");
		     				break; 	// nie jest takze plusem koniec
		     			}
		      			else if(++plus>1)
		      			{
		      				printf("\nNiepoprawne dane.\n");
		      				break;	// za duzo plusow raz po raz ++
		      			}
		   			}
		   			else plus = 0;	// licznik plusow od nowa
				}
				if(i==n) 
				{
		   			write(fd_pp[1], bufor, tmp);	// poprawnie, odeslano dalej do procesu 3
				}
	   		}
	   		else
	   			printf("\nNiepoprawne dane.\n");
		}
    }   
}

void wypisz()	//obliczenie sumy oraz wypisani wyniku i sklanikow na ekraz
{
    int tab_pom[100];	//tablica na pobrane liczby
    int i;
    int n;
    int tmp;
    char separator[]= "+";
    char * schowek;
    int suma=0;
    close(fd_pp[1]);
    while (ok)
    {
		if(tmp=read(fd_pp[0], bufor, SIZE) > 0 && zawies_pisarz==0)	//odczytanie danych z procesu 2
		{	
			printf("\nWprowadzone liczby:\n");
			i=0;
			schowek = strtok( bufor, separator );
			tab_pom[i]=atoi(schowek);				//zamiana cigu znakow na liczby
			printf("%d\n",tab_pom[i++]);
			
    		while( schowek != NULL )
    		{
        		schowek = strtok( NULL, separator );
        		if(schowek!=NULL)
        		{
        			tab_pom[i]=atoi(schowek);
        			printf("%d\n",tab_pom[i++]);
        		}
    		}
    		i--;
    		for(i; i>=0; i--)		//sumowanie
    		{
    			suma+=tab_pom[i];
    		}
    		printf("Suma wyrazenia wynosi: %d\n",suma);
    		suma=0;

		}
		
    }
}

void send_message(int rozkaz)		//wysylanie komunikatow
{
    struct msgbuf sndbufor = { 2, "Kolejka komunikaow:", rozkaz };
	sprintf(sndbufor.mtext, "Otrzymano rozkaz %d ",rozkaz);

    msgsnd(msqid, &sndbufor, sizeof(struct msgbuf) - sizeof(long), 0);
}

void s_usr1()	//sygnal SIGUSR1 powoduje wstrzymanie wszystkich procesow
{
    printf("Otrzymano SIGUSR1 z PID %d\n", getpid());
    if (tab[0] == getpid())
    {
		zawies_czytelnik = 1;
 		send_message(2);		//wysylanie komunikatu do pozostalych procesow jaka operacje nalezy wykonac
		send_message(3);

		kill(tab[1], SIGCONT);	//wyslanie sygnalu do pozostalych procesow
		kill(tab[2], SIGCONT);
		//zawies_czytelnik = 1;
    }
    else if(tab[1] == getpid())
    {
		zawies_sprawdz = 1;
 		send_message(1);
		send_message(3);

		kill(tab[0], SIGCONT);
		kill(tab[2], SIGCONT);
		//zawies_sprawdz = 1;
    }
    else if(tab[2] == getpid())
    {
		zawies_pisarz = 1;
 		send_message(1);
		send_message(2);

		kill(tab[0], SIGCONT);
		kill(tab[1], SIGCONT);
		//zawies_pisarz = 1;
    }

}


void s_usr2()	//sygnal SIGUSR2 powoduje wznowienie wszystkich procesow
{
    printf("Otzymano SIGUSR2 z PID %d\n", getpid());
    if (tab[0] == getpid())
    {
		send_message(5);	//wysylanie komunikatu do pozostalych procesow jaka operacje nalezy wykonac
		send_message(6);

		kill(tab[1], SIGCONT);	//wyslanie sygnalu do pozostalych procesow
		kill(tab[2], SIGCONT);
		zawies_czytelnik = 0;

    }
    else if(tab[1] == getpid())
    {
		send_message(4);
		send_message(6);


		kill(tab[0], SIGCONT);
		kill(tab[2], SIGCONT);
		zawies_sprawdz = 0;

    }
    else if(tab[2] == getpid())
    {
		send_message(4);
		send_message(5);


		kill(tab[0], SIGCONT);
		kill(tab[1], SIGCONT);
		zawies_pisarz = 0;
    }
}


void s_cont()	//sygnal SIGCONT zapewnia synchronizacje miedzy procesami, odpowiedzialny jest za wykonanie operacji, jaka zostala nadana
{
  	printf("\nOtrzymano SIGCONT z PID %d\n", getpid());
  
	struct msgbuf rcvbufor;

	msgrcv(msqid, &rcvbufor, sizeof(struct msgbuf) - sizeof(long), 2, 0);	//odczytanie kodu operacji
	printf("%s \n", rcvbufor.mtext);
	
	
	if (rcvbufor.rozkaz == 1)	//odczytywanie kodu operacji
	{
		zawies_czytelnik = 1;
		printf("Zatrzymanie pobierania\n\n");	//realizacja operacji
	}
	else if (rcvbufor.rozkaz == 2)
	{
		zawies_sprawdz = 1;
		printf("Zatrzymanie przetwarzania\n\n");
	}

	else if (rcvbufor.rozkaz == 3)
	{
		zawies_pisarz = 1;
		printf("Zatrzyamnie wyprowadzania danych\n\n");
	}
	else if (rcvbufor.rozkaz == 4)
	{
		printf("Wznowienie pobierania\n\n");
		zawies_czytelnik = 0;
	}
	else if (rcvbufor.rozkaz == 5)
	{
		printf("Wznowienie przetwarzania\n\n");
		zawies_sprawdz = 0;
	}
	else if (rcvbufor.rozkaz == 6)
	{
		printf("Wznowienie wyprowadzania danych\n\n");
		zawies_pisarz = 0;
	}
	else if (rcvbufor.rozkaz == 7)
	{
		printf("Usuwanie pobierania\n\n");
		exit(0);
	}
	else if (rcvbufor.rozkaz == 8)
	{
		printf("Usuwanie przetwarzania\n\n");
		exit(0);
	}
	else if (rcvbufor.rozkaz == 9)
	{
		printf("Usuwanie wyprowadzania danych\n\n");

		exit(0);
	}
	else exit(0);

}

void s_int()	//SIGINT odpowiedzialny jest za zakonczenie dzialania wszystkich procesow
{
	printf("\nOtrzymano SIGINT z PID %d\n\n", getpid());
    if (tab[0] == getpid())
    {
		send_message(8);
		send_message(9);

		kill(tab[1], SIGCONT);
		kill(tab[2], SIGCONT);

    }
    else if(tab[1] == getpid())
    {
		send_message(7);
		send_message(9);

		kill(tab[0], SIGCONT);
		kill(tab[2], SIGCONT);
    }
    else if(tab[2] == getpid())
    {
		send_message(7);
		send_message(8);

		kill(tab[0], SIGCONT);
		kill(tab[1], SIGCONT);

    }

    exit(0);
}

void signal_hendlers()	//powoluje hendlery
{
  	if(signal(SIGINT, s_int) == SIG_ERR || signal(SIGUSR1, s_usr1) == SIG_ERR || signal(SIGUSR2, s_usr2) == SIG_ERR || signal(SIGCONT, s_cont) == SIG_ERR)
    {
		printf("Nie mozna przechwycic sygnalu.\n");
  	}
}

int main()
{
 	int status;
    int PID_M = getpid();
    signal_hendlers();

    key = ftok(".", 'B');	//utworzenie oryginalnego klucza dostepu
    msqid = msgget(key, 0744 | IPC_CREAT);	//utworzenie kolejki komunikatow

	int shmid;
	shmid = shmget(IPC_PRIVATE,3*sizeof(int), 0744 | IPC_CREAT);	//utworzenie pamieci wspoldzielonej

	tab = (int*)shmat(shmid, NULL, 0);	//podlaczenie pamieci wspoldzilonej pod proces
	   
    pipe(fd_cp);	//pipe potrzebne do przekazywania danych miedzy procesami
    pipe(fd_pp);

    if(fork()==0)	//proces odpowiedzialny za pobieranie danych
    {
    	PID_A = getpid();
   		tab[0] = PID_A;
    	pobierz();
    
    }
    
    if(fork()==0)	//proces odpowiedzialny za sprawdzenie poprawnosci danych
    {
    	PID_B = getpid();
   		tab[1] = PID_B;
    	sprawdz();
    
    }
    
    if(fork()==0)	//proces odpowiedzialny za obliczenie i wyprowadzenie wyniku na ekran
    {
    	PID_C = getpid();
   		tab[2] = PID_C;
    	wypisz();
    }
    usleep(100);
    printf("PID odpowiedzialn za czytanie: %d\n",tab[0]);
    printf("PID odpowiedzialn za przetwarzanie: %d\n",tab[1]);
    printf("PID odpowiedzialn za wyprowadzanie danych: %d\n\n",tab[2]);
    
    //wait(NULL);
    waitpid((pid_t)tab[0],&status,0);	//oczekiwanie na zakonczenie sie wszystkich procesow potomnych
    waitpid((pid_t)tab[1],&status,0);
    waitpid((pid_t)tab[2],&status,0);

    if(getpid()==PID_M)
    {		
		msgctl(msqid, IPC_RMID, NULL);		//usuniecie pamieci wspoldzielonej oraz kolejki komunikatow
		shmctl(shmid,IPC_RMID,0);
    }

    
    return 0;
}
