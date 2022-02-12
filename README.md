# SleepingBarbers
Simulates the sleeping barbers problem with M barbers and N chairs. The barbers will serve all the customers they are able to and when all the barbers are busy the customers will take a seat in an available waiting chair. If there are no chairs available the customer will leave the shop. When there are no more customers to serve, all the barbers go to sleep and when a customer enters the shop, the customer will wake a single Barber. Implemented  synchronization using hash maps of monitors for the barbers being paid, sleeping barbers, and the customers being served. There was also a monitor for the condition of waiting customers when there are no available barbers. The mutex is locked and unlocked whenever the barber or customer enters a critical section of the code.

![image](https://user-images.githubusercontent.com/53063791/153731949-0ba8e2b9-048d-4f6b-8a66-5aca53bd79f3.png)

Sleeping Barbers Shop Implementation
Overview
Simulates the sleeping barbers problem with M barbers and N chairs using monitors to signal and wait between barber and customer. Every method called by a customer or barber thread locks the mutex before entering the critical section, and then unlocks the mutex before the method ends. Utilizes maps for the conditions so that each condition is individual to the Barber or customer thread. Queues are used to store the available barbers and waiting customers.

Constructors
Shop() 
Description: Initializes the number of barbers and customers to their default values of 1 and 3, respectively. Initializes the customer drop offs to 0 and calls init() to set default values.
Returns: Shop object with initialized values, lists, maps, monitors, and mutex.

Shop(int num_barbers, int num_chairs) 
Description: Initializes the number of barbers and customers to the numbers specified in the parameters. Initializes the customer drop offs to 0 and calls init() to set default values.
Parameters: The number of barbers in the number of chairs In the shop.
Returns: Shop object with initialized values, lists, maps, monitors, and mutex.

Methods
void init() 
Description: initializes the vectors and queues used with their default values. Initializes the mutex and a condition for waiting customers and condition maps for the 

int visitShop(int id) 
Description: Called by a customer thread. If all the chairs are full then the customer leaves the shop If all the barbers are busy then the customer takes a seat in an available waiting chair until a barber is available to cut the customer’s hair. If the available barber is sleeping then the customer will send a signal to wake the Barber up. 
Parameters: The customer ID used for index lookup for signals and chairs.
Returns: The Barber ID or -1 if the customer leaves.

void leaveShop(int customer_id, int barber_id) 
Description: Called by a customer thread. The customer takes a seat in the barbers chair and waits for the service to be complete. Once the customer receives a signal they will pay the Barber appropriately and send a signal to the Barber before leaving.
Parameters: The customer and Barber ID.




void byeCustomer(int id) 
Description: After finishing the haircut the Barber signals of the customer and waits for the customer to pay them. Once the Barber has been paid the Barber will be pushed back to the queue and will attempt to signal another customer into the chair.
Parameters: The Barber ID.

void helloCustomer(int id) 
Description: Called by a barber thread. If there are no customers available, the Barber will sleep; otherwise the Barber will start a haircut for the customer that was either already in their chair or woke them up. 
Parameters: The Barber ID.

int get_cust_drops() const 
Description: Gets the number of customers that have left the shop because there were no available waiting share
Returns: The number of lost customers.

string int2string(int i)
Description: Takes an integer and converts it to a string.
Parameters: Any integer i.
Returns: The string form of the integer.

void print(int person, string message, bool barber) 
Description: Takes in a person ID as well as a Barber boolean value that determines whether or not the person is about is a Barber or not and then prints the message to standard out.
Parameters: Person ID, message string, as well as the Barber boolean to determine 

void void exit_on_error(int val, const char *error) 
Description: Takes in a value for the error as well as the error message and prints the error message to perror before exiting the program
Parameters: The value representative of an error as well as the error message.

Sleeping Barbers Report
Step 5:
There needs to be approximately 98 chairs for all the customers to be served without waiting. With a value of 95 chairs I was able to get 0 customer drop offs about 60% of the time but as I increased this number the number of drop offs were dropping and by the time I was using 98 chairs there were no customers that were being lost due to busy barbers and full chairs. I think that a value of 100 chairs would be very safe considering I was not able to get anything other than a 0 for 98 but there could be a low probability of one occurring and I just did not get one.

Step 6:
There needs to be approximately 5 barbers for the barbers to serve all 200 customers without any of them waiting. With a value of 4 barbers there were usually no customers waiting in the available chairs, but with a value of 5 barbers, I never had any customers waiting although there still could be a small probability of there being waiting customers that I just didn’t encounter. However I think that the value of 5 is fairly safe for no waiting customers because I did test it many times. 

Limitations:
Microseconds are used for the service time which is likely to cause skews in the clock that affect the order of some operations carried out such as the order of barbers calling in customers or the order of barbers sleeping, etc.
Barbers can not choose which customer to serve with implementation; only the next one waiting will be selected. With this implementation the customer knows what barber will be serving them but the barber is not sure who the next customer is.
Customers and Barbers signal and wait for each other meaning that neither person can move on with their next task until they receive the signal that they were waiting for.
Using very large numbers for the customers or barbers can end up crashing the program because it is not designed to scale to 100,000+.

Extensions:
Maps used for conditions for the monitors allow for expansion 
Vectors used for variables for the barber such as the barbers chairs and the customers in them, the barbers that have been paid, and customers that are in service.
Deques are used for the available barbers and waiting chairs for the customers to sit in. The deque functions as a queue, enabling the ability to push and pop barbers/customers, while also allowing access by index using the bracket operators which makes it easy to access the barber or customer at the specified index.
Modularity of code allows for easy improvement and makes it easy to add features in the future.


