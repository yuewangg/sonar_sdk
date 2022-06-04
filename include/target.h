#pragma once

#include <config.h>
#include <frame.h>

class Target
{
public:
    Target();
    Target(const int num, const Frame& frame);
    ~Target(){}

    void setFrame(const Frame& frame);
    void getFrame(Frame& frame) const;
    double execFusion(const double& conf);
    void calTargetPos(const Vector2d& _pos);

#ifdef DEBUG
    vFrame frames_; // 存储关联成功的帧，用于 matlab 画图
    vector<double> confidences_;    // 存储置信度融合过程，用于matlab画图
#endif

    int iTargetNum = -1;   // 编号
    Vector2d posSum;    // 关联帧位置和
    int iFrameNum = 0;  // 关联帧数  

    int iFusionNum = 0; // 融合次数
    bool bHasLocalReport = false;

protected:
    Frame frame_;   // 优化后的位置和协方差
    double depth_ = 0;   // 深度，用于前视声呐
};

typedef vector<Target, aligned_allocator<Target>> vTarget;
typedef vector<Target*> vpTarget;
