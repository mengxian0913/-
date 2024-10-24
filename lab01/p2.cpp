#pragma GCC optimize("O3")
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

#define INF (double)(1e9)

const string imageURL = "./images/duck.jpeg";

void getImageInfo(Mat image) {
  cout << "image info: \nheight: " << image.rows << "\nwidth: " << image.cols << "\n";
  cout << "------------------\n";
  return;
}

double getMaxPixel(int currentI, int currentJ, int n, Mat image) {
  double maxColor = -INF;
  for(int i = 0; i < n; i++, currentI++) {
    for(int j = 0; j < n; j++, currentJ++) {
      Vec3b color = image.at<Vec3b>(currentI, currentJ);
      double averageColor = 0;
      for(int k = 0; k < 3; k++) {
        averageColor += (double)(color[k]);
      }
      averageColor /= 3.0;
      maxColor = max(maxColor, averageColor);
    }
  }
  return maxColor;
}


double getAveragePixel( int currentI, int currentJ, int n, Mat image ) {
  double avgColor = 0;
  for(int i = 0; i < n; i++, currentI++) {
    for(int j = 0; j < n; j++, currentJ++) {
      Vec3b color = image.at<Vec3b>(currentI, currentJ);
      double averageColor = 0;
      for(int k = 0; k < 3; k++) {
        averageColor += (double)(color[k]);
      }
      averageColor /= 3.0;
      avgColor += averageColor;
    }
  }

  avgColor /= ( n * n );
  return avgColor;
}

Mat pool(Mat image, int n, int stride, int type) {
  int height = image.rows;
  int width = image.cols;
  vector<vector<vector<double>>> resultVec;
  for(int i = 0; i <= height - n; i += stride) {
    vector<vector<double>> colColor;
    for(int j = 0; j <= width - n; j += stride) {
      double filterValue = (type ? getAveragePixel(i, j, n, image)  : getMaxPixel(i, j, n, image));
      colColor.push_back({filterValue, filterValue, filterValue});
    }
    resultVec.push_back(colColor);
  }

  Mat resultImage(resultVec.size(), resultVec[0].size(), CV_8UC3);

  for(int i = 0; i < resultImage.rows; i++) {
    for(int j = 0; j < resultImage.cols; j++) {
      Vec3b &color = resultImage.at<Vec3b>(i, j);
      for(int k = 0; k < 3; k++) {
        color[k] = resultVec[i][j][k];
      }
    }
  }
  
  return resultImage;
}

int main() {
  int n, step, type;
  cout << "input n: ";
  cin >> n;
  cout << "input step: ";
  cin >> step;
  cout << "input type: ";
  cin >> type;

  Mat image = imread(imageURL); 
  if (image.empty()) {
    cerr << "Could not open or find the image!" << endl;
    return -1;
  }

  getImageInfo(image);
  imshow("origin image", image);

  Mat resultImage = pool(image, n, step, type);
  getImageInfo(resultImage);
  imshow("result image", resultImage);

  waitKey(0);


  return 0;
}
