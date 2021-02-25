/**
 * rootveg.c
 * Lab 5 Threads and Synchronization: The Root Vegtable Farm
 * Written by: Michael Primeau
 * 
 **/

#include <stdio.h>
#include <sys/stat.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>

volatile int start;

int currentTurnips = 0;
int currentRadishes = 0;
int maxTurnips = 0;
int maxRadishes = 0;
int totalProduced = 0;
int maxVegetables;

int customerIndex = 0;
int maxCustomers = 0;

sem_t turnipFlag;
sem_t radishFlag;
sem_t customerFlag;

typedef struct Customer {
    int numTurnips;
    int numRadishes;
    int waitTime;
    int totalOrders;
} Customer;

typedef enum Vegetable {Turnip, Radish} Vegetable;

typedef struct Farm {
    enum Vegetable type;
    int growTime;
    int vegInSeason;
    int totalProduced;
} Farm;

/**
 *  producer_routine
 *      Thread routine for producers
 *      Produces vegetables to be bought by consumers
 *      Parameters: args- pointer to farm struct for this thread
 **/
void* producer_routine(void* args) {
    Farm* farm = (Farm*)args;
    char vegType[7];
    // Set field type
    switch(farm->type) {
        case(Turnip) : strcpy(vegType, "Turnip"); break;
        case(Radish) : strcpy(vegType, "Radish"); break;
    }

    // Wait for start
    while(!start);

    // Grow vegtables until season complete
    for(int i = 0; i < farm->vegInSeason; i++) {
        // Grow turnips if turnip field
        if(farm->type == Turnip) {
            sem_wait(&turnipFlag);
            currentTurnips++;
            totalProduced++;
            if(maxTurnips < currentTurnips) {
                maxTurnips = currentTurnips;
            }
            sem_post(&turnipFlag);
        // Grow radishes if radish field
        } else {
            sem_wait(&radishFlag);
            currentRadishes++;
            totalProduced++;
            if(maxRadishes < currentRadishes) {
                maxRadishes = currentRadishes;
            }
            sem_post(&radishFlag);
        }
        usleep(farm->growTime);
    }
    return NULL;
}

/**
 *  consumer_routine
 *      Thread routine for consumers
 *      Buys vegetables from producers
 *      Parameters: args- pointer to customer struct for this thread
 **/
void* consumer_routine(void* args) {
    Customer* customer = (Customer*)args;
    customer->totalOrders = 0;
    int myIndex;

    // Set up this customers order in line
    sem_wait(&customerFlag);
    myIndex = maxCustomers;
    maxCustomers++;
    sem_post(&customerFlag);

    while(!start);
    
    // Continue to buy vegetables until there are not enough to fill order
    while(totalProduced < maxVegetables || (currentTurnips >= customer->numTurnips && currentRadishes >= customer->numRadishes)) {
        sem_wait(&customerFlag);
        // Check if a customer filled max orders possible
        while(myIndex >= maxCustomers) {
            myIndex--;
        }

        // If it is this customers turn then complete order
        if(customerIndex == myIndex) {
            // Buy turnips
            for(int i = 0; i < customer->numTurnips; i++) {
                sem_wait(&turnipFlag);
                if(currentTurnips > 0) {
                    currentTurnips--;
                } else {
                    i--;
                }
                sem_post(&turnipFlag);
            }
            // Buy radishes
            for(int i = 0; i < customer->numRadishes; i++) {
                sem_wait(&radishFlag);
                if(currentRadishes > 0) {
                    currentRadishes--;
                } else {
                    i--;
                }
                sem_post(&radishFlag);
            }
            // Change index to next customer
            if(customerIndex < maxCustomers - 1) {
                customerIndex++;
            } else {
                customerIndex = 0;
            }
            customer->totalOrders++;
            usleep(customer->waitTime);
            sem_post(&customerFlag);
        } else {
            // If not my turn then give up semaphore
            sem_post(&customerFlag);
        }
    }

    // If customer cannot fill any more orders, remove from queue and return
    sem_wait(&customerFlag);
    maxCustomers--;
    if(customerIndex != 0) {
        customerIndex--;
    }
    sem_post(&customerFlag);
    return NULL;
}

int main(int argc, char* argv[]) {
    // Read file to get program parameters
    FILE* file = fopen(argv[1], "r");
    start = 0;

    // Store parameters for turnips and radishes
    int numTurnipFields;
    int turnipGrowTime;
    int numTurnipSeason;

    int numRadishFields;
    int radishGrowTime;
    int numRadishSeason;

    int numCustomers;

    // Initialize semaphore flags
    sem_init(&customerFlag, 0, 1);
    sem_init(&turnipFlag, 0, 1);
    sem_init(&radishFlag, 0, 1);

    // Scan file
    fscanf(file, "%d", &numTurnipFields);
    fscanf(file, "%d", &turnipGrowTime);
    fscanf(file, "%d", &numTurnipSeason);

    fscanf(file, "%d", &numRadishFields);
    fscanf(file, "%d", &radishGrowTime);
    fscanf(file, "%d", &numRadishSeason);

    fscanf(file, "%d", &numCustomers);

    printf("Number of Turnip Fields: %d\n", numTurnipFields);
    printf("Time to grow: %d\n", turnipGrowTime);
    printf("Turnips produced per field per season: %d\n\n", numTurnipSeason);

    printf("Number of Radish Fields: %d\n", numRadishFields);
    printf("Time to grow: %d\n", radishGrowTime);
    printf("Radishes produced per field per season: %d\n\n", numRadishSeason);

    printf("Number of customers: %d\n\n", numCustomers);

    maxVegetables = numTurnipFields*numTurnipSeason + numRadishFields*numRadishSeason;

    // Create threads
    pthread_t turnips[numTurnipFields];
    pthread_t radishes[numRadishFields];
    pthread_t customerThreads[numCustomers];

    Farm turnipFarm;
    Farm radishFarm;

    // Create customer threads
    Customer customers[numCustomers];
    for(int i = 0; i < numCustomers; i++) {
        fscanf(file, "%d", &customers[i].numTurnips);
        fscanf(file, "%d", &customers[i].numRadishes);
        fscanf(file, "%d", &customers[i].waitTime);
        printf("Customer %d- Turnips: %d Radishes: %d Wait: %d\n", 
        i, customers[i].numTurnips, customers[i].numRadishes, customers[i].waitTime);
        if(pthread_create(&customerThreads[i], NULL, consumer_routine, (void*)&customers[i]) == -1) {
            printf("Could not create thread./n");
            exit(EXIT_FAILURE);           
        }
    }
    fclose(file);

    // Create turnip fields
    for(int i = 0; i < numTurnipFields; i++) {
        turnipFarm.type = Turnip;
        turnipFarm.growTime = turnipGrowTime;
        turnipFarm.vegInSeason = numTurnipSeason;
        if(pthread_create(&turnips[i], NULL, producer_routine, (void*)&turnipFarm) == -1) {
            printf("Could not create thread./n");
            exit(EXIT_FAILURE);
        }
    }

    // Create radish fields
    for(int i = 0; i < numRadishFields; i++) {
        radishFarm.type = Radish;
        radishFarm.growTime = radishGrowTime;
        radishFarm.vegInSeason = numRadishSeason;
        if(pthread_create(&radishes[i], NULL, producer_routine, (void*)&radishFarm) == -1) {
            printf("Could not create thread./n");
            exit(EXIT_FAILURE);
        }
    }
    
    // Start simulation
    start = 1;

    // Wait for all threads to finish
    for(int i = 0; i < numTurnipFields; i++) {
        pthread_join(turnips[i], NULL);
    }
    for(int i = 0; i < numRadishFields; i++) {
        pthread_join(radishes[i], NULL);
    }
    for(int i = 0; i < numCustomers; i++) {
        pthread_join(customerThreads[i], NULL);
    }

    // Print off simulation results
    printf("*********SIMULATION COMPLETE*********\n\n");
    printf("Current turnips:  %d\n", currentTurnips);
    printf("Current radishes: %d\n", currentRadishes);
    printf("Max Turnips in bin:  %d\n", maxTurnips);
    printf("Max Radishes in bin: %d\n", maxRadishes);

    for(int i = 0; i < numCustomers; i++) {
        printf("Customer %d completed %d orders\n", i, customers[i].totalOrders);
    }

    return 0;

}