#include <frame.h>

Frame::Frame(const Frame& frame)
{
    pos_ = frame.pos_;
    conf_ = frame.conf_;
    hemi_ = frame.hemi_;
    zone_ = frame.zone_;
    type_ = frame.type_;
}

Frame::Frame(const Vector2d& pos, const double& conf) : pos_(pos), conf_(conf){}

// 输入：目标位置 + 声呐位置（原点） + 分辨率 + uuv位姿(角度制) + 置信度；位置都是先横坐标再纵坐标
// (横倾角（绕x轴）：phi，纵倾角（绕y轴）：theta，偏航角（绕z轴）：psi)
// uuv位姿表示从惯性坐标系（世界）到非惯性坐标系（）的转换
// 输入极坐标的角度是相对纵轴的角度，右侧为正，角度制
double Frame::polar2Frame(const Vector2d& pos, const Vector6d& uuvPoseUTMOrigin, 
                          const Vector6d& uuvPoseCur, const double& conf, 
                          const double& installAngle)
{        
    double installAngleRadian = installAngle / 180.0 * PI;    
    // y是极线长度，alpha是极线与横轴夹角
    double r = pos(0, 0);    
    double alpha = (90 - pos(1, 0));
    double distXY = r * cos(installAngleRadian);    // 将极线投影到UUV平面
    double yRel = distXY * cos(alpha / 180.0 * PI);  // y是横轴
    double xRel = distXY * sin(alpha / 180.0 * PI);  // x是纵轴
    double zRel = r * sin(installAngleRadian); // 深度
    Vector3d posBody(xRel, yRel, zRel); // 载体坐标系下的坐标

    // 笛卡尔坐标到世界坐标的转换    
    // bw：world->body，posBody = Rbw*posWorld + tbw 
    // wb：body->world，posWorld = Rwb(posBody + twb)
    double phi2   = uuvPoseUTMOrigin(3, 0) / 180.0 * PI;
    double theta2 = uuvPoseUTMOrigin(4, 0) / 180.0 * PI;
    double psi2   = uuvPoseUTMOrigin(5, 0) / 180.0 * PI;
    Matrix3d R21;
    R21 <<  cos(psi2)*cos(theta2), -sin(psi2)*cos(phi2) + cos(psi2)*sin(theta2)*sin(phi2),  sin(psi2)*sin(phi2) + cos(psi2)*sin(theta2)*cos(phi2),
            sin(psi2)*cos(theta2),  cos(psi2)*cos(phi2) + sin(psi2)*sin(theta2)*sin(phi2), -sin(phi2)*cos(psi2) + sin(psi2)*sin(theta2)*cos(phi2),
            -sin(theta2),          cos(theta2)*sin(phi2),                               cos(theta2)*cos(phi2)                             ;     
    Vector3d t12(uuvPoseUTMOrigin(0, 0), uuvPoseUTMOrigin(1, 0), uuvPoseUTMOrigin(2, 0));    
    
    double phi3   = uuvPoseCur(3, 0) / 180.0 * PI;
    double theta3 = uuvPoseCur(4, 0) / 180.0 * PI;
    double psi3   = uuvPoseCur(5, 0) / 180.0 * PI;
    Matrix3d R31;
    R31 <<  cos(psi3)*cos(theta3), -sin(psi3)*cos(phi3) + cos(psi3)*sin(theta3)*sin(phi3),  sin(psi3)*sin(phi3) + cos(psi3)*sin(theta3)*cos(phi3),
            sin(psi3)*cos(theta3),  cos(psi3)*cos(phi3) + sin(psi3)*sin(theta3)*sin(phi3), -sin(phi3)*cos(psi3) + sin(psi3)*sin(theta3)*cos(phi3),
            -sin(theta3),          cos(theta3)*sin(phi3),                               cos(theta3)*cos(phi3)                             ;     
    Vector3d t13(uuvPoseCur(0, 0), uuvPoseCur(1, 0), uuvPoseCur(2, 0));    
    
    // posWorld 相对于初始位姿
    Vector3d posWorld = R21*(R31.transpose() * posBody + t13 - t12);

    // 保存横坐标                                                                               
    pos_ = Vector2d(posWorld(0, 0), posWorld(1, 0));
    double depth = posWorld(2, 0);

    // 设置其他属性
    conf_ = conf;
    type_ = FORWARD_LOOKING_SONAR;

    return depth;
}

// 输入：侧扫声呐经纬度坐标 + 置信度，pos = (经度, 纬度)
// 输出：NED坐标
// utm.cc中，LonLat2UTM和UTM2LonLat可能有问题，从LonLat2UTM，输出的x、y顺序交换了，已经同时将这两个代码的形参顺序做了调整
bool Frame::ll2Frame(const Vector2d& pos, const double& conf, const Vector2d& uuvUTMOrigin, const double& uuvPsiOrigin, bool rel)
{
    // 伪墨卡托？
#if 0
    pos_ = 60 * 1852 * pos;
    if(rel == true)
        pos_ = pos_ - posOrigin;

    conf_ = conf;
    type_ = SIDE_SCAN_SONAR;
#else
    long zone;
    char hemi;
    double east;
    double north;
    int ret;
  
#ifdef DEBUG  
    //cout << "lat, lon = " << pos(0, 0) << ", " << pos(1, 0) << endl;
    //cout << "pos = " << pos.transpose() << endl;
#endif
    ret = LonLat2UTM(pos(0, 0), pos(1, 0), &zone, &hemi, &east, &north);    
    

    // 出错
    if(ret != 0x0000)
    {
#ifdef DEBUG
    //cout << "north = " << north << " ";
    //cout << "east = " << east << " ";
    //cout << "zone = " << zone << " ";
    //cout << "hemi = " << hemi << endl;   
#endif

        return false;    
    }   

    pos_ << north, east;
#ifdef DEBUG    
    //cout << "Before change: " << pos_.transpose() << endl;
#endif
    if(rel)
    {
        double psi = uuvPsiOrigin / 180 * PI;
        Matrix2d R21;
        R21 << cos(psi), sin(psi), -sin(psi), cos(psi);
        pos_ = R21 * (pos_ - uuvUTMOrigin);
    }
#ifdef DEBUG
    //cout << "Before change: " << pos_.transpose() << endl;
#endif    
    conf_ = conf;
    type_ = SIDE_SCAN_SONAR;
    zone_ = zone;
    hemi_ = hemi;
#endif
    return true;
}

// posOrigin：起始UTM坐标
bool Frame::frame2LL(const Vector2d& uuvUTMOrigin, const double& uuvPsiOrigin)
{
#if 0
    Vector2d posUTM = posOrigin + pos_;    
    pos_ = posUTM / 60.0 / 1852.0;
#else
    Vector2d pos;
    double psi = uuvPsiOrigin / 180 * PI;
    Matrix2d R21;
    R21 << cos(psi), sin(psi), -sin(psi), cos(psi);
    Vector2d posUTM = R21.transpose() * pos_ + uuvUTMOrigin;
    
    int ret = UTM2LonLat(zone_, hemi_, posUTM(1, 0)+1e-9, posUTM(0, 0)+1e-9, &pos(0, 0), &pos(1, 0));

    if(ret != 0x0000)
    {
#ifdef DEBUG
        cout << "Frame2LL: ret = " << ret << endl;
        cout << "lat,lon: " << pos_.transpose();
        cout << " -- zone: " << zone_;
        cout << " -- hemi: " << hemi_;
        cout << endl;
#endif
        return false;    
    }

    pos_ = pos;
#endif
    return true;
}

// 计算帧间距离
double Frame::calDistance(const Vector2d& posSrc)
{
    return sqrt(pow((posSrc(0, 0) - pos_(0, 0)), 2) + pow((posSrc(1, 0) - pos_(1, 0)), 2));
}

double Frame::calDistance(const Frame& frame)
{
    return calDistance(frame.pos_);
}

