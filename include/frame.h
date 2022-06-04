#pragma once

#include <config.h>
#include <utm.h>

class Frame
{
public:
    Frame(){}
    Frame(const Frame& frame);
    Frame(const Vector2d& pos, const double& conf);
    ~Frame(){}

    // 输入：目标极坐标(角度制) + uuv位姿 + 置信度
    double polar2Frame(const Vector2d& pos, const Vector6d& uuvPoseUTMOrigin, 
                       const Vector6d& uuvPoseCur, const double& conf, 
                       const double& installAngle = 0.0);
    // 输入侧扫声呐经纬度坐标 + 深度 + 置信度，pos = (纬度, 经度)
    // rel: true，相对起始点坐标
    //      false，绝对坐标
    bool ll2Frame(const Vector2d& pos, const double& conf, const Vector2d& uuvUTMOrigin = Vector2d(0.0, 0.0), const double& uuvPsiOrigin = 0.0, const bool rel = false);
    bool frame2LL(const Vector2d& uuvUTMOrigin, const double& uuvPsiOrigin);

    // 计算两帧之间的距离
    double calDistance(const Vector2d& posSrc);
    double calDistance(const Frame& frame);

    Vector2d pos_;
    double conf_;
    int type_;

    // 侧扫声呐需要用到时区等信息
    long zone_ = ZONE_ERROR;
    char hemi_ = HEMI_ERROR;
};

typedef vector<Frame, aligned_allocator<Frame>> vFrame;
