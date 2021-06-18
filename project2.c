#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <math.h>

// Project Members:
// Burcu Özer - 64535 
// Sedat Çoban - 60545

// Global Variables
int commentor_number = 0; // to keep track of how many commentors tried to generate answer
int travel = 0; // to keep track of the place of the commentor in the queue
struct Queue* queue; // to keep track of the turn of the commentors
double p; // probability
double b; // for breaking news
int t; // time for speaking
int n; // number of commentors
int q; // number of questions
long* presents; // to keep the ID of the commentor who tried to generate an answer for the question
int presentIndex; // to keep the track of presents array
int questionNumber; // to keep track of how many questions were asked
int start_sec; // to store the start second
int start_mil; // to store the start microsecond
int minute = 0; // to store the start minute

// Mutex
pthread_mutex_t count_mutex; // mutex for all threads

// Counting Semaphore
sem_t countingSemaphore; // counting semaphore to store the number of commentors that will request an answer with probability p

// Conditions
pthread_cond_t question; // condition to signal that a question is asked
pthread_cond_t answers_finished; // condition to signal the moderator that all the commentors have answered
pthread_cond_t firstCommentor; // condition to signal that a commentor will start waiting for a question


// Method for thread to speak (Provided by the TAs)
int pthread_sleep(double seconds){
    pthread_mutex_t mutex;
    pthread_cond_t conditionvar;
    if(pthread_mutex_init(&mutex,NULL)){
        return -1;
    }
    if(pthread_cond_init(&conditionvar,NULL)){
        return -1;
    }

    struct timeval tp;
    struct timespec timetoexpire;
    // When to expire is an absolute time, so get the current time and add
    // it to our delay time
    gettimeofday(&tp, NULL);
    long new_nsec = tp.tv_usec * 1000 + (seconds - (long)seconds) * 1e9;
    timetoexpire.tv_sec = tp.tv_sec + (long)seconds + (new_nsec / (long)1e9);
    timetoexpire.tv_nsec = new_nsec % (long)1e9;

    pthread_mutex_lock(&mutex);
    int res = pthread_cond_timedwait(&conditionvar, &mutex, &timetoexpire);
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&conditionvar);

    //Upon successful completion, a value of zero shall be returned
    return res;
}

// Implementation of Queue //
struct Queue {
    int front, rear, size;
    unsigned capacity;
    long* array;
};

struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
    queue->rear = capacity - 1;
    queue->array = (long*)malloc(queue->capacity * sizeof(long));
    return queue;
}

void enqueue(struct Queue* queue, long thread)
{
    queue->rear = (queue->rear + 1) % queue->capacity;
    queue->array[queue->rear] = thread;
    queue->size = queue->size + 1;
}

long dequeue(struct Queue* queue)
{
    pthread_t thread = queue->array[queue->front];
    queue->front = (queue->front + 1) % queue->capacity;
    queue->size = queue->size - 1;
    return thread;
}

// End of the implementation of Queue //

// Moderator thread's function
void* ask(void* m) {
    questionNumber = 1;
    for (int i = 1; i <= q; i++) { // iterate for each all questions
        pthread_mutex_lock(&count_mutex);
        commentor_number = 0;
        pthread_cond_destroy(&question); // previous question signals are destroyed
        pthread_cond_init(&question, NULL); // new question condition variable is initialized for the new question
        
        for (int j = 0; j < n; j++) { // increase the semaphore count for each commentor
            sem_post(&countingSemaphore);
        }
        
        presents = malloc(sizeof(long) * n);
        presentIndex = 0;
        pthread_cond_wait(&firstCommentor, &count_mutex); // wait for the commentor to be ready for a question
        pthread_mutex_unlock(&count_mutex);
        printf("Moderator asks question %d\n", i);
        
        while (1) { // ask question by signalling question until all commentors have requested an answer with prob p
            pthread_mutex_lock(&count_mutex);
            pthread_cond_signal(&question);
            if (commentor_number == n) {
                pthread_cond_wait(&answers_finished, &count_mutex); // wait until all commentors speak
                questionNumber++; // increases question number to move to the next question
                break;
            }
            pthread_mutex_unlock(&count_mutex);
        }
        
        pthread_mutex_unlock(&count_mutex);
    }
}

// Commentor threads' function
void* request(void* i) {
    while (questionNumber <= q) {
        pthread_mutex_lock(&count_mutex);
        long id = (long)i; // commentor's id
        int c;
        sem_getvalue(&countingSemaphore, &c); // to get how many semaphore we have
        pthread_cond_signal(&firstCommentor); // signal that a commentor is ready for a question
        int contained = 0;
        
        for (int m = 0; m < n; m++) { // This for loop is used to check whether the commentor tried to generate answer or not
            if (presents[m] == id) {
                contained = 1;
            }
        }
        
        if (c != 0 && commentor_number != n && contained == 0) { // If we have enough space in queue, not everybody tried to enter the queue, coming thread can generate an answer

            pthread_cond_wait(&question, &count_mutex); // wait for a question
            sem_wait(&countingSemaphore); // decrease the counting semaphore
            commentor_number++;
            presents[presentIndex] = id;
            presentIndex++;
            double random = (double)rand() / (double)RAND_MAX; // generate double number between 0 and 1
            
            if (random <= p) { //If generated random number is smaller than the probability, commentor can generate an answer
                enqueue(queue, id); //putting ID of the commentor into the global queue
                printf("Commentator #%ld generates answer, position in queue: %d\n",
                    id, travel);
                travel++; 
            }
        }
        pthread_mutex_unlock(&count_mutex);
        pthread_mutex_lock(&count_mutex);
        
        if (c == 0 && queue->size != 0) { // If everybody tried their chance to generate answer for the question, threads can start to speak.
            if (queue->array[queue->front] == id) { // If the ID of the commentor is same with the first member of the queue, thread can actually speak.
      		      struct timeval current_time; 
  	              gettimeofday(&current_time, NULL);
  	              int second=current_time.tv_sec; // to get current seconds
  	              int milisecond= current_time.tv_usec/1000; // to get current microseconds
  	              int constantSec = 60;
  	              int constantMil = 1000;
		      // Converting log time into minute:second.millisecond //
  	              if(start_mil>milisecond){
  	               int remainder = (start_mil-milisecond)/1000 +1;
  	              	second = second -1*remainder;
  	              	milisecond = milisecond + 1000*remainder;
  	              }
  	              
  	              if(start_sec > second){
  	              int remainder2 = (start_sec-second)/60 +1;
  	              	second = second + 60*remainder2;
  	              	minute = minute - remainder2;
  	              }
  	              
  	              minute = minute + (second-start_sec)/60;
  	              second = (second-start_sec)%constantSec + (milisecond-start_mil)/1000;
  	              milisecond =  (milisecond-start_mil)%constantMil;
  	              // Completion of convertion of log time into minute:second.millisecond //
  	              float speak_time = ((float) rand() / RAND_MAX)*t; // generating speaking time between 0 and t.
  	              
		      printf("[%.2d:%.2d.%.3d] Commentator #%ld's turn to speak for %.3f seconds\n",minute,second,milisecond,id,speak_time);
		      
		      pthread_sleep(speak_time); // thread sleeps for speak_time
		      
		      // Converting log time into minute:second.millisecond //
		      gettimeofday(&current_time, NULL);
  	              second=current_time.tv_sec;
  	              milisecond= current_time.tv_usec/1000;
  	              constantSec = 60;
  	              constantMil = 1000;

  	              if(start_mil>milisecond){
  	               int remainder = (start_mil-milisecond)/1000 +1;
  	              	second = second -1*remainder;
  	              	milisecond = milisecond + 1000*remainder;
  	              }
  	              
  	              if(start_sec > second){
  	               int remainder2 = (start_sec-second)/60;
  	              	second = second + 60*remainder2 +1;
  	              	minute = minute - remainder2;
  	              }
  	      	     
  	              minute = minute + (second-start_sec)/60;
  	              second = (second-start_sec)%constantSec + (milisecond-start_mil)/1000;
  	              milisecond =  (milisecond-start_mil)%constantMil;
  	              // Completion of convertion of log time into minute:second.millisecond //
  	              printf("[%.2d:%.2d.%.3d] Commentator #%ld finished speaking\n",minute,second,milisecond,id);
  	              
                      dequeue(queue); // removing thread from the queue
            }
        }
        pthread_mutex_unlock(&count_mutex);
        pthread_mutex_lock(&count_mutex);

        if (queue->size == 0 && c == 0 && commentor_number == n) { // If everybody in the queue spoke, moderator can start to ask the next question
            travel = 0;
            pthread_cond_signal(&answers_finished); // signaling to moderator that everybody found in the queue finished speaking
        }
        pthread_mutex_unlock(&count_mutex);
    }
}



int main(int argc, char* argv[])
{

// Getting Arguments from the user
  if (argc != 11) {
    printf("Arguments are not correct.\n");
    return 1;
  } else {
    for (int i = 1; i < argc; i++) {
      if (strcmp(argv[i], "-n") == 0) {
        n = atoi(argv[i + 1]);
      } else if (strcmp(argv[i], "-p") == 0) {
        p = atof(argv[i + 1]);
      } else if (strcmp(argv[i], "-q") == 0) {
        q = atoi(argv[i + 1]);
      } else if (strcmp(argv[i], "-t") == 0) {
        t = atoi(argv[i + 1]);
      } else if (strcmp(argv[i], "-b") == 0) {
        b = atof(argv[i + 1]);
      }
    }
  }
  
  // Getting the start time of the program and assigning them to global variable
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  start_sec=current_time.tv_sec;
  start_mil=current_time.tv_usec/1000;
    pthread_t threads[n + 1];
    presents = malloc(sizeof(long) * n);
    presentIndex = 0;
    queue = createQueue(n);
    sem_init(&countingSemaphore, 1, 0);
    /* Initialize mutex and condition variable objects */
    pthread_mutex_init(&count_mutex, NULL);
    pthread_cond_init(&question, NULL);
    pthread_cond_init(&answers_finished, NULL);
    pthread_cond_init(&firstCommentor, NULL);

   
    pthread_create(&threads[0], NULL, ask, NULL); // creation of moderator thread
    for (int i = 1; i <= n; i++) {
        pthread_create(&threads[i], NULL, request, (void*)(long)i); // creation of commentor thread
    }


    /* Wait for all threads to complete */
    for (int i = 0; i < n + 1; i++) {
        pthread_join(threads[i], NULL);
    }

    /* Clean up and exit */

    pthread_mutex_destroy(&count_mutex);
    pthread_cond_destroy(&question);
    pthread_cond_destroy(&answers_finished);
    pthread_cond_destroy(&firstCommentor);
    pthread_exit(NULL);
}

