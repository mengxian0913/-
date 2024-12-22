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

    // 初始化 ( 可做可不做 )
    for(int i = 0; i < 256; i++) {
      rHis[i] = gHis[i] = bHis[i] = 0;
    }
  }

  // 取得檔名
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

  // 計算 R G B Value 出現的次數
  void calculateHis() {
    for(int i = 0; i < height; i++) {
      for(int j = 0; j < width; j++) {
        Vec3b &color = originImage.at<Vec3b>(i, j);
        bHis[color[0]]++; // 出現次數加一
        gHis[color[1]]++; // ..
        rHis[color[2]]++; // ..
      }
    }

    rHisImage  = generateHis(rHis);
    gHisImage  = generateHis(gHis);
    bHisImage  = generateHis(bHis);
    
    return;
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
}