#include <windows.h>
#include <iostream>
#include <vector>
#include <list>

using namespace std;

int N, meals;

vector<HANDLE> forks;
HANDLE hButler;
HANDLE hMutex;

list<int> eatingHistory;

// Philosopher thread
DWORD WINAPI Philosopher(LPVOID param) {

    int id = *(int*)param;

    int left = id;
    int right = (id + 1) % N;

    for (int i = 1; i <= meals; i++) {

        WaitForSingleObject(hMutex, INFINITE);

        cout << "Philosopher " << id + 1
             << " is THINKING." << endl;

        ReleaseSemaphore(hMutex, 1, NULL);

        Sleep(300);

        WaitForSingleObject(hMutex, INFINITE);

        cout << "Philosopher " << id + 1
             << " is HUNGRY." << endl;

        ReleaseSemaphore(hMutex, 1, NULL);

        // Limit the number of philosophers
        WaitForSingleObject(hButler, INFINITE);

        // Take two forks
        WaitForSingleObject(forks[left], INFINITE);
        WaitForSingleObject(forks[right], INFINITE);

        WaitForSingleObject(hMutex, INFINITE);

        cout << "Philosopher " << id + 1
             << " took both forks and is EATING."
             << endl;

        ReleaseSemaphore(hMutex, 1, NULL);

        Sleep(500);

        WaitForSingleObject(hMutex, INFINITE);

        eatingHistory.push_back(id + 1);

        cout << "Philosopher " << id + 1
             << " finished meal " << i
             << "/" << meals << endl;

        ReleaseSemaphore(hMutex, 1, NULL);

        // Release forks
        ReleaseSemaphore(forks[right], 1, NULL);
        ReleaseSemaphore(forks[left], 1, NULL);

        ReleaseSemaphore(hButler, 1, NULL);
    }

    WaitForSingleObject(hMutex, INFINITE);

    cout << "Philosopher " << id + 1
         << " finished all meals." << endl;

    ReleaseSemaphore(hMutex, 1, NULL);

    return 0;
}


// Main function
int main() {

    cout << "=== DINING PHILOSOPHERS ===" << endl;

    cout << "Enter number of philosophers: ";
    cin >> N;

    cout << "Enter number of meals: ";
    cin >> meals;

    if (!cin || N < 2 || N > 20 || meals <= 0) {

        cout << "Invalid input!" << endl;
        return 1;
    }

    hButler = CreateSemaphore(NULL, N - 1, N - 1, NULL);
    hMutex = CreateSemaphore(NULL, 1, 1, NULL);

    if (hButler == NULL || hMutex == NULL) {
        cout << "Semaphore creation failed!" << endl;
        if (hButler) CloseHandle(hButler);
        if (hMutex) CloseHandle(hMutex);
        return 1;
    }

    // Create fork semaphores
    for (int i = 0; i < N; i++) {

        HANDLE h = CreateSemaphore(NULL, 1, 1, NULL);

        if (h == NULL) {
            cout << "Fork semaphore creation failed!" << endl;
            for (HANDLE f : forks) CloseHandle(f);
            CloseHandle(hButler);
            CloseHandle(hMutex);
            return 1;
        }

        forks.push_back(h);
    }

    vector<HANDLE> threads;
    vector<int> ids(N);

    for (int i = 0; i < N; i++) {

        ids[i] = i;

        HANDLE h = CreateThread(
            NULL, 0, Philosopher,
            &ids[i], 0, NULL
        );

        if (h == NULL) {
            cout << "Thread creation failed!" << endl;

            for (HANDLE t : threads) {
                WaitForSingleObject(t, INFINITE);
                CloseHandle(t);
            }

            for (HANDLE f : forks) CloseHandle(f);
            CloseHandle(hButler);
            CloseHandle(hMutex);
            return 1;
        }

        threads.push_back(h);
    }

    // Wait for all philosophers
    for (HANDLE h : threads) {

        WaitForSingleObject(h, INFINITE);
        CloseHandle(h);
    }

    cout << "\n=== ALL PHILOSOPHERS FINISHED ===" << endl;

    cout << "Total completed meals: "
         << eatingHistory.size() << endl;

    // Release resources
    for (HANDLE h : forks) {
        CloseHandle(h);
    }

    CloseHandle(hButler);
    CloseHandle(hMutex);

    return 0;
}
