/* Pedro Henrique Gomes Peixoto Januario
DRE 119042303
Ubuntu 20.04 LTS
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

pthread_t *clients;
pthread_t barber;
pthread_t *queue;

int numberOfClients, numberOfChairs;
pthread_mutex_t mutex;

void *handle_barber()
{
    printf("Getting you a magnificent haircut >.<\n");
    pthread_exit(NULL);
}

void *client(int threadID)
{
    printf("%d", threadID);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Getting Parameters from argv or request input
    printf("Welcome to our barber shop!\n");
    if (argc != 3)
    {
        printf("Please, enter the number of clients(threads):\n");
        scanf("%d", &numberOfClients);
        printf("Please enter the number of chairs in the waiting room(queue size):\n");
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
        pthread_create(&clients[i], NULL, client, i);
    }
    // Waiting for barber to exit
    pthread_join(barber, NULL);

    // Freeing memory space
    free(clients);
    free(queue);
    return 0;
}