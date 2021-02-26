/*


   @author Sabini Ethan
   @date 2/20/20201
   @version 9.3.0
*/
#include "Shop.h"

// initialize
void Shop::init()
{
    // initialize lists
    customers_in_chair_.resize(max_waiting_cust_, 0);
    in_service_.resize(max_waiting_cust_, false);
    money_paid_.resize(max_barbers_, false);

    for (int i = 0; i < max_barbers_; i++)
    {
        barbers_.push_back(i);
    }
    // initialize mutex
    pthread_mutex_init(&mutex_, NULL);

    //initialize customer conditions
    //pthread_cond_init(&cond_customers_waiting_, NULL);
    //pthread_cond_init(&cond_customers_served_, NULL);

    // initialize barber conditions
    for (int i = 0; i < max_barbers_; i++)
    {
        pthread_cond_init(&conds_barbers_paid_[i], NULL);
        pthread_cond_init(&conds_barbers_sleeping_[i], NULL);
    }
    for (int i = 0; i < max_waiting_cust_; i++)
    {
        pthread_cond_init(&cond_customers_waiting_[i], NULL);
        pthread_cond_init(&cond_customers_served_[i], NULL);
    }
}

Shop::~Shop()
{
    // delete mutexes
    pthread_mutex_destroy(&mutex_);

    // delete conditions
    //pthread_cond_destroy(&cond_customers_waiting_);
    //pthread_cond_destroy(&cond_customers_served_);

    // deletes all conditions c
    for (auto c : conds_barbers_paid_)
    {
        pthread_cond_destroy(&c.second);
    }
    for (auto c : conds_barbers_sleeping_)
    {
        pthread_cond_destroy(&c.second);
    }
    for (auto c : cond_customers_waiting_)
    {
        pthread_cond_destroy(&c.second);
    }
    for (auto c : cond_customers_served_)
    {
        pthread_cond_destroy(&c.second);
    }
}

// returns string from int
string Shop::int2string(int i)
{
    stringstream out;
    out << i;
    return out.str();
}

// print person or barber message
void Shop::print(int person, string message, bool barber)
{
    if (barber)
    {
        cout << "barber  [" << person << "]: " << message << endl;
    }
    else
    {
        cout << "customer[" << person << "]: " << message << endl;
    }
}

// get customers
int Shop::get_cust_drops() const
{
    return cust_drops_;
}

// Called by a customer thread. Returns avilable barberID
int Shop::visitShop(int id)
{
    int ret = pthread_mutex_lock(&mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on customer lock: visitShop");
    }

    // If all chairs are full then leave shop
    if (max_waiting_cust_ == waiting_chairs_.size())
    {
        print(id, "leaves the shop because of no available waiting chairs.", false);
        ++cust_drops_;
        ret = pthread_mutex_unlock(&mutex_);
        if (ret != 0)
        {
            exit_on_error(ret, "Error on unlock: visitShop");
        }
        return -1;
    }
    // If someone is being served or transitioning waiting to service chair
    // then take a chair and wait for service
    if (barbers_.empty() && max_waiting_cust_ - waiting_chairs_.size() != 0)
    {
        waiting_chairs_.push_back(id);
        print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()), false);
        while (barbers_.empty())
        {
            ret = pthread_cond_wait(&cond_customers_waiting_[id], &mutex_);
            if (ret != 0)
            {
                exit_on_error(ret, "Error on customer waiting: visitShop");
            }
        }
        waiting_chairs_.pop_front();
    }
    if (barbers_.empty())
    {
        cout << "id " << id << "There are no barbers available" << endl;
        cout << "#chair " << max_waiting_cust_ - waiting_chairs_.size() << endl;
        exit(EXIT_FAILURE);
    }
    int barberID = barbers_.front();
    barbers_.pop_front();
    print(id, "moves to the service chair[" + int2string(barberID) + "], # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()), false);
    customers_in_chair_[barberID] = id;
    in_service_[barberID] = true;

    // wake up the barber just in case if he is sleeping
    ret = pthread_cond_signal(&conds_barbers_sleeping_[barberID]);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber wake: visitShop");
    }
    // leave critical section
    ret = pthread_mutex_unlock(&mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on unlock: visitShop");
    }
    return barberID;
}

// called by customer
void Shop::leaveShop(int customer_id, int barber_id)
{
    int ret = pthread_mutex_lock(&mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on lock: leaveShop");
    }
    customers_in_chair_[barber_id] = customer_id;
    // Wait for service to be completed
    print(customer_id, "wait for barber[" + int2string(barber_id) + "] to be done with hair-cut.", false);
    struct timespec ts;
    struct timeval tv;
    while (in_service_[barber_id] == true)
    {
        int ret = gettimeofday(&tv, NULL);
        if (ret != 0)
        {
            exit_on_error(ret, "Error on get time: helloCustomer");
        }
        ts.tv_sec = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec * 1000;
        ts.tv_sec += 1;
        pthread_cond_timedwait(&cond_customers_served_[customer_id], &mutex_, &ts); // timedwait to keep synchronization
        //pthread_cond_wait(&cond_customers_served_[customer_id], &mutex_);
    }

    // Pay the barber and signal barber appropriately
    money_paid_[barber_id] = true;
    ret = pthread_cond_signal(&conds_barbers_paid_[barber_id]);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber paid: leaveShop");
    }
    print(customer_id, "says good-bye to barber[" + int2string(barber_id) + "]", false);

    ret = pthread_mutex_unlock(&mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on unlock: leaveShop");
    }
}

// called by barber thread with barber id
void Shop::helloCustomer(int id)
{
    int ret = pthread_mutex_lock(&mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on lock: helloCustomer");
    }
    // If no customers than barber can sleep
    if (waiting_chairs_.empty() && customers_in_chair_[id] == 0)
    {
        print(id, "sleeps because of no customers.", true);
        pthread_cond_wait(&conds_barbers_sleeping_[id], &mutex_);
    }
    struct timespec ts;
    struct timeval tv;
    while (customers_in_chair_[id] < 1) // check if the customer, sit down.
    {
        int ret = gettimeofday(&tv, NULL); // used for timed wait
        if (ret != 0)
        {
            exit_on_error(ret, "Error on get time: helloCustomer");
        }
        ts.tv_sec = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec * 1000;
        ts.tv_sec += 1;
        pthread_cond_timedwait(&conds_barbers_sleeping_[id], &mutex_, &ts); // timedwait to keep synchronization
        //pthread_cond_wait(&conds_barbers_sleeping_[id], &mutex_);
    }

    print(id, "starts a hair-cut service for customer [" + int2string(customers_in_chair_[id]) + "]", true);
    pthread_mutex_unlock(&mutex_);
}

// called by barber thread with barber id
void Shop::byeCustomer(int id)
{
    int ret = pthread_mutex_lock(&mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on lock: byeCustomer");
    }

    // Hair Cut-Service is done so signal customer and wait for payment
    in_service_[id] = false;
    print(id, "says he's done with a hair-cut service for customer[" + int2string(customers_in_chair_[id]) + "].", true);
    money_paid_[id] = false;
    int customer_id = customers_in_chair_[id];
    pthread_cond_signal(&cond_customers_served_[customer_id]);
    while (money_paid_[id] == false)
    {
        pthread_cond_wait(&conds_barbers_paid_[id], &mutex_);
    }

    // Signal to customer to get next one
    barbers_.push_back(id);
    customers_in_chair_[id] = 0;
    print(id, "calls in another customer", true);
    //TODO debug
    int newCustomer = waiting_chairs_.front();
    ret = pthread_cond_signal(&cond_customers_waiting_[newCustomer]);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on customer signal: byeCustomer");
    }
    ret = pthread_mutex_unlock(&mutex_); // unlock
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber unlock: byeCustomer");
    }
}

void Shop::exit_on_error(int val, const char *error)
{
    errno = val;
    perror(error);
    exit(EXIT_FAILURE);
}
