#include <stdio.h>

#define MAX 100

int main(void)
{
    int count = 10;
    float temperature = 36.5;
    char grade = 'A';

    // Calculate the result
    if (count >= 10 && temperature < 40.0)
    {
        printf("Grade = %c\n", grade);
    }

    /*
     * This is a
     * multi-line comment
     */

    count += 5;
    count++;

    if (count == MAX)
    {
        return 0;
    }

    return 1;
}