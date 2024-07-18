#pragma once
#include <opencv2/opencv.hpp>
#include <string>
#include <tesseract/baseapi.h>
#include <tuple>

namespace eagle {
class OCR {
  tesseract::TessBaseAPI *tesseractAPI = nullptr;
public:
  std::tuple<cv::Mat, float> resize_image(const cv::Mat &image);
  cv::Mat expose_contours(cv::Mat &image);
  std::vector<std::vector<cv::Point>> find_contours(const cv::Mat &contours_image);
  std::tuple<std::vector<cv::Point>, bool> get_document_rectangle_contours(
      const std::vector<std::vector<cv::Point>> &contours,
      const cv::Mat & image);
  std::tuple<cv::Mat, bool> get_bird_eye_view(const std::tuple<std::vector<cv::Point>, bool> & contours, cv::Mat & image, const float & scale);
  std::tuple<std::tuple<cv::Mat, std::string>, bool> peform_ocr(const cv::Mat & image);
  std::tuple<std::string, bool> get_utf8_text_with_tesseract(const std::tuple<cv::Mat, bool> & imaget);
  OCR();
  ~OCR();
};
} // namespace eagle