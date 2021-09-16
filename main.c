/* Pedro Henrique Gomes Peixoto Januario
DRE 119042303
Ubuntu 20.04 LTS
Problem chosen: FIFO Barber Shop
*/
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <pthread.h>

pthread_t *clients;
pthread_t barber;
pthread_t *queue;

int numberOfClients, numberOfChairs;
int amountOfCustomers = 0;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t haircutMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t customerIshere = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberisFree = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberDone = PTHREAD_COND_INITIALIZER;
pthread_cond_t clientDone = PTHREAD_COND_INITIALIZER;
int barberIsDoneForTheDay = 0;

void *handle_barber()
{
    while (1){
        pthread_mutex_lock(&haircutMutex);
        pthread_cond_wait(&customerIshere,&haircutMutex);
        pthread_cond_signal(&barberisFree);
        pthread_cond_wait(&clientDone,&haircutMutex);
        pthread_cond_signal(&barberDone);
        pthread_mutex_unlock(&haircutMutex);
        // Haircut Done

        // Checking if there's someone else in line:
        pthread_mutex_lock(&queueMutex);
        amountOfCustomers = amountOfCustomers - 1; // He just cut someone's hair
        while (amountOfCustomers == 0){
            barberIsDoneForTheDay = 1;
            pthread_mutex_unlock(&queueMutex);
            pthread_exit(NULL);
        }
        pthread_mutex_unlock(&queueMutex);

    }
    printf("%s\n","Looks like there are no more customers today!");
    pthread_exit(NULL);
}

void *client(void *threadID)
{   
    // Grabbing the index of the thread.
    int id = *((int*) threadID);
    // Freeing memory of the index.
    free(threadID);

    // Trying to grab the lock:
    pthread_mutex_lock(&queueMutex);
    if (barberIsDoneForTheDay){
        printf("%s\n","Oh crap, the barber has already closed the shop!");
        pthread_exit(NULL);
    }
    while (amountOfCustomers == numberOfChairs){
        printf("It's full. (ID:%d) is going home\n", id);
        pthread_mutex_unlock(&queueMutex);
        pthread_exit(NULL);
    }
    // If the thread hasnt exited in the while block above:
    amountOfCustomers += 1;
    printf("(id:%d) is in line\n",id);
    // Append to queue
    pthread_mutex_unlock(&queueMutex);

    // Grabbing haircut lock
    pthread_mutex_lock(&haircutMutex);
    // Signaling there is a customer and waits for barber to be free
    pthread_cond_signal(&customerIshere);
    pthread_cond_wait(&barberisFree,&haircutMutex);
    // Getting the haircut...
    printf("Thank you for cutting my hair, Mr. Barber. Im (id:%d)\n",id);
    pthread_cond_signal(&clientDone);
    pthread_cond_wait(&barberDone,&haircutMutex);
    pthread_mutex_unlock(&haircutMutex);

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