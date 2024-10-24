
import cv2
import numpy as np

def create_grayscale_image(width, height, value):
    # 生成一個指定寬度和高度的灰階圖像，所有像素的值設置為 value（0-255）
    grayscale_image = np.full((height, width), value, dtype=np.uint8)
    return grayscale_image

def main():
    width, height = 200, 100  # 圖像的寬度和高度
    value = 128  # 設置灰階值（0-255）

    # 創建灰階圖像
    grayscale_image = create_grayscale_image(width, height, value)

    # 保存圖像
    cv2.imwrite("./images/grayscale_image.png", grayscale_image)

    # 顯示圖像
    cv2.imshow("Grayscale Image", grayscale_image)
    cv2.waitKey(0)
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()

