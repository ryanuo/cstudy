#include <iostream>
using namespace std;
int main()
{
    int n;
    cin >> n;
    // write your code here......
    for (int i = 0; i < n; i++)
    {
        for (int j = 0; j < n; j++)
        {
            cout << i + j;
        }
    }

    return 0;
}
