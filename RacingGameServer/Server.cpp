#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pthread.h>
#include <iostream>
using namespace std;

#define SERVER_PORT 1105
#define QUEUE_SIZE 5
const int AMOUNT_OF_CHAR_IN_MSG = 256;
const int AMOUNT_OF_PLAYERS = 2;
//struktura zawierająca dane, które zostaną przekazane do wątku
struct thread_data_t
{
int clients_sockets[AMOUNT_OF_PLAYERS];
};


//jak nazwa wskazuje
void sendMessage(int fd, char *msg)
{
    for (int i = 0; i < sizeof(msg)/sizeof(char); i++)
    {
        write(fd, &msg[i], sizeof(char));
    }
    cout<<"Wyslalem wiadomosc: ";
    cout<<msg<<endl;
}


//jak nazwa wskazuje
char * getMessage(int fd)
{
    char *msg = new char[AMOUNT_OF_CHAR_IN_MSG]; 
    char oneChar;
    int i = 0;
    while (oneChar != '\n') {
        read(fd, &oneChar, 1);
        msg[i] = oneChar;
        i++;
    }
    cout<<"Odebralem wiadomosc: ";
    cout<<msg<<endl;
    return msg;
    
}


//funkcja opisującą zachowanie wątku - musi przyjmować argument typu (void *) i zwracać (void *)
void *ThreadBehavior(void *t_data)
{
    cout<<"Stworzylem watek"<<endl;
    pthread_detach(pthread_self());
    struct thread_data_t *th_data = (struct thread_data_t*)t_data;
    int clients_sockets[AMOUNT_OF_PLAYERS];
    for(int i = 0; i < AMOUNT_OF_PLAYERS; i++)
    {
        clients_sockets[i]= th_data->clients_sockets[i]; 
    }
    char *msg[AMOUNT_OF_PLAYERS];
    for(int i = 0; i < AMOUNT_OF_PLAYERS; i++)
    {
        msg[i] = new char[AMOUNT_OF_CHAR_IN_MSG];
    }
    while(1)
    {
        for(int i = 0; i < AMOUNT_OF_PLAYERS; i++)  //pobierz wszystkie wiadomosci od graczy
        {
            //msg[i] = getMessage(clients_sockets[i]);
            read(clients_sockets[i], &msg[i], AMOUNT_OF_CHAR_IN_MSG);
            cout<<msg[i];
        }
        
        for(int i = 0; i < AMOUNT_OF_PLAYERS; i++)  //dla kazdego gracza
        {
            for(int j = 0; j< AMOUNT_OF_PLAYERS; j++)   //dla kazdej wiadomosci
            {
                //sendMessage(clients_sockets[i], msg[j]);    //zamiast msg[j] bedzie kurna stan samochodu j-tego
                      write(clients_sockets[i], msg[j], strlen(msg[j]));

            }
        }
            
    }
    
    pthread_exit(NULL);
}


//funkcja obsługująca połączenie z nowym klientem
void handleConnection(int clients_sockets[]) {
    cout<<"Jestem w handleConnection"<<endl;
    //wynik funkcji tworzącej wątek
    int create_result = 0;

    //uchwyt na wątek
    pthread_t thread1;

    //dane, które zostaną przekazane do wątku
    struct thread_data_t *t_data;
    for(int i = 0; i< AMOUNT_OF_PLAYERS; i++)
    {
        t_data->clients_sockets[i] = clients_sockets[i];
    }
    create_result = pthread_create(&thread1, NULL, ThreadBehavior, (void *)t_data);
    if (create_result){
       printf("Błąd przy próbie utworzenia wątku, kod błędu: %d\n", create_result);
       exit(-1);
    }
    //TODO (przy zadaniu 1) odbieranie -> wyświetlanie albo klawiatura -> wysyłanie
}


//inicjalizacja serwera
void initializeServer(int &server_socket_descriptor, int &bind_result, int &listen_result, char &reuse_addr_val, struct sockaddr_in &server_address)
{
   memset(&server_address, 0, sizeof(struct sockaddr));
   server_address.sin_family = AF_INET;
   server_address.sin_addr.s_addr = htonl(INADDR_ANY);
   server_address.sin_port = htons(SERVER_PORT);

   server_socket_descriptor = socket(AF_INET, SOCK_STREAM, 0);
   if (server_socket_descriptor < 0)
   {
       fprintf(stderr, ": Błąd przy próbie utworzenia gniazda..\n");
       exit(1);
   }
   setsockopt(server_socket_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&reuse_addr_val, sizeof(reuse_addr_val));

   bind_result = bind(server_socket_descriptor, (struct sockaddr*)&server_address, sizeof(struct sockaddr));
   if (bind_result < 0)
   {
       fprintf(stderr, ": Błąd przy próbie dowiązania adresu IP i numeru portu do gniazda.\n");
       exit(1);
   }

   listen_result = listen(server_socket_descriptor, QUEUE_SIZE);
   if (listen_result < 0) {
       fprintf(stderr, ": Błąd przy próbie ustawienia wielkości kolejki.\n");
       exit(1);
   }
    
}

//sprawdzenie czy poprawnie zostalo utworzone gniazdo
void check_client(int connection_socket_descriptor) 
{
    if (connection_socket_descriptor < 0)
    {
        fprintf(stderr, ": Błąd przy próbie utworzenia gniazda dla połączenia.\n");
        exit(1);
    }
    
}

//ustawia clients_sockets na -1
void makeMinusOne(int clients_sockets[])
{
    
    for(int i = 0; i < AMOUNT_OF_PLAYERS; i++)
    {
        clients_sockets[i] = -1;
    }
    
}

int main()
{
    //takie tam gowienka do dzialania serwera
    int server_socket_descriptor;
    int connection_socket_descriptor;
    int bind_result;
    int listen_result;
    char reuse_addr_val = 1;
    struct sockaddr_in server_address;
    
    int clients_sockets[AMOUNT_OF_PLAYERS];
    
    initializeServer(server_socket_descriptor, bind_result, listen_result, reuse_addr_val, server_address);  //jak nazwa wskazuje
   
    while(1) //Petla glowna programu
    {
        makeMinusOne(clients_sockets);
        
        for(int i = 0; i < AMOUNT_OF_PLAYERS; i++) //dla kazdego gracza ktory bedzie w jednym wyscigu
        {
            connection_socket_descriptor = accept(server_socket_descriptor, NULL, NULL); //Nowy klient przyszedl
            
            check_client(connection_socket_descriptor);  //sprawdz utworzenie gniazda, jesli cos poszlo nie tak wylacz program, malo prawdp sytuacja

            clients_sockets[i] = connection_socket_descriptor;  //dodaj gniazdo do tablicy
        }
        
        handleConnection(clients_sockets); //wyslij tablicę
       
    }    

    close(server_socket_descriptor);    //no widac co robi, co nie? :D 
    return(0);
}



