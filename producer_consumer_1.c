#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * CPSC 457 Assignment 1
 * Section 2: Part 1
 *
 * Ryan Loi
 * Lecture: 02
 * Tutorial: T10
 */

/**
 * Function to print the main menu that allows the user to interact with this program.
 * Menu option 1: will cause the producer (Balloon Bob) to Produce a balloon animal and put it into a cart (bounded buffer).
 * Menu option 2: will cause the consumer (a customer) to buy a balloon animal from the cart (bounded buffer).
 * Menu option 3: will display more information about how many balloon animals are currently in the cart (bounded buffer)
 *                and what position they are in (in regards to the position in the bounded buffer), it will also display some
 *                information on how many full slots and empty slots there are left in the bounded buffer, and finally
 *                display if the mutex is available.
 * Menu option 4: exit the program
 */
void printMenu(){
    printf("1) Press 1 for Producer (make Balloon Bob Produce a Balloon animal and put it in the cart\n");
    printf("2) Press 2 for Consumer (make a customer purchase a Balloon animal and from the cart\n");
    printf("3) Press 3 for Information (Buffer information, slot information, and mutex status\n");
    printf("4) Press 4 to Exit the program\n");
}


/**
 * Producer function, whenever called the producer will create another balloon and put it into the bounded buffer at index
 * full. If the bounded buffer is full producer will print an error and go back to the user menu, if mutex lock is unavailable
 * then the producer will have to wait (so it goes back to the main menu).
 * @param mutex: lock controlling if the producer can produce to the bounded buffer
 * @param full: semaphore that indicates how many full slots the bounded buffer has and can be used as the index to
 *              place a balloon inside of the buffer.
 * @param empty: sempahore that indicates how many free slots the bounded buffer has left
 * @param boundedBuffer: bounded buffer (int array) where producer will place balloons
 * @param boundedBufferSize: size of the bounded buffer
 */
void producer(int *mutex, int *full, int *empty, int boundedBuffer[], int boundedBufferSize){
    // check if mutex lock is available if it is acquire it if not wait (go back to menu - and this shouldn't ever happen
    // on a suer controlled system)
    if (*mutex == 1){
        // acquire lock
        *mutex = 0;

        // check if the buffer is full before producing another balloon to be put into the buffer
        if(*full == boundedBufferSize){
            printf("Error: Bounded Buffer is full, no more Balloons can be produced until a consumer buys one ("
                   "Balloon Bob's cart is full, he needs to wait until a customer buys a balloon animal before making more)...\n\n");

        }else if (*full < boundedBufferSize){
            // print message
            printf("Producer is producing (Balloon Bob is currently making a balloon animal)...\n");
            // print info about semaphores
            printf("Current full slots = %d; current empty slots = %d\n", *full, *empty);

            // creating a new balloon to put into the buffer at index indicated by full
            boundedBuffer[*full] = 1;
            // increment full and decrement empty
            (*full)++;
            (*empty)--;

            // print message
            printf("Producer has produced (Balloon Bob is done making a Balloon animal)...\n");
            // print info about semaphores
            printf("Updated full slots = %d; Updated empty slots = %d\n\n", *full, *empty);
        }
        // release lock
        *mutex = 1;

    }else if (*mutex == 0){
        printf("Error: mutex lock is unavailable, producer needs to wait until consumer is done (Balloon Bob needs"
               " to wait until the customer has finished taking a balloon out of the cart before he can put another one in)...\n\n");
    }
}


/**
 * Consumer function, whenever called the consumer will consume another balloon at index = max index of bounded array - empty.
 * If the bounded buffer is empty consumer will print an error and go back to the user menu, if mutex lock is unavailable
 * then the consumer will have to wait (so it goes back to the main menu).
 * @param mutex: lock controlling if the consumer can consume from the bounded buffer
 * @param full: semaphore that indicates how many full slots the bounded buffer has
 * @param empty: semaphore that indicates how many empty slots the bounded buffer has and can be used to determine what
 *               index the consumer will consume from.
 * @param boundedBuffer: bounded buffer (int array) where consumer will consume balloons from
 * @param boundedBufferSize: size of the bounded buffer
 */
void consumer(int *mutex, int *full, int *empty, int boundedBuffer[], int boundedBufferSize){
    // check if mutex lock is available if it is acquire it if not wait (go back to menu - and this shouldn't ever happen
    // on a suer controlled system)
    if (*mutex == 1){
        // acquire lock
        *mutex = 0;

        // check if the buffer is empty before consuming a balloon animal
        if(*empty == boundedBufferSize){
            printf("Error: Bounded Buffer is empty, no more Balloons can be consumed until a producer makes another one ("
                   "Balloon Bob's cart is empty, he needs make more balloon animals before the customer can buy more)...\n\n");

        }else if (*empty < boundedBufferSize){
            // print message
            printf("Consumer is consuming (Customer is currently about to buy a balloon animal)...\n");
            // print info about semaphores
            printf("Current full slots = %d; current empty slots = %d\n", *full, *empty);

            // removing a balloon animal at index = bounded buffer max index - empty
            boundedBuffer[(boundedBufferSize - 1) - *empty] = 0;
            // decrement full and increment empty
            (*full)--;
            (*empty)++;

            // print message
            printf("Consumer has consumed (Customer has taken a balloon animal out of the basket and purchased it)...\n\n");
            // print info about semaphores
            printf("Updated full slots = %d; Updated empty slots = %d\n\n", *full, *empty);
        }
        // release lock
        *mutex = 1;

    }else if (*mutex == 0){
        printf("Error: mutex lock is unavailable, consumer needs to wait until producer is done (Customer needs to"
               " wait until Balloon Bob is done putting a balloon animal into the cart before one can be bought)...\n\n");
    }
}

/**
 * Function to display the contents of the bounded buffer at each specific index, the number of full slots available and
 * the number of empty slots available, and if the mutex is available or not.
 * @param mutex: lock for the bounded buffer to allow allow single access for either the producer or consumer at any given
 *               time (lock only allows either one producer or one consumer to access the bounded buffer at a time)
 * @param full: indicates how many slots are full in the bounded buffer
 * @param empty: indicates how many slots are empty in the bounded buffer
 * @param boundedBuffer: the bounded buffer
 * @param boundedBufferSize: size of the bounded buffer
 */
void information(int *mutex, int *full, int *empty, int boundedBuffer[], int boundedBufferSize){

    // loop through the entire bounded buffer and print it contents at its indices (0 = empty, 1 = full)
    for(int i = 0; i < boundedBufferSize; i++){
        printf("[%d] -> %d ;;; ", i, boundedBuffer[i]);
    }

    // print slot information (how many full and empty slots)
    printf("Number of Full Slots = %d and Number of Empty slots = %d ;;;", *full, *empty);

    // print if mutex is available or not (mutex = 1 = available, mutex = 0 = unavailable)
    if(*mutex == 1){
        printf(" Mutex is available\n\n");

    }else if (*mutex == 0){
        printf(" Mutex is unavailable\n\n");
    }

}

/**
 * Main function to control Producer Consumer behaviour for part 1 of section 2
 * @return: returns 0 upon successful execution
 */
int main() {
    // defining the bounded buffer size to be 10
    #define boundedBufferSize 10

    // initialize a mutex lock (it is a binary semaphore so it can only be 0 = unavailable or 1 = available), it will
    // initially be set to available
    int mutex = 1;

    // initialize a counting semaphore to count full slots, it will start at 0 because the bounded buffer is initially empty
    int full = 0;

    // initialize a counting semaphore to count empty slots, it will start at 10 because the bounded buffer is initially empty
    int empty = 10;

    // creating our bounded buffer of size 10 initialized to all 0
    int boundedBuffer[boundedBufferSize] = {0};

    // variable to hold user's input as a string, clear memory buffer for userInput to 0 as well
    char strUserInput [3];
    memset(strUserInput, 0, sizeof (strUserInput));

    // variable to hold user's input once converted to an int
    int intUserInput;

    // loop until user enters menu option 4 to exit the program
    do {
        // print main menu
        printMenu();

        // print user choice message
        printf("Please enter your menu choice: ");

        // get user input by scanning only 2 bytes
        scanf("%2s",strUserInput);
        // cleaning up standard input, first command scans everything up to a newline and discards it, second command
        // discards newline
        scanf("%*[^\n]");
        scanf("%*c");

        // convert string input to integer
        intUserInput = atoi(strUserInput);
        // print space to terminal
        printf("\n");

        // check user's input
        // if user input == 1 then begin producer process
        if(intUserInput == 1){
            // call producer function
            producer(&mutex, &full, &empty, boundedBuffer, boundedBufferSize);

         // if user input == 2 then begin consumer process
        }else if (intUserInput == 2){
            // call the consumer function
            consumer(&mutex, &full, &empty, boundedBuffer, boundedBufferSize);

        // if user input == 3 then begin displaying information
        }else if (intUserInput == 3){
            // call information function
            information(&mutex, &full, &empty, boundedBuffer, boundedBufferSize);

         // if user input == 4 then print exiting program
        }else if (intUserInput == 4){
            printf("Exiting program...\n");

            // otherwise print invalid input and then reloop
        } else{
            printf("Error Invalid input!\n\n");
        }


    } while (intUserInput!=4);


    return 0;
}
