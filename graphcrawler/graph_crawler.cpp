#include <iostream>
#include <queue>
#include <unordered_set>
#include <vector>
#include <string>
#include <curl/curl.h>
#include <rapidjson/document.h>
#include <chrono>

using namespace std;
using namespace rapidjson;

const string BASE_URL = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

// store api response
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// fetch neighbors
vector<string> fetch_neighbors(const string& node) {
    CURL* curl;
    CURLcode res;
    string readBuffer;
    vector<string> neighbors;

    curl = curl_easy_init();
    if (curl) {
        string url = BASE_URL + node;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);

        if (res == CURLE_OK) {
            Document d;
            d.Parse(readBuffer.c_str());

            if (d.HasMember("neighbors") && d["neighbors"].IsArray()) {
                for (auto& v : d["neighbors"].GetArray()) {
                    neighbors.push_back(v.GetString());
                }
            }
        }
    }
    return neighbors;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Usage: " << argv[0] << " <start_node> <depth>\n";
        return 1;
    }

    string start_node = argv[1];
    int max_depth = stoi(argv[2]);
    // start time measurement
    auto start_time = chrono::high_resolution_clock::now();

    queue<pair<string, int>> q;
    unordered_set<string> visited;

    q.push({start_node, 0});
    visited.insert(start_node);

    while (!q.empty()) {
        auto [current, depth] = q.front();
        q.pop();

        cout << current << " (depth " << depth << ")\n";

        if (depth < max_depth) {
            vector<string> neighbors = fetch_neighbors(current);
            for (const auto& neighbor : neighbors) {
                if (visited.find(neighbor) == visited.end()) {
                    visited.insert(neighbor);
                    q.push({neighbor, depth + 1});
                }
            }
        }
    }
    // end time measurement
    auto end_time = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end_time - start_time;
    cout << "\nTraversal completed in " << elapsed.count() << " seconds.\n";

    return 0;
}
