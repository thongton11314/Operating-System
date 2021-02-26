#include <iostream>
#include <sys/time.h>
#include <unistd.h>
#include "Shop.h"
using namespace std;

void *barber(void *);
void *customer(void *);


/*
Constraint:
argv[1] : num_barbers : The number of barbers working in your barbershop
argv[2] : num_chairs : The number of chairs available for customers to wait in
argv[3] : num_customers : The number of customers who need a haircut service
argv[4] : service_timeEach : barber’s service time (in μ seconds).

+id is customer
-id is baber
*/

// ThreadParam class
// This class is used as a way to pass more
// than one argument to a thread. 
class ThreadParam
{
public:
    ThreadParam(Shop_org* shop, int id, int service_time) :
        shop(shop), id(id), service_time(service_time) {};
    Shop_org* shop;         
    int id;             
    int service_time;    
};


// Have change in here
int main(int argc, char *argv[]) 
{

    // Read arguments from command line
    // TODO: Validate values
    if (argc != 5)
    {
        cout << "Usage: num_barbers num_chairs num_customers service_time" << endl;
        return -1;
    }
    int num_barbers = atoi(argv[1]);
    int num_chairs = atoi(argv[2]);
    int num_customers = atoi(argv[3]);
    int service_time = atoi(argv[4]);

    // Multiple barber, one shop, many customers
    pthread_t barber_threads[num_barbers];
    pthread_t customer_threads[num_customers];
    Shop_org shop(num_barbers, num_chairs, num_customers);
    //ThreadParam *barber_params[num_barbers];

    // Create multiple Babers
   
    for (int i = 0; i < num_barbers; i++) 
    {
        ThreadParam* barber_param = new ThreadParam(&shop, i, service_time);
        pthread_create(&barber_threads[i], NULL, barber, barber_param);
    }

    // Create multiple Customers
    for (int i = 0; i < num_customers; i++) 
    {
        usleep(rand() % 1000);
        int id = i + 1;
        ThreadParam* customer_param = new ThreadParam(&shop, id, 0);
        pthread_create(&customer_threads[i], NULL, customer, customer_param);
    }

    // Wait for customers to finish
    for (int i = 0; i < num_customers; i++)
    {
        pthread_join(customer_threads[i], NULL);
    }

    // Wait for barber to finish and cancel barber
    for (int i = 0; i < num_barbers; i++) 
    {
        pthread_cancel(barber_threads[i]);
    }

    cout << "# customers who didn't receive a service = " << shop.get_cust_drops() << endl;
    return 0;
}

void *barber(void *arg) 
{
    ThreadParam* barber_param = (ThreadParam*) arg;
    Shop_org& shop = *barber_param->shop;
    int barber_id = barber_param->id;
    int service_time = barber_param->service_time;
    
    barber_param->shop = nullptr;
    delete barber_param;
    
    while(true) 
    {
        shop.helloCustomer(barber_id);
        usleep(service_time);
        shop.byeCustomer(barber_id);
    }
    return nullptr;
}

void *customer(void *arg) 
{
    ThreadParam* customer_param = (ThreadParam*)arg;
    Shop_org& shop = *customer_param->shop;
    int customerId = customer_param->id;

    customer_param->shop = nullptr;
    delete customer_param;

    int barber = -1;
    barber = shop.visitShop(customerId);
    if (barber != -1)
    {
        shop.leaveShop(customerId, barber);
    }
    return nullptr;
}
