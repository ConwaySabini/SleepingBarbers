# SleepingBarbers
Simulates the sleeping barbers problem with M barbers and N chairs. The barbers will serve all the customers they are able to and when all the barbers are busy the customers will take a seat in an available waiting chair. If there are no chairs available the customer will leave the shop. When there are no more customers to serve, all the barbers go to sleep and when a customer enters the shop, the customer will wake a single Barber. Implemented  synchronization using hash maps of monitors for the barbers being paid, sleeping barbers, and the customers being served. There was also a monitor for the condition of waiting customers when there are no available barbers. The mutex is locked and unlocked whenever the barber or customer enters a critical section of the code.
