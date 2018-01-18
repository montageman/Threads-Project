//Phillip Yellott
//Project 2

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>
#include <queue>

//declation of all semaphores to be used
sem_t max_customers;
sem_t scale;
sem_t customer_ready;
sem_t worker_ready;
sem_t MUTEX1;
sem_t MUTEX2;
sem_t MUTEX3;
sem_t finished[50];


//global ints
int count = 0;
int finishedcount = 0;
int postalworkernumber = 0;

//these are named constants for the total number of 
//customers and workers to be created
int customernumber = 50;
int workernumber = 3;

//global queue
std::queue<int> gqueue;


//---------------------------------------------------------------------------
void *customer(void *)
{
    int customerid;
    int task;
    
    //safely create each customer, without possibility of interuption
    sem_wait (&MUTEX1);

    customerid = count;
    count++;
    task = rand() % 3; //random number generation between 0 and 2
    
    sem_wait (&MUTEX3);
    std::cout << "Customer " << count <<  " created" <<std::endl;
    sem_post (&MUTEX3);

    sem_post (&MUTEX1);

    //see if customer can enter the post office
    sem_wait (&max_customers);   

    sem_wait (&MUTEX3);
    std::cout << "Customer " << customerid <<  " enters the post office" <<std::endl;
    sem_post (&MUTEX3);

    //Here, The customer puts their information into the queue 
    //to be processed by the next available postal worker.
    sem_wait (&MUTEX2);

    gqueue.push (customerid);
    gqueue.push (task);
    sem_post(&customer_ready);
    
    sem_post (&MUTEX2);


    sem_wait (&finished[customerid]);
    
    sem_wait (&MUTEX3);
    std::cout << "Customer " << customerid <<  " finished ";
      
      //conversation switch
      switch (task)
      {
         
	 case 0: std::cout << "buying stamps"<<std::endl;       
		 break;

	 case 1: std::cout << "mailing a letter"<<std::endl;		 
	         break;

         case 2: std::cout << "mailing a package"<<std::endl;
		 break;
      
      }//end switch
    
    sem_post (&MUTEX3);
    sem_post (&max_customers);
    
    //and finally the customer leaves, allowing another customer to enter.
    sem_wait (&MUTEX3);
    std::cout << "Customer " << customerid <<  " leaves the post office" <<std::endl;
    sem_post (&MUTEX3);
}

//---------------------------------------------------------------------------
void *worker(void *)
{
   int customer_id;
   int customer_task;
   int workerid;

   //safely creates all workers, as I did with customers
   sem_wait(&MUTEX1);
   
   workerid = postalworkernumber;
   postalworkernumber++;

   sem_wait (&MUTEX3);
   std::cout << "Postal worker "<< workerid << " created " <<std::endl;
   sem_post (&MUTEX3);

   sem_post(&MUTEX1);

   //finished count is simply the number of completed customers.
   //here, the workers will self terminate once all customers are dealt with.
   while (finishedcount<customernumber)
   {
      sem_wait (&customer_ready);

      sem_wait (&worker_ready);
      
      //here, we get the customers information as posititioned in the queue.
      sem_wait (&MUTEX2);

      customer_id = gqueue.front();
      gqueue.pop(); 
      customer_task = gqueue.front();
      gqueue.pop();
      
      sem_wait (&MUTEX3);
      std::cout<<"Customer" <<customer_id <<" asks Postal worker "<< workerid << " to  ";
      
      //conversation switch
      switch (customer_task)
      {
         
	 case 0: std::cout << "buy stamps"<<std::endl;       
		 break;

	 case 1: std::cout << "mail a letter"<<std::endl;		 
	         break;

         case 2: std::cout << "mail a package"<<std::endl;
		 break;
      
      }//end switch
      sem_post (&MUTEX3);

      sem_post (&MUTEX2);


      //here we no longer need mutual exclusion
      //note: usleep uses microseconds as its argument.
      switch (customer_task)
      {
         
	 case 0: usleep (1000000); //wait 1 second        
		 break;

	 case 1: usleep (1500000); //wait 1.5 seconds
		 break;

         case 2: sem_wait (&scale);
	         
		 sem_wait (&MUTEX3);
		 std::cout <<"Scale in use by Postal Worker " << workerid <<std::endl;
		 sem_post (&MUTEX3);

		 usleep (2000000); //wait 2 seconds
		 sem_post(&scale);
		 break;
      }//end switch
      
      finishedcount++;
      sem_post (&worker_ready);
      
      sem_post (&finished[customer_id]);

   }//end while loop

}

//---------------------------------------------------------------------------
int main()
{
   //i is used as my counter.
   int i;
   
   //innitialize all semaphores
   sem_init(&max_customers, 0, 10);   
   sem_init(&scale, 0, 1);
   sem_init(&customer_ready, 0, 0);
   sem_init(&worker_ready, 0, 3);
   sem_init(&MUTEX1, 0, 1);
   sem_init(&MUTEX2, 0, 1);
   sem_init(&MUTEX3, 0 ,1);

   for (i=0; i<50; i++)
   {
      sem_init(&finished[i], 0, 0);
   }
 

   //declaration of pthread objects
   pthread_t customerthread[customernumber]; 
   pthread_t workerthread[workernumber];

   
   //simple temp for ease of use
   int temp;

   
   //create customers
   for (i=0; i<customernumber;i++)
   {
      temp = pthread_create(&customerthread[i], 0, customer, (void *)i);
   }

      
   //create workers
   for (i=0; i<workernumber; i++)
   {
      temp = pthread_create(&workerthread[i], 0 , worker, (void *)i);
   }


   //joins threads, forcing main to remain open until they are all done
   for (i=0; i<customernumber;i++)
   {
      temp = pthread_join(customerthread[i], NULL);
      sem_wait (&MUTEX3);
      std::cout << "Customer " << i << " joined" <<std::endl;
      sem_post (&MUTEX3);
   }


}//end of main
