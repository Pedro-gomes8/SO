/* Pedro Henrique Gomes Peixoto Januario
DRE 119042303
Ubuntu 20.04 LTS
*/
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

int main(int argc, char *argv[])
{
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

    return 0;
}