#include "file_watcher.hpp"
#include "httplib.h"
#include "md2html.hpp"
#include <atomic>
#include <chrono>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

using namespace std::string_literals;

std::string readFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cerr << "Error: Unable to open file " << filename << std::endl;
    return ""s;
  }
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <markdown_file>" << std::endl;
    return 1;
  }

  std::string filename = std::string(argv[1]);
  FileWatcher watcher(filename);

  httplib::Server svr;
  std::atomic<bool> content_changed(true); // Set to true initially
  std::string current_html;

  // Initialize current_html with the initial content
  std::string initial_md = readFile(filename);
  std::cout << "Initial Markdown content: " << initial_md << std::endl;
  current_html = md2html(initial_md);
  std::cout << "Initial HTML content: " << current_html << std::endl;
  if (current_html.empty()) {
    std::cerr << "Warning: Initial HTML content is empty" << std::endl;
  }

  svr.Get("/", [&](const httplib::Request &, httplib::Response &res) {
    std::string html = R"(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Markdown Preview</title>
</head>
<body>
    <div id="content">Loading...</div>
    <div id="debug"></div>
    <script>
        function appendDebug(message) {
            const debug = document.getElementById('debug');
            debug.innerHTML += message + '<br>';
        }

        function pollForChanges() {
            appendDebug('Polling for changes...');
            fetch('/poll')
                .then(response => {
                    appendDebug('Response status: ' + response.status);
                    return response.text();
                })
                .then(html => {
                    appendDebug('Received HTML length: ' + html.length);
                    if (html) {
                        document.getElementById('content').innerHTML = html;
                        appendDebug('Content updated');
                    } else {
                        appendDebug('Received empty content');
                    }
                    pollForChanges();  // Poll again immediately
                })
                .catch(error => {
                    appendDebug('Polling error: ' + error);
                    setTimeout(pollForChanges, 1000);  // Try again after 1 second
                });
        }

        pollForChanges();
    </script>
</body>
</html>
        )";
    res.set_content(html, "text/html");
  });

  svr.Get("/poll", [&](const httplib::Request &, httplib::Response &res) {
    for (int i = 0; i < 100; ++i) { // Poll for up to 10 seconds
      if (content_changed.exchange(false)) {
        std::cout << "Sending updated content. Length: "
                  << current_html.length() << std::endl;
        res.set_content(current_html, "text/html");
        return;
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "No changes detected, sending 304" << std::endl;
    res.status = 304; // Not Modified
  });

  std::thread([&]() {
    while (true) {
      if (watcher.fileChanged()) {
        std::cout << "File change detected" << std::endl;
        std::string md_content = readFile(filename);
        std::cout << "Updated Markdown content: " << md_content << std::endl;
        current_html = md2html(md_content); // 更新 HTML 内容
        std::cout << "Updated HTML content: " << current_html << std::endl;
        if (current_html.empty()) {
          std::cerr << "Warning: Updated HTML content is empty" << std::endl;
        } else {
          std::cout << "Updated HTML content. Length: " << current_html.length()
                    << std::endl;
        }
        content_changed.store(true); // 设置标志为 true
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
  }).detach();

  std::cout << "Server started at http://localhost:8080" << std::endl;
  svr.listen("localhost", 8080);

  return 0;
}