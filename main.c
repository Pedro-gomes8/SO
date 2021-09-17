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

//
enum {secsToSleep = 0, NsectoSleep = 500000000};
struct timespec haircutTime = {secsToSleep, NsectoSleep};

int numberOfClients, numberOfChairs;
int amountOfCustomers = 0;
pthread_mutex_t queueMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t BarberMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t WaitforBarber = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t WaitForClient = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t customerIsHere = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberIsDone = PTHREAD_COND_INITIALIZER;
pthread_cond_t barberIsFree = PTHREAD_COND_INITIALIZER;
pthread_cond_t ClientLeft = PTHREAD_COND_INITIALIZER;

int BarberHasSleptOnce = 0; // Stop condition. Barber can sleep once (i.e. when he is the first created thread to execute, thus he shall sleep until a customer arrives), but if he sleeps a second time, that means the queue is empty.
int BarberIsSleeping = 0; 


// Below is the implementation of a Queue Data Structure, it was made by Ryan "ryankurte".
// https://gist.github.com/ryankurte/61f95dc71133561ed055ff62b33585f8
typedef struct {
    size_t head;
    size_t tail;
    size_t size;
    void** data;
} queue_t;
void* queue_read(queue_t *queue) {
    if (queue->tail == queue->head) {
        return NULL;
    }
    void* handle = queue->data[queue->tail];
    queue->data[queue->tail] = NULL;
    queue->tail = (queue->tail + 1) % queue->size;
    return handle;
}

int queue_write(queue_t *queue, void* handle) {
    if (((queue->head + 1) % queue->size) == queue->tail) {
        return -1;
    }
    queue->data[queue->head] = handle;
    queue->head = (queue->head + 1) % queue->size;
    return 0;
}
// End of external code.
queue_t customerQueue;

// Barber Thread
void *handle_barber()
{
    while(1){
        pthread_mutex_lock(&queueMutex); // Prevents other threads changing the value of 'amountOfCustomers' while the barber is checking it
        if (amountOfCustomers == 0){
            pthread_mutex_unlock(&queueMutex);
            if (BarberHasSleptOnce){
                printf("Looks like there's no one else left in line. Im gonna sleep again and close the shop!\n");
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&queueMutex);
            pthread_mutex_lock(&BarberMutex);
            BarberIsSleeping = 1;
            // Barber goes to sleep if he is the first thread to be executed (no client yet).
            pthread_cond_wait(&customerIsHere,&BarberMutex);
            // When he wakes up;
            BarberIsSleeping = 0;
            pthread_mutex_unlock(&BarberMutex);
        }
        else {
            // If he is awake and there is a customer, he cuts the customer's hair and say goodbye to him. 
            pthread_mutex_unlock(&queueMutex);
            printf("Cutting Hair...\n");
            nanosleep(&haircutTime,NULL);
            pthread_mutex_lock(&queueMutex);
            // Gets the customer's "name".
            void * threadid = queue_read(&customerQueue);
            int id = (int) (long) threadid;
            printf("\tThere we go! Have a nice day Mr.(id:%d)\n",id);
            // pthread_mutex_unlock(&queueMutex);
            // Changing the amount of customers left to offer haircut services as he just cut someone's hair.
            // pthread_mutex_lock(&queueMutex);
            amountOfCustomers = amountOfCustomers - 1;
            pthread_mutex_unlock(&queueMutex);
            // After he's finished with the current customer and the queue has been rearranged, he signals another customer.
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
    while (amountOfCustomers >= numberOfChairs){
        printf("\tOh no, looks like the shop is full,(id:%d) is going home\n",id);
        pthread_mutex_unlock(&queueMutex);
        pthread_exit(NULL);
    }
    amountOfCustomers = amountOfCustomers + 1;
    queue_write(&customerQueue,(void *)(long) id);
    printf("Customer (id:%d) enters the shop\n",id);
    if(BarberIsSleeping){
        pthread_cond_signal(&customerIsHere);
    }
    pthread_mutex_unlock(&queueMutex);
    pthread_mutex_lock(&WaitforBarber);
    pthread_cond_wait(&barberIsFree,&WaitforBarber);
    // Barber begins cutting hair...
    // when this thread wakes up:
    pthread_mutex_unlock(&WaitforBarber);
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    printf("\nIMPORTANTE. Apos alguns testes, eu percebi que esse codigo possui uma falha que pode aparecer, ou nao, quando for testar que nao consegui identificar. De alguma forma, apenas uma unica thread (independente dos parametros que esse codigo recebe) 'perde' seu ID. seu ID passa a ser 0 como se fosse a primeira thread criada, e essa thread que perde o ID acaba no final da queue. Isso pode aparecer nos resultados. Embora uma unica thread possa perder, ou nao, seu lugar na fila, todas as threads que entram na barbearia continuam tendo seu cabelo cortado yay!\n\n");

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

    // Creating Clients as well as allocating memory for each client and queue
    clients = (pthread_t *)malloc(sizeof(pthread_t) * numberOfClients);
    // Initializing queue structure
    customerQueue.head = 0;
    customerQueue.tail = 0;
    customerQueue.size = numberOfChairs;
    customerQueue.data = malloc(sizeof(void*) * numberOfClients);
    // Creating barber thread
    pthread_create(&barber, NULL, handle_barber, NULL);
    // Creating Clients
    int i;
    enum {secsTocreateOtherThread = 0, NsecsTocreateOtherThread = 100000000};
    struct timespec loopsleep = {secsTocreateOtherThread,NsecsTocreateOtherThread};
    for (i = 0; i < numberOfClients; i++)
    {
        // Allocating memory for the index. That way it's easier to identify the thread by their "ID", which, in this case, is just "i".
        int *id = malloc(sizeof(int));
        *id = i;
        pthread_create(&clients[i], NULL, client, id);
        // Sleeping so the queue doesnt get "immediately" filled in.
        // Enables barber to clear spots in queue while he cuts some thread's hair. Other threads can then, if they see a spot available, join the queue.
        nanosleep(&loopsleep,NULL);
    }
    // Waiting for barber to exit
    pthread_join(barber, NULL);

    // Freeing memory space
    free(clients);
    return 0;
}