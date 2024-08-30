#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define WIDTH 128
#define HEIGHT 64
#define GRAYSCALE 256

int image[HEIGHT][WIDTH];

// 模糊大津法计算阈值
int fuzzyOTSU(int *image) {
    int N = HEIGHT * WIDTH;
    int Histogram[GRAYSCALE] = {0};
    int L1 = 130, L2 = 150, L3 = 0, L4 = 240;
    double uA[150] = {0}, uB[240] = {0}, uC[256] = {0};
    double PA = 0, PB = 0, PC = 0;
    double PAA[150] = {0}, PBB[240] = {0}, PCC[256] = {0};
    double mA = 0, mB = 0, mC = 0;
    double S = 0, preS = 0;
    int gray1 = 0, gray2 = 0;
    int temp_point[256] = {0}, temp_k = 0, temp_i = 0, Break_Point = 0;
    int temp1 = 0, temp2 = 0;

    int threshold = 0;
 
    // 统计灰度直方图
    for (int i = 0; i < N; i++) {
        int val = image[i];
        // if (val == 0) val = 1;
        Histogram[val]++;
    }

    // 寻找合适的分割点
    for (int i = 1; i < GRAYSCALE; i++) {
        temp_point[i] = temp_point[i - 1] + (Histogram[i] >= Histogram[i - 1] ? 1 : -1);
    }
    for (int i = 11; i < GRAYSCALE - 11; i++) {
        if (temp_point[i] < temp_point[i + 10] && temp_point[i] < temp_point[i - 10]) {
            temp_i += i;
            temp_k++;
        }
    }
    Break_Point = round((double)temp_i / temp_k);

    // 寻找α区域和B区域最高频率灰度值
    for (int i = 1; i <= Break_Point; i++) {
        if (temp1 < Histogram[i]) {
            temp1 = Histogram[i];
            gray1 = i;
        }
    }
    for (int i = Break_Point; i <= 255; i++) {
        if (temp2 < Histogram[i]) {
            temp2 = Histogram[i];
            gray2 = i;
        }
    }

    // 重新定义区域门限
    L1 = gray1;
    L2 = gray1 + 30;
    L4 = gray2;
    if (L4 <= L2) {
        L4 = L2 + 20;
    }

    // 求A类区域灰度值相关变量
    for (int i = 1; i <= L2; i++) {
        uA[i] = (i <= L1) ? 1 : ((i > L1 && i <= L2) ? (i / (double)(L1 - L2)) - (L2 / (double)(L1 - L2)) : 0);
        PA += uA[i] * Histogram[i];
    }
    for (int i = 1; i <= L2; i++) {
        PAA[i] = uA[i] * Histogram[i] / PA;
        mA += i * PAA[i];
    }
    PA /= N;

    // 滑动 L3 求阈值
    for (L3 = L2; L3 <= L4; L3++) {
        mB = 0; mC = 0; PB = 0; PC = 0;
        for (int i = L1; i <= L4; i++) {
            uB[i] = (i >= L1 && i < L2) ? (i / (double)(L2 - L1)) - (L1 / (double)(L2 - L1)) : 
                    (i >= L2 && i <= L3) ? 1 : 
                    (i > L3 && i <= L4) ? (i / (double)(L3 - L4)) - (L4 / (double)(L3 - L4)) : 0;
            PB += uB[i] * Histogram[i];
        }
        for (int i = L3; i <= GRAYSCALE; i++) {
            uC[i] = (i < L4 && i >= L3) ? (i / (double)(L4 - L3)) - (L3 / (double)(L4 - L3)) : 
                    (i >= L4) ? 1 : 0;
            PC += uC[i] * Histogram[i];
        }
        for (int i = L1; i <= L4; i++) {
            PBB[i] = uB[i] * Histogram[i] / PB;
            mB += i * PBB[i];
        }
        for (int i = L3; i <= GRAYSCALE; i++) {
            PCC[i] = uC[i] * Histogram[i] / PC;
            mC += i * PCC[i];
        }
        PB /= N;
        PC /= N;
        S = PA * PB * PC * (mC - mA) * (mC - mB) * (mB - mA);
        if (S > preS) {
            preS = S;
            threshold = (L1 + L2 + L3) / 3;
        }
    }

    return threshold;
}

// 颜色阈值分割
void thresholdImage(int image[HEIGHT][WIDTH], int threshold) {
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            image[i][j] = (image[i][j] > threshold) ? 255 : 0;
        }
    }
}

// 形态学操作：开运算（先腐蚀后膨胀）
void morphologyOpen(int image[HEIGHT][WIDTH]) {
    int temp[HEIGHT][WIDTH];
    // 腐蚀
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int min = 255;
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int ni = i + k, nj = j + l;
                    if (ni >= 0 && ni < HEIGHT && nj >= 0 && nj < WIDTH && image[ni][nj] < min) {
                        min = image[ni][nj];
                    }
                }
            }
            temp[i][j] = min;
        }
    }
    // 膨胀
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int max = 0;
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int ni = i + k, nj = j + l;
                    if (ni >= 0 && ni < HEIGHT && nj >= 0 && nj < WIDTH && temp[ni][nj] > max) {
                        max = temp[ni][nj];
                    }
                }
            }
            image[i][j] = max;
        }
    }
}

// 形态学操作：闭运算（先膨胀后腐蚀）
void morphologyClose(int image[HEIGHT][WIDTH]) {
    int temp[HEIGHT][WIDTH];
    // 膨胀
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int max = 0;
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int ni = i + k, nj = j + l;
                    if (ni >= 0 && ni < HEIGHT && nj >= 0 && nj < WIDTH && image[ni][nj] > max) {
                        max = image[ni][nj];
                    }
                }
            }
            temp[i][j] = max;
        }
    }
    // 腐蚀
    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            int min = 255;
            for (int k = -1; k <= 1; k++) {
                for (int l = -1; l <= 1; l++) {
                    int ni = i + k, nj = j + l;
                    if (ni >= 0 && ni < HEIGHT && nj >= 0 && nj < WIDTH && temp[ni][nj] < min) {
                        min = temp[ni][nj];
                    }
                }
            }
            image[i][j] = min;
        }
    }
}

void generateImage(int image[HEIGHT][WIDTH]) {
    int centerX = WIDTH / 2;
    int centerY = HEIGHT / 2;
    int maxBrightness = 255; // 最大亮度

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            // 计算当前像素到图像中心的距离
            int distance = abs(centerX - j);
            image[i][j] = (distance < 30) ? 200 : (distance < 32) ? 40 : 100;
        }
    }
}

void saveImageToFile(const char *filename, int image[HEIGHT][WIDTH]) {
    FILE *file = fopen(filename, "wb");
    if (!file) {
        perror("无法打开文件");
        return;
    }

    // BMP文件头
    unsigned char bmpFileHeader[14] = {
        'B', 'M', // 文件类型
        0, 0, 0, 0, // 文件大小（稍后填充）
        0, 0, // 保留字
        0, 0, // 保留字
        54, 0, 0, 0 // 数据偏移
    };

    // BMP信息头
    unsigned char bmpInfoHeader[40] = {
        40, 0, 0, 0, // 信息头大小
        0, 0, 0, 0, // 图像宽度（稍后填充）
        0, 0, 0, 0, // 图像高度（稍后填充）
        1, 0, // 颜色平面数
        8, 0, // 每像素位数
        0, 0, 0, 0, // 压缩类型
        0, 0, 0, 0, // 图像数据大小（稍后填充）
        0, 0, 0, 0, // 水平分辨率
        0, 0, 0, 0, // 垂直分辨率
        256, 0, 0, 0, // 颜色数
        0, 0, 0, 0 // 重要颜色数
    };

    int rowSize = (WIDTH * 8 + 31) / 32 * 4; // 每行字节数（4字节对齐）
    int imageSize = rowSize * HEIGHT;
    int fileSize = 54 + imageSize;

    bmpFileHeader[2] = (unsigned char)(fileSize);
    bmpFileHeader[3] = (unsigned char)(fileSize >> 8);
    bmpFileHeader[4] = (unsigned char)(fileSize >> 16);
    bmpFileHeader[5] = (unsigned char)(fileSize >> 24);

    bmpInfoHeader[4] = (unsigned char)(WIDTH);
    bmpInfoHeader[5] = (unsigned char)(WIDTH >> 8);
    bmpInfoHeader[6] = (unsigned char)(WIDTH >> 16);
    bmpInfoHeader[7] = (unsigned char)(WIDTH >> 24);

    bmpInfoHeader[8] = (unsigned char)(HEIGHT);
    bmpInfoHeader[9] = (unsigned char)(HEIGHT >> 8);
    bmpInfoHeader[10] = (unsigned char)(HEIGHT >> 16);
    bmpInfoHeader[11] = (unsigned char)(HEIGHT >> 24);

    bmpInfoHeader[20] = (unsigned char)(imageSize);
    bmpInfoHeader[21] = (unsigned char)(imageSize >> 8);
    bmpInfoHeader[22] = (unsigned char)(imageSize >> 16);
    bmpInfoHeader[23] = (unsigned char)(imageSize >> 24);

    // 写入文件头和信息头
    fwrite(bmpFileHeader, 1, 14, file);
    fwrite(bmpInfoHeader, 1, 40, file);

    // 写入调色板
    unsigned char palette[1024];
    for (int i = 0; i < 256; i++) {
        palette[i * 4 + 0] = i; // 蓝色
        palette[i * 4 + 1] = i; // 绿色
        palette[i * 4 + 2] = i; // 红色
        palette[i * 4 + 3] = 0; // 保留
    }
    fwrite(palette, 1, 1024, file);

    // 写入图像数据
    unsigned char *row = (unsigned char *)malloc(rowSize);
    for (int y = HEIGHT - 1; y >= 0; y--) {
        for (int x = 0; x < WIDTH; x++) {
            row[x] = image[y][x];
        }
        fwrite(row, 1, rowSize, file);
    }

    free(row);
    fclose(file);
}

int main() {
    // 执行Python脚本
    // if (system("./picture.py") != 0) {
    //     printf("执行Python脚本失败\n");
    //     return 1;
    // }

    // 读取灰度图像数据
    FILE *file = fopen("./output/gray_image.txt", "r");
    if (!file) {
        perror("无法打开文件");
        return 1;
    }

    for (int i = 0; i < HEIGHT; i++) {
        for (int j = 0; j < WIDTH; j++) {
            fscanf(file, "%d", &image[i][j]);
        }
    }
    fclose(file);

    // 保存图像到文件
    saveImageToFile("./output/output.bmp", image);
    printf("图像已保存到 output.bmp\n");

    // // 使用模糊大津法计算阈值
    // int threshold = fuzzyOTSU((int *)image);
    // printf("模糊大津法计算的阈值：%d\n", threshold);

    // 使用模糊大津法计算阈值
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); // 开始计时
    int threshold = fuzzyOTSU((int *)image);
    clock_gettime(CLOCK_MONOTONIC, &end); // 结束计时
    double time_taken = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9; // 计算时间
    printf("模糊大津法计算的阈值：%d\n", threshold);
    printf("模糊大津法计算时间：%f 秒\n", time_taken);

    // 颜色阈值分割
    thresholdImage(image, threshold);

    // 保存图像到文件
    saveImageToFile("./output/output1.bmp", image);
    printf("图像已保存到 output1.bmp\n");

    return 0;
}
