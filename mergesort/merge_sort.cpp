#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>

using namespace std;

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

// Merge Sort function
void mergeSort(vector<int>& arr, int left, int right) {
    if (left < right) {
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid);
        mergeSort(arr, mid + 1, right);
        merge(arr, left, mid, right);
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        cerr << "Usage: " << argv[0] << " <array_size>\n";
        return 1;
    }

    int size = atoi(argv[1]);
    vector<int> arr(size);
    srand(time(0));

    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 10000; // Random numbers between 0 and 9999
    }

    auto start = chrono::high_resolution_clock::now();
    mergeSort(arr, 0, size - 1);
    auto end = chrono::high_resolution_clock::now();

    chrono::duration<double> duration = end - start;

    // Fix: Print correctly formatted output
    cout << size << " " << duration.count() << endl;

    return 0;
}

