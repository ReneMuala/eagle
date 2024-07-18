#pragma once
#include <crow/app.h>
#include <crow/multipart.h>
#include "opencv2/core/mat.hpp"

namespace eagle {
class Server {
protected:
  crow::SimpleApp app;
  const std::string host;
  const int port;
  
  crow::response home_handler();
  std::tuple<std::string, bool>
  read_image_data(crow::multipart::message &multipart_request);
  std::tuple<cv::Mat, bool> read_image_from_buffer(const std::string &buffer);
  crow::response ocr_handler(const crow::request &req);

public:
  void run();
  Server(const std::string &host = "0.0.0.0", const int &port = 1234);
};
} // namespace eagle