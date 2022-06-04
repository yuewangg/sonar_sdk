#include <config.h>
#include <string>

#include <frame.h>
#include <target.h>
#include <yaml-cpp/yaml.h>

#define TARGET_DO_NEED_TO_SEARCH true
#define TARGET_DO_NOT_NEED_TO_SEARCH false

class TargetPosFusion{
public:
    TargetPosFusion(const char* yamlFileName);

    int run();
    void getInitStatus();
    bool getInitFlag();
    bool getEndFlag();
    void report();
    void localReport(const Target& target, const bool flag);
    void reportOne(const Target& target);

#ifdef DEBUG
    void write2File();
#endif

private:
    string strYamlFileName;

    // yaml文件配置信息
    double installAngle;
    //double horizontalAngle;

    //int iTargetBufReserveNum;

    double distMax;
    double localReportConfThreshold;
    int localReportNumThreshold;
    double globalReportConfThreshold;
    int globalReportNumThreshold;
    int globalReportNum;
    
    char clientIp[20];
    char serverIp[20];
    char localReportIp[20];
    char reportIp[20];
    int clientPort;
    int serverPort;
    int localReportPort;
    int reportPort;

    // udp
    int clientFd;
    int serverFd;

    // uuv初始位姿：uuvPoseOrigin
    // uuv初始UTM坐标: uuvUTMOrigin
    // uuv当前位姿：uuvPoseCur
    Vector6d uuvPoseOrigin;
    Vector2d uuvUTMOrigin;
    Frame frameOrigin;
    Vector6d uuvPoseCur;

    // 测试用
    vFrame localReportFrames;

    // 初始化标志
    bool initFlag = false;
    // 结束标志
    bool endFlag = false;
    // 目标
    int iTargetNum = 0; // 编号
    vTarget targets_;
    vpTarget vpTargets_;
    vpTarget vpTargetsRet_;
    vpTarget vpSearchList_;
    vpTarget vpHasReportList_;
};