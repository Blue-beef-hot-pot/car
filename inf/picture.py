import cv2
import numpy as np

def read_and_convert_image(image_path, target_width, target_height):
    # 读取图片
    image = cv2.imread(image_path)
    if image is None:
        raise ValueError("无法读取图片")
    
    # 转换为灰度图像
    gray_image = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
    
    # 调整大小
    resized_image = cv2.resize(gray_image, (target_width, target_height))
    
    return resized_image

image_path = "./inf/input/input6.jpg"
target_width = 128
target_height = 64
gray_image = read_and_convert_image(image_path, target_width, target_height)

# 将灰度图像数据保存到文件，以便C程序读取
np.savetxt("./inf/output/gray_image.txt", gray_image, fmt='%d')
