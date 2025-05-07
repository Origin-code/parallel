#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread> 

using namespace std;

const int THRESHOLD = 10000; 

// Merge function to merge two halves
void merge(vector<int>& arr, int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    vector<int> L(n1), R(n2);
    
    for (int i = 0; i < n1; i++)
        L[i] = arr[left + i];
    for (int j = 0; j < n2; j++)
        R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        if (L[i] <= R[j]) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }
    
    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }
}

// merge sort sequential
void mergeSortSequential(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSortSequential(arr, left, mid);
        mergeSortSequential(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

// merge sort parallel
void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;

        if ((right - left) > THRESHOLD) {
            thread leftThread(mergeSort, ref(arr), left, mid);
            thread rightThread(mergeSort, ref(arr), mid + 1, right);
            leftThread.join();
            rightThread.join();
        } else {
            mergeSort(arr, left, mid);
            mergeSort(arr, mid + 1, right);
        }

        merge(arr, left, mid, right);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <array_size>\n";
        return 1;
    }

    int size = atoi(argv[1]);
    vector<int> original(size);
    srand(time(0));
    for (int i = 0; i < size; i++) {
        original[i] = rand() % 10000;
    }

    // Sequential benchmark
    vector<int> seq = original;
    auto start_seq = chrono::high_resolution_clock::now();
    mergeSortSequential(seq, 0, size - 1);
    auto end_seq = chrono::high_resolution_clock::now();
    chrono::duration<double> dur_seq = end_seq - start_seq;
    cout << "Sequential " << size << " " << dur_seq.count() << endl;

    // Parallel benchmark
    vector<int> par = original;
    auto start_par = chrono::high_resolution_clock::now();
    mergeSort(par, 0, size - 1);
    auto end_par = chrono::high_resolution_clock::now();
    chrono::duration<double> dur_par = end_par - start_par;
    cout << "Parallel   " << size << " " << dur_par.count() << endl;
    cout << "Speedup: " << dur_seq.count() / dur_par.count() << "x" << endl;



    return 0;
}
