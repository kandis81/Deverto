
#include <iostream>
//#include <cmath>
#include <ctime>

//unsigned int pow( unsigned short int base, unsigned short int exp )
//{
//   return exp == 1 ? base : base * pow( base, exp -1 );
//}

int main(void)
{
//   const unsigned short int exp = 5;
   unsigned result = 0;

   for ( unsigned int num = 2; num < 0xffffffff; ++num )
   {
      double sum = 0;
      unsigned int tmp = num;
      for ( unsigned short int nr = tmp % 10;
            tmp && num > sum;
            nr = tmp % 10 )
      {
//         sum += pow( nr, exp );
         sum += nr * nr * nr * nr * nr; // this is the faster
//         sum += std::pow( nr, exp );
         tmp /= 10;
      }

      if ( tmp ) continue;

      if ( sum == num )
      {
         std::cout << "Found: " << num << std::endl;
         result += sum;
      }
   }

   std::cout << "Result: " << result << std::endl;

   return 0;
}

