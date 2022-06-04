#include <target.h>
#include <cmath>

Target::Target(const int num, const Frame& frame)
{
    frame_ = frame;

    posSum = Vector2d(0, 0);

    iTargetNum = num;
}

void Target::setFrame(const Frame& frame)
{
    frame_ = frame;
}

void Target::getFrame(Frame& frame) const
{
    frame = frame_;
}

double Target::execFusion(const double& conf)
{
    double delta;
    
    delta = pow((1 - frame_.conf_) * conf, 3);

    frame_.conf_ += delta;
    iFusionNum++;

    return frame_.conf_;
}

void Target::calTargetPos(const Vector2d& _pos)
{    
    posSum = posSum + _pos;
    iFrameNum++;
    
    frame_.pos_ = posSum / iFrameNum;
}

