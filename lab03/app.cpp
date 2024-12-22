# pragma GCC optimize("O3")
#include <iostream>
#include <cmath>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define pb push_back
#define ff first
#define ss second
#define INF 0x7f7f7f

const vector<vector<double>> GaussianKernel = {
    {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
    {2.0 / 16.0, 4.0 / 16.0, 2.0 / 16.0},
    {1.0 / 16.0, 2.0 / 16.0, 1.0 / 16.0},
};

// Sobel kernel (Y direction)
const vector<vector<double>> SobelKernelY = {
    {-1, -2, -1},
    {0, 0, 0},
    {1, 2, 1},
};

// Sobel kernel (X direction)
const vector<vector<double>> SobelKernelX = {
    {-1, 0, 1},
    {-2, 0, 2},
    {-1, 0, 1},
};


map<int, vector<pair<int, int>>> AngleMove; // 做 non maximum offset 的 mapping 關係


const string nameURL = "./images/name.png";

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

  return;
}


struct Image {
  string imageName;
  Mat imageOrigin, imageGaussian, imageSobelX, imageSobelY;
  Mat imageMagnitude;
  Mat houghImage;
  Mat lineImage;
  int height, width;
  map<int, int> gradientCnt;


  Image(string imagePath) {
    this -> imageOrigin = imread(imagePath);
    this -> imageName = getImageName(imagePath);
    this -> height = (this -> imageOrigin).rows;
    this -> width  = (this -> imageOrigin).cols;
    this -> imageName = getImageName(imagePath);
  }

  string getImageName(string imagePath) {
    size_t lastSlash = imagePath.find_last_of("/");
    string fileName = (lastSlash == string::npos) ? 
      imagePath : 
      imagePath.substr(lastSlash + 1);

    // 移除副檔名
    size_t lastDot = fileName.find_last_of(".");
    if (lastDot != string::npos) {
      return fileName.substr(0, lastDot);
    }
    return fileName;
  }

  // 實作一個通用介面來做卷積
  vector<double> getConvPixel(Mat background, int y, int x, vector<vector<double>> kernel) {
    vector<double> color(3, 0);
    for(int r = 0; r < 3; r++, y++) {
      for(int c = 0; c < 3; c++, x++) {
        Vec3b curColor = background.at<Vec3b>(y, x); 
        for(int k = 0; k < 3; k++) {
          color[k] += curColor[k] * kernel[r][c];
        }
      }
    }
    return color;
  }


  // 這邊生成有 padding 的圖片
  Mat getBackground(Mat image) {
    Mat background(height + ( 3 - 1 ), width + ( 3 - 1 ), CV_8UC3, Scalar(0, 0, 0));

    for(int i = 1; i <= height; i++) {
      for(int j = 1; j <= width; j++) {
        Vec3b color = image.at<Vec3b>(i - 1, j - 1);
        Vec3b &curColor = background.at<Vec3b>(i, j);
        for(int k = 0; k < 3; k++) {
          curColor[k] = color[k];
        }
      }
    }

    return background;
  }

  // 使用高斯函數去一些噪點
  void gaussianProcess() {
    (this -> imageGaussian) = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));
    Mat background = getBackground(imageOrigin);

    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        vector<double> colorOfPixel = getConvPixel(background, i, j, GaussianKernel);
        Vec3b &curColor = imageGaussian.at<Vec3b>(i, j);
        for(int k = 0; k < 3; k++) {
          curColor[k] = colorOfPixel[k];
        }
      }
    }
  }

  // 使用 sobel filter 的 X 和 Y，最後再合併
  void sobelFilter() {
    (this -> imageSobelX) = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));
    (this -> imageSobelY) = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));
    (this -> imageMagnitude) = Mat(height, width, CV_8UC3, Scalar(0, 0, 0));
    Mat background = getBackground(imageGaussian); // padding
    

    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        vector<double> colorOfPixelY = getConvPixel(background, i, j, SobelKernelY);
        vector<double> colorOfPixelX = getConvPixel(background, i, j, SobelKernelX);
        Vec3b &curColorY = imageSobelY.at<Vec3b>(i, j);
        Vec3b &curColorX = imageSobelX.at<Vec3b>(i, j);
        for(int k = 0; k < 3; k++) {
          curColorY[k] = colorOfPixelY[k];
          curColorX[k] = colorOfPixelX[k];
        }
      }
    }

    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b colorX = imageSobelX.at<Vec3b>(i, j);  
        Vec3b colorY = imageSobelY.at<Vec3b>(i, j);  
        Vec3b &curColor = imageMagnitude.at<Vec3b>(i, j);
        double gx = ( static_cast<double>(colorX[0]) + static_cast<double>(colorX[1]) + static_cast<double>(colorX[2]) ) / 3.0;
        double gy = ( static_cast<double>(colorY[0]) + static_cast<double>(colorY[1]) + static_cast<double>(colorY[2]) ) / 3.0;
        curColor[0] = curColor[1] = curColor[2] = abs(gx) + abs(gy);
      }
    }
    

    imshow("magnitude", imageMagnitude);
  }


  // 將角度量化為四個方向
  int getQuantizeAngle(double angle) {
    // 確保角度在 0-180 範圍內
    if (angle < 0) {
      angle += 180;
    }

    // 量化為四個方向
    if ((angle >= 0 && angle < 22.5) || (angle >= 157.5 && angle <= 180)) {
      return 0;     // 水平方向
    } else if (angle >= 22.5 && angle < 67.5) {
      return 45;    // 45度方向
    } else if (angle >= 67.5 && angle < 112.5) {
      return 90;    // 垂直方向
    } else {
      return 135;   // 135度方向 (112.5 ~ 157.5)
    }
  }

  // 將一些可能不是邊的點拿掉
  void nonMaximumProcess() {
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b colorX = imageSobelX.at<Vec3b>(i, j);
        Vec3b colorY = imageSobelY.at<Vec3b>(i, j);

        double gx = static_cast<double>(imageSobelX.at<Vec3b>(i, j)[0]);
        double gy = static_cast<double>(imageSobelY.at<Vec3b>(i, j)[0]);

        // 利用先前利用 SobelY 和 SobelX 算出來的 gy, gx 表，取得該點為邊的法向量
        double angle = atan2(gy, gx) * 270 / M_PI;

        // 把方向簡單化，只有四個方向
        int quantizedAngle = getQuantizeAngle(angle);
        gradientCnt[quantizedAngle]++;

        Vec3b &curColor = imageMagnitude.at<Vec3b>(i, j);
        int color = static_cast<int>(curColor[0]);
        for(pair<int, int> move : AngleMove[quantizedAngle]) {
          int newX = j + move.ff;
          int newY = i + move.ss;
          if ( newX < 0 || newX >= width || newY < 0 || newY >= height ) continue;
          int cmpColor = static_cast<int>(imageMagnitude.at<Vec3b>(newY, newX)[0]);
          if ( color < cmpColor ) {
            curColor[0] = curColor[1] = curColor[2] = 0;
            break;
          }
        }
      }
    }

    imshow("after-non-max", imageMagnitude);

  } 

  // 印出向量分佈 debug 用
  void showGradientCnt() {
    for(auto it : gradientCnt) {
      cout << it.first << ":  " << it.second << "\n";
    }
  }

  // strong and weak edge process
  // 先將邊劃分為 weak / non / strong 三個等級，再用八連通連接邊
  void edgeProcess() {
    int highThreshold = -1;
    int lowTrhreshod  = 0;
    for( int i = 0; i < height; i++ ) {
      for( int j = 0; j < width; j++ ) {
        int color = static_cast<int>(imageMagnitude.at<Vec3b>(i, j)[0]);
        highThreshold = max(highThreshold, color);
      }
    }

    highThreshold = highThreshold * (0.7);
    lowTrhreshod = highThreshold * (0.3);

    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b &curColor = imageMagnitude.at<Vec3b>(i, j);
        int repColor = static_cast<int>(curColor[0]);

        if ( repColor > highThreshold ) curColor[0] = curColor[1] = curColor[2] = 255; // strong
        else if ( repColor < lowTrhreshod ) curColor[0] = curColor[1] = curColor[2] = 0; // non
        else curColor[0] = curColor[1] = curColor[2] = 75; // weak
      }
    }



    // 4. 連接邊緣（Hysteresis）
    for(int i = 1; i < height-1; i++) {
      for(int j = 1; j < width-1; j++) {
        // 如果是弱邊緣點
        if(imageMagnitude.at<Vec3b>(i, j)[0] == 75) {
          // 檢查8鄰域是否有強邊緣點
          bool hasStrongNeighbor = false;
          for(int ki = -1; ki <= 1; ki++) {
            for(int kj = -1; kj <= 1; kj++) {
              if(imageMagnitude.at<Vec3b>(i+ki, j+kj)[0] == 255) {
                hasStrongNeighbor = true;
                break;
              }
            }
            if(hasStrongNeighbor) break;
          }
          // 如果有強邊緣鄰居，升級為強邊緣
          if(hasStrongNeighbor) {
            imageMagnitude.at<Vec3b>(i, j)[0] = 
              imageMagnitude.at<Vec3b>(i, j)[1] = 
              imageMagnitude.at<Vec3b>(i, j)[2] = 255;
          }
          // 否則抑制
          else {
            imageMagnitude.at<Vec3b>(i, j)[0] = 
              imageMagnitude.at<Vec3b>(i, j)[1] = 
              imageMagnitude.at<Vec3b>(i, j)[2] = 0;
          }
        }
      }
    }

    writeName(imageMagnitude);
    imshow("edge process", imageMagnitude);
    imwrite("./res/" + this->imageName + "_magnitude.jpeg", imageMagnitude);
  }


  // 霍夫轉換畫 theta, p 圖
  void houghTransform2() {
    // 1. 建立 Hough Table 並計算
    int maxRho = width + height;  // 最大的 p 就是 sin 和 cos 都是一的時候
    int maxVotes = -1;
    vector<vector<int>> houghTable(2 * maxRho + 1, vector<int>(180, 0)); // 投票表

    // 對每個角度，檢查所有邊緣點
    for(int theta = 0; theta < 180; theta++) {
      double thetaRad = theta * M_PI / 180.0;
      for(int i = 0; i < height; i++) {
        for(int j = 0; j < width; j++) {
          int curColor = static_cast<int>(imageMagnitude.at<Vec3b>(i, j)[0]);
          if(curColor == 255) {
            int p = j * cos(thetaRad) + i * sin(thetaRad) + maxRho; // 算出 p 值
            houghTable[p][theta]++; // 投票
            maxVotes = max(maxVotes, houghTable[p][theta]); // 取得最大票數
          }
        }
      }
    }

    // 2. 建立視覺化圖像
    int displayHeight = 800;
    int displayWidth = 600;
    houghImage = Mat(displayHeight, displayWidth, CV_8UC3, Scalar(0, 0, 0));

    // 計算縮放比例
    double rhoScale = (double)displayHeight / (2.0 * maxRho);
    double thetaScale = (double)displayWidth / 180.0;

    // 3. 繪製 Hough 空間
    for(int p = 0; p < 2 * maxRho; p++) {
      for(int theta = 0; theta < 180; theta++) {
        if(houghTable[p][theta] > 0) {
          // 計算起始和結束位置
          int startRho = floor(p * rhoScale);
          int endRho = floor((p + 1) * rhoScale);
          int startTheta = floor(theta * thetaScale);
          int endTheta = floor((theta + 1) * thetaScale);

          // 填充這個區域的所有像素
          for(int displayRho = startRho; displayRho < endRho && displayRho < displayHeight; displayRho++) {
            for(int displayTheta = startTheta; displayTheta < endTheta && displayTheta < displayWidth; displayTheta++) {
              double intensity = (double)houghTable[p][theta] / maxVotes;
              int pixelIntensity = 255 * intensity;
              houghImage.at<Vec3b>(displayRho, displayTheta) = 
                Vec3b(pixelIntensity, pixelIntensity, pixelIntensity);
            }
          }
        }
      }
    }


    // 4. 輸出一些 debug 訊息
    cout << "Max votes: " << maxVotes << "\n";
    cout << "Original Hough space size: " << 2 * maxRho << " x 180\n";
    cout << "Display size: " << displayHeight << " x " << displayWidth << "\n";

    writeName(houghImage);
    imshow("Hough Space", houghImage);
    imwrite("./res/" + this->imageName + "_hough.jpeg", houghImage);
    
    lineProcess(maxRho, maxVotes, houghTable);
  }


  // 霍夫畫線
  void lineProcess(int maxRho, int maxVotes, vector<vector<int>> &houghTable) {
    lineImage = imageMagnitude.clone();
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        if (  lineImage.at<Vec3b>(i, j)[0] == 255 ) {
          lineImage.at<Vec3b>(i, j) = Vec3b(75, 75, 75);
        }
      }
    }


    vector<pair<int, int>> lines;

    // 設定票數閥值
    int voteTreshold = maxVotes * 0.4;

    for(int p = 0; p < 2 * maxRho; p++) {
      for(int theta = 0; theta <= 180; theta++) {
        if ( houghTable[p][theta] > voteTreshold ) {
          // 將加上去的 offset 減回來
          lines.pb({theta, p - maxRho});
        }
      }
    }
    

    for(auto line : lines) {
      int theta = line.ff;
      int rho = line.ss;

      double thetaRad = theta * M_PI / 180.0;

      // 計算直線的兩個端點
      double cos_t = cos(thetaRad);
      double sin_t = sin(thetaRad);
      int x0 = rho * cos_t;
      int y0 = rho * sin_t;

      // 延伸直線
      Point pt1(x0 + 2000 * (-sin_t), y0 + 2000 * cos_t);
      Point pt2(x0 - 2000 * (-sin_t), y0 - 2000 * cos_t);

      // 畫線
      cv::line(lineImage, pt1, pt2, Scalar(255, 255, 255), 2);

    }

    writeName(lineImage);
    imshow("lines image", lineImage);
    imwrite("./res/" + this->imageName + "_lines.jpeg", lineImage);
    return;
  }
  
};


// mapping 角度(向量)和比較方向的關係
void init() {
  // x, y
  AngleMove[0] = vector<pair<int, int>>{{-1, 0}, {1, 0}};
  AngleMove[45] = vector<pair<int, int>>{{1, 1}, {-1, -1}};
  AngleMove[90] = vector<pair<int, int>>{{0, 1}, {0, -1}};
  AngleMove[135] = vector<pair<int, int>>{{-1, 1}, {1, -1}};
}

int main () {
  init(); 
  Image image("./images/playground.jpeg");


  // 順序不要動
  image.gaussianProcess();
  image.sobelFilter();
  image.nonMaximumProcess();
  image.showGradientCnt();
  image.edgeProcess();
  image.houghTransform2();


  waitKey(0); // 等待按鍵
  return 0;
}
