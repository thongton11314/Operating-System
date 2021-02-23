
#include "Shop.h"

// Modified
void Shop_org::init() 
{
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   pthread_cond_init(&cond_customer_served_, NULL);
   pthread_cond_init(&cond_barber_paid_, NULL);
   pthread_cond_init(&cond_barber_sleeping_, NULL);

   for(int i = 0; i < max_barbers_; i++) {
      barberInfor aBarber {i, 0, false, false};
      barbers_.insert(pair<int, barberInfor>(i, aBarber)); 
   }
}

string Shop_org::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

// Modified
void Shop_org::print(int person, string message)
{
   if (person > 0) {
      cout << "customer[" << person << "]: " << message << endl;
   }
   else {
      cout << "barber  [" << person * (-1) << "]: " << message << endl;
   }
}

int Shop_org::get_cust_drops() const
{
    return cust_drops_;
}

// Modify
int Shop_org::visitShop(int id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      print( id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_;
      pthread_mutex_unlock(&mutex_);
      return -1;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service

   // check if all barbers are busy
   int currentBarBer = -1;
   bool isFull = true;
   for (int i = 0; i < max_barbers_; i++) {
      if (barbers_.at(i).customer_in_chair_ == 0) {
         currentBarBer = i;
         isFull = false;
      }
   }
   if (isFull) {
      if (!waiting_chairs_.empty()) {
         waiting_chairs_.push(id);
         print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
         pthread_cond_wait(&cond_customers_waiting_, &mutex_);
         waiting_chairs_.pop();
      }
   }

   print(id, "moves to a service chair [" + int2string(barbers_.at(currentBarBer).barber_id_) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_ = id;
   in_service_ = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_);

   pthread_mutex_unlock(&mutex_); 
   return currentBarBer;
}

// Modify
void Shop_org::leaveShop(int cusID, int barID) 
{
   pthread_mutex_lock( &mutex_ );

   // Wait for service to be completed
   print(cusID, "wait for barber[" + int2string(barID) + "] the hair-cut to be done");
   while (barbers_.at(barID).in_service_ == true)
   {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   barbers_.at(barID).money_paid_ = true;
   pthread_cond_signal(&cond_barber_paid_);
   print(cusID, "says good-bye to the barber[" + int2string(barID) + "]." );
   pthread_mutex_unlock(&mutex_);
}

// Modify
void Shop_org::helloCustomer(int id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && barbers_.at(id).customer_in_chair_ == 0 ) 
   {
      print(-id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   if (barbers_.at(id).customer_in_chair_ == 0)               // check if the customer, sit down.
   {
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   print(-id, "starts a hair-cut service for customer[" + int2string(barbers_.at(id).customer_in_chair_) + "]");
   pthread_mutex_unlock(&mutex_);
}

// Modify
void Shop_org::byeCustomer(int id) 
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  barbers_.at(id).in_service_ = false;  
  print(-id, "says he's done with a hair-cut service for customer[" + int2string(barbers_.at(id).customer_in_chair_)+ "]");
  barbers_.at(id).money_paid_ = false;
  pthread_cond_signal(&cond_customer_served_);
  while (barbers_.at(id).money_paid_ == false)
  {
      pthread_cond_wait(&cond_barber_paid_, &mutex_);
  }

  barbers_.at(id).customer_in_chair_ = 0;

  print(-id, "calls in another customer");
  pthread_cond_signal(&cond_customers_waiting_);

  pthread_mutex_unlock(&mutex_);  // unlock
}
