#include <stdio.h>
#include <math.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0
#define ROW 60
#define COL 80
#define MID_POINT 40   //中值,调大轨迹偏左，调小轨迹偏右
#define Lstart                  1
#define Rstart                  2
#define Mstart                  3

typedef struct {
  //图像X坐标
  uint8_t ui8_ImageX;
  //图像Y坐标
  uint8_t ui8_ImageY;
  //图像单边最远点
  uint8_t ui8_MaxY;
  //图像最远点
  uint8_t ui8_AllMaxY;
} LadderMovePoint;

typedef struct {
    //状态判断计数
  uint16_t        ui16_counter[5];
  //状态标志
  int8_t          i8_StatusFlag[9];
  //状态处理变量
  int8_t          i8_StatusHandle[9];
  //图像处理范围
  uint8_t         ui8_DisposeScopeUp;
  uint8_t         ui8_DisposeScopeDown;
  uint8_t         ui8_DisposeScopeLeft;
  uint8_t         ui8_DisposeScopeRight;
    //直道行位置
  float        f_BaseY[10];
    //标准数组
  uint8_t*        ui8_LineWidth;
    //标准权重
  float        f_BaseLineWeight[10];
      //行权重
  float        f_LineWeight[10];
  //控制量数组
  int16_t*        i16p_dataImage;
  //图像数组
  uint8_t         ui8_ImageArray[ROW][COL];
  //左边界
  int8_t         ui8_LPoint[ROW];
  //右边界
  int8_t         ui8_RPoint[ROW];
  //扫描行距离
  uint8_t         ui8_ScanLineY[10];
  //扫描行左边界(补线)
  uint8_t         ui8_ScanLineL[10];
  //扫描行右边界(补线)
  uint8_t         ui8_ScanLineR[10];
  //扫描行左边界(最边界)
  uint8_t         ui8_ScanLineToL[10];
  //扫描行右边界(最边界)
  uint8_t         ui8_ScanLineToR[10];
  //扫描赛道宽度
  uint8_t         ui8_ScanLineWidth[10];
  //中值求取起点
  uint8_t         ui8_ScanDirection;
  //初始中值
  int16_t         i16_Mid[10];
  //最终中值
  int16_t         i16_FinallyMid[10];
  //最小可视距离
  uint8_t         ui8_MinH;
  int8_t          i8_MinHX;
  //反向可视距离
  uint8_t         VisitableScope;
  //爬梯最远点
  uint8_t        MaxPoint;
  uint8_t         ui8_LRealPoint[ROW];                    //左线爬梯边界
  uint8_t         ui8_RRealPoint[ROW];                    //右线爬梯边界
} Dispose_Image;

LadderMovePoint         L_Move;
//左右两点爬梯算法右点
LadderMovePoint         R_Move;

Dispose_Image DI={
   /*（环岛）状态判定计数*/ 
  {FALSE},  
   /*(直道）（环岛）*/ 
  {FALSE},
     /*(直道）（环岛）*/ 
  {FALSE},
    /*图像处理范围*/
  /*上*/ 0, /*下*/ 59, /*左*/ 5, /*右*/ 74,
    /*直道行距*/   
  { 55.0, 44.0, 37.0, 28.0, 20.0, 13.0, 8.0, 4.0, 2.0, 1.0 },
};

#define SIZE_HRE 10

void Ladder(void) {
    uint8_t ui8_LF[SIZE_HRE] = {0};
    uint8_t ui8_RF[SIZE_HRE] = {0};

    L_Move.ui8_ImageX = DI.ui8_DisposeScopeLeft;
    L_Move.ui8_ImageY = DI.ui8_DisposeScopeDown;
    L_Move.ui8_MaxY = DI.ui8_DisposeScopeDown;
    L_Move.ui8_AllMaxY = DI.ui8_DisposeScopeDown;

    R_Move.ui8_ImageX = DI.ui8_DisposeScopeRight;
    R_Move.ui8_ImageY = DI.ui8_DisposeScopeDown;
    R_Move.ui8_MaxY = DI.ui8_DisposeScopeDown;
    R_Move.ui8_AllMaxY = DI.ui8_DisposeScopeDown;

    // 左点爬梯
    while (LeftPointLadder(ui8_LF) && L_Move.ui8_ImageY > DI.ui8_DisposeScopeUp && L_Move.ui8_ImageX < DI.ui8_DisposeScopeRight) {
        if (L_Move.ui8_ImageX < MID_POINT && L_Move.ui8_MaxY > L_Move.ui8_ImageY) {
            L_Move.ui8_MaxY = L_Move.ui8_ImageY;
        }
    }

    // 右点爬梯
    while (RightPointLadder(ui8_RF) && R_Move.ui8_ImageY > DI.ui8_DisposeScopeUp && R_Move.ui8_ImageX > DI.ui8_DisposeScopeLeft) {
        if (R_Move.ui8_ImageX > MID_POINT && R_Move.ui8_MaxY > R_Move.ui8_ImageY) {
            R_Move.ui8_MaxY = R_Move.ui8_ImageY;
        }
    }

    // 中值求取起点
    if (L_Move.ui8_MaxY < R_Move.ui8_MaxY) {
        DI.ui8_ScanDirection = Lstart;
    } else if (L_Move.ui8_MaxY > R_Move.ui8_MaxY) {
        DI.ui8_ScanDirection = Rstart;
    } else {
        DI.ui8_ScanDirection = Mstart;
    }
}

/************************************************************************
函数名：左点爬梯
功能：左点记录边界
返回值：能否继续移动
************************************************************************/
uint8_t LeftPointLadder(uint8_t* ui8p_LF) {
    if (!DI.ui8_ImageArray[L_Move.ui8_ImageY][L_Move.ui8_ImageX]) { // 黑点进入白区
        while (L_Move.ui8_ImageX < DI.ui8_DisposeScopeRight && !DI.ui8_ImageArray[L_Move.ui8_ImageY][L_Move.ui8_ImageX + 1]) {
            L_Move.ui8_ImageX++;
        }
        if (L_Move.ui8_ImageX < DI.ui8_DisposeScopeRight) {
            L_Move.ui8_ImageX++;
            return TRUE;
        }
        return FALSE;
    } else if (DI.ui8_ImageArray[L_Move.ui8_ImageY - 1][L_Move.ui8_ImageX]) { // 白区内向上
        if (L_Move.ui8_ImageY == DI.ui8_DisposeScopeDown && DI.ui8_LPoint[L_Move.ui8_ImageY] - DI.ui8_DisposeScopeLeft > MID_POINT / 4 && DI.ui8_LPoint[L_Move.ui8_ImageY] - L_Move.ui8_ImageX > MID_POINT / 4) {
            L_Move.ui8_ImageX = (DI.ui8_LPoint[L_Move.ui8_ImageY] + L_Move.ui8_ImageX) / 2; // 左下角出现噪点，根据上次左点比较跳变
        } else {
            if (!ui8p_LF[L_Move.ui8_ImageY]) {
                DI.ui8_LPoint[L_Move.ui8_ImageY] = L_Move.ui8_ImageX; // 找到并记录！！！
                L_Move.ui8_AllMaxY = L_Move.ui8_ImageY;
                ui8p_LF[L_Move.ui8_ImageY] = 1;
            }
            L_Move.ui8_ImageY--;
        }
        return TRUE;
    } else if (DI.ui8_ImageArray[L_Move.ui8_ImageY][L_Move.ui8_ImageX + 1]) { // 向上是黑则向右
        L_Move.ui8_ImageX++;
        return TRUE;
    } else if (L_Move.ui8_ImageY < DI.ui8_DisposeScopeDown && DI.ui8_ImageArray[L_Move.ui8_ImageY + 1][L_Move.ui8_ImageX]) { // 向上找不到白点，且向右找不到白点，则返回找
        while (L_Move.ui8_ImageY < DI.ui8_DisposeScopeDown && DI.ui8_ImageArray[L_Move.ui8_ImageY + 1][L_Move.ui8_ImageX] && !DI.ui8_ImageArray[L_Move.ui8_ImageY + 1][L_Move.ui8_ImageX + 1]) {
            L_Move.ui8_ImageY++;
        }
        if (L_Move.ui8_ImageY < DI.ui8_DisposeScopeDown) {
            L_Move.ui8_ImageY++;
            L_Move.ui8_ImageX++;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}

/************************************************************************
函数名：右点爬梯
功能：右点记录边界
返回值：能否继续移动
************************************************************************/
uint8_t RightPointLadder(uint8_t* ui8p_RF) {
    if (!DI.ui8_ImageArray[R_Move.ui8_ImageY][R_Move.ui8_ImageX]) {
        while (R_Move.ui8_ImageX > DI.ui8_DisposeScopeLeft && !DI.ui8_ImageArray[R_Move.ui8_ImageY][R_Move.ui8_ImageX - 1]) {
            R_Move.ui8_ImageX--;
        }
        if (R_Move.ui8_ImageX > DI.ui8_DisposeScopeLeft) {
            R_Move.ui8_ImageX--;
            return TRUE;
        }
        return FALSE;
    } else if (DI.ui8_ImageArray[R_Move.ui8_ImageY - 1][R_Move.ui8_ImageX]) {
        if (R_Move.ui8_ImageY == DI.ui8_DisposeScopeDown && DI.ui8_DisposeScopeRight - DI.ui8_RPoint[R_Move.ui8_ImageY] > MID_POINT / 4 && R_Move.ui8_ImageX - DI.ui8_RPoint[R_Move.ui8_ImageY] > MID_POINT / 4) {
            R_Move.ui8_ImageX = (DI.ui8_RPoint[R_Move.ui8_ImageY] + R_Move.ui8_ImageX) / 2;
        } else {
            if (!ui8p_RF[R_Move.ui8_ImageY]) {
                DI.ui8_RPoint[R_Move.ui8_ImageY] = R_Move.ui8_ImageX;
                R_Move.ui8_AllMaxY = R_Move.ui8_ImageY;
                ui8p_RF[R_Move.ui8_ImageY] = 1;
            }
            R_Move.ui8_ImageY--;
        }
        return TRUE;
    } else if (DI.ui8_ImageArray[R_Move.ui8_ImageY][R_Move.ui8_ImageX - 1]) {
        R_Move.ui8_ImageX--;
        return TRUE;
    } else if (R_Move.ui8_ImageY < DI.ui8_DisposeScopeDown && DI.ui8_ImageArray[R_Move.ui8_ImageY + 1][R_Move.ui8_ImageX]) {
        while (R_Move.ui8_ImageY < DI.ui8_DisposeScopeDown && DI.ui8_ImageArray[R_Move.ui8_ImageY + 1][R_Move.ui8_ImageX] && !DI.ui8_ImageArray[R_Move.ui8_ImageY + 1][R_Move.ui8_ImageX - 1]) {
            R_Move.ui8_ImageY++;
        }
        if (R_Move.ui8_ImageY < DI.ui8_DisposeScopeDown) {
            R_Move.ui8_ImageY++;
            R_Move.ui8_ImageX--;
            return TRUE;
        }
        return FALSE;
    }
    return FALSE;
}
