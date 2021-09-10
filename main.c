/* Pedro Henrique Gomes Peixoto Januario
DRE 119042303
Ubuntu 20.04 LTS
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

pthread_t *clients;
pthread_t barber;

void *handle_barber()
{
    pthread_exit(NULL);
}

void *client(pthread_t customer)
{
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // Getting Parameters from argv or request input
    int numberOfClients, numberOfChairs;
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

    // Creating Clients as well as allocating memory for each client
    clients = (pthread_t *)malloc(sizeof(pthread_t) * numberOfClients);
    int i;
    for (i = 0; i < numberOfChairs; i++)
    {
        pthread_create(clients[i], NULL, client, clients[i]);
    }
    // Waiting for barber to exit
    pthread_join(barber, NULL);

    // Freeing memory space
    free(clients);
    return 0;
}