#pragma once

#include <iostream>
#include <vector>
#include <cmath>
#include <Eigen/Core>
#include <Eigen/Dense>

using namespace std;
using namespace Eigen;

typedef vector<Vector2d, aligned_allocator<Vector2d>> vVector2d;
typedef vector<Matrix2d, aligned_allocator<Matrix2d>> vMatrix2d;
typedef Matrix<double, 6, 1> Vector6d;

//#define DEBUG

#define PI           	(3.14159265358979323e0)    /* PI                        */

// udp缓冲区大小
#define UDP_BUFFER_SIZE 1024

// udp传输数据类型
#define LL_INIT                 (1)
#define SIDE_SCAN_SONAR         (2)
#define FORWARD_LOOKING_SONAR   (3)
#define UUV_POSE                (4)
#define END                     (5)

#define FUSION_OK           (0)

#define ZONE_ERROR              (-1)
#define HEMI_ERROR              (-2)

#define RECV_NO_DATA                 (-3) // 没有数据
#define LL2FRAME_ERROR          (-4)
#define POLOR2FRAME_ERROR       (-5)
#define DATATYPE_ERROR          (-6)



/* 后期改为读写yaml实现配置 */
/*
#define FORWARD_LOOKING_SONAR_INSTALL_ANGLE     (45.0)   // 前视声呐安装夹角（与水平面的夹角）
#define FORWARD_LOOKING_SONAR_HORIZONTAL_ANGLE  (90.0)  // 水平开角


// 卡尔曼参数
#define KF_A            (1)
#define KF_BU           (0.01)
#define KF_H            (1)
#define KF_Q            (0.1)
#define KF_R            (0.01)
#define KF_P_INIT       (0.1)
*/