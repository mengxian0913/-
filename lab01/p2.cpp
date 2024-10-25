#pragma GCC optimize("O3")
#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv; 

#define INF (double)(1e9)

const string imageURL = "./images/otters.jpg";
const string nameURL = "./images/name2.png";

void getImageInfo(Mat image) {
  cout << "image info: \nheight: " << image.rows << "\nwidth: " << image.cols << "\n";
  cout << "------------------\n";
  return;
}


void writeName(Mat &image) {
  Mat nameImage   = imread(nameURL); 
  int nameHeight  = nameImage.rows;
  int nameWidth   = nameImage.cols;
  int startHeight = image.rows - nameHeight;
  int startWidth  = image.cols - nameWidth;
  for(int i = startHeight, ni = 0; ni < nameHeight; i++, ni++) {
    for(int j = startWidth, nj = 0; nj < nameWidth; j++, nj++) {
      Vec3b &imageColor = image.at<Vec3b>(i, j);
      Vec3b &nameColor = nameImage.at<Vec3b>(ni, nj);
      if ( (int)nameColor[0] > 200 && (int)nameColor[1] > 200 && (int)nameColor[2] > 200 ) {
        for(int k = 0; k < 3; k++) {
          imageColor[k] = nameColor[k];
        }
      } 
    }
  }
  getImageInfo(nameImage);

  return;
}


double getMaxPixel(int currentI, int currentJ, int n, Mat image) {
  double maxColor = -INF; // 最大的 pixel
  for(int i = 0; i < n; i++, currentI++) {
    for(int j = 0; j < n; j++, currentJ++) {
      Vec3b color = image.at<Vec3b>(currentI, currentJ);
      double averageColor = 0;
      for(int k = 0; k < 3; k++) {
        averageColor += (double)(color[k]);
      }
      averageColor /= 3.0; // 取得 r/g/b 的平均
      maxColor = max(maxColor, averageColor); // 計算最大的 maxColor
    }
  }
  return maxColor;
}


double getAveragePixel( int currentI, int currentJ, int n, Mat image ) {
  double avgColor = 0; // pixel 平均的 color 
  for(int i = 0; i < n; i++, currentI++) {
    for(int j = 0; j < n; j++, currentJ++) {
      Vec3b color = image.at<Vec3b>(currentI, currentJ);
      double averageColor = 0;
      for(int k = 0; k < 3; k++) {
        averageColor += (double)(color[k]);
      }
      averageColor /= 3.0; // get average of r/g/b
      avgColor += averageColor;
    }
  }

  avgColor /= ( n * n ); // get average of matrix n * n
  return avgColor;
}

Mat pool(Mat image, int n, int stride, int type) {
  int height = image.rows; // 取得 image 的高度
  int width  = image.cols; // 取得 image 的寬度
  vector<vector<vector<double>>> resultVec; 
  /*
   * 創建一型態為 3 維 vector 的 resultVec 來儲存結果圖片需要的資訊。
   * 之後會將它轉成 Mat 型態。
   * */
  for(int i = 0; i <= height - n; i += stride) {
    vector<vector<double>> colColor;
    for(int j = 0; j <= width - n; j += stride) {
      // 根據 type 決定 average 或是 max
      double filterValue = (!type ? getAveragePixel(i, j, n, image)  : getMaxPixel(i, j, n, image));
      colColor.push_back({filterValue, filterValue, filterValue});
    }
    resultVec.push_back(colColor);
  }

  Mat resultImage(resultVec.size(), resultVec[0].size(), CV_8UC3);

  // 將 resultVec 的資訊塞到 resultImage 
  for(int i = 0; i < resultImage.rows; i++) {
    for(int j = 0; j < resultImage.cols; j++) {
      Vec3b &color = resultImage.at<Vec3b>(i, j);
      // 取得 color 指標
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

  Mat resultImage = pool(image, n, step, type);
  writeName(resultImage);
  getImageInfo(resultImage);
  // imwrite("./result/pool_otters_maximum.png", resultImage);
  imshow("result image", resultImage);

  waitKey(0);

  return 0;
}
