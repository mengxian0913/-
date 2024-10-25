# pragma GCC optimize("O3")
#include <iostream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

const string imageURL = "./images/me.JPG";
const string nameURL = "./images/name.png";

void getImageInfo(Mat image) {
  cout << "image info: \nheight: " << image.rows << "\nwidth: " << image.cols << "\n";
  cout << "------------------\n";
  return;
}


// 姓名烙印
void writeName(Mat &image) {
  Mat nameImage   = imread(nameURL); // 打開姓名圖片檔案 
  int nameHeight  = nameImage.rows; // 取得高度
  int nameWidth   = nameImage.cols; // 取得寬度
  int startHeight = image.rows - nameHeight; // 取得 y 軸起始位置
  int startWidth  = image.cols - nameWidth; // 取得 x 軸起始位置
  /* 
   * 遍歷 image 的 pixel 從 startHeight 到底和從 startWidth 到底
   * 並且將 nameImage 為白色的地方烙印在對應的 image 座標上。
  */
  for(int i = startHeight, ni = 0; ni < nameHeight; i++, ni++) {
    for(int j = startWidth, nj = 0; nj < nameWidth; j++, nj++) {
      Vec3b &imageColor = image.at<Vec3b>(i, j); // 取得該座標的顏色
      Vec3b &nameColor = nameImage.at<Vec3b>(ni, nj); // 取得該座標的顏色
      if ( (int)nameColor[0] > 200 && (int)nameColor[1] > 200 && (int)nameColor[2] > 200 ) {
        for(int k = 0; k < 3; k++) {
          imageColor[k] = nameColor[k];
        }
      } 
    }
  }

  getImageInfo(nameImage); // 打印圖片的相關資訊

  return;
}


double getFilterPixel(int currentI, int currentJ, int currentK, int n, int m, Mat background, vector<vector<double>>&kernel) {
  double calRes = 0;
  for(int i = 0; i < n; i++, currentI++) {
    for(int j = 0; j < m; j++, currentJ++) {
      Vec3b color = background.at<Vec3b>(currentI, currentJ);
      // 把從 (0, 0) 到 (n, m) 對應到 (currentI, currentJ) 的 color[currentK] ( 可能是 r/g/b ) 計算相乘累加
      calRes += ( color[currentK] * kernel[i][j] );
    }
  }
  return calRes;
}

Mat conv(Mat image, int n, int m, vector<vector<double>> &kernel, double bias, int padding) {

  // 判斷高度和寬度是否合理 ( 不能為偶數 )
  if ( !(n & 1) || !( m & 1 ) ) {
    cerr << "Invaild value of n or m\n";
    exit(0);
  }

  int height = image.rows; // 取得 image 高度
  int width = image.cols; // 取得 image 寬度
  Mat background; // 創建一個圖片作為加上 padding 後的圖片
  Mat resultImage; // 儲存過濾後的結果

  // 如果不需要加 padding
  if ( padding == -1 ) {
    background = Mat(height, width, CV_8UC3); // 設定 background 寬高。
    resultImage = Mat(height - ( n - 1 ), width - (m - 1), CV_8UC3); // 設定 resultImage 寬高。

    // 把 image 的 color 塞到 background
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
    background = Mat(height + ( n - 1 ), width + ( m - 1 ), CV_8UC3, Scalar(padding, padding, padding)); // 設定 background 的寬高。
    resultImage = Mat(height, width, CV_8UC3); // 設定 resultImage 的寬高。

    // 把 image 的 color 塞到 backgorund 對應的座標
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
  // 計算 background 和 kernel 過濾後的結果塞到 resultImage
  // getFilterPixel 會計算遮罩內所有的 r/g/b 的平均
  for(int i = 0; i <= background.rows - n; i++) {
    for(int j = 0; j <= background.cols - m; j++) {
      Vec3b &color = resultImage.at<Vec3b>(i, j);
      for(int k = 0; k < 3; k++) {
        color[k] = getFilterPixel(i, j, k, n, m, background, kernel) + bias;
      }
    }
  }

  return resultImage;
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

  int n, m, bias, padding;
  cout << "input n: ";
  cin >> n;
  cout << "input m: ";
  cin >> m;
  cout << "input bias: ";
  cin >> bias;
  cout << "input padding: ";
  cin >> padding;
  vector<vector<double>> kernel(n, vector<double>(m, 0));
  cout << "input kernel:\n";
  for(int i = 0; i < n; i++) {
    for(int j = 0; j < m; j++) {
      cin >> kernel[i][j];
    }
  }
  
  imshow("origin image", image);
  Mat resultImage = conv(image, n, m, kernel, bias, padding);
  
  writeName(resultImage);
  
  getImageInfo(resultImage);
  // imwrite("./result/conv_me_average.png", resultImage); // 寫入結果
  imshow("result image", resultImage);
  
  waitKey(0); // 等待按鍵
  return 0;
}

