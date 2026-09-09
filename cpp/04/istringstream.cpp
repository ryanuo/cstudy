#include <iostream>
#include <sstream>
#include <string>
using namespace std;

int main()
{
    istringstream iss{"1 2 3 4 5 6 7 8 9 10"};
    int n;
    while (iss >> n)
    {
        iss >> n;
    }

    cout << n << endl;
}