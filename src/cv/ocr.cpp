#include "ocr.hpp"
#include "../imutils/perspective.h"
#include <tuple>
#include <crow/logging.h>

using namespace eagle;

std::tuple<cv::Mat, float> OCR::resize_image(const cv::Mat &image) {
  cv::Mat resized;
  const float percentage = 600.0 / image.cols;
  const float reverse_percentage = 1 - percentage;
  cv::resize(image, resized, cv::Size(), percentage, percentage);
  return {resized, reverse_percentage};
}

cv::Mat OCR::expose_contours(cv::Mat &image) {
  cv::Mat im = image.clone(), im2 = im.clone();
  cv::GaussianBlur(im, im, cv::Size(101, 101), 4.0);
  cv::cvtColor(im, im, cv::COLOR_RGB2GRAY);
  cv::Canny(im, im, 50, 150, 5);
  imwrite("0-canny-edges.jpeg", im);
  return im;
}

std::vector<std::vector<cv::Point>>
OCR::find_contours(const cv::Mat &contours_image) {
  std::vector<std::vector<cv::Point>> contours;
  cv::findContours(contours_image, contours, cv::RETR_EXTERNAL,
                   cv::CHAIN_APPROX_SIMPLE);
  return std::move(contours);
}

std::tuple<std::vector<cv::Point>, bool> OCR::get_document_rectangle_contours(
    const std::vector<std::vector<cv::Point>> &contours, const cv::Mat &image) {
  std::vector<cv::Point> coordinates;
  cv::Mat copy = image.clone();
  int i = 0; // [-]
  for (const auto &contour : contours) {
    const auto &rect = boundingRect(contour);
    if (rect.width > (0.5 * image.cols) && rect.height > (0.3 * image.rows)) {
      auto epsilon = 0.04 * cv::arcLength(contour, true);
      cv::approxPolyDP(contour, coordinates, epsilon, true);
      if (coordinates.size() == 4) {
        drawContours(copy, contours, i, cv::Scalar(255, 255, 0), 2); // [-]
        CROW_LOG_INFO << "DONE: " << __FUNCTION__;
        imwrite("1-detected-document.jpg", copy); // [-]
        return {coordinates, true};
      }
    }
    i++; // [-]
  }
  CROW_LOG_INFO << "ERROR on " << __FUNCTION__;
  return {coordinates, false};
}

std::tuple<cv::Mat, bool>
OCR::get_bird_eye_view(const std::tuple<std::vector<cv::Point>, bool> &contours,
                       cv::Mat &image, const float &scale) {
  cv::Mat result;

  if (not std::get<1>(contours)) {
    CROW_LOG_INFO << "ERROR on " << __FUNCTION__;
    return {result, false};
  }

  const auto &points = std::get<0>(contours);

  const cv::Point_<float> tl = {(float)points[0].x * scale,
                                (float)points[0].y * scale};
  const cv::Point_<float> bl = {(float)points[1].x * scale,
                                (float)points[1].y * scale};
  const cv::Point_<float> tr = {(float)points[3].x * scale,
                                (float)points[3].y * scale};
  const cv::Point_<float> br = {(float)points[2].x * scale,
                                (float)points[2].y * scale};

  result = Perspective::fourPointTransformation(image, {
                                                           tl,
                                                           bl,
                                                           br,
                                                           tr,
                                                       });
  GaussianBlur(result, result, cv::Size(3, 3), 1.0);
  CROW_LOG_INFO << "DONE: " << __FUNCTION__;

  imwrite("2-bird-eye-view.jpg", result);

  return {result, true};
}

std::tuple<std::tuple<cv::Mat, std::string>, bool>
OCR::peform_ocr(const cv::Mat &original_image) {

  CROW_LOG_INFO << "BEGINING: " << __FUNCTION__;

  auto [image, reverse_percentage] = resize_image(original_image);

  const cv::Mat exposed_contours = expose_contours(image);

  const std::vector<std::vector<cv::Point>> found_contours =
      find_contours(exposed_contours);

  const std::tuple<std::vector<cv::Point>, bool> document_rectangle_contourst =
      get_document_rectangle_contours(found_contours, image);

  const std::tuple<cv::Mat, bool> bird_eye_viewt =
      get_bird_eye_view(document_rectangle_contourst, image, 1);

  const std::tuple<std::string, bool> resultt =
      get_utf8_text_with_tesseract(bird_eye_viewt);

  CROW_LOG_INFO << "ENDING: " << __FUNCTION__;

  return {{std::get<0>(bird_eye_viewt), std::get<0>(resultt)},
          std::get<1>(bird_eye_viewt)};
}

std::tuple<std::string, bool>
OCR::get_utf8_text_with_tesseract(const std::tuple<cv::Mat, bool> &imaget) {
  std::string result;
  const auto &[image, is_valid] = imaget;

  if (not is_valid) {
    CROW_LOG_INFO << "ERROR on " << __FUNCTION__;
    return {result, false};
  }
  tesseractAPI->SetImage(image.data, image.cols, image.rows, 3, image.step);
  CROW_LOG_INFO << "DOING: "
                << "Recognize";

  tesseractAPI->Recognize(0);

  CROW_LOG_INFO << "BEGIN: " << __FUNCTION__;
  char *result_cstr = tesseractAPI->GetUTF8Text();
  CROW_LOG_INFO << "DONE: " << __FUNCTION__;
  result = result_cstr;
  delete[] result_cstr;
  return {result, true};
}

OCR::OCR() {
  CROW_LOG_INFO << "Initializig tesseract API";
  tesseractAPI = new tesseract::TessBaseAPI();
  tesseractAPI->Init("./", "por", tesseract::OEM_LSTM_ONLY);
  tesseractAPI->SetPageSegMode(tesseract::PSM_AUTO);
  CROW_LOG_INFO << "Tesseract API is ready";
}

OCR::~OCR() {
  if (tesseractAPI) {
    CROW_LOG_INFO << "Destroyng tesseract API";
    tesseractAPI->Clear();
    tesseractAPI->End();
    delete tesseractAPI;
    tesseractAPI = nullptr;
    CROW_LOG_INFO << "Tesseract API is destroyed";
  }
}