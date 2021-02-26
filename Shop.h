/*
   Simulates the sleeping barbers problem with M barbers and N chairs.
   The barbers will serve all the customers they are able to and when all the 
   barbers are busy the customers will take a seat in an available waiting 
   chair. If there are no chairs available the customer will leave the shop. 
   When there are no more customers to serve, all the barbers go to sleep 
   and when a customer enters the shop, the customer will wake a single Barber.

   @author Sabini Ethan
   @date 2/20/20201
   @version 9.3.0
*/

#ifndef SHOP_H_
#define SHOP_H_
#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <deque>
#include <unordered_map>

using namespace std;

#define kDefaultNumChairs 3
#define kDefaultBarbers 1

class Shop
{
public:
   Shop(int num_barbers, int num_chairs) : max_waiting_cust_((num_chairs > 0) ? num_chairs : kDefaultNumChairs),
                                           max_barbers_((num_barbers > 0) ? num_barbers : kDefaultBarbers), cust_drops_(0)
   {
      init();
   };
   Shop() : max_waiting_cust_(kDefaultNumChairs), max_barbers_(kDefaultBarbers), cust_drops_(0)
   {
      init();
   };
   ~Shop();

   int visitShop(int id); // return barber id if customer gets service
   void leaveShop(int customer_id, int barber_id);
   void helloCustomer(int id);
   void byeCustomer(int id);
   int get_cust_drops() const;

private:
   const int max_waiting_cust_;     // the max number of threads that can wait
   const int max_barbers_;          // the max number of barbers that can cut hair
   vector<int> customers_in_chair_; // barberID is index and customerID is value
   vector<bool> in_service_;
   vector<bool> money_paid_;   // list of barbers who have been paid
   deque<int> waiting_chairs_; // includes the ids of all waiting threads
   int cust_drops_;            // number of customers who left while the shop was full
   deque<int> barbers_;        // includes the ids of all barbers

   // Mutexes and condition variables to coordinate threads
   // mutex_ is used in conjuction with all conditional variables
   pthread_mutex_t mutex_;

   // waiting condition for customers
   pthread_cond_t cond_customers_waiting_;

   // hash maps used to map conditions for barbers
   // maps are used so that conditons are individual to the barber
   unordered_map<int, pthread_cond_t> conds_barbers_paid_;
   unordered_map<int, pthread_cond_t> conds_barbers_sleeping_;
   unordered_map<int, pthread_cond_t> cond_customers_served_;

   void init();
   string int2string(int i);
   void print(int person, string message, bool barber);
   void exit_on_error(int val, const char *error);
};
#endif
