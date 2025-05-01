#include <iostream>
#include <string>
#include <queue>
#include <unordered_set>
#include <cstdio>
#include <cstdlib>
#include <curl/curl.h>
#include <stdexcept>
#include "rapidjson/error/error.h"
#include "rapidjson/reader.h"

struct ParseException : std::runtime_error, rapidjson::ParseResult {
    ParseException(rapidjson::ParseErrorCode code, const char* msg, size_t offset) : 
        std::runtime_error(msg), 
        rapidjson::ParseResult(code, offset) {}
};

#undef RAPIDJSON_PARSE_ERROR_NORETURN
#define RAPIDJSON_PARSE_ERROR_NORETURN(code, offset) \
    throw ParseException(code, #code, offset)

#include <rapidjson/document.h>
#include <chrono>
#include <thread>
#include <mutex>

using namespace std;
using namespace rapidjson;

bool debug = false;

// Updated service URL
const string SERVICE_URL = "http://hollywood-graph-crawler.bridgesuncc.org/neighbors/";

// Function to HTTP encode parts of URLs. for instance, replace spaces with '%20' for URLs
string url_encode(CURL* curl, string input) {
  char* out = curl_easy_escape(curl, input.c_str(), input.size());
  string s = out;
  curl_free(out);
  return s;
}

// Callback function for writing response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

// Function to fetch neighbors using libcurl with debugging
string fetch_neighbors(CURL* curl, const string& node) {
    string url = SERVICE_URL + url_encode(curl, node);
    string response;

    if (debug)
      cout << "Sending request to: " << url << endl;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "User-Agent: C++-Client/1.0");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK) {
        cerr << "CURL error: " << curl_easy_strerror(res) << endl;
    } else {
      if (debug)
        cout << "CURL request successful!" << endl;
    }

    curl_slist_free_all(headers);

    if (debug) 
      cout << "Response received: " << response << endl;

    return (res == CURLE_OK) ? response : "{}";
}

// Function to parse JSON and extract neighbors
vector<string> get_neighbors(const string& json_str) {
    vector<string> neighbors;
    try {
        Document doc;
        doc.Parse(json_str.c_str());

        if (!doc.IsObject()) {
            cerr << "Invalid JSON object: " << json_str << endl;
            return {};
        }

        if (doc.HasMember("neighbors") && doc["neighbors"].IsArray()) {
            for (const auto& neighbor : doc["neighbors"].GetArray())
                neighbors.push_back(neighbor.GetString());
        }
    } catch (const ParseException& e) {
        std::cerr << "Error while parsing JSON: " << json_str << std::endl;
        throw e;
    }
    return neighbors;
}

// BFS Traversal Function
vector<vector<string>> bfs(CURL* unused, const string& start, int depth) {
  vector<vector<string>> levels;
  unordered_set<string> visited;
  mutex visited_mutex;
  mutex level_mutex;

  levels.push_back({start});
  visited.insert(start);

  for (int d = 0;  d < depth; d++) {
    if (debug)
      std::cout << "starting level: " << d << "\n";
    levels.push_back({});
    vector<string> next_level;
    vector<string>& current_level = levels[d];

    int num_nodes = current_level.size();
    const int max_threads = 8;
    int num_threads = std::min(max_threads, num_nodes);
    vector<thread> threads(num_threads);

    auto worker = [&](int tid) {
      CURL* thread_curl = curl_easy_init();
      if (!thread_curl) {
        std::cerr << "CURL error: Failed initialization in thread " << tid << std::endl;
        return;
      }

      int chunk_size = (num_nodes + num_threads - 1) / num_threads;
      int start_idx = tid * chunk_size;
      int end_idx = std::min(start_idx + chunk_size, num_nodes);
      for (int i = start_idx; i < end_idx; ++i) {
        string& s = current_level[i];
        try {
          if (debug)
            std::cout << "Trying to expand" << s << "\n";
          for (const auto& neighbor : get_neighbors(fetch_neighbors(thread_curl, s))) {
            if (debug)
              std::cout << "neighbor " << neighbor << "\n";
            std::lock_guard<std::mutex> guard1(visited_mutex);
            if (!visited.count(neighbor)) {
              visited.insert(neighbor);
              std::lock_guard<std::mutex> guard2(level_mutex);
              next_level.push_back(neighbor);
            }
          }
        } catch (const ParseException& e) {
          std::cerr << "Error while fetching neighbors of: " << s << std::endl;
          throw e;
        }
      }

      curl_easy_cleanup(thread_curl);
    };

    for (int t = 0; t < num_threads; ++t)
      threads[t] = std::thread(worker, t);

    for (int t = 0; t < num_threads; ++t)
      threads[t].join();

    levels[d + 1] = std::move(next_level);
  }

  return levels;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        cerr << "Usage: " << argv[0] << " <node_name> <depth>\n";
        return 1;
    }

    // Global init for libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    string start_node = argv[1];
    int depth;
    try {
        depth = stoi(argv[2]);
    } catch (const exception& e) {
        cerr << "Error: Depth must be an integer.\n";
        return 1;
    }

    const auto start = std::chrono::steady_clock::now(); // start timing

    // std::cout << "===== BFS up to depth " << depth << " =====" << std::endl;

    for (const auto& n : bfs(nullptr, start_node, depth)) {
        for (const auto& node : n)
            cout << "- " << node << "\n";
        std::cout << n.size() << "\n";
    }

    const auto finish = std::chrono::steady_clock::now(); // end timing and print elapsed
    const std::chrono::duration<double> elapsed_seconds = finish - start;
    std::cout << "Time to crawl: " << elapsed_seconds.count() << "s\n";

    curl_global_cleanup();

    return 0;
}
