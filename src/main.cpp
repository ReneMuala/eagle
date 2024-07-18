#include "opencv2/core/mat.hpp"
// #include <leptonica/allheaders.h>
#include "cv/ocr.hpp"
#include "http/server.hpp"
#include <crow/logging.h>
#include <cstdlib>
#include <opencv2/opencv.hpp>
#include <string>
#include <tesseract/baseapi.h>

using namespace std;
using namespace cv;

void run_http_mode(int port) {
  CROW_LOG_INFO << __FUNCTION__ << " at " << port;
  eagle::Server server("0.0.0.0", port);
  server.run();
}

void run_fs_mode(std::string filename) {
  CROW_LOG_INFO << __FUNCTION__ << " with " << filename;

  cv::Mat im = cv::imread(filename, IMREAD_COLOR);

  eagle::OCR eagleOcr;

  const auto &[resultt, sucess] = eagleOcr.peform_ocr(im);
  const auto &[mat, text] = resultt;
  CROW_LOG_INFO << "OCR result: " << text;
}

int main(int argc, char *argv[]) {
  if (argc == 2) {
    run_fs_mode(argv[1]);
  } else if (argc == 3 and std::string(argv[1]) == "http") {
    int port = atoi(argv[2]);
    if (port == 0) {
      CROW_LOG_ERROR << __FUNCTION__
                     << " wrong value for param port: " << argv[2];
    } else
      run_http_mode(port);
  } else {
    cout << "Usage: "
    << "\t" << argv[0] << " <image_path>" << endl
    << "\t" << argv[0] << " http" << " <port>" << endl;
    return 1;
  }

  return 0;

  return 0;
}