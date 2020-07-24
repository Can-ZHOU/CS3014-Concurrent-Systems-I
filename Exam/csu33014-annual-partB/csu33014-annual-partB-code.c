//
// CSU33014 Summer 2020 Additional Assignment
// Part B of a two-part assignment
//
// Please write your solution in this file

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <omp.h>
#include "csu33014-annual-partB-person.h"

// Can Zhou 19324118 zhouc@tcd.ie
// Detailed algorithm explanation, time complexity analysis, parallel method explanation 
// are in the Supplemental Material_Can Zhou.pdf which I attached.
// Please have a look!
// The explanation below may have some duplicate with supplemental material 
// and some figures are shown in supplemental material as well.

/********************************************* Original Code *************************************************************************/
void find_reachable_recursive(struct person * current, int steps_remaining,
			      bool * reachable) {
  // mark current root person as reachable
  reachable[person_get_index(current)] = true;
  // now deal with this person's acquaintances
  if ( steps_remaining > 0 ) {
    int num_known = person_get_num_known(current);
    for ( int i = 0; i < num_known; i++ ) {
      struct person * acquaintance = person_get_acquaintance(current, i);
      find_reachable_recursive(acquaintance, steps_remaining-1, reachable);
    }
  }
}

// computes the number of people within k degrees of the start person
int number_within_k_degrees(struct person * start, int total_people, int k) {
  bool * reachable;
  int count;

  // maintain a boolean flag for each person indicating if they are visited
  reachable = malloc(sizeof(bool)*total_people);
  for ( int i = 0; i < total_people; i++ ) {
    reachable[i] = false;
  }

  // now search for all people who are reachable with k steps
  find_reachable_recursive(start, k, reachable);

  // all visited people are marked reachable, so count them
  count = 0;
  for ( int i = 0; i < total_people; i++ ) {
    if ( reachable[i] == true ) {
      count++;
    }
  }
  return count;
}

/********************************************* Less Redundant Code *******************************************************************/

// Detailed algorithm explanation, time complexity analysis, timing
// are in the Supplemental Material_Can Zhou.pdf which I attached.
// Please have a look!
// The explanation below may have duplicate with supplemental material
// and some figures are shown in supplemental material as well.
// So, it will be better to read supplemental material, but just in case, I copy those explanation from supplemental material.
/*
A part of sample graph in supplemental material (see the full version in supplemental material): 
A---E
|\  |
| \ |
D---F---J 
My algorithm for reducing redundant visits:
1.	The algorithm I used is a modified version of DFS(Depth First Search).
    a.	This question is based on an undirected graph and each edge has same weight. 
        So, the degree of separation is the number of edges between two nodes (persons).

    b.	I have added a new array distance[] to record the number of steps between two nodes. 
        And initializing all the distance to -1 
        (I am using -1 instead of infinity for initialising the elements of the array. 
        For using infinity as initializer, I need to import additional libraries which I did not feel necessary).

    c.	Instead of using traditional DFS to find the goal node, 
        my modified DFS algorithm will terminate when the depth is equal to the degree of septation. 

    d.	Assuming the start point is A, so the value of point A in distance array is 0. 
        When B will be visited next from A, point B begins to search the points connected. 
        When the B searches A, it can find the value of point A in distance array is 0 not -1 
        which means point A has been visited, so B will not revisit the node A. 
        In this way, we can reduce redundant visits.

    e.	However, we need to consider that DFS perhaps cannot find the minimum distance between two nodes. 
        For instance, assuming the degree of separation is 2 and the start point is A, 
        the point F can be found in this route: A->D->F and because the distance[F] is 2, J cannot be reached in this route, 
        but after DFS’s backtrack, F can be directly reached by A. 
        In this situation, we can update distance[F] to 1, so point J can be reachable under this route: A->F->J.

    f.	As I mentioned above whether the node is accessed depends on two conditions. 
        One is if the node has not been visited before, then we visit it. 
        Or the new distance between this node and start node is less than the value recorded in the distance array 
        which means this node has potential to reach more nodes.

    g.	Reason why I have not deleted the bool * reachable in original code which’s function can be replaced by int * distance 
        (if distance is -1, then this node hasn’t been visited).
        A Bool value only take up 1 bit, an Int takes 64 bits on a 64-bit machine. 
        Bool array (reachable) is much faster than int array (distance) when counting the number of visited node.

2.	I have used recursive version instead of iteration version.
    In C programming, iteration should faster than recursion, 
    so first, I tried to implement the algorithm in iteration way by using stack data structure based on linked list. 
    However, I found iteration version slower than recursive version as the result below.
        Correct count 184464, Time: 187051
        Less redundant count (recursion) 184464, Time: 136085
        Less redundant count (iteration) 184464, Time: 722381
    I think the main explanation is I need to malloc the memory from heap for linked list stack each time 
    which has a high time overhead 
    and recursion allocates the memory from stack which is faster. 
    So, I use the recursive version instead of iteration version.

Time complexity:
Assuming the number of total people is n and the degree of separation is k.
1.  Original code’s time complexity: O(n^k).
    In the worst case, all the nodes are connected to each other, 
    so in k degree, the code will run n^k time.
2.  My less redundant code: O(n^k).
    Similar reason as above. 
    However, although less redundant code 
    has same big O time complexity with original code, 
    the constant of O(n^k) sis very tiny compared to original code.
*/

void find_reachable_recursive_less_redundant(struct person * current, int steps_remaining,
			      bool * reachable, int * distance, int total_steps) {
  // mark current root person as reachable
  reachable[person_get_index(current)] = true;
  // now deal with this person's acquaintances
  if ( steps_remaining > 0 ) {
    int num_known = person_get_num_known(current);
    for ( int i = 0; i < num_known; i++ ) {
      struct person * acquaintance = person_get_acquaintance(current, i);
      // If the node hasn’t been visited before, then we visit it. 
      // Or the new distance between this node and start node is less than the value recorded in the distance array 
      // which means this node has potential to reach more nodes, we visit it as well.
      if(reachable[acquaintance->person_index] == false || distance[acquaintance->person_index] > total_steps-steps_remaining) {
        distance[acquaintance->person_index] = total_steps-steps_remaining;
        find_reachable_recursive_less_redundant(acquaintance, steps_remaining-1, reachable, distance, total_steps);
      }
    }
  }
}

int less_redundant_number_within_k_degrees(struct person * start,
					   int total_people, int k) {
  bool * reachable;
  int count;
  int * distance;

  // maintain a boolean flag for each person indicating if they are visited
  reachable = malloc(sizeof(bool)*total_people);
  // added a new array distance[] to record the number of steps between two nodes and initializing all the distance to -1
  distance = malloc(sizeof(int)*total_people);

  for ( int i = 0; i < total_people; i++ ) {
    reachable[i] = false;
    distance[i] = -1;
  }

  // update the start point's distance.
  distance[start->person_index] = 0;

  // now search for all people who are reachable with k steps
  // in this recursive version, in order to reduce redundant visits, I have add an if condition when explore the more nodes.
  find_reachable_recursive_less_redundant(start, k, reachable, distance, k);

  // all visited people are marked reachable, so count them
  count = 0;
  for ( int i = 0; i < total_people; i++ ) {
    if ( reachable[i] == true ) {
      count++;
    }
  }
  return count;
}

/********************************************* Original Code with Parallel ***********************************************************/

// Detailed algorithm explanation, time complexity analysis, parallel method explanation, timing
// are in the Supplemental Material_Can Zhou.pdf which I attached.
// Please have a look!
// The explanation below may have some duplicate with supplemental material
// and some figures are shown in supplemental material as well.
// So, it will be better to read supplemental material, but just in case, I copy those explanation from supplemental material.

/*
The code I used to parallelize is original code. 
Here is the reason why it is difficult to parallelize my less redundant code:
    In my algorithm, when the recursion is deeper, there will have less nodes need to be visited. 
    However, OpenMP directives have overheads and when the number of threads is increasing, 
    the overhead of OpenMP is increasing as well (see the reference figure in supplemental material). 
    When the data set is small, it is not appropriate to use OpenMP 
    as it takes more time to generate and handle more threads which not really do anything. 
    In my less redundant code, the data set is small and OpenMP cannot show its optimal power 
    (overheads of OpenMP directives will cost more than the time that multithread saved). 
    So, I used original code to parallelize instead of my less redundant code.

My parallelization strategy:
1.	#pragma omp parallel if(total_people > 10000)
    a.	#pragma omp parallel 
    As I have use parallel for each for loop and recursive function, 
    so I only set one parallel region. 
    Parallel overhead can be reduced slightly by having only one parallel region.

    b.	if condition
    Since OpenMP has overheads, when the dataset is small, 
    it is not useful to use OpenMP and may cause longer execution time. 
    Hence, I used an if condition, if the dataset is small (total_people <= 10000), 
    OpenMP won’t be used.

2.	#pragma omp parallel for
    It can divide the iterations of a for loop between the threads. 
    This has been used in initializing the reachable array 
    and counting the visited people number 
    (when counting, I also use atomic to make sure result is correct 
    and I have mentioned it below).

3.	#pragma omp task && #pragma omp single
    Parallel task can be used to parallelize the recursive algorithm 
    which are added to a pool of tasks and executed when the system is ready.
    #pragma omp single is used to make sure only one thread executes the initial recursive function.
4.	#pragma omp atomic
    An atomic update can update a variable in a single, unbreakable step. 
    So, with #pragma omp atomic, we are guaranteed that when counting visited people, 
    the value of count is correct.

Time complexity:
    I used original code and as I have mentioned above, 
    the time complexity for original code is O(n^k).
    If there are P parallel processing cores, in ideally, 
    the parallel code’s complexity is O(n^k/p).
    where n is the number of total people, 
          k is the degree of separation,
          p is the number of parallel processing cores.

Timing:
1.  When the dataset is small ./graph 500 8
        Correct count 483, Time: 8011
        Parallel count 483, Time: 7833
    Parallel code has not use OpenMP because of if condition, 
    so it has similar execution time with original code.
2.  When the dataset is large ./graph 800000 10
    a.  Correct count 714020, Time: 1771759
        Parallel count 714020, Time: 547692
    b.  Correct count 480932, Time: 520863
        Parallel count 480932, Time: 391600
    c.  Correct count 584623, Time: 1329039
        Parallel count 584623, Time: 842479
    The parallel code is faster than original code.

    Note*: sometime parallel code may slower than original code 
           because the random graph connection may cause the dataset 
           which used in OpenMP very small.
*/

void find_reachable_recursive_parallel(struct person * current, int steps_remaining,
			      bool * reachable) {
  // mark current root person as reachable
  reachable[person_get_index(current)] = true;
  // now deal with this person's acquaintances
  if ( steps_remaining > 0 ) {
    int num_known = person_get_num_known(current);
    for ( int i = 0; i < num_known; i++ ) {
      struct person * acquaintance = person_get_acquaintance(current, i);
      // Parallel task can be used to deal with recursive algorithm 
      // which are added to a pool of tasks and executed when the system is ready.
      #pragma omp task
      find_reachable_recursive(acquaintance, steps_remaining-1, reachable);
    }
  }
}

int parallel_number_within_k_degrees(struct person * start,
				     int total_people, int k) {
  bool * reachable;
  int count = 0;

  // maintain a boolean flag for each person indicating if they are visited
  reachable = malloc(sizeof(bool)*total_people);

// As I have use parallel for each for loop and recursive function, so I only set one parallel region. 
// Parallel overhead can be reduced slightly by having only one parallel region.
// Since OpenMP has overheads, when the dataset is small, it is worthless to use OpenMP and may cause longer execution time. 
// So, I use if condition, if the dataset is small (total_people <= 10000), OpenMP won’t be used.
#pragma omp parallel if(total_people > 10000)
{
  // parallel region has been set before, so just use #pragma omp for.
  // #pragma omp for: It can divide the iterations of a for loop between the threads.
  #pragma omp for 
  for ( int i = 0; i < total_people; i++ ) {
    reachable[i] = false;
  }

  // now search for all people who are reachable with k steps
  // parallel region has been set before, so just use #pragma omp single.
  // #pragma omp single: one thread executes the initial recursive function.
  // In the recursive function, parallel task has been used.
  #pragma omp single
  {
    find_reachable_recursive_parallel(start, k, reachable);
  }

  // all visited people are marked reachable, so count them
  // parallel region has been set before, so just use #pragma omp for.
  // #pragma omp for: It can divide the iterations of a for loop between the threads
  #pragma omp for
  for ( int i = 0; i < total_people; i++ ) {
    if ( reachable[i] == true ) {
      // An atomic update can update a variable in a single, unbreakable step. 
      // So, with #pragma omp atomic, we are guaranteed that when counting visited people, the value of count is correct.
      #pragma omp atomic
      count++;
    }
  }
}
  return count;
}
