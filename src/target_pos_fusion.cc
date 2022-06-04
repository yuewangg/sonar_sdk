#include <target_pos_fusion.h>

#include <utm.h>
#include <unistd.h>
#include <fstream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <udp.h>


#ifdef DEBUG
void createFrameBuf(const Vector2d& uuvUTMOrigin, const double& uuvPsiOrigin,
                    const char& hemi, const long& zone, vFrame& frameBuffer,
                    const unsigned int& range, const unsigned int& bufSize);
#endif

bool greaterSort(const Target* a, const Target* b);
// 通过udp获取数据，解析buf头
int recvData(const int fd, double* data, int& dataType);
// 通过udp获取uuv初始位姿和初始经纬度坐标



TargetPosFusion::TargetPosFusion(const char* yamlFileName)
{
    strYamlFileName = yamlFileName;

    YAML::Node config;
    try{
        config = YAML::LoadFile(strYamlFileName.c_str());  
    }catch(...) { 
        std::cout << "usage:./main configFile" << std::endl;
        return ;
    }
    installAngle = config["install_angle"].as<double>();
    //horizontalAngle = config["horizontal_angle"].as<double>();

    //iTargetBufReserveNum = config["target_buf_num"].as<int>();

    distMax = config["distance_max"].as<double>();
    localReportConfThreshold = config["local_report_conf_threshold"].as<double>();
    localReportNumThreshold = config["local_report_num_threshold"].as<int>();
    globalReportConfThreshold = config["global_report_conf_threshold"].as<double>();
    globalReportNumThreshold = config["global_report_num_threshold"].as<int>();
    globalReportNum = config["global_report_num"].as<int>();
    {
        string tmp;
        tmp = config["client_ip"].as<string>();
        strcpy(clientIp, tmp.c_str());
        tmp = config["server_ip"].as<string>();
        strcpy(serverIp, tmp.c_str());
        tmp = config["local_report_ip"].as<string>();
        strcpy(localReportIp, tmp.c_str());
        tmp = config["report_ip"].as<string>();
        strcpy(reportIp, tmp.c_str());
        clientPort = config["client_port"].as<int>();
        serverPort = config["server_port"].as<int>();
        localReportPort = config["local_report_port"].as<int>();
        reportPort = config["report_port"].as<int>();
    }
 

#ifdef DEBUG
    // 测试前先创建数据
    int dataCreateFlag = config["test_flag"].as<int>();
    if(dataCreateFlag != 0)
    {
        vFrame frameBuffer;
        Vector2d uuvLLOrigin;
        uuvPoseOrigin <<  0.0, 0.0, 0.0,
                          0.0, 0.0, 0.0;    //初始位姿
        unsigned int dataRange = config["test_data_range"].as<unsigned int>();
        unsigned int bufSize = config["test_data_size"].as<unsigned int>();
	frameOrigin.hemi_ = 'N';
	frameOrigin.zone_ = 48;
        uuvLLOrigin << 103.0, 26.0;
        frameOrigin.ll2Frame(uuvLLOrigin, 1.0);
	cout << "frameOrigin.pos = " << frameOrigin.pos_.transpose() << endl;
	uuvPoseOrigin(0, 0) = frameOrigin.pos_(0, 0);
	uuvPoseOrigin(1, 0) = frameOrigin.pos_(1, 0);
        uuvUTMOrigin = frameOrigin.pos_;
        cout << uuvUTMOrigin.transpose() << " " << frameOrigin.zone_ << " " << frameOrigin.hemi_ << endl;    
        createFrameBuf(uuvUTMOrigin, uuvPoseOrigin(5, 0), frameOrigin.hemi_, frameOrigin.zone_, frameBuffer, dataRange, bufSize);  
        return ;  
    }

#endif

    // 2.udp初始化
    clientFd = createSocket(clientIp, clientPort);
    serverFd = createSocket(serverIp, serverPort);
    
    // 3.通过udp读取UUV初始位姿 uuvPoseOrigin
    getInitStatus();
    frameOrigin.hemi_ = 'N';
    frameOrigin.zone_ = (int)uuvPoseOrigin(0, 0) / 6 + 31;
#ifdef DEBUG    
    cout << "----------  Start to init  ----------" << endl;
#endif
    // 获得 UTM坐标
    Frame frameTmp;
    frameTmp.pos_ = Vector2d(uuvPoseOrigin(0, 0), uuvPoseOrigin(1, 0));    
    frameTmp.ll2Frame(frameTmp.pos_, 0);
    uuvUTMOrigin = frameTmp.pos_;
    
    uuvPoseCur = uuvPoseOrigin;

    frameOrigin.pos_ << uuvPoseOrigin(0,0), uuvPoseOrigin(1, 0);
#ifdef DEBUG
    cout << "frameOrigin.pos_ = " << frameOrigin.pos_.transpose() << endl;
    cout << "uuvUTMOrigin = " << uuvUTMOrigin.transpose() << endl;
    cout << "uuvPoseOrigin = " << uuvPoseOrigin.transpose() << endl;
    cout << "zone = " << frameOrigin.zone_ << " -- hemi = " << frameOrigin.hemi_ << endl;
    int dataIndex = 0;    
    cout << "----------  Init end  ----------" << endl;
#endif

    //targets_.reserve(iTargetBufReserveNum);
    //vpTargets_.reserve(iTargetBufReserveNum);
    //vpTargetsRet_.reserve(globalReportNum);

    initFlag = true;
}

int TargetPosFusion::run()
{
#ifdef DEBUG
    static int dataIndex = 0;
#endif
    bool retFlag = false;
    Frame frameCur;

    // 1.接收到声呐数据，转换为帧 frameCur
    // 2.接收到前视声呐数据，根据UUV最新位姿数据，转换为帧 frameCur
    //   如果每一时刻位姿相对于起始时刻位姿，则位姿需要加上初始位姿uuvPoseOrigin，这里按这种情况实现
    // side_scan_data: lat, lon, conf
    // forward_looking_data: target_x, target_y, conf, origin_x, origin_y, resolution
    double data[6] = {0};
    int dataType = -1;

    if(vpHasReportList_.size() >= globalReportNum)
    {
        endFlag = true;
        return FUSION_OK;
    }


    int len = recvData(clientFd, data, dataType);
    if(len == RECV_NO_DATA)
        return RECV_NO_DATA;
    if(dataType == END)
    {
        endFlag = true;
        return END;
    }
    
    if(len != 3 && len != 5 && len != 6)
    {
        return RECV_NO_DATA;
    }
    // 根据数据类型处置
    if(dataType == SIDE_SCAN_SONAR)
    {
        //声呐数据转换为帧
        Vector2d sideData(data[0], data[1]);
        double conf = data[2];
#ifdef DEBUG
        //cout << "sideData: " << dataIndex << ": " << sideData.transpose() << " " << conf << endl;
#endif            
        // true：计算后的坐标相对于起始UTM坐标 uuvUTMOrigin = frameOrigin.pos_
        if(frameCur.ll2Frame(sideData, conf, uuvUTMOrigin, uuvPoseOrigin(5, 0), true) == false)
            return LL2FRAME_ERROR;
#ifdef DEBUG
        //cout << "after ll2Frame" << dataIndex << ": " << frameCur.pos_.transpose() << " " << conf << endl;
        dataIndex++;
#endif                        
    }
    else if(dataType == FORWARD_LOOKING_SONAR)
    {
        Vector2d pos(data[0], data[1]);
        double conf = data[2];
        // 前视声呐数据转帧，这里假设了uuvPoseCur是一个相对于frameOrigin.pos_的位置
        // 经过 polar2Frame，得到的目标位置是相对于frameOrigin.pos_的位置
#ifdef  DEBUG         
        //cout << "forwardData " << dataIndex << ": " << pos.transpose() << "\t" << "\tconf = " << conf << endl;
#endif        
        Vector6d uuvPoseUTMOrigin, uuvPoseUTMCur;

        /********** 为了配合以前的代码版本 **********/
        uuvPoseUTMOrigin << uuvUTMOrigin(0, 0), uuvUTMOrigin(1, 0), uuvPoseOrigin(2, 0), uuvPoseOrigin(3, 0), uuvPoseOrigin(4, 0), uuvPoseOrigin(5, 0);

        Frame frameTmp;                   
        if(frameTmp.ll2Frame(Vector2d(uuvPoseCur(0, 0), uuvPoseCur(1, 0)), 0) == false)
        {
#ifdef DEBUG
            cout << "Err in " << __FILE__ << " -- " << __LINE__ << endl;
#endif            
            return LL2FRAME_ERROR;
        }
        uuvPoseUTMCur << frameTmp.pos_(0, 0), frameTmp.pos_(1, 0), uuvPoseCur(2, 0), uuvPoseCur(3, 0), uuvPoseCur(4, 0), uuvPoseCur(5, 0);
        /********** 为了配合以前的代码版本 **********/

        // pos是目标在图像的位置，posOrigin是声呐头在图像的位置
        double depth = frameCur.polar2Frame(pos, uuvPoseUTMOrigin, uuvPoseUTMCur, conf, installAngle);
        if(depth < 0)   return POLOR2FRAME_ERROR;

        // 保存zone、hemi，不跨区的话这样做
        frameCur.zone_ = frameOrigin.zone_;
        frameCur.hemi_ = frameOrigin.hemi_;
#ifdef  DEBUG                   
        //cout << "after polar2Frame" << dataIndex << ": "  << frameCur.pos_.transpose() << "\tconf = " << frameCur.conf_ << " " << frameCur.zone_ << " " << frameCur.hemi_<< endl;                    
        dataIndex++;
#endif            
    }
    else if(dataType == UUV_POSE)
    {
        // 更新最新位姿
        uuvPoseCur << data[0], data[1], data[2], data[3], data[4], data[5];
        return FUSION_OK;
    }       
    else
    {
        return DATATYPE_ERROR;
    }

    // 目标关联
    double distMin = 1000000;
    Target* targetAssociated = nullptr;
    for(vTarget::iterator it = targets_.begin(); it != targets_.end(); it++)
    {
        Frame framTmp;
        double dist;

        it->getFrame(framTmp);
        dist = framTmp.calDistance(frameCur);
        if(dist < distMin)
        {
            int index = std::distance(targets_.begin(), it);
            targetAssociated = &targets_[index];
            distMin = dist;
        }
    }
    
    // 关联失败，创建目标
    if(distMin > distMax)
    {
        iTargetNum++;
        Target targetTmp(iTargetNum, frameCur);
#ifdef  DEBUG
        targetTmp.confidences_.push_back(frameCur.conf_);
        // 关联到新目标，用于matlab画图
        targetTmp.frames_.push_back(frameCur);
//cout << "GetFrame: pos = " << frameCur.pos_.transpose() << "\tconf = " << frameCur.conf_ << "\t---\t";
//cout << "Failed to associate!" << endl;
#endif
        targets_.push_back(targetTmp);      

        return FUSION_OK;
    }
    
    // 关联成功，执行融合，更新目标信息
    for(int i = 0; i < vpHasReportList_.size(); i++)
    {
        // 已经上报
        if(targetAssociated->iTargetNum == vpHasReportList_[i]->iTargetNum)
        {
            return FUSION_OK;
        }
    }



    Frame targetFrame;
#ifdef DEBUG    
    targetAssociated->frames_.push_back(frameCur);  // 关联帧数据，用于matlab画图
    double confTmp = targetAssociated->execFusion(frameCur.conf_);
    targetAssociated->confidences_.push_back(confTmp); // 融合结果存起来,用于matlab画图
#else
    targetAssociated->execFusion(frameCur.conf_);
#endif        
    // 取历史关联帧位置的平均值
    targetAssociated->calTargetPos(frameCur.pos_);
    targetAssociated->getFrame(targetFrame);    
#ifdef  DEBUG
//cout << "frameCur: pos = " << frameCur.pos_.transpose() << "\tconf = " << frameCur.conf_ << "\t---\t";
//cout << "Associate success! Target：" << targetFrame.pos_.transpose() << "\tconf = "  << targetFrame.conf_ << "\tdistance = " << distMin << endl;        
#endif


    // 置信度达到上报阈值，直接上报，加入上报列表
    //if(targetFrame.conf_ > globalReportConfThreshold || (targetAssociated->iFusionNum >= globalReportNumThreshold && targetFrame.conf_ > 0.65))
    if(targetFrame.conf_ > globalReportConfThreshold)
    {
        reportOne(*targetAssociated);
        vpHasReportList_.push_back(targetAssociated);

        for (vpTarget::iterator it = vpSearchList_.begin(); it != vpSearchList_.end(); it++)
        {
            int dist = std::distance(vpSearchList_.begin(), it);
            // 在搜索列表里，从列表里删除，并通知搜索模块
            if(targetAssociated->iTargetNum == vpSearchList_[dist]->iTargetNum)
            {
                vpSearchList_.erase(it);
                localReport(*targetAssociated, TARGET_DO_NOT_NEED_TO_SEARCH);
                break;
            }
        }

        return FUSION_OK;
    }

    // 置信度达到搜索阈值，上报给搜索模块
    if(targetFrame.conf_ > localReportConfThreshold)
    {
        for(int i = 0; i < vpSearchList_.size(); i++)
        {
            if(targetAssociated->iTargetNum == vpSearchList_[i]->iTargetNum)
            {
                
                return FUSION_OK;
            }
        }
        // 如果不存在于搜索列表，加入列表并通知搜索模块
        vpSearchList_.push_back(targetAssociated);
        localReport(*targetAssociated, TARGET_DO_NEED_TO_SEARCH);
    }


    return FUSION_OK;
}

bool TargetPosFusion::getInitFlag()
{
    return initFlag;
}

bool TargetPosFusion::getEndFlag()
{
    return endFlag;
}

void TargetPosFusion::report()    
{
    int reportNum = globalReportNum - vpHasReportList_.size();
    // 上报数目足够
    if(reportNum <= 0)
    {
#ifdef DEBUG
        cout << "Do not need to report!" << endl;
#endif
        return ;
    }
#ifdef DEBUG
    cout << "reportNum = " << reportNum << "!" << endl;
#endif
    
    vpTarget reportTargetList;
    // 上报目标数目不够，剔除已上报成员，得到reportTargetList
    for(int i = 0; i < targets_.size(); i++)
    {
        bool exist = false;
        for(int j = 0; j < vpHasReportList_.size(); j++)
        {
            if(targets_[i].iTargetNum == vpHasReportList_[j]->iTargetNum)
            {
                exist = true;
                break;   
            }
        }
        if(exist == false)
        {
            reportTargetList.push_back(&targets_[i]);
        }
    }

    // 目标数目冗余，排序，挑选上报，目标数目没有冗余，全部上报
    if(reportTargetList.size() > reportNum)
    {
        std::sort(reportTargetList.begin(), reportTargetList.end(), greaterSort);
    }

    for(int i = 0; i < reportTargetList.size() && i < reportNum; i++)
    {
        // 可能需要一些准备工作
        reportOne(*reportTargetList[i]);
        vpHasReportList_.push_back(reportTargetList[i]);

        
        for (vpTarget::iterator it = vpSearchList_.begin(); it != vpSearchList_.end(); it++)
        {
            int dist = std::distance(vpSearchList_.begin(), it);
            // 在搜索列表里，从列表里删除，并通知搜索模块
            if(reportTargetList[i]->iTargetNum == vpSearchList_[dist]->iTargetNum)
            {
                vpSearchList_.erase(it);
                localReport(*reportTargetList[i], TARGET_DO_NOT_NEED_TO_SEARCH);
                break;
            }
        }
    }
}

#ifdef  DEBUG
void createFrameBuf(const Vector2d& uuvUTMOrigin, const double& uuvPsiOrigin,
                    const char& hemi, const long& zone, vFrame& frameBuffer,
                    const unsigned int& range, const unsigned int& bufSize)
{
    // 前视声呐距离分辨率和声呐像素位置
    double psi = uuvPsiOrigin / 180 * PI;
    double distSum = 0;
    ofstream ofsSide, ofsForward;
    ofsSide.precision(12);
    ofsForward.precision(12);
    ofsSide.open("../data/dataSide.txt", ios::out);
    ofsForward.open("../data/dataForward.txt", ios::out);
    for(int i = 0; i < bufSize; i++)
    {
	// x是横轴
        double decimal = rand() % 1000000000 / 1000000000.0;
        double x = rand() % range + decimal;
        decimal = rand() % 1000000000 / 1000000000.0;
        double y = rand() % range + decimal;
        //decimal = rand() % 100 / 100.0;
        //double z = rand() % 5 + decimal;
        
        // 以(0.0, 0.0)为均值
        Vector2d pos(0.0 + x, 0.0 + y);
        //dist = tan[(theta-0.5)*PI]
        //theta = atan(dist) / PI + 0.5
        double dist = 1.0 / sqrt(pos(0, 0)*pos(0, 0) + pos(1, 0)*pos(1, 0));
        dist = atan(dist) / PI + 0.5;   //映射到(0, 1)

        Frame frame(pos, dist);

        // dataSide:    x, y, lat, lon  , conf, zone, hemi
        // dataForward: x, y, r  , theta, conf, zone, hemi
        frame.type_ = SIDE_SCAN_SONAR;
        frame.hemi_ = hemi;
        frame.zone_ = zone;
        frameBuffer.push_back(frame);
        //cout << i << ":\t" << pos.transpose() << "\t" << dist << "\t" << frame.hemi_ << "\t" << frame.zone_ << endl;        
        //cout << pos(0,0)+uuvUTMOrigin(0,0) << " " << pos(1,0)+uuvUTMOrigin(1,0) << endl;
        frame.frame2LL(uuvUTMOrigin, psi);
        int zone = (int)(frame.pos_(0, 0) / 6) + 31;
        cout << "dataSide" << " " << x << " " << y << " " << frame.pos_(0, 0) << " " << frame.pos_(1, 0) << " " << dist << " " << zone << " " << 'N' << endl;
        ofsSide  << x << " " << y << " " << frame.pos_(0, 0) << " " << frame.pos_(1, 0) << " " << dist << " " << zone << " " << 'N'  << endl;

        double xForward = x;
        double yForward = y;   
        double r = sqrt(pow(xForward, 2) + pow(yForward, 2));
        double theta = atan2(yForward, xForward) / PI * 180;

        cout << "dataForward" << " " << x << " " << y << " " << r << " " << theta << " " << dist << " " << zone << " " << 'N' << endl;
        ofsForward << x << " " << y << " " << r << " " << theta << " " << dist << " " << zone << " " << 'N' << endl;
    }
    ofsSide.close();
}

void TargetPosFusion::write2File()
{ 
    // "../data/local_report.txt"保存上报到搜索模块的数据
    ofstream ofs;    
    ofs.precision(12);
    ofs.open("../data/local_report.txt", ios::out);
    for(vFrame::iterator it = localReportFrames.begin(); it != localReportFrames.end(); it++)
    {
        Frame frameTmp = *it;
        Vector2d llPosTmp = frameTmp.pos_;
        double confTmp = frameTmp.conf_;

        frameTmp.ll2Frame(llPosTmp, confTmp, uuvUTMOrigin, uuvPoseOrigin(5, 0), true);
        ofs  << frameTmp.pos_(0, 0) << " " << frameTmp.pos_(1, 0) << " " << llPosTmp(0, 0) << " " << llPosTmp(1, 0) << " " << confTmp << endl;
    }
    ofs.close();
    
    // "../data/target%d.txt"保存目标关联到的帧，以及融合后的置信度
    // "../data/target_kf.txt"保存目标经过kf后的置信度，以及平均后的位置
    ofstream ofs2;
    ofs2.precision(12);
    ofs2.open("../data/target_kf.txt", ios::out);
    int cnt = 1;   
    for(vTarget::iterator it = targets_.begin(); it != targets_.end(); it++)
    {
        string targetDataPath("../data/target");
        targetDataPath = targetDataPath + to_string(cnt) + ".txt";        
        ofs.open(targetDataPath.c_str(), ios::out);
        for(vFrame::iterator frameIt = it->frames_.begin(); frameIt != it->frames_.end(); frameIt++)
        {
            int dist = std::distance(it->frames_.begin(), frameIt);
            // 保存NED坐标
            ofs << frameIt->pos_(0, 0) << " " << frameIt->pos_(1, 0) << " ";
            // 转换为经纬度坐标
            frameIt->frame2LL(uuvUTMOrigin, uuvPoseOrigin(5, 0));
            // 保存经纬度坐标
            ofs << frameIt->pos_(0, 0) << " " << frameIt->pos_(1, 0) << " ";
            // 保存关联帧置信度
            ofs << frameIt->conf_ << " ";
            // 保存融合后的置信度
            ofs << it->confidences_[dist] << endl;
        }
        Frame frameTmp;
        it->getFrame(frameTmp);
        ofs2 << frameTmp.pos_(0, 0) << " " << frameTmp.pos_(1, 0) << " ";
        frameTmp.frame2LL(uuvUTMOrigin, uuvPoseOrigin(5, 0));
        ofs2 << frameTmp.pos_(0, 0) << " " << frameTmp.pos_(1, 0) << " " << frameTmp.conf_ << endl;
        ofs.close();
        cnt++;
    }
    ofs2.close();
}

#endif

void TargetPosFusion::getInitStatus()
{
    bool poseFlag = false;

    double data[6] = {0};
    int dataType = -1;

    char buf[UDP_BUFFER_SIZE];

    while(!poseFlag)
    {
        bzero(buf, sizeof(buf));
        int cnt = udpRead(clientFd, buf, sizeof(buf));
        if(cnt <= 0) continue ;        
        
        string strBuf(buf);
        size_t index = strBuf.find('#');
        if(index == string::npos)
        {
            printf("index == string::npos\r\n");
            continue;
        }
        string head;
        head = strBuf.substr(0, index);
        strBuf.erase(0, index + 1);

        // 位姿：pose_data#0,0,0,0,0,0
        if(!strncmp(head.c_str(), "pose_data", strlen("pose_data")))
        {
            // 截取数据
            string item;
            cnt = 0;
            while((index = strBuf.find(',')) != string::npos)
            {            
                item = strBuf.substr(0, index);
                data[cnt++] = stod(item);
                strBuf.erase(0, index + 1);
            }
            item = strBuf.substr(0, index);
            data[cnt++] = stod(item);    
            
            uuvPoseOrigin << data[0], data[1], data[2], data[3], data[4], data[5];
            poseFlag = true;        
        }       
    }
}

int recvData(const int fd, double* data, int& dataType)
{
    char buf[UDP_BUFFER_SIZE];
    int cnt;

        bzero(buf, sizeof(buf));
        cnt = udpRead(fd, buf, sizeof(buf));
        if(cnt <= 0)
        {
#ifdef DEBUG            
            //cout << "udpRead error return cnt <= 0" << endl;
#endif      
            //usleep(50);
            return -1;
        }
        
        // 截取头部
        string strBuf(buf);
        size_t index = strBuf.find('#');
        if(index == string::npos)
        {
#ifdef DEBUG
            cout << "index == string::npos\r\n" << endl;
#endif
            return -1;
	}
        string head;
        head = strBuf.substr(0, index);
        strBuf.erase(0, index + 1);
        
        // 解析buf头
        if(!strncmp(head.c_str(), "ifff", strlen("ifff")))
        {
            dataType = FORWARD_LOOKING_SONAR;
        }
        else if(!strncmp(head.c_str(), "side_data", strlen("side_data")))
        {
            dataType = SIDE_SCAN_SONAR;
        }
        else if(!strncmp(head.c_str(), "pose_data", strlen("pose_data")))
        {
            dataType = UUV_POSE;
        }
        else if(!strncmp(head.c_str(), "ll_init", strlen("ll_init")))
        {
            dataType = LL_INIT;
        }
        else if(!strncmp(head.c_str(), "end", strlen("end")))
        {
            dataType = END;
        }


	if(dataType == FORWARD_LOOKING_SONAR)
	{
	    void* s1 = (void*)buf;
	    for(int i = 0; i < 5; i++)
	    {
	        //printf("%c", *(char*)s1);
		s1 = (void*)((char*)s1 + 1);
	    }
	    
	    // ID, 暂时不需要
            s1 = (void*)((int*)s1 + 1);
	    //printf("%d\r\n", *((int*)s1));
	    cnt = 0;
	    data[cnt++] = *((double*)s1);
	    s1 = (void*)((double*)s1 + 1);
	    data[cnt++] = *((double*)s1);
	    s1 = (void*)((double*)s1 + 1);
	    data[cnt++] = *((double*)s1);

	    //cout << data[0] << " " << data[1] << " " << data[2] << endl;
	}
	else
	{
            if(strlen(strBuf.c_str()) != 0)
            {
                // 截取数据
                string item;
                cnt = 0;
                while((index = strBuf.find(',')) != string::npos)
                {
                    item = strBuf.substr(0, index);
                    data[cnt++] = stod(item);
                    strBuf.erase(0, index + 1);
                }
                item = strBuf.substr(0, index);
                data[cnt++] = stod(item);
             }
	}


    return cnt;
}


void TargetPosFusion::localReport(const Target& target, const bool flag)
{   
    char buf[UDP_BUFFER_SIZE];
    Frame frame;

    target.getFrame(frame);
    bool ret;
    ret = frame.frame2LL(uuvUTMOrigin, uuvPoseOrigin(5, 0));
    if(!ret) return;      
    
    if(flag == TARGET_DO_NEED_TO_SEARCH)
        sprintf(buf, "$set: %d -- %f,%f,%f", target.iTargetNum, frame.pos_(0, 0), frame.pos_(1, 0), frame.conf_);
    else
        sprintf(buf, "$unset: %d -- %f,%f,%f", target.iTargetNum, frame.pos_(0, 0), frame.pos_(1, 0), frame.conf_);
    udpWrite(serverFd, localReportIp, (std::to_string(localReportPort)).c_str(), buf, strlen(buf));
    // 不清空后面再次发送可能出问题
    bzero(buf, sizeof(buf));

#ifdef DEBUG     
    //cout << "localReport: target.iTargetNum -- pos = " << frame.pos_.transpose()  << "\tconf = " << frame.conf_ << endl;
#endif
}

// 按 confidence 排序，输出最大的 num 帧 frame
bool greaterSort(const Target* a, const Target* b)
{
    Frame f1, f2;
    a->getFrame(f1);
    b->getFrame(f2);

    return (f1.conf_>f2.conf_);
}

void TargetPosFusion::reportOne(const Target& target)
{
    char buf[UDP_BUFFER_SIZE];

    Frame frame;
    target.getFrame(frame);
    bool ret;
    ret = frame.frame2LL(uuvUTMOrigin, uuvPoseOrigin(5, 0));
    if(!ret) return;        

#ifdef DEBUG            
    cout << "report: " << target.iTargetNum << " -- pos = " << frame.pos_.transpose()  << "\tconfidence = " << frame.conf_ << endl;    
#endif
    sprintf(buf, "Report#%.12f,%.12f,%.12f", frame.pos_(0, 0), frame.pos_(1, 0), frame.conf_);
    udpWrite(serverFd, reportIp, (std::to_string(reportPort)).c_str(), buf, strlen(buf));
    // 不清空后面再次发送可能出问题
    bzero(buf, sizeof(buf));
}


