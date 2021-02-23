#ifndef SHOP_ORG_H_
#define SHOP_ORG_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
using namespace std;

#define kDefaultNumChairs 3

class Shop_org 
{
public:
   Shop_org(int num_barbers, int num_chairs) : max_barbers_(num_barbers), max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), customer_in_chair_(0),
      in_service_(false), money_paid_(false), cust_drops_(0)
   { 
      init(); 
   };

   Shop_org(int num_chairs) : max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), customer_in_chair_(0),
      in_service_(false), money_paid_(false), cust_drops_(0), max_barbers_(0)
   { 
      init(); 
   };
   Shop_org() : max_waiting_cust_(kDefaultNumChairs), customer_in_chair_(0), in_service_(false),
      money_paid_(false), cust_drops_(0), max_barbers_(0)
   { 
      init(); 
   };

   int visitShop(int id);   // return true only when a customer got a service
   void leaveShop(int customer_id_, int barber_id_);
   void helloCustomer(int id);
   void byeCustomer(int id);
   int get_cust_drops() const;

 private:
   const int max_waiting_cust_; // the max number of threads that can wait
   int customer_in_chair_;      // use to check current customer is in working
   bool in_service_;            
   bool money_paid_;
   queue<int> waiting_chairs_;  // includes the ids of all waiting threads
   int cust_drops_;

   int customer_in_barber = 0;
   const int max_barbers_;
   queue<int> barbers_;         // includes the ids of all waiting barber threads

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;
   pthread_cond_t  cond_customers_waiting_;
   pthread_cond_t  cond_customer_served_;
   pthread_cond_t  cond_barber_paid_;
   pthread_cond_t  cond_barber_sleeping_;

   //static const int barber = 0; // the id of the barber thread
  
   void init();
   string int2string(int i);
   void print(int person, string message);
};
#endif
