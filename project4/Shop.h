#ifndef SHOP_ORG_H_
#define SHOP_ORG_H_
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <string>
#include <queue>
#include <vector>
#include <map>
using namespace std;

#define kDefaultNumChairs 3
#define kDefaultBarbers 1 // the default number of barbers = 1

// Prototype
class Barber;

class Shop_org 
{
public:
   Shop_org(int num_barbers, int num_chairs) : max_barbers_(num_barbers), max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), cust_drops_(0)
   { 
      init(); 
   };

   Shop_org(int num_chairs) : max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), cust_drops_(0), max_barbers_(0)
   { 
      init(); 
   };
   Shop_org() : max_waiting_cust_(kDefaultNumChairs), cust_drops_(0), max_barbers_(0)
   { 
      init(); 
   };
   
   ~Shop_org();

   int visitShop(int id);   // return true only when a customer got a service
   void leaveShop(int customer_id_, int barber_id_);
   void helloCustomer(int id);
   void byeCustomer(int id);
   int get_cust_drops() const;

 private:
   const int max_waiting_cust_; // the max number of threads that can wait   
   queue<int> waiting_chairs_;  // includes the ids of all waiting threads
   int cust_drops_;
   const int max_barbers_;
   map<int, Barber> barbers_;

   // Mutexes and condition variables to coordinate threads
   pthread_mutex_t mutex_;   

   void init();
   string int2string(int i);
   void print(int person, string message);

};

class Barber {
   public:
      Barber(const int barber_id_, int customer_in_chair_, bool in_service_, bool money_paid_) :
      barber_id_(barber_id_), customer_in_chair_(customer_in_chair_), in_service_(in_service_), money_paid_(money_paid_) {}
      pthread_cond_t* cond_customers_waiting_;
      pthread_cond_t* cond_customer_served_;
      pthread_cond_t* cond_barber_paid_;
      pthread_cond_t* cond_barber_sleeping_;
      const int barber_id_;
      int customer_in_chair_;
      bool in_service_;            
      bool money_paid_;
};
#endif
