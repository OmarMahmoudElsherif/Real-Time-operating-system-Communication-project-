# Real-Time-operating-system-Communication-project                    
 

The project is implemented using FreeRTOS on the target emulation board provided via Eclipse CDT
Embedded.

Three tasks communicate via a queue of fixed size as described below:
 -There are two sender tasks. Each sender task sleeps for a RANDOM period of time Tsender and when it 
  wakes up it sends a message to the queue containing the string “Time is XYZ” where XYZ is current time in 
  system ticks. If the queue is full, the sending operation fails and a counter counting total number of blocked 
  messages is incremented. Upon successful sending, a counter counting total number of transmitted messages 
  is incremented. The sender task is then blocked for another random period again. The random period is 
  drawn from a uniform distribution as specified below.
  
-The receiver task sleeps for another FIXED period of time Treceiver and then wakes up and checks for any 
received message in the queue. If there is a message in the queue, it reads it, increments total number of 
received messages and sleeps again. If there is no message it sleeps again immediately. Note that receiver 
reads one message at a time even if there are more than one message in the queue.
Embedded Systems Project 2022 Page 2 of 2 Revision: 1.0
The sleep/wake control of the three tasks is performed via three timers one for each task. The callback
function for each timer is specified as follows:
Sender Timer Callback Function: When called it releases a dedicated semaphore on which the sender task is
waiting/blocked on. The sender task is then unblocked and can send to the queue.
Receiver Timer Callback Function: When called it releases a dedicated semaphore on which the receiver task
is waiting/blocked on. The receiver task is then unblocked and performs a read on the queue as described
above. When the receiver receives 500 messages, the receiver timer callback function calls the “Reset”
function that performs the following:
1- Print the total number of successfully sent messages and the total number of blocked messages
2- Reset the total number of successfully sent messages, the total number of blocked messages
and received message
3- Clears the queue
4- Configure the values controlling the sender timer period Tsender to the next values in two arrays
specifying the lower and upper bound values of the uniformly distributed timer period. The first array
holds the values {50, 80, 110, 140, 170, 200} and the second holds the values {150, 200, 250, 300, 
350, 400} expressing in msec the timer lower and upper bounds for a uniform distribution. When the 
system starts initially it starts with the values 50 and 150. If all values in the array are used, destroy 
the timers and print a message “Game Over”and stop execution.
5- In all iterations Treceiver is fixed at 100 msec
