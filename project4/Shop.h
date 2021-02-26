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

#define kDefaultNumChairs 3   // The default number of chair
#define kDefaultBarbers 1     // The default number of barbers = 1

// Classes Prototype
class Barber;
class Customer;

// This function
class Shop_org 
{
public:

   // Constructor
   Shop_org(int num_barbers, int num_chairs, int max_customer) : max_barbers_(num_barbers), max_waiting_cust_((num_chairs > 0 ) ? num_chairs : kDefaultNumChairs), max_customer_(max_customer), cust_drops_(0)
   { 
      init(); 
   };

   // Deconstructor
   ~Shop_org(); 

   // Return true only when a customer got a service
   int visitShop(int id);  

   // Customer leave the shop
   void leaveShop(int customer_id_, int barber_id_);

   // Baber say hello to customer
   void helloCustomer(int id);

   // Barber say bye to customer
   void byeCustomer(int id);

   // Return the amount of customer not get service
   int get_cust_drops() const;

private:
   const int max_waiting_cust_; // The max number of threads that can wait   
   queue<int> waiting_chairs_;  // Includes the ids of all waiting threads
   int cust_drops_;             // The amount of customer not getting service

   // Customer Part
   const int max_customer_;     // Max customer
   map<int, Customer> customer_;// Collection of customer

   // Barber part
   const int max_barbers_;      // Max babers
   map<int, Barber> barbers_;   // Collection of barber
   queue<int> available_barber_;// Collection of available barber

   // Mutexes and condition variables to coordinate threads
   pthread_mutex_t mutex_;

   // Utility functions
   void init();                              // Use to initialize pthread of mutex and barbers
   string int2string(int i);                 // Convert number into string
   void print(int person, string message);   // Use to convert id into barber's or customer string
};

// Barber Class
class Barber {
   public:

      // Constructor
      Barber(const int barber_id_, int customer_in_chair_, bool in_service_, bool money_paid_) :
      barber_id_(barber_id_), customer_in_chair_(customer_in_chair_), in_service_(in_service_), money_paid_(money_paid_) {
         cond_barber_paid_ = new pthread_cond_t();
         cond_barber_sleeping_ = new pthread_cond_t();
      };

      // Initilized pthread condition
      void initPThread() {
         pthread_cond_init(this->cond_barber_paid_, nullptr);
         pthread_cond_init(this->cond_barber_sleeping_, nullptr); 
      };

      // Delete dynamic pthread condition
      void delBarberPThreadCondition() {
         delete this->cond_barber_paid_;
         delete this->cond_barber_sleeping_;
      };

      // Data members
      pthread_cond_t* cond_barber_paid_;
      pthread_cond_t* cond_barber_sleeping_;
      const int barber_id_;
      int customer_in_chair_;
      bool in_service_;            
      bool money_paid_;
};

// Customer Class
class Customer {
   public:

      // Constructor
      Customer(const int customer_id) : customer_id_(customer_id) {
         cond_customers_waiting_ = new pthread_cond_t();
         cond_customer_served_ = new pthread_cond_t();
      };

      // Initilized pthread condition
      void initPThread() {
         pthread_cond_init(this->cond_customers_waiting_, nullptr);
         pthread_cond_init(this->cond_customer_served_, nullptr);
      };

      // Delete dynamic pthread condition
      void delCustomerPThreadCondition() {
         delete this->cond_customer_served_;
         delete this->cond_customers_waiting_;
      };

      // Data members
      const int customer_id_;
      pthread_cond_t* cond_customers_waiting_;
      pthread_cond_t* cond_customer_served_;
};
#endif