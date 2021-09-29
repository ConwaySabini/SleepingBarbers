/*


   @author Sabini Ethan
   @date 2/20/20201
   @version 9.3.0
*/
#include "Shop.h"

//TODO debug empty logic does it make sense

// initialize
void Shop::init()
{
    // initialize mutex
    pthread_mutex_init(&barber_mutex_, NULL);
    pthread_mutex_init(&customer_mutex_, NULL);

    //initialize customer conditions
    pthread_cond_init(&cond_customers_waiting_, NULL);
    pthread_cond_init(&cond_customers_served_, NULL);

    // initialize barber conditions
    for (int i = 0; i < max_barbers_; i++)
    {
        pthread_cond_init(&conds_barbers_paid_[i], NULL);
        pthread_cond_init(&conds_barbers_sleeping_[i], NULL);
    }
}

Shop::~Shop()
{
    // delete mutexes
    pthread_mutex_destroy(&mutex_);

    // delete conditions
    pthread_cond_destroy(&cond_customers_waiting_);
    pthread_cond_destroy(&cond_customers_served_);

    // deletes all conditions c
    for (auto c : conds_barbers_paid_)
    {
        pthread_cond_destroy(c.second);
    }
    for (auto c : conds_barbers_sleeping_)
    {
        pthread_cond_destroy(c.second);
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
void Shop::print(int person, string message)
{
    bool isBarber = false;
    if (person < 0)
    {
        isBarber = true;
    }
    cout << ((!isBarber) ? "customer[" : "barber  [") << person << "]: " << message << endl;
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
    if (waiting_chairs_.size() == max_waiting_cust_)
    {
        print(id, "leaves the shop because of no available waiting chairs.");
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
    if (!customers_in_chair_.empty() || !waiting_chairs_.empty()) // TODO debug because list is not actually empty
    {
        waiting_chairs_.push(id);
        print(id, "takes a waiting chair. # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
        ret = pthread_cond_wait(&cond_customers_waiting_, &mutex_);
        if (ret != 0)
        {
            exit_on_error(ret, "Error on customer waiting: visitShop");
        }
        waiting_chairs_.pop();
    }

    if (barbers_.empty())
    {
        cout << "There are no barbers available" << endl;
        exit(EXIT_FAILURE);
    }
    int barberID = this->barbers_.pop_front();
    print(id, "moves to the service chair[" + int2string(barberID) + ". # waiting seats available = " + int2string(max_waiting_cust_ - waiting_chairs_.size()));
    customers_in_chair_[barberID] = id;
    in_service_[id] = true;

    // wake up the barber just in case if he is sleeping
    int ret = pthread_cond_signal(conds_barber_sleeping_[barberID]);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber wake: visitShop");
    }
    // leave critical section
    ret = pthread_mutex_unlock(&barber_mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber unlock: visitShop");
    }
    ret = pthread_mutex_unlock(&customer_mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on customer unlock: visitShop");
    }
    return barberID;
}

// called by customer
void Shop::leaveShop(int customer_id, int barber_id)
{
    int ret = pthread_mutex_lock(&barber_mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber lock: leaveShop");
    }
    customers_in_chair_[barber_id] = customer_id;
    // Wait for service to be completed
    cout << customer_id << "wait for barber[" << barber_id << "] to be done with hair-cut." << endl;
    while (in_service_[customer_id] == true)
    {
        pthread_cond_wait(&conds_customer_served_[customer_id], &barber_mutex_); // TODO timed wait?
    }

    // Pay the barber and signal barber appropriately
    money_paid_[barber_id] = true;
    pthread_cond_signal(conds_barber_paid_[barber_id]);
    print(customer_id, "says good-bye to the barber.");
    pthread_mutex_unlock(&barber_mutex_);
}

// called by barber thread with barber id
void Shop::helloCustomer(int id)
{
    int ret = pthread_mutex_lock(&barber_mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber lock: helloCustomer");
    }
    for (int b : barbers_) //TODO FIX push barber
    {
        cout << "barber iterator " << *b << endl;
        if (b == id)
        {
            this->barbers_.push(id);
            break;
        }
    }
    // If no customers than barber can sleep
    if (waiting_chairs_.empty() && customers_in_chair_.empty())
    {
        print(-id, "sleeps because of no customers.");
        pthread_cond_wait(conds_barbers_sleeping_[id], &barber_mutex_);
    }

    if (customers_in_chair_.empty()) // check if the customer, sit down.
    {
        //TODO check for customer
        //TODO signal customer from visit shop
        pthread_cond_wait(conds_barbers_sleeping_[id], &barber_mutex_);
    }

    struct timespec timespec;
    struct timeval timeval;
    int ret = gettimeofday(&tp, NULL);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on get time: helloCustomer");
    }
    timespec.tv_sec = timeval.tv_sec;
    timespec.tv_nsec = timeval.tv_usec * 1000;
    timespec.tv_sec += 1;
    //ret = pthread_cond_timedwait(&this->barber_waiting_conds[id], &this->barbermtx, &ts);

    //TODO timed wait so that barber checks twice for customer before sleeping

    print(-id, "starts a hair-cut service for " + int2string(customers_in_chair_[id]));
    pthread_mutex_unlock(&barber_mutex_);
}

// called by barber thread with barber id
void Shop::byeCustomer(int id)
{
    int ret = pthread_mutex_lock(&barber_mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber lock: byeCustomer");
    }

    // Hair Cut-Service is done so signal customer and wait for payment
    in_service_[id] = false;
    print(-id, "says he's done with a hair-cut service for customer[" + int2string(customers_in_chair_[id]) + "].");
    money_paid_[id] = false;

    pthread_cond_signal(conds_customer_served_[id]);
    while (money_paid_[id] == false)
    {
        pthread_cond_wait(conds_barber_paid_[id], &barber_mutex_);
    }

    int ret = pthread_mutex_lock(&customer_mutex_);
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber lock: byeCustomer");
    }

    // Signal to customer to get next one
    customers_in_chair_[id] = -1;
    print(-id, "calls in another customer");
    //TODO signal
    ret = pthread_mutex_unlock(&barber_mutex_); // unlock
    if (ret != 0)
    {
        exit_on_error(ret, "Error on barber unlock: byeCustomer");
    }
}

void exit_on_error(int val, string error)
{
    errno = val;
    perror(error);
    exit(EXIT_FAILURE);
}
