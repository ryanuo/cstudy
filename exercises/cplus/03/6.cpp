#include <iostream>
using namespace std;

template <typename T, int N>
T findminData(T arr[N])
{
    T min = arr[0];
    for (int i = 0; i < N; i++)
        if (arr[i] < min)
            min = arr[i];

    return min;
}

int main()
{
    int arr[5] = {1, 2, 3, 4, 5};
    cout << findminData<int, sizeof(arr) / sizeof(arr[0])>(arr) << endl;
}