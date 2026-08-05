#include <iostream>
#include <string>
#include <fstream>
using namespace std;

int main()
{
    string filename{"04/input.txt"};
    ifstream fin;
    fin.open(filename, ios::in);
    if (!fin)
    {
        cout << "Error: " << filename << " does not exist" << endl;
        ofstream fout;
        fout.open(filename, ios::out);
        fout << "1-2,3-4" << endl;
        fout.close();
    }

    string st;
    while (getline(fin, st))
    {
    }

    fin.close();
}