
#include "Shop.h"

// Modified
void Shop_org::init() 
{
   pthread_mutex_init(&mutex_, NULL);
   pthread_cond_init(&cond_customers_waiting_, NULL);
   pthread_cond_init(&cond_customer_served_, NULL);
   pthread_cond_init(&cond_barber_paid_, NULL);
   pthread_cond_init(&cond_barber_sleeping_, NULL);

   for (int i = 0; i < max_barbers_; i++) {
      barbers_.push(i);
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
   if (customer_in_chair_ != 0 || !waiting_chairs_.empty()) {
      waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(&cond_customers_waiting_, &mutex_);
      waiting_chairs_.pop();
   }

   // here
   print(id, "moves to a service chair [" + int2string(customer_in_barber) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   customer_in_chair_ = id;
   in_service_ = true;

   // wake up the barber just in case if he is sleeping
   pthread_cond_signal(&cond_barber_sleeping_);

   pthread_mutex_unlock(&mutex_); 
   return customer_in_barber;
}

// Modify
void Shop_org::leaveShop(int customer_id_, int barber_id_) 
{
   pthread_mutex_lock( &mutex_ );

   // Wait for service to be completed
   print(customer_id_, "wait for barber[" + int2string(barber_id_) + "] the hair-cut to be done");
   while (in_service_ == true)
   {
      pthread_cond_wait(&cond_customer_served_, &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   money_paid_ = true;
   pthread_cond_signal(&cond_barber_paid_);
   print(customer_id_, "says good-bye to the barber[" + int2string(barber_id_) + "]." );
   pthread_mutex_unlock(&mutex_);
}

// Modify
void Shop_org::helloCustomer(int id) 
{
   pthread_mutex_lock(&mutex_);
   
   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && customer_in_chair_ == 0 ) 
   {
      print(-id, "sleeps because of no customers.");
      pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }

   if (customer_in_chair_ == 0)               // check if the customer, sit down.
   {
       pthread_cond_wait(&cond_barber_sleeping_, &mutex_);
   }
   customer_in_barber = id;
   print(-id, "starts a hair-cut service for customer[" + int2string(customer_in_chair_) + "]");
   pthread_mutex_unlock( &mutex_ );
}

// Modify
void Shop_org::byeCustomer(int id) 
{
  pthread_mutex_lock(&mutex_);

  // Hair Cut-Service is done so signal customer and wait for payment
  in_service_ = false;
  print(-id, "says he's done with a hair-cut service for customer[" + int2string(customer_in_chair_)+ "]");
  money_paid_ = false;
  pthread_cond_signal(&cond_customer_served_);
  while (money_paid_ == false)
  {
      pthread_cond_wait(&cond_barber_paid_, &mutex_);
  }
  customer_in_chair_ = 0;
  print(-id, "calls in another customer");
  pthread_cond_signal( &cond_customers_waiting_ );

  pthread_mutex_unlock( &mutex_ );  // unlock
}
