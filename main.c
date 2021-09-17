/* Pedro Henrique Gomes Peixoto Januario
DRE 119042303
Ubuntu 20.04 LTS
Problem chosen: FIFO Barber Shop
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <math.h>

pthread_t *clients;
pthread_t barber;
pthread_t *queue;

//
enum {secsToSleep = 0, NsectoSleep = 500000000};
struct timespec haircutTime = {secsToSleep, NsectoSleep};

int numberOfClients, numberOfChairs,thread_ID;
int amountOfCustomers = 0;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BarberMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t WaitforBarber = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t WaitForClient = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t ID_Mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t customerIsHere = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberIsDone = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberIsFree = PTHREAD_COND_INITIALIZER;
pthread_cond_t ClientLeft = PTHREAD_COND_INITIALIZER;

int BarberHasSleptOnce = 0; // Stop condition. Barber can sleep once (i.e. when he is the first created thread to execute, thus he shall sleep until a customer arrives), but if he sleeps a second time, that means the queue is empty.
int BarberIsSleeping = 0; 

void *handle_barber()
{
    while(1){
        pthread_mutex_lock(&queueMutex); // Prevents other threads changing the value of 'amountOfCustomers' while the barber is checking it
        if (amountOfCustomers == 0){
            pthread_mutex_unlock(&queueMutex);
            if (BarberHasSleptOnce){
                printf("Looks like there's no one in line. Im gonna sleep again and close the shop\n");
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&queueMutex);
            pthread_mutex_lock(&BarberMutex);
            BarberIsSleeping = 1;
            pthread_cond_wait(&customerIsHere,&BarberMutex);
            // When he wakes up;
            BarberIsSleeping = 0;
            pthread_mutex_unlock(&BarberMutex);
        }
        else {
            pthread_mutex_unlock(&queueMutex);
            printf("Cutting Hair...\n");
            nanosleep(&haircutTime,NULL);
            pthread_mutex_lock(&ID_Mutex);
            printf("Thank you. Have a nice day (id:%d)\n",thread_ID);
            pthread_mutex_unlock(&ID_Mutex);

            pthread_mutex_lock(&queueMutex);
            amountOfCustomers = amountOfCustomers - 1;
            pthread_mutex_unlock(&queueMutex);
            pthread_cond_signal(&barberIsFree);

            switch (BarberHasSleptOnce) // If barber wasnt the first created thread to be executed, that means there was no need for him to sleep for the first time. In that case, the first time he'd sleep would be whenever the queue would be empty. Essentially, it means: "If barber hasnt slept up to this point, lie about it and say he has slept once."
            {
            case 0:
                BarberHasSleptOnce = 1;
                break;
            case 1:
                break;       
            }
        }
        
    }
    printf("Helloo Mr. Rodrigo, this should never print\n");
}

void *client(void *threadID)
{   
    // Grabbing the index of the thread.
    int id = *((int*) threadID);
    // Freeing memory of the index.
    free(threadID);

    pthread_mutex_lock(&queueMutex);
    if (amountOfCustomers >= numberOfChairs){
        printf("Oh no, this shop is full, (id:%d) is leaving\n",id);
        pthread_mutex_unlock(&queueMutex);
        pthread_exit(NULL);
    }
    amountOfCustomers = amountOfCustomers + 1;

    if(BarberIsSleeping){
        pthread_cond_signal(&customerIsHere);
    }
    pthread_mutex_unlock(&queueMutex);
    pthread_mutex_lock(&WaitforBarber);
    pthread_cond_wait(&barberIsFree,&WaitforBarber);
    // Barber begins cutting hair...
    // when this thread wakes up:
    pthread_mutex_lock(&ID_Mutex);
    thread_ID = id;
    pthread_mutex_unlock(&ID_Mutex);
    pthread_mutex_unlock(&WaitforBarber);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Getting Parameters from argv or request input
    printf("%s\n","Welcome to our barber shop!");
    if (argc != 3)
    {
        printf("%s\n","Please, enter the number of customers(threads):");
        scanf("%d", &numberOfClients);
        printf("%s\n","Please enter the number of chairs in the waiting room(queue size):");
        scanf("%d", &numberOfChairs);
    }
    else
    {
        numberOfClients = atoi(argv[1]);
        numberOfChairs = atoi(argv[2]);
    }
    // Creating barber thread
    pthread_create(&barber, NULL, handle_barber, NULL);

    // Creating Clients as well as allocating memory for each client and queue
    clients = (pthread_t *)malloc(sizeof(pthread_t) * numberOfClients);
    queue = (pthread_t *)malloc(sizeof(pthread_t) * numberOfChairs);

    int i;
    for (i = 0; i < numberOfChairs; i++)
    {
        // Allocating memory for the index. That way it's easier to identify the thread by their "ID", which, in this case, is just "i".
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&clients[i], NULL, client, id);
    }
    // Waiting for barber to exit
    pthread_join(barber, NULL);

    // Freeing memory space
    free(clients);
    free(queue);
    return 0;
}