#include "curl/curl.h"
#include <curl/easy.h>
#include <fstream>
#include <iostream>
#include <string>

// Callback to write data to a file
size_t write_to_file(void *ptr, size_t size, size_t nmemb, void *userdata) {
  std::ofstream *file = static_cast<std::ofstream *>(userdata);
  file->write(static_cast<char *>(ptr), size * nmemb);
  return size * nmemb;
}

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr
        << "Usage: " << argv[0]
        << " <URL> [--header <header>] [--post <data>] [--output <file>]\n";
    return 1;
  }

  std::string url = argv[1];
  std::string output_file;

  CURL *curl = curl_easy_init();
  if (!curl) {
    std::cerr << "Failed to initialize cURL\n";
    return 1;
  }

  // Set URL
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

  // Handle optional arguments
  struct curl_slist *headers = nullptr; // Initialize headers list
  for (int i = 2; i < argc; i++) {
    if (std::string(argv[i]) == "--header" && i + 1 < argc) {
      headers = curl_slist_append(headers, argv[++i]); // Add the header
    } else if (std::string(argv[i]) == "--post" && i + 1 < argc) {
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, argv[++i]); // Set POST data
    } else if (std::string(argv[i]) == "--output" && i + 1 < argc) {
      output_file = argv[++i]; // Set the output file name
    }
  }

  std::ofstream file;
  if (!output_file.empty()) {
    file.open(output_file, std::ios::binary);
    if (!file.is_open()) {
      std::cerr << "Unable to open file: " << output_file << "\n";
      return 1;
    }
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_to_file);
  }

  // Set headers if provided
  if (headers) {
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  }

  // Perform the request
  CURLcode res = curl_easy_perform(curl);
  if (res != CURLE_OK) {
    std::cerr << "cURL error: " << curl_easy_strerror(res) << "\n";
  } else if (!output_file.empty()) {
    std::cout << "Response saved to " << output_file << "\n";
  }

  // Cleanup
  if (headers) {
    curl_slist_free_all(headers); // Free the headers list
  }
  curl_easy_cleanup(curl);
  if (file.is_open()) {
    file.close();
  }

  return 0;
}
