#include "opencv2/core/mat.hpp"
// #include <leptonica/allheaders.h>
#include <opencv2/opencv.hpp>
#include <string>
#include <tesseract/baseapi.h>
#include "cv/ocr.hpp"
#include <crow/logging.h>

using namespace std;
using namespace cv;

int main(int argc, char *argv[]) {

  if (argc != 2) {
    cout << "Usage: " << argv[0] << " <image_path>" << endl;
    return 1;
  }

  std::string outText, imPath = argv[1];

  cv::Mat im = cv::imread(imPath, IMREAD_COLOR);
  
  eagle::OCR eagleOcr;

  const auto & [resultt, sucess] = eagleOcr.peform_ocr(im);
  const auto & [mat, text] = resultt;
  CROW_LOG_INFO << "OCR result: " << text;

  return 0;
}