#include <assert.h>

class random_generator_t
{
private:
  unsigned int a;               // (sqrt(5)-1)/2 = 0.61803398875
  unsigned int b;
  unsigned int last_val;
  unsigned int sd;
public:
  void forth() {
    last_val = a*last_val + b;
  }


  random_generator_t(unsigned int seed = 1) 
    : a(2654435769), b(seed), last_val(1), sd(seed){
    forth();
  }

  unsigned int item() const {
    return last_val;
  }

  void reset() {
    last_val = 1;
    b        = sd;
    forth();
  }
};




void sort(int len, int arr[] )
{
  int tmp;
  int again;
  int i;

  for(again=1; again; )
    for( again=0, i=0; i < (len-1); ++i){
      assert(0<=i && i+1 <len);
      if( arr[i] > arr[i+1] ){
        tmp      = arr[i];
        arr[i]   = arr[i+1];
        arr[i+1] = tmp;
        again    = 1;
      }
    }
}




int main()
{
  const unsigned int arr_size = 10000; //50000;

  int arr[arr_size];
  int i;
  random_generator_t rand;

  for(i=0; i!=arr_size; ++i){
    arr[i] = rand.item();
    rand.forth();
  }

  sort(arr_size, arr);
}

