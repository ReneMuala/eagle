#include "server.hpp"
#include "../cv/ocr.hpp"
#include "crow/common.h"
#include "opencv2/core/mat.hpp"
#include "opencv2/imgcodecs.hpp"
#include <crow/app.h>

using namespace eagle;

constexpr const char *version = "v0.1";

crow::response Server::home_handler() {
  return crow::response(
      std::string("Eagle OCR Web Server ") + version +
      " :: Routes:\n"
      "\n/ -> this page\n"
      "\n/do-ocr -> expects a multipart with an image named image\n");
}

std::tuple<std::string, bool>
Server::read_image_data(crow::multipart::message &multipart_request) {
  crow::multipart::message file_message(multipart_request);
  std::string data;
  for (const auto &part : file_message.part_map) {
    const auto &part_name = part.first;
    const auto &part_value = part.second;
    CROW_LOG_DEBUG << "Part: " << part_name;
    if ("image" == part_name) {
      auto headers_it = part_value.headers.find("Content-Disposition");
      if (headers_it == part_value.headers.end()) {
        CROW_LOG_ERROR << "No Content-Disposition found";
        return {data, false};
      }
      auto params_it = headers_it->second.params.find("filename");
      if (params_it == headers_it->second.params.end()) {
        CROW_LOG_ERROR << "Part with name \"image\" should have a file";
        return {data, false};
      }
      data = part_value.body;
      return {data, true};
      CROW_LOG_INFO << " Contents written to memory";
    } else {
      CROW_LOG_DEBUG << " Value: " << part_value.body << '\n';
    }
  }
  return {data, false};
}

std::tuple<cv::Mat, bool>
Server::read_image_from_buffer(const std::string &buffer) {
  int byte_buffer_size = buffer.size();
  uchar *byte_buffer = (uchar *)buffer.data();
  CROW_LOG_INFO << __FUNCTION__ << __LINE__;
  cv::Mat raw_data(1, byte_buffer_size, CV_8UC1, (void *)byte_buffer);
  CROW_LOG_INFO << __FUNCTION__ << __LINE__;
  cv::Mat decodedImage =
      cv::imdecode(cv::InputArray(raw_data), cv::IMREAD_COLOR);
  CROW_LOG_INFO << __FUNCTION__ << __LINE__;
  // cv::imwrite("A-uploaded.jpg", decodedImage);
  CROW_LOG_INFO << __FUNCTION__ << ": "
                << (decodedImage.data ? "Success" : "Failed");
  return {decodedImage, decodedImage.data != NULL};
}

crow::response Server::ocr_handler(const crow::request &req) {
  crow::json::wvalue response_body;
  crow::multipart::message message(req);
  const auto &[buffer, ok_buffer] = read_image_data(message);
  if (not ok_buffer or buffer.empty()) {
    response_body["type"] = "NO_IMAGE_FILE_FOUND";
    response_body["title"] = "No image";
    response_body["detail"] = "The multipart request should contain the image "
                              "to be scanned under a field named 'image'";
    return crow::response(crow::status::BAD_REQUEST, response_body);
  }

  const auto &[mat, ok_mat] = read_image_from_buffer(buffer);

  if (not ok_mat) {
    response_body["type"] = "INVALID_IMAGE_FILE";
    response_body["title"] = "Invaid image";
    response_body["detail"] =
        "The uploaded file could not be readed as a valid OpenCV matrix";
    response_body["read_more"] =
        "https://docs.opencv.org/3.4/d4/da8/group__imgcodecs.html";
    return crow::response(crow::status::BAD_REQUEST, response_body);
  }
  eagle::OCR engine;
  const auto &[ocr_resultt, ok_ocr_resultt] = engine.peform_ocr(mat);
  const auto &[bird_eye_view_mat, orc_readed_text] = ocr_resultt;

  if (not ok_ocr_resultt) {
    response_body["type"] = "OCR_FAILED";
    response_body["title"] = "ocr scanned";
    response_body["detail"] =
        "Could not detect document shape from the provided image";
    return crow::response(crow::status::BAD_REQUEST, response_body);
  }
  response_body["content"] = orc_readed_text;
  
  return crow::response(crow::status::OK, response_body);
}

void Server::run() {
  app.multithreaded().bindaddr(host).port(port).signal_clear().run();
}

Server::Server(const std::string &host, const int &port)
    : host(host), port(port) {
  CROW_ROUTE(app, "/").methods(crow::HTTPMethod::Get)(
      [&](const crow::request &request) { return home_handler(); });
  CROW_ROUTE(app, "/do-ocr")
      .methods(crow::HTTPMethod::Post)(
          [&](const crow::request &req) { return ocr_handler(req); });
}