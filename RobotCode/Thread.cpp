#include <iostream>
#include <thread>
using namespace std;

void call_from_thread() {
    cout << "Hello World!" << endl;
}

int main()
{
    thread t1(call_from_thread);
    t1.join();
    return 0;   
}
    