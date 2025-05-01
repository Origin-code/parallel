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
#include <condition_variable>
#include <atomic>

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

// blocking queue class
template<typename T>
class BlockingQueue {
    queue<T> q;
    mutex m;
    condition_variable cv;
    bool finished = false;

public:
    void push(const T& item) {
      {
        unique_lock<mutex> lock(m);
        q.push(item);
      }
        cv.notify_one();
    }

    bool pop(T& item) {
      unique_lock<mutex> lock(m);
      while (q.empty() && !finished)
        cv.wait(lock);
      if (q.empty()) return false;
      item = q.front();
      q.pop();
      return true;
    }

    bool is_empty(){
      unique_lock<mutex> lock(m);
      return q.empty();
    }

    void set_finished() {
      {
        unique_lock<mutex> lock(m);
        finished = true;
      }
      cv.notify_all();
    }
};

// BFS Traversal Function
vector<vector<string>> bfs(CURL* unused, const string& start, int depth, int thread_count) {
  vector<vector<string>> levels;
  unordered_set<string> visited;
  mutex visited_mutex;
  mutex level_mutex;

  BlockingQueue<pair<string, int>> q;
  atomic<int> working_threads(0);
  atomic<bool> done(false);

  levels.push_back({start});
  visited.insert(start);
  q.push({start, 0});
  vector<thread> threads;

  for (int t = 0; t < thread_count; ++t) {
    threads.emplace_back([&]() {
      CURL* thread_curl = curl_easy_init();
      if (!thread_curl) {
        std::cerr << "CURL error: Failed initialization" << std::endl;
        return;
      }

      pair<string, int> task;
      while (true) {
        if (!q.pop(task)) {
          if (done) break;
          continue;
        }

        working_threads++;
        const string& node = task.first;
        int level = task.second;

        if (level >= depth) {
          working_threads--;
          continue;
        }

        if (debug)
          std::cout << "Trying to expand" << node << "\n";

        for (const auto& neighbor : get_neighbors(fetch_neighbors(thread_curl, node))) {
          if (debug)
            std::cout << "neighbor " << neighbor << "\n";

          bool is_new = false;
          {
            std::lock_guard<std::mutex> guard1(visited_mutex);
            if (!visited.count(neighbor)) {
              visited.insert(neighbor);
              is_new = true;
            }
          }

          if (is_new) {
            q.push({neighbor, level + 1});
            std::lock_guard<std::mutex> guard2(level_mutex);
            if (levels.size() <= level + 1)
              levels.push_back({});
            levels[level + 1].push_back(neighbor);
          }
        }

        working_threads--;
      }

      curl_easy_cleanup(thread_curl);
    });
  }

  while (true) {
    if (q.is_empty() && working_threads == 0)
      break;
    this_thread::sleep_for(chrono::milliseconds(10));
  }

  done = true;
  q.set_finished();

  for (auto& t : threads)
    t.join();

  return levels;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cerr << "Usage: " << argv[0] << " <node_name> <depth> <threads>\n";
        return 1;
    }

    // Global init for libcurl
    curl_global_init(CURL_GLOBAL_ALL);

    string start_node = argv[1];
    int depth;
    int thread_count;
    try {
        depth = stoi(argv[2]);
        thread_count = stoi(argv[3]);
    } catch (const exception& e) {
        cerr << "Error: Depth and thread count must be integers.\n";
        return 1;
    }

    const auto start = std::chrono::steady_clock::now(); // start timing

    for (const auto& n : bfs(nullptr, start_node, depth, thread_count)) {
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
