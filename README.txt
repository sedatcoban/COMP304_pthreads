COMP 304 
Spring 2021
Project 2
Sedat Çoban

Part I
In this part, first, the moderator thread and the commentor threads are initialized in the main function with ask and request methods respectively.
A mutex variable called count_mutex is initialized to allow the threads of commentors and moderator to work one after another.
A counting semaphore is initialized to keep track of the commentors that will make a request.
Condition variables are also initialized. Question condition is used to signal that a moderator has asked a question to the commentors.
Answers_finished condition is used to signal that all commentors have finished speaking.
Finally, firstCommentor condition is used to signal that a commentor is ready for a question.
A queue is implemented to store the commentor ids that will give an answer to the question.
In the ask method the moderator thread iterates over each question. At the begining all previous question conditions are destroyed
and reinitialized for the new question. Semaphore count is increased for each commentor. The moderator thread waits until the first commentor
is ready for a question (signal of firstCommentor condition). When this signal is obtained, the moderator starts signalling question condition
until all commentors have requested an answer with prob p. Then it waits for all the commentors to finish answering with the condition answers_finished.
When this condition is obtained, moderator thread moves to the next question.
In the request method, firstCommentor is signalled to show that a commentor is ready for question. Then, if the count of the semaphore
is not 0 meaning that there exists commentors that have not make a request yet, the commentor thread waits for a question. 
When the question signal comes, the count of semaphore is decreased and the thread is added to the queue with probability p.
When the count decreases to zero meaning that all commentors have made a request with probability p, the thread at the front of
the queue speaks by calling pthread_sleep. Then the thread id is removed from the queue. 
When everybody in the queue spoke, moderator can start to ask the next question. This condition is signalled with the answers_finished variable.
Mutex locks are used appropriately in ask and request methods to ensure that each commentor and the moderator do not interrupt each other.

Part II
We could not implement this part.

Logging
At the main, the current time is taken with the gettimeofday() method. Then current second and microsecond are obtained and stored in the global variables
to indicate the starting second and microsecond. For each speach, before calling the pthread_sleep function, we get the current time with gettimeofday()
function. From the currenttime, we get the second and the microsecond. By using globally stored start second and microsecond, we calculated the second and
millisecond differences from the current time and the start time. After the speach, we used the same notion to calculate the second and millisecond differences.


Logs of a sample run with parameters: -n 4 -p 0.75 -q 5 -t 3 -b 0.05:
Moderator asks question 1
Commentator #3 generates answer, position in queue: 0
[00:00.001] Commentator #3's turn to speak for 2.735 seconds
[00:02.736] Commentator #3 finished speaking
Moderator asks question 2
Commentator #2 generates answer, position in queue: 0
Commentator #4 generates answer, position in queue: 1
Commentator #3 generates answer, position in queue: 2
[00:02.739] Commentator #2's turn to speak for 1.662 seconds
[00:04.402] Commentator #2 finished speaking
[00:04.402] Commentator #4's turn to speak for 1.432 seconds
[00:05.838] Commentator #4 finished speaking
[00:05.839] Commentator #3's turn to speak for 1.887 seconds
[00:07.726] Commentator #3 finished speaking
Moderator asks question 3
Commentator #2 generates answer, position in queue: 0
Commentator #3 generates answer, position in queue: 1
[00:07.729] Commentator #2's turn to speak for 1.907 seconds
[00:09.637] Commentator #2 finished speaking
[00:09.638] Commentator #3's turn to speak for 2.152 seconds
[00:11.795] Commentator #3 finished speaking
Moderator asks question 4
Commentator #3 generates answer, position in queue: 0
Commentator #1 generates answer, position in queue: 1
Commentator #4 generates answer, position in queue: 2
Commentator #2 generates answer, position in queue: 3
[00:11.798] Commentator #3's turn to speak for 0.412 seconds
[00:12.211] Commentator #3 finished speaking
[00:12.212] Commentator #1's turn to speak for 2.413 seconds
[00:14.625] Commentator #1 finished speaking
[00:14.626] Commentator #4's turn to speak for 0.470 seconds
[00:15.098] Commentator #4 finished speaking
[00:15.098] Commentator #2's turn to speak for 1.203 seconds
[00:16.302] Commentator #2 finished speaking
Moderator asks question 5
Commentator #3 generates answer, position in queue: 0
Commentator #2 generates answer, position in queue: 1
Commentator #1 generates answer, position in queue: 2
[00:16.305] Commentator #3's turn to speak for 1.539 seconds
[00:17.845] Commentator #3 finished speaking
[00:17.845] Commentator #2's turn to speak for 2.517 seconds
[00:20.363] Commentator #2 finished speaking
[00:20.366] Commentator #1's turn to speak for 1.838 seconds
[00:22.218] Commentator #1 finished speaking


