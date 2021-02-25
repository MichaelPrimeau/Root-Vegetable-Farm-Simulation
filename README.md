# Root-Vegetable-Farm-Simulation
This project simulates a vegetable market by using threads to simulate fields and customers. This was completed as a school assignment and this readme file includes my lab writeup for this assignment.

## Introduction
In this lab, we wrote a program that simulates a vegetable market using threads. There were vegetable fields that produced veggies, each field having its own thread. Then there were also customers, each having its own thread as well. The fields produced a specified number of veggies as described in a file, the customers then purchased these vegetables until no more orders could be made. The results of the simulation were then printed out to the user.

## Design
The design of this program was fairly complex. We needed a system for the field and customer threads to share variables for the current number of vegetables. To ensure that the value of these variables did not become skewed due to context switching I used semaphores to lock access to the variables to only one thread at a time.
  
The process of creating the simulation was a difficult one. I worked in pieces, starting with creating the producers and having them consistently producing vegetables. I then moved on to the consumers, making sure they were able to buy vegetables without skewing the variables due to context switching. The final part of the program I implemented was the waiting queue. Implementing a waiting queue for the customers instead of having each customer buy whenever they were ready.

## Build Instructions
To build the program the command “gcc rootveg.c -pthread” will compile the program and then “./a.out” plus the source file that describes the simulation as an argument will run it.

## Analysis
For this simulation to be fair, I feel that each customer should have an equal chance to have their order filled. In my implementation, the queue makes it so that each customer does have an equal chance to get their order filled since they must wait for the customer ahead of them before their order can be filled.
  
If a customer at the front of the line could not have their order filled so they were then skipped would cause unfair behavior in the program. This would be unfair to customers that request larger amounts of vegetables because they would always be skipped by someone that is requesting fewer vegetables.
  
To implement a ticket system in this simulation, instead of having customers wait for the amount of vegetables they ordered to be put into the storage bins, I would have them subtract the amount immediately to create a negative number in the bin. I would then have the customer wait until the bin total gets back to zero before they signal that their order was completed.
  
It does not make sense for the storage bins to be limitless. In real life, there would not be room to store endless amounts of vegetables so the simulation should reflect that. To change the program to behave this way, the producers would have to check if the storage bins are full before producing any vegetables. If they are full, they must wait for a customer to create some space and then continue as normal.

## Conclusion
The most challenging part of this lab for me was creating the consumer thread. Finding a way for each customer to wait for their orders to be completed and recognizing when they could not have any more orders be completed was a bit challenging. Even still my third test case file does not run to completion. This lab was a fun way of demonstrating a use for threads and the fact that it was challenging made it frustrating but also more rewarding whenever a test ran correctly.
