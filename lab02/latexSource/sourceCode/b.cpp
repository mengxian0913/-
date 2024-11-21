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

  
  void calculateHis() {
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b &color = originImage.at<Vec3b>(i, j);
        bHis[color[0]]++;
        gHis[color[1]]++;
        rHis[color[2]]++;
      }
    }


    rEquaImage = generateEquaHis(rHis, rEqua);
    gEquaImage = generateEquaHis(gHis, gEqua);
    bEquaImage = generateEquaHis(bHis, bEqua);

    
    return;
  }

  // 計算均化後的直方圖表
  Mat generateEquaHis(map<int, int>& color, map<int, int>& equaColor) {
    // 兩個參數對應的是均化前和後
    // 計算前綴和
    int sum = 0;
    for(int i = 0; i < 256; i++) {
      sum += color[i];
      subSum.pb(sum);
    }

    // 計算新的 key 值，參考以上公式
    for(int i = 0; i < 256; i++) {
      int curKey = 255.0 * (( double )subSum[i] / (double)*subSum.rbegin());
      equaColor[curKey] += color[i]; 
    }

    return generateHis(equaColor);
  }

  // 根據直方圖的 map 生成直方圖
  Mat generateHis(map<int, int>& color) {
    // 先尋找直方圖的最大高度
    int maxHeight = -INF;
    for(auto it : color) {
      maxHeight = max(maxHeight, it.second);
    }

    maxHeight += 1;

    Mat image(maxHeight, 256, CV_8UC3, Scalar(0, 0, 0));

    // j 就是 x，也就是 0 ~ 255
    for(int j = 0; j < 256; j++) {
      int amountOfColor = color[j];
      for(int i = 0; i < maxHeight; i++) {
        // 如果當前 y 軸的值比 color[i] 來得小那就標記白色(255)
        if ( maxHeight - i <= amountOfColor ) {
          Vec3b &curColor = image.at<Vec3b>(i, j);
          for(int k = 0; k < 3; k++) curColor[k] = 255;
        }
      }
    }
    return image;
  }

  // 生成均化後的圖片
  void transformImage() {
    Mat background(height, width, CV_8UC3, Scalar(0, 0, 0));
    imageAfterProcess = background;

    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b originColor = originImage.at<Vec3b>(i, j);
        Vec3b &curColor   = imageAfterProcess.at<Vec3b>(i, j);
        
        // 根據 b, g, r 計算新的數值並替換
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