# pragma GCC optimize("O3")
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const string imageURL = "./images/grayduck.png";

void getImageInfo(Mat image) {
  cout << "image info: \nheight: " << image.rows << "\nwidth: " << image.cols << "\n";
  cout << "------------------\n";
  return;
}


double getFilterPixel(int currentI, int currentJ, int n, int m, Mat background, vector<vector<double>>&kernel) {
  double calRes = 0;
  for(int i = 0; i < n; i++, currentI++) {
    for(int j = 0; j < m; j++, currentJ++) {
      Vec3b color = background.at<Vec3b>(currentI, currentJ);
      double averageColor = 0;
      for(int k = 0; k < 3; k++) {
        averageColor +=  ( kernel[i][j] * color[k] );
      }
      averageColor /= 3.0;
      calRes += averageColor;
    }
  }
  
  return calRes / 3.0;
}

void conv(Mat image, int n, int m, vector<vector<double>> &kernel, double bias, int padding) {

  if ( !(n & 1) || !( m & 1 ) ) {
    cerr << "Invaild value of n or m\n";
    return;
  }

  int height = image.rows;
  int width = image.cols;
  Mat background;
  Mat resultImage;

  if ( padding == -1 ) {
    background = Mat(height, width, CV_8UC3);
    resultImage = Mat(height - ( n - 1 ), width - (m - 1), CV_8UC3);

    for(int i = 0; i < image.rows; i++) {
      for(int j = 0; j < image.cols; j++) {
        Vec3b imageColor = image.at<Vec3b>(i, j);
        Vec3b &backgroundColor = background.at<Vec3b>(i, j);
        for(int k = 0; k < 3; k++) {
          backgroundColor[k] = imageColor[k];
        }
      }
    }
  } else {
    background = Mat(height + ( n - 1 ), width + ( m - 1 ), CV_8UC3, Scalar(padding, padding, padding));
    resultImage = Mat(height, width, CV_8UC3);

    for(int i = 0, bi = (n - 1) / 2; i < image.rows; i++, bi++) {
      for(int j = 0, bj = ( m - 1 ) / 2; j < image.cols; j++, bj++) {
        Vec3b imageColor = image.at<Vec3b>(i, j);
        Vec3b &backgroundColor = background.at<Vec3b>(bi, bj);
        for(int k = 0; k < 3; k++) {
          backgroundColor[k] = imageColor[k];
        }
      }
    }
  }


  // ------------------------------------------------------------------

  for(int i = 0; i <= background.rows - n; i++) {
    for(int j = 0; j <= background.cols - m; j++) {
      double filterColor = getFilterPixel(i, j, n, m, background, kernel) + bias;
      Vec3b &color = resultImage.at<Vec3b>(i, j);
      for(int k = 0; k < 3; k++) {
        color[k] = filterColor;
      }
    }
  }

  getImageInfo(resultImage);
  imshow("result image", resultImage);
  return;
}


int getTunnel(Mat image, int height, int width) {
  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      Vec3b color = image.at<Vec3b>(i, j);
      for(int k = 0; k < 2; k++) {
        if ( color[k] != color[k + 1] ) return 3;
      }
    }
  }
  return 1;
}

int main() {
  Mat image = imread(imageURL); 
  if (image.empty()) {
    cerr << "Could not open or find the image!" << endl;
    return -1;
  }

  int height = image.rows;
  int width = image.cols;
  int d = getTunnel(image, height, width);
  getImageInfo(image);
  // --------------------------

  int n, m;
  cout << "input n: ";
  cin >> n;
  cout << "input m: ";
  cin >> m;
  vector<vector<double>> kernel(n, vector<double>(m, 0));
  cout << "input kernel:\n";
  for(int i = 0; i < n; i++) {
    for(int j = 0; j < m; j++) {
      cin >> kernel[i][j];
    }
  }
  
  imshow("origin image", image);
  conv(image, n, m, kernel, 0, 1);
  
  waitKey(0); // 等待按鍵
  return 0;
}

