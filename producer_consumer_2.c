#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
/*
 * CPSC 457 Assignment 1
 * Section 2: Part 2
 *
 * Ryan Loi
 * Lecture: 02
 * Tutorial: T10
 */



// variables

// size of bounded buffers
#define boundedBufferSize 10
// max running time of program
#define maxRunningTime 45
// define our pseudo string builder's size
#define strBuilderSize 3000
// define out temp string size
#define tempStrSize 500



/**
 * A function to generate a random number between a given minimum number and maximum number (inclusive).
 * @param minNum: lower bound of the random number range
 * @param maxNum: upper bound of the random number range
 * @return: a random number between minNum and maxNum (inclusive)
 */
int genRanNum(int minNum, int maxNum){
    // returns a random number in the specified range between min and max num (inclusive)
    return ((rand() % (maxNum - minNum + 1) + minNum));
}



/**
 * Data structure called Balloon Brigade (the balloon company that we own in section 2 of assignment 1), and this data
 * structure will represent the infrastructure of the company that we own (the carts holding the balloons and the locks
 * preventing over and under flowing).
 *
 * This data structure contains 2 bounded buffers of size 10, and 2 sets of locking variables (mutex, full, empty).
 * Variables ending in 1 belong to Balloon Bob and are associated with his balloon animals, variables ending in 2 belong
 * to Helium Harry and are associated with his Balloon houses.
 *
 * There is also a variable called terminate that all threads that utilize this data structure will observe, and this
 * variable will signal when the thread should self-terminate (when the 45 seconds of execution time are up, as controlled
 * by the main function).
 */
struct BalloonBrigade{
    // variable that all threads will continually monitor, and is used to indicate when all threads need to terminate -
    // which is when the main program has executed for 45 seconds (1 = terminate, 0 = do not terminate)
    int terminate;

    /* variables that will tell the print buffer thread to print (1 = print buffer info, 0 = wait). These variables don't
     require a mutex lock because the main mutex locks will protect these variables. This is because these variables
     can only be changed to 1 by the producer and consumer threads in the critical section, and since these variables
     will be kept in the critical section they are already protected. The print info thread will be responsible to change
     it back to 0, but it will only be doing this inside the critical section (and enforced by a conditional to ensure
     execution stays in the critical section until the change is done) so the main mutex will protect it.
    */
    int printStatus1;
    int printStatus2;

    // pointer to a pseudo string builder so print buffer can store data (1 for each boundedBuffer)
    char *strBuilder1;
    char *strBuilder2;

    // variables for Balloon Bob and his balloon animals

    // Bounded buffer representing Balloon Bob's balloon animal cart
    int boundedBuffer1[boundedBufferSize];
    // mutex to control access to boundedBuffer1 (1= available/unlocked, 0 = unavailable/locked)
    int mutex1;
    // full semaphore to track number of free slots in boundedBuffer1
    int full1;
    // empty semaphore to track number of empty slots in boundedBuffer1
    int empty1;


    // variables for Helium Harry and his balloon houses

    // Bounded buffer representing Helium Harry's balloon house cart
    int boundedBuffer2[boundedBufferSize];
    // mutex to control access to boundedBuffer2 (1= available/unlocked, 0 = unavailable/locked)
    int mutex2;
    // full semaphore to track number of free slots in boundedBuffer2
    int full2;
    // empty semaphore to track number of empty slots in boundedBuffer2
    int empty2;
};



/**
 * A thread function that will print the information of the bounded buffers
 * @param arg: pointer to the Balloon Brigade structure
 * @return: Null
 */
void* printBuffer(void *arg){
    // creating a pointer for the balloon brigade structure
    struct BalloonBrigade *cart = (struct BalloonBrigade*) arg;

    // loop infinitely
    while(1){
        // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
        if((*cart).terminate == 1){
            // set print status to 0
            (*cart).printStatus1 = 0;
            (*cart).printStatus2 = 0;
            pthread_exit(0);
        }

        // temporary string
        char tempStr[500];
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // otherwise check if a print is required via the print status variable (1 = print buffer info, 0 = no print needed
        // so keep looping and waiting)
        if((*cart).printStatus1 == 1) {
            // first print bounded buffer 1's info
            sprintf(tempStr,"Bounded Buffer 1 (Balloon Bob's Cart):\n");
            strcat((*cart).strBuilder1,tempStr);
            //clearing memory to string termination characters so it will not concatenate empty spots
            memset(tempStr, '\0', sizeof tempStr);

            // loop through the entire bounded buffer1 and print it contents at its indices (0 = empty, 1 = full)
            for (int i = 0; i < boundedBufferSize; i++) {
                sprintf(tempStr,"[%d] -> %d ;;; ", i, (*cart).boundedBuffer1[i]);
                strcat((*cart).strBuilder1,tempStr);
                //clearing memory to string termination characters so it will not concatenate empty spots
                memset(tempStr, '\0', sizeof tempStr);
            }

            // print slot information (how many full and empty slots)
            sprintf(tempStr,"Number of Full Slots = %d and Number of Empty slots = %d ;;;", (*cart).full1, (*cart).empty1);
            strcat((*cart).strBuilder1,tempStr);
            //clearing memory to string termination characters so it will not concatenate empty spots
            memset(tempStr, '\0', sizeof tempStr);

            // print if mutex1 is available or not (mutex = 1 = available, mutex = 0 = unavailable)
            if ((*cart).mutex1 == 1) {
                sprintf(tempStr," Mutex 1 is available\n\n");
                strcat((*cart).strBuilder1,tempStr);
                //clearing memory to string termination characters so it will not concatenate empty spots
                memset(tempStr, '\0', sizeof tempStr);

            } else if ((*cart).mutex1 == 0) {
                sprintf(tempStr," Mutex 1 is unavailable\n\n");
                strcat((*cart).strBuilder1,tempStr);
                //clearing memory to string termination characters so it will not concatenate empty spots
                memset(tempStr, '\0', sizeof tempStr);
            }
            // now change print status back to 0 as the print job has been completed
            (*cart).printStatus1 = 0;
        }

        // check if second bound buffer needs to print
        if((*cart).printStatus2 == 1){
            // first print bounded buffer 2's info
            sprintf(tempStr,"Bounded Buffer 2 (Helium Harry's Cart):\n");
            strcat((*cart).strBuilder2,tempStr);
            //clearing memory to string termination characters so it will not concatenate empty spots
            memset(tempStr, '\0', sizeof tempStr);

            // loop through the entire bounded buffer1 and print it contents at its indices (0 = empty, 1 = full)
            for(int i = 0; i < boundedBufferSize; i++){
                sprintf(tempStr,"[%d] -> %d ;;; ", i, (*cart).boundedBuffer2[i]);
                strcat((*cart).strBuilder2,tempStr);
                //clearing memory to string termination characters so it will not concatenate empty spots
                memset(tempStr, '\0', sizeof tempStr);
            }

            // print slot information (how many full and empty slots)
            sprintf(tempStr, "Number of Full Slots = %d and Number of Empty slots = %d ;;;", (*cart).full2, (*cart).empty2);
            strcat((*cart).strBuilder2,tempStr);
            //clearing memory to string termination characters so it will not concatenate empty spots
            memset(tempStr, '\0', sizeof tempStr);

            // print if mutex2 is available or not (mutex = 1 = available, mutex = 0 = unavailable)
            if((*cart).mutex2 == 1){
                sprintf(tempStr," Mutex 2 is available\n\n");
                strcat((*cart).strBuilder2,tempStr);
                //clearing memory to string termination characters so it will not concatenate empty spots
                memset(tempStr, '\0', sizeof tempStr);

            }else if ((*cart).mutex2 == 0){
                sprintf(tempStr," Mutex 2 is unavailable\n\n");
                strcat((*cart).strBuilder2,tempStr);
                //clearing memory to string termination characters so it will not concatenate empty spots
                memset(tempStr, '\0', sizeof tempStr);
            }

            // now change print status back to 0 as the print job has been completed
            (*cart).printStatus2 = 0;
        }
    }
}


/**
 * Thread function for Balloon Bob, this function makes balloon animals and puts them into Bob's balloon cart (bounded
 * buffer 1).
 * @param arg: pointer to the balloon brigade structure
 * @return: Null
 */
void* BalloonBob(void* arg){
    // creating a pointer for the balloon brigade structure
    struct BalloonBrigade *cart = (struct BalloonBrigade*) arg;

    // loop infinitely
    while(1) {
        // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
        if ((*cart).terminate == 1) {
            pthread_exit(0);
        }

        // generate a random time that the producer will have to wait
        int waitTime = genRanNum(1,10);
        // print wait time
        printf("Balloon Bob (Producer) will make the next Balloon animal in %d seconds...\n\n", waitTime);

        // wait that amount of time
        // Creating Timer to allow the producer to wait waitTime seconds

        // Variable to hold initial time
        time_t initialTime;
        time(&initialTime);
        // variable to hold current time
        time_t currentTime;
        // variable to hold amount of elapsed time
        double elapsedTime = 0;

        // loop until wait time is over
        do{
            // get current time
            time(&currentTime);
            // calculate elapsed time
            elapsedTime = difftime(currentTime, initialTime);

            // check terminate signal while waiting, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        } while (elapsedTime <= waitTime);

        // Production time

        // check if mutex1 lock is available if it is acquire it if not wait until it is free
        if((*cart).mutex1 == 0){
            printf("Error: mutex1 lock is unavailable, Balloon Bob (Producer) needs to wait until the customer has finished "
                   "taking a balloon out of the cart before he can put another one in)...\n\n");
        }

        // check if the buffer is full before producing another balloon to be put into the buffer, otherwise wait until
        // space in the buffer is available
        if((*cart).full1 == boundedBufferSize){
            printf("Error: Balloon Bob's (Producer) cart (boundedBuffer1) is full, no more Animal Balloons can be produced until a consumer buys one ...\n\n");
        }

        // wait until mutex1 is free and there is space in the bounded buffer
        while(((*cart).mutex1 == 0) || ((*cart).full1 >= boundedBufferSize)){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }
        // acquire lock after waiting
        (*cart).mutex1 = 0;


        // implementing a "psuedo string builder" (3000 characters) so everything is printed at once and not jumbled up
        char strBuilder[strBuilderSize];
        //clearing memory to string termination characters so it will not print empty spots
        memset(strBuilder, '\0', sizeof strBuilder);

        // temporary string
        char tempStr[tempStrSize];
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // store message then concatenate into strBuilder
        sprintf(tempStr,"Balloon Bob (Producer) is currently making a balloon animal...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // store info about semaphores into temp then concatenate into strBuilder
        sprintf(tempStr,"Current full slots = %d; current empty slots = %d\n", (*cart).full1, (*cart).empty1);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // creating a new balloon to put into the buffer at index indicated by full
        (*cart).boundedBuffer1[(*cart).full1] = 1;
        // increment full and decrement empty
        (*cart).full1++;
        (*cart).empty1--;

        // print message
        sprintf(tempStr,"Balloon Bob (Producer) is done making a Balloon animal and has added it to his cart (boundedBuffer1)...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // print info about semaphores
        sprintf(tempStr,"Updated full slots = %d; Updated empty slots = %d\n\n", (*cart).full1, (*cart).empty1);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // assign strBuilder address to structure
        (*cart).strBuilder1 = strBuilder;

        // signal the print buffer function thread
        (*cart).printStatus1 = 1;

        // wait until the print is done before finishing
        while((*cart).printStatus1 != 0){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }

        //printing the string builder
        printf("%s",strBuilder);

        // removing the string builder from the cart
        (*cart).strBuilder1 = NULL;

        // release lock
        (*cart).mutex1 = 1;
    }
}



/**
 * Thread function for Helium Harry, this function makes balloon houses and puts them into Harry's balloon cart (bounded
 * buffer 2).
 * @param arg: pointer to the balloon brigade structure
 * @return: Null
 */
void* HeliumHarry(void* arg){
    // creating a pointer for the balloon brigade structure
    struct BalloonBrigade *cart = (struct BalloonBrigade*) arg;

    // loop infinitely
    while(1) {
        // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
        if ((*cart).terminate == 1) {
            pthread_exit(0);
        }

        // generate a random time that the producer will have to wait
        int waitTime = genRanNum(1,10);
        // print wait time
        printf("Helium Harry (Producer) will make the next Balloon house in %d seconds...\n\n", waitTime);

        // wait that amount of time
        // Creating Timer to allow the producer to wait waitTime seconds

        // Variable to hold initial time
        time_t initialTime;
        time(&initialTime);
        // variable to hold current time
        time_t currentTime;
        // variable to hold amount of elapsed time
        double elapsedTime = 0;

        // loop until wait time is over
        do{
            // get current time
            time(&currentTime);
            // calculate elapsed time
            elapsedTime = difftime(currentTime, initialTime);

            // check terminate signal while waiting, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        } while (elapsedTime <= waitTime);

        // Production time

        // check if mutex2 lock is available if it is acquire it if not wait until it is free
        if((*cart).mutex2 == 0){
            printf("Error: mutex1 lock is unavailable, Helium Harry (Producer) needs to wait until the customer has finished "
                   "taking a balloon out of the cart before he can put another one in)...\n\n");
        }

        // check if the buffer is full before producing another balloon to be put into the buffer, otherwise wait until
        // space in the buffer is available
        if((*cart).full2 == boundedBufferSize){
            printf("Error: Helium Harry's (Producer) cart (boundedBuffer2) is full, no more Balloon Houses can be produced until a consumer buys one ...\n\n");
        }

        // wait until mutex2 is free and there is space in the bounded buffer
        while(((*cart).mutex2 == 0) || ((*cart).full2 >= boundedBufferSize)){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }
        // acquire lock after waiting
        (*cart).mutex2 = 0;


        // implementing a "psuedo string builder" (3000 characters) so everything is printed at once and not jumbled up
        char strBuilder[strBuilderSize];
        //clearing memory to string termination characters so it will not print empty spots
        memset(strBuilder, '\0', sizeof strBuilder);

        // temporary string
        char tempStr[tempStrSize];
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // store message then concatenate into strBuilder
        sprintf(tempStr,"Helium Harry(Producer) is currently making a balloon house...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // store info about semaphores into temp then concatenate into strBuilder
        sprintf(tempStr,"Current full slots = %d; current empty slots = %d\n", (*cart).full2, (*cart).empty2);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // creating a new balloon to put into the buffer at index indicated by full
        (*cart).boundedBuffer2[(*cart).full2] = 1;
        // increment full and decrement empty
        (*cart).full2++;
        (*cart).empty2--;

        // print message
        sprintf(tempStr,"Helium Harry (Producer) is done making a Balloon House and has added it to his cart (boundedBuffer2)...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // print info about semaphores
        sprintf(tempStr,"Updated full slots = %d; Updated empty slots = %d\n\n", (*cart).full2, (*cart).empty2);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // assign strBuilder address to structure
        (*cart).strBuilder2 = strBuilder;

        // signal the print buffer function thread
        (*cart).printStatus2 = 1;

        // wait until the print is done before finishing
        while((*cart).printStatus2 != 0){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }

        //printing the string builder
        printf("%s",strBuilder);

        // removing the string builder from the cart
        (*cart).strBuilder2 = NULL;

        // release lock
        (*cart).mutex2 = 1;
    }
}



/**
 * Thread function for a consumer (animal) that buys balloon animals, this function take balloon animals from Bob's balloon cart (bounded
 * buffer 1).
 * @param arg: pointer to the balloon brigade structure
 * @return: Null
 */
void* buyBalloonAnimal(void* arg){
    // creating a pointer for the balloon brigade structure
    struct BalloonBrigade *cart = (struct BalloonBrigade*) arg;

    // loop infinitely
    while(1) {
        // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
        if ((*cart).terminate == 1) {
            pthread_exit(0);
        }

        // generate a random time that the consumer will have to wait before iterating again
        int waitTime = genRanNum(5,15);
        // print wait time
        printf("Consumer (animal) will buy a balloon animal in %d seconds...\n\n", waitTime);

        // wait that amount of time
        // Creating Timer to allow the producer to wait waitTime seconds

        // Variable to hold initial time
        time_t initialTime;
        time(&initialTime);
        // variable to hold current time
        time_t currentTime;
        // variable to hold amount of elapsed time
        double elapsedTime = 0;

        // loop until wait time is over
        do{
            // get current time
            time(&currentTime);
            // calculate elapsed time
            elapsedTime = difftime(currentTime, initialTime);

            // check terminate signal while waiting, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        } while (elapsedTime <= waitTime);

        // consumption time

        // check if mutex1 lock is available if it is acquire it if not wait until it is free
        if((*cart).mutex1 == 0){
            printf("Error: mutex1 lock is unavailable, consumer (animal) needs to wait until Balloon Bob (producer) has finished "
                   "making balloon animals before one can be purchased)...\n\n");
        }

        // check if the buffer is empty before consuming another balloon from the buffer, otherwise wait until
        // there are more balloons in the buffer
        if((*cart).empty1 == boundedBufferSize){
            printf("Error: Balloon Bob's (Producer) cart (boundedBuffer1) is empty, consumer (animal) can't buy a balloon animal until Bob makes more ...\n\n");
        }

        // wait until mutex1 is free and there is more balloons in the bounded buffer
        while(((*cart).mutex1 == 0) || ((*cart).empty1 == boundedBufferSize)){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }
        // acquire lock after waiting
        (*cart).mutex1 = 0;


        // implementing a "psuedo string builder" (3000 characters) so everything is printed at once and not jumbled up
        char strBuilder[strBuilderSize];
        //clearing memory to string termination characters so it will not print empty spots
        memset(strBuilder, '\0', sizeof strBuilder);

        // temporary string
        char tempStr[tempStrSize];
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // store message then concatenate into strBuilder
        sprintf(tempStr,"Consumer (animal) is currently purchasing a balloon animal...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // store info about semaphores into temp then concatenate into strBuilder
        sprintf(tempStr,"Current full slots = %d; current empty slots = %d\n", (*cart).full1, (*cart).empty1);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // removing a balloon animal at index = bounded buffer max index - empty
        (*cart).boundedBuffer1[(boundedBufferSize-1)-(*cart).empty1] = 0;
        // decrement full and increment empty
        (*cart).full1--;
        (*cart).empty1++;

        // print message
        sprintf(tempStr,"Consumer (animal) is done buying a Balloon animal and has removed it from Bob's cart (boundedBuffer1)...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // print info about semaphores
        sprintf(tempStr,"Updated full slots = %d; Updated empty slots = %d\n\n", (*cart).full1, (*cart).empty1);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // assign strBuilder address to structure
        (*cart).strBuilder1 = strBuilder;

        // signal the print buffer function thread
        (*cart).printStatus1 = 1;

        // wait until the print is done before finishing
        while((*cart).printStatus1 != 0){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }

        //printing the string builder
        printf("%s",strBuilder);

        // removing the string builder from the cart
        (*cart).strBuilder1 = NULL;

        // release lock
        (*cart).mutex1 = 1;
    }
}



/**
 * Thread function for a consumer (house) that buys balloon houses, this function take balloon houses from Harry's balloon cart (bounded
 * buffer 2).
 * @param arg: pointer to the balloon brigade structure
 * @return: Null
 */
void* buyBalloonHouse(void* arg){
    // creating a pointer for the balloon brigade structure
    struct BalloonBrigade *cart = (struct BalloonBrigade*) arg;

    // loop infinitely
    while(1) {
        // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
        if ((*cart).terminate == 1) {
            pthread_exit(0);
        }

        // generate a random time that the consumer will have to wait before iterating again
        int waitTime = genRanNum(5,15);
        // print wait time
        printf("Consumer (house) will buy a balloon house in %d seconds...\n\n", waitTime);

        // wait that amount of time
        // Creating Timer to allow the producer to wait waitTime seconds

        // Variable to hold initial time
        time_t initialTime;
        time(&initialTime);
        // variable to hold current time
        time_t currentTime;
        // variable to hold amount of elapsed time
        double elapsedTime = 0;

        // loop until wait time is over
        do{
            // get current time
            time(&currentTime);
            // calculate elapsed time
            elapsedTime = difftime(currentTime, initialTime);

            // check terminate signal while waiting, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        } while (elapsedTime <= waitTime);

        // consumption time

        // check if mutex2 lock is available if it is acquire it if not wait until it is free
        if((*cart).mutex2 == 0){
            printf("Error: mutex2 lock is unavailable, consumer (house) needs to wait until Helium Harry (producer) has finished "
                   "making balloon houses before one can be purchased)...\n\n");
        }

        // check if the buffer is empty before consuming another balloon from the buffer, otherwise wait until
        // there are more balloons in the buffer
        if((*cart).empty2 == boundedBufferSize){
            printf("Error: Helium Harry's (Producer) cart (boundedBuffer2) is empty, consumer can't buy a balloon house until Harry makes more ...\n\n");
        }

        // wait until mutex2 is free and there is more balloons in the bounded buffer
        while(((*cart).mutex2 == 0) || ((*cart).empty2 == boundedBufferSize)){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }
        // acquire lock after waiting
        (*cart).mutex2 = 0;


        // implementing a "psuedo string builder" (3000 characters) so everything is printed at once and not jumbled up
        char strBuilder[strBuilderSize];
        //clearing memory to string termination characters so it will not print empty spots
        memset(strBuilder, '\0', sizeof strBuilder);

        // temporary string
        char tempStr[tempStrSize];
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // store message then concatenate into strBuilder
        sprintf(tempStr,"Consumer (house) is currently purchasing a balloon house...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // store info about semaphores into temp then concatenate into strBuilder
        sprintf(tempStr,"Current full slots = %d; current empty slots = %d\n", (*cart).full2, (*cart).empty2);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // removing a balloon animal at index = bounded buffer max index - empty
        (*cart).boundedBuffer2[(boundedBufferSize-1)-(*cart).empty2] = 0;
        // decrement full and increment empty
        (*cart).full2--;
        (*cart).empty2++;

        // print message
        sprintf(tempStr,"Consumer (house) is done buying a Balloon house and has removed it from Harry's cart (boundedBuffer2)...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // print info about semaphores
        sprintf(tempStr,"Updated full slots = %d; Updated empty slots = %d\n\n", (*cart).full2, (*cart).empty2);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // assign strBuilder address to structure
        (*cart).strBuilder2 = strBuilder;

        // signal the print buffer function thread
        (*cart).printStatus2 = 1;

        // wait until the print is done before finishing
        while((*cart).printStatus2 != 0){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }

        //printing the string builder
        printf("%s",strBuilder);

        // removing the string builder from the cart
        (*cart).strBuilder2 = NULL;

        // release lock
        (*cart).mutex2 = 1;
    }
}



/**
 * Thread function for a consumer (hybrid) that buys both balloon houses and balloon animals, this function take balloon houses
 * from Harry's balloon cart (bounded buffer 2) and from Bob's balloon cart (bounded buffer 1). Since a customer can
 * only buy from one cart at a time (there are 3 queues of customers), the customer in this case must buy one balloon first,
 * then get back in line and buy the second balloon.
 * @param arg: pointer to the balloon brigade structure
 * @return: Null
 */
void* buyBoth(void* arg){

    // creating a pointer for the balloon brigade structure
    struct BalloonBrigade *cart = (struct BalloonBrigade*) arg;

    // loop infinitely
    while(1) {
        // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
        if ((*cart).terminate == 1) {
            pthread_exit(0);
        }

        // generate a random time that the consumer will have to wait before iterating again
        int waitTime = genRanNum(5,15);
        // print wait time
        printf("Consumer (hybrid) will buy a balloon animal and balloon house in %d seconds...\n\n", waitTime);

        // wait that amount of time
        // Creating Timer to allow the producer to wait waitTime seconds

        // Variable to hold initial time
        time_t initialTime;
        time(&initialTime);
        // variable to hold current time
        time_t currentTime;
        // variable to hold amount of elapsed time
        double elapsedTime = 0;

        // loop until wait time is over
        do{
            // get current time
            time(&currentTime);
            // calculate elapsed time
            elapsedTime = difftime(currentTime, initialTime);

            // check terminate signal while waiting, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        } while (elapsedTime <= waitTime);

        // consumption time for an animal balloon

        // check if mutex1 lock is available if it is acquire it if not wait until it is free
        if((*cart).mutex1 == 0){
            printf("Error: mutex1 lock is unavailable, consumer (hybrid) needs to wait until Balloon Bob (producer) has finished "
                   "making balloon animals before one can be purchased)...\n\n");
        }

        // check if the buffer is empty before consuming another balloon from the buffer, otherwise wait until
        // there are more balloons in the buffer
        if((*cart).empty1 == boundedBufferSize){
            printf("Error: Balloon Bob's (Producer) cart (boundedBuffer1) is empty, consumer (hybrid) can't buy a balloon animal until Bob makes more ...\n\n");
        }

        // wait until mutex1 is free and there is more balloons in the bounded buffer
        while(((*cart).mutex1 == 0) || ((*cart).empty1 == boundedBufferSize)){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }
        // acquire lock after waiting
        (*cart).mutex1 = 0;


        // implementing a "psuedo string builder" (3000 characters) so everything is printed at once and not jumbled up
        char strBuilder[strBuilderSize];
        //clearing memory to string termination characters so it will not print empty spots
        memset(strBuilder, '\0', sizeof strBuilder);

        // temporary string
        char tempStr[tempStrSize];
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // store message then concatenate into strBuilder
        sprintf(tempStr,"Consumer (hybrid) is currently purchasing a balloon animal...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // store info about semaphores into temp then concatenate into strBuilder
        sprintf(tempStr,"From Bob's Balloon Cart (boundedBuffer1) Current full slots (Bob's cart/bounded buffer 1) = %d; current empty slots = %d\n", (*cart).full1, (*cart).empty1);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // removing a balloon animal at index = bounded buffer max index - empty
        (*cart).boundedBuffer1[(boundedBufferSize-1)-(*cart).empty1] = 0;
        // decrement full and increment empty
        (*cart).full1--;
        (*cart).empty1++;

        // print message
        sprintf(tempStr,"Consumer (hybrid) is done buying a Balloon animal and has removed it from Bob's cart (boundedBuffer1)...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // print info about semaphores
        sprintf(tempStr,"From Bob's Balloon Cart (boundedBuffer1) Updated full slots = %d; Updated empty slots = %d\n\n", (*cart).full1, (*cart).empty1);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // assign strBuilder address to structure
        (*cart).strBuilder1 = strBuilder;

        // signal the print buffer function thread
        (*cart).printStatus1 = 1;

        // wait until the print is done before finishing
        while((*cart).printStatus1 != 0){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }

        // removing the string builder from the cart
        (*cart).strBuilder1 = NULL;

        // release lock
        (*cart).mutex1 = 1;


        // consumption time for a balloon house

        // check if mutex2 lock is available if it is acquire it if not wait until it is free
        if((*cart).mutex2 == 0){
            printf("Error: mutex2 lock is unavailable, consumer (hybrid) needs to wait until Helium Harry (producer) has finished "
                   "making balloon houses before one can be purchased)...\n\n");
        }

        // check if the buffer is empty before consuming another balloon from the buffer, otherwise wait until
        // there are more balloons in the buffer
        if((*cart).empty2 == boundedBufferSize){
            printf("Error: Helium Harry's (Producer) cart (boundedBuffer2) is empty, consumer (hybrid) can't buy a balloon house until Harry makes more ...\n\n");
        }

        // wait until mutex2 is free and there is more balloons in the bounded buffer
        while(((*cart).mutex2 == 0) || ((*cart).empty2 == boundedBufferSize)){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }
        // acquire lock after waiting
        (*cart).mutex2 = 0;

        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // store message then concatenate into strBuilder
        sprintf(tempStr,"now consumer (hybrid) is currently purchasing a balloon house...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // store info about semaphores into temp then concatenate into strBuilder
        sprintf(tempStr,"From Harry's balloon cart (boundedBuffer2) Current full slots = %d; current empty slots = %d\n", (*cart).full2, (*cart).empty2);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // removing a balloon animal at index = bounded buffer max index - empty
        (*cart).boundedBuffer2[(boundedBufferSize-1)-(*cart).empty2] = 0;
        // decrement full and increment empty
        (*cart).full2--;
        (*cart).empty2++;

        // print message
        sprintf(tempStr,"Consumer (hybrid) is done buying a Balloon house and has removed it from Harry's cart (boundedBuffer2)...\n");
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);
        // print info about semaphores
        sprintf(tempStr,"From Harry's balloon cart (boundedBuffer2) Updated full slots = %d; Updated empty slots = %d\n\n", (*cart).full2, (*cart).empty2);
        strcat(strBuilder,tempStr);
        //clearing memory to string termination characters so it will not concatenate empty spots
        memset(tempStr, '\0', sizeof tempStr);

        // assign strBuilder address to structure
        (*cart).strBuilder2 = strBuilder;

        // signal the print buffer function thread
        (*cart).printStatus2 = 1;

        // wait until the print is done before finishing
        while((*cart).printStatus2 != 0){
            sleep(1);
            // check terminate signal, if == 1 then the program max run time is reached so have the thread exit
            if ((*cart).terminate == 1) {
                pthread_exit(0);
            }
        }

        //printing the string builder
        printf("%s",strBuilder);

        // removing the string builder from the cart
        (*cart).strBuilder2 = NULL;

        // release lock
        (*cart).mutex2 = 1;
    }
}



/**
 * Main function to control Producer Consumer behaviour for part 2 of section 2
 * @return: 0 upon successful completion of the program
 */
int main() {
    // time seed for srand
    time_t seedTime;
    // seed srand with current system time
    srand((unsigned)time(&seedTime));

    // initialize the balloon brigade data structure - we will call it cart
    struct BalloonBrigade cart;

    // set terminate variable to 0 (not terminate)
    cart.terminate = 0;

    // set the print variables status to 0 initially
    cart.printStatus1 = 0;
    cart.printStatus2 = 0;

    // set both mutex's to 1 for available
    cart.mutex1 = 1;
    cart.mutex2 = 1;

    // set both full to 0, as initially the bounded buffers will have no full slots
    cart.full1 = 0;
    cart.full2 = 0;

    // set both empty to the size of the bounded buffer, as initially the bounded buffers will be completely empty
    cart.empty1 = boundedBufferSize;
    cart.empty2 = boundedBufferSize;

    // initialize both bounded buffers to contain all 0's with memset
    memset(cart.boundedBuffer1, 0, sizeof cart.boundedBuffer1);
    memset(cart.boundedBuffer2, 0, sizeof cart.boundedBuffer2);


    // Variables to keep track of the threads

    // threads for: producers  - Balloon Bob and helium harry, consumers - buyBalloonAnimal and buyBalloonHouse and buyBoth,
    //              and print buffer info
    pthread_t BalloonBobThread, HeliumHarryThread, buyBalloonAnimalThread, buyBalloonHouseThread, buyBothThread, printBufferThread;

    // print simulation start message
    printf("Simulation Start, welcome to the Balloon Brigade!\n\n");

    //create the threads
    pthread_create(&printBufferThread, NULL, printBuffer, &cart);
    pthread_create(&BalloonBobThread, NULL, BalloonBob, &cart);
    pthread_create(&buyBalloonAnimalThread, NULL, buyBalloonAnimal, &cart);
    pthread_create(&HeliumHarryThread, NULL, HeliumHarry, &cart);
    pthread_create(&buyBalloonHouseThread, NULL, buyBalloonHouse, &cart);
    pthread_create(&buyBothThread, NULL, buyBoth, &cart);

    // Creating Timer to allow program to run for ~45 seconds

    // Variable to hold initialization time (start time of thread processing)
    time_t initialTime;
    time(&initialTime);
    // variable to hold current time
    time_t currentTime;
    // variable to hold amount of elapsed time
    double elapsedTime;

    // keep checking to see if we have reached the max running time of the program yet with a do while loop
    do{
        // get current time
        time(&currentTime);
        // calculate elapsed time
        elapsedTime = difftime(currentTime, initialTime);
    } while (elapsedTime <= maxRunningTime);

    // Execution will only reach this point when the do while loop ends (elapsed time >= maxRunningTime of the program)
    // so at this point the max running time of the program has been reached so call the threads to terminate via the
    // termination variable
    cart.terminate = 1;

    // wait for threads to complete
    pthread_join(printBufferThread, NULL);
    pthread_join(BalloonBobThread, NULL);
    pthread_join(buyBalloonAnimalThread, NULL);
    pthread_join(HeliumHarryThread, NULL);
    pthread_join(buyBalloonHouseThread, NULL);
    pthread_join(buyBothThread,NULL);

    printf("Simulation time reached, program exiting...\n");
    return 0;
}
