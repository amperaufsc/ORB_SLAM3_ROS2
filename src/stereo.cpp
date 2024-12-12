#include<iostream>
#include<algorithm>
#include<fstream>
#include<chrono>
#include <string> 

#include "rclcpp/rclcpp.hpp"
#include "stereo-slam-node.hpp"

#include <opencv2/core/core.hpp>

#include "System.h"


using std::placeholders::_1;
using std::placeholders::_2;

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    if(argc < 4)
    {
        std::cerr << "\nUsage: ros2 run orbslam stereo path_to_vocabulary path_to_settings do_rectify" << std::endl;
        return 1;
    }
    auto node = std::make_shared<rclcpp::Node>("run_slam");

    
    bool visualization = strcmp(argv[4],"True") == 0;
    ORB_SLAM3::System pSLAM(argv[1], argv[2], ORB_SLAM3::System::STEREO, visualization);

    std::shared_ptr<StereoSlamNode> slam_ros;
    slam_ros = std::make_shared<StereoSlamNode>(&pSLAM, node.get(), argv[2], argv[3]);
    std::cout << "============================ " << std::endl;

    rclcpp::spin(slam_ros->node_->get_node_base_interface());
    rclcpp::shutdown();

    return EXIT_SUCCESS;
}




StereoSlamNode::StereoSlamNode(ORB_SLAM3::System* pSLAM, rclcpp::Node* node, const std::string &strSettingsFile, const std::string &strDoRectify)
: SlamNode(pSLAM, node)
{
    stringstream ss(strDoRectify);
    ss >> boolalpha >> doRectify;

    if (doRectify) {
        cv::FileStorage fsSettings(strSettingsFile, cv::FileStorage::READ);
        if (!fsSettings.isOpened()) {
            cerr << "ERROR: Wrong path to settings" << endl;
            assert(0);
        }

        // Carrega os parâmetros de calibração
        cv::Mat K_l, K_r, P_l, P_r, R_l, R_r, D_l, D_r;
        fsSettings["LEFT.K"] >> K_l;
        fsSettings["RIGHT.K"] >> K_r;
        fsSettings["LEFT.P"] >> P_l;
        fsSettings["RIGHT.P"] >> P_r;
        fsSettings["LEFT.R"] >> R_l;
        fsSettings["RIGHT.R"] >> R_r;
        fsSettings["LEFT.D"] >> D_l;
        fsSettings["RIGHT.D"] >> D_r;

        int rows_l = fsSettings["LEFT.height"];
        int cols_l = fsSettings["LEFT.width"];
        int rows_r = fsSettings["RIGHT.height"];
        int cols_r = fsSettings["RIGHT.width"];

        if (K_l.empty() || K_r.empty() || P_l.empty() || P_r.empty() || R_l.empty() || R_r.empty() || D_l.empty() || D_r.empty() ||
            rows_l == 0 || rows_r == 0 || cols_l == 0 || cols_r == 0) {
            cerr << "ERROR: Calibration parameters to rectify stereo are missing!" << endl;
            assert(0);
        }

        // Inicializa os mapas de retificação
        cv::initUndistortRectifyMap(K_l, D_l, R_l, P_l.rowRange(0, 3).colRange(0, 3), cv::Size(cols_l, rows_l), CV_32F, M1l, M2l);
        cv::initUndistortRectifyMap(K_r, D_r, R_r, P_r.rowRange(0, 3).colRange(0, 3), cv::Size(cols_r, rows_r), CV_32F, M1r, M2r);
    }

    // Cria os subscritores usando o nó passado
    left_sub = std::make_shared<message_filters::Subscriber<ImageMsg>>(node, "camera/left");
    right_sub = std::make_shared<message_filters::Subscriber<ImageMsg>>(node, "camera/right");

    // Sincroniza os subscritores
    syncApproximate = std::make_shared<message_filters::Synchronizer<approximate_sync_policy>>(approximate_sync_policy(10), *left_sub, *right_sub);
    syncApproximate->registerCallback(&StereoSlamNode::GrabStereo, this);
}

StereoSlamNode::~StereoSlamNode() 
{
}

void StereoSlamNode::GrabStereo(const ImageMsg::SharedPtr msgLeft, const ImageMsg::SharedPtr msgRight) {
    // Copia a imagem RGB da mensagem ROS para cv::Mat
    try {
        cv_ptrLeft = cv_bridge::toCvShare(msgLeft);
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    // Copia a imagem de profundidade da mensagem ROS para cv::Mat
    try {
        cv_ptrRight = cv_bridge::toCvShare(msgRight);
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(this->get_logger(), "cv_bridge exception: %s", e.what());
        return;
    }

    current_frame_time_ = now();

    if (doRectify) {
        cv::Mat imLeft, imRight;
        cv::remap(cv_ptrLeft->image, imLeft, M1l, M2l, cv::INTER_LINEAR);
        cv::remap(cv_ptrRight->image, imRight, M1r, M2r, cv::INTER_LINEAR);
        SE3 = m_SLAM->TrackStereo(imLeft, imRight, Utility::StampToSec(msgLeft->header.stamp));
    } else {
        SE3 = m_SLAM->TrackStereo(cv_ptrLeft->image, cv_ptrRight->image, Utility::StampToSec(msgLeft->header.stamp));
    }

    Update();
}


