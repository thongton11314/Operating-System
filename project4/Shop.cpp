
#include "shop.h"

// init
// This function uses to 
//    - Initialize mutex thread
//    - Create the amount of barbers
//       - Initialize conditional thread
//    - Initialize all barbers available
void Shop_org::init() 
{

   // Initialize the main thread
   pthread_mutex_init(&mutex_, nullptr);

   // Create amount of max barbers
   for(int i = 0; i < max_barbers_; i++) {

      // Creat new barber
      Barber* aBarber = new Barber(i, 0, false, false);

      // Insert all barber into map
      barbers_.insert(pair<int, Barber*>(i, aBarber));

      // since this is initialize then all barber are available
      available_barber_.push(i);
   }
}

// ~Shop_org
// This function use to carefully delete Barber and Customer pthread_condition
Shop_org::~Shop_org() {
   for (auto it = barbers_.begin(); it != barbers_.end(); it++)
      delete it->second;
   for (auto it = customers_.begin(); it != customers_.end(); it++)
      delete it->second;
}

// int2string
// This function uses to convert number to string
string Shop_org::int2string(int i) 
{
   stringstream out;
   out << i;
   return out.str( );
}

// print
// This function uses to print out customer or barber
//    - first argument < 0 is barber
//    - first argument > 0 is customer
void Shop_org::print(int person, string message)
{
   if (person > 0) {
      cout << "customer[" << person << "]: " << message << endl;
   }
   else {
      cout << "barber  [" << person * (-1) << "]: " << message << endl;
   }
}

// get_cust_drops
// This function uses to get the amount of customer left the shop
int Shop_org::get_cust_drops() const
{
    return cust_drops_;
}

// visitShop
// This function uses to assign customer to available barber
int Shop_org::visitShop(int id) 
{

   // Start the lock
   pthread_mutex_lock(&mutex_);

   // Create a new customer;
   Customer* aCustomer = new Customer(id);

   // Insert into customer collection
   customers_.insert(pair<int, Customer*>(id, aCustomer));
   
   // If all chairs are full then leave shop
   if (waiting_chairs_.size() == max_waiting_cust_) 
   {
      print( id,"leaves the shop because of no available waiting chairs.");
      ++cust_drops_; // increase by one if customer left the shop
      pthread_mutex_unlock(&mutex_);
      return -1;
   }
   
   // If someone is being served or transitioning waiting to service chair
   // then take a chair and wait for service
   if (available_barber_.empty() || !waiting_chairs_.empty()) {
      waiting_chairs_.push(id);
      print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
      pthread_cond_wait(customers_.at(id)->cond_customer_waiting_, &mutex_);
      waiting_chairs_.pop();
   }

   // Get current working barber
   int curBarber = available_barber_.front();
   print(id, "moves to a service chair [" + int2string(barbers_.at(curBarber)->barber_id_) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
   
   // Make current barber busy
   available_barber_.pop();
   barbers_.at(curBarber)->customer_in_chair_ = id;
   barbers_.at(curBarber)->in_service_ = true;   

   // Wake up the barber just in case if he is sleeping
   pthread_cond_signal(barbers_.at(curBarber)->cond_barber_sleeping_); 

   // Release the lock
   pthread_mutex_unlock(&mutex_);
   return curBarber;
}

// leaveShop
// This function uses to make sure the customer left the shop
void Shop_org::leaveShop(int cusID, int barID) 
{

   // Start the lock
   pthread_mutex_lock(&mutex_);

   // Wait for service to be completed
   print(cusID, "wait for barber[" + int2string(barID) + "] the hair-cut to be done");
   while (barbers_.at(barID)->in_service_ == true)
   {
      pthread_cond_wait(customers_.at(cusID)->cond_customer_served_, &mutex_);
   }
   
   // Pay the barber and signal barber appropriately
   barbers_.at(barID)->money_paid_ = true;
   pthread_cond_signal(barbers_.at(barID)->cond_barber_paid_);
   print(cusID, "says good-bye to the barber[" + int2string(barID) + "]." );
   
   // Release the lock
   pthread_mutex_unlock(&mutex_);
}

// helloCustomer
// This function uses to make barber work if customer in char
void Shop_org::helloCustomer(int id) 
{

   // Start the lock
   pthread_mutex_lock(&mutex_);
   
   // If no customers than barber can sleep
   if (waiting_chairs_.empty() && barbers_.at(id)->customer_in_chair_ == 0) {
      print(-id, "sleeps because of no customers.");
      pthread_cond_wait(barbers_.at(id)->cond_barber_sleeping_, &mutex_);
   }

   // check if the customer, sit down.
   if (barbers_.at(id)->customer_in_chair_ == 0)
      pthread_cond_wait(barbers_.at(id)->cond_barber_sleeping_, &mutex_);

   // Barber was waken up
   if (barbers_.at(id)->customer_in_chair_ != 0)
      print(-id, "starts a hair-cut service for customer[" + int2string(barbers_.at(id)->customer_in_chair_) + "]");

   // Release the lock
   pthread_mutex_unlock(&mutex_);
}


// Bug in here function
// byeCustomer
// This function uses to make barber available after finishing
void Shop_org::byeCustomer(int id) 
{

   // Start the lock
   pthread_mutex_lock(&mutex_);
   
   // Bug here
   print(-id, "says he's done with a hair-cut service for customer[" + int2string(barbers_.at(id)->customer_in_chair_)+ "]");

   // Some how barbers_.at(id)->customer_in_chair_ = 0 before set new one
   barbers_.at(id)->in_service_ = false; 
   pthread_cond_signal(customers_.at(barbers_.at(id)->customer_in_chair_)->cond_customer_served_);
   barbers_.at(id)->money_paid_ = false;

   // Wait for my customer to pay before I take a new one
   while (barbers_.at(id)->money_paid_ == false)
   {
      pthread_cond_wait(barbers_.at(id)->cond_barber_paid_, &mutex_);
   }

   // Set for the next customer
   barbers_.at(id)->customer_in_chair_ = 0;

   print(-id, "calls in another customer");
   available_barber_.push(id);

   // Make sure there is still have customer to signal
   if (!waiting_chairs_.empty()) { 
      pthread_cond_signal(customers_.at(waiting_chairs_.front())->cond_customer_waiting_);
   }

   // Release the lock
   pthread_mutex_unlock(&mutex_);
}
