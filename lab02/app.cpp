# pragma GCC optimize("O3")
#include <iostream>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

#define pb push_back
#define INF 0x7f7f7f

struct Image {
  string imageName;
  Mat originImage, imageAfterProcess;
  Mat rHisImage, bHisImage, gHisImage;
  Mat rEquaImage, bEquaImage, gEquaImage;
  int height, width;
  map<int, int> rHis, gHis, bHis;
  map<int, int> rEqua, gEqua, bEqua;
  vector<int> subSum;


  Image(const string imagePath) {
    this -> originImage  = imread(imagePath); 
    this -> height = (this -> originImage).rows;
    this -> width  = (this -> originImage).cols;
    this -> imageName = getImageName(imagePath);

    for(int i = 0; i < 256; i++) {
      rHis[i] = gHis[i] = bHis[i] = 0;
    }
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

  
  void calculateHis() {
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b &color = originImage.at<Vec3b>(i, j);
        bHis[color[0]]++;
        gHis[color[1]]++;
        rHis[color[2]]++;
      }
    }

    rHisImage  = generateHis(rHis);
    gHisImage  = generateHis(gHis);
    bHisImage  = generateHis(bHis);
    rEquaImage = generateEquaHis(rHis, rEqua);
    gEquaImage = generateEquaHis(gHis, gEqua);
    bEquaImage = generateEquaHis(bHis, bEqua);

    
    return;
  }

  Mat generateEquaHis(map<int, int>& color, map<int, int>& equaColor) {
    int sum = 0;
    for(int i = 0; i < 256; i++) {
      sum += color[i];
      subSum.pb(sum);
    }

    for(int i = 0; i < 256; i++) {
      int curKey = 255.0 * (( double )subSum[i] / (double)*subSum.rbegin());
      equaColor[curKey] += color[i]; 
    }

    return generateHis(equaColor);
  }


  Mat generateHis(map<int, int>& color) {
    int maxHeight = -INF;
    for(auto it : color) {
      maxHeight = max(maxHeight, it.second);
    }

    maxHeight += 1;

    Mat image(maxHeight, 256, CV_8UC3, Scalar(0, 0, 0));

    for(int j = 0; j < 256; j++) {
      int amountOfColor = color[j];
      for(int i = 0; i < maxHeight; i++) {
        if ( maxHeight - i <= amountOfColor ) {
          Vec3b &curColor = image.at<Vec3b>(i, j);
          for(int k = 0; k < 3; k++) curColor[k] = 255;
        }
      }
    }
    return image;
  }


  void saveAllImages() {
    string imagePrefix = "./res/" + imageName;
    imwrite(imagePrefix + "_origin.png", originImage);
    imwrite(imagePrefix + "_process.png", imageAfterProcess);

    imwrite(imagePrefix + "_R_HIS.png", rHisImage);
    imwrite(imagePrefix + "_G_HIS.png", gHisImage);
    imwrite(imagePrefix + "_B_HIS.png", bHisImage);

    imwrite(imagePrefix + "_B_EQUA.png", bHisImage);
    imwrite(imagePrefix + "_G_EQUA.png", gHisImage);
    imwrite(imagePrefix + "_R_EQUA.png", rHisImage);
    return;
  }


  void showAllHis() {
    imshow(imageName + " R HIS",  rHisImage);
    imshow(imageName + " R Equa", rEquaImage);
    imshow(imageName + " G HIS",  gHisImage);
    imshow(imageName + " G Equa", gEquaImage);
    imshow(imageName + " B HIS",  bHisImage);
    imshow(imageName + " B Equa", bEquaImage);
    return;
  }

  void showBeforeAfter() {
    imshow(imageName + " original Image", originImage);
    imshow(imageName + " processing Image", imageAfterProcess);
    return;
  }


  void transformImage() {
    Mat background(height, width, CV_8UC3, Scalar(0, 0, 0));
    imageAfterProcess = background;

    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b originColor = originImage.at<Vec3b>(i, j);
        Vec3b &curColor   = imageAfterProcess.at<Vec3b>(i, j);
        
        // b, g, r
        int bColorOfOrigin  = originColor[0];
        int bColorOfProcess = 255 * subSum[bColorOfOrigin] / *subSum.rbegin();
        curColor[0] = bColorOfProcess;

        int gColorOfOrigin  = originColor[1];
        int gColorOfProcess = 255 * subSum[gColorOfOrigin] / *subSum.rbegin();
        curColor[1] = gColorOfProcess;


        int rColorOfOrigin  = originColor[2];
        int rColorOfProcess = 255 * subSum[rColorOfOrigin] / *subSum.rbegin();
        curColor[2] = rColorOfProcess;
      }
    }
    return; 
  }

};

int main () {
  Image arr[4] = {Image("./images/tiger.jpeg"), Image("./images/woman.jpeg"), Image("./images/dog.jpeg"), Image("./images/Canada.png")};

  for(int i = 0; i < 4; i++) {
    arr[i].calculateHis();
    arr[i].transformImage();
    arr[i].saveAllImages();
    // arr[i].showAllHis();
    // arr[i].showBeforeAfter();
  }

  // waitKey(0);
  return 0;
}
