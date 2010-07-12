#include <cstdio>
#include <cmath>
#include <ctime>
#include <cstdlib>
#include <vector>
#include <string>

#include "interval_tree.h"

int main(int argc, char *argv[])
{
  int low;
  int high;

  IntervalTree<std::string> intervalTree;
  int count = 1000000;
  int domain = 1000;
  printf("Inserting %i nodes into [0,%i].\n", count, domain);
  for(int i=0; i<count; i++)
  {
    low = rand() % domain;
    high = (rand() % (domain-low)) + low;
    
    intervalTree.insert("test", low, high);
    //printf("Added: [%i,%i]\n", low,high);
    if(!(i%25000)){printf("*");fflush(0);}
  }
  printf("\n");
  
  low = domain * 0.4f;
  high = domain * 0.5f;//10% of the domain is being tested
  printf("Enumerating intervals between %i and %i\n", low, high);
  std::vector<std::string> results = intervalTree.fetch(low,high);

  printf("%zu intervals found.\n", results.size());
  return 0;
}

