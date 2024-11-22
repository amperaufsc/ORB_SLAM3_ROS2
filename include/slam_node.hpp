#ifndef __STEREO_SLAM_NODE_HPP__
#define __STEREO_SLAM_NODE_HPP__

#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/image.hpp"

#include "sensor_msgs/msg/point_cloud2.hpp"
#include "sensor_msgs/msg/point_field.hpp"

#include "geometry_msgs/msg/transform_stamped.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/transform.hpp"

#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2/LinearMath/Transform.h"

#include "nav_msgs/msg/path.hpp"

#include "message_filters/subscriber.h"
#include "message_filters/synchronizer.h"
#include "message_filters/sync_policies/approximate_time.h"


#include <cv_bridge/cv_bridge.h>
#include <string> 
#include <opencv2/opencv.hpp>
#include "System.h"
#include "Frame.h"
#include "Map.h"
#include "Tracking.h"
#include "utility.hpp"

class SlamNode : public rclcpp::Node
{
public:
    SlamNode(ORB_SLAM3::System* pSLAM, rclcpp::Node* node, const std::string &strSettingsFile, const std::string &strDoRectify);

    ~SlamNode();
    void PublishCurrentPointCloud();
    void PublishTrackedPointCloud();
    void PublishPath();
    void PublishPose(Sophus::SE3f &pose);
    tf2::Transform TransformFromSophus(Sophus::SE3f &pose);

    rclcpp::Node* node_;

protected:
    ORB_SLAM3::System* m_SLAM;
    std::vector<ORB_SLAM3::KeyFrame*> trajectory;
    std::vector<ORB_SLAM3::MapPoint*> map_points;
    Sophus::SE3f* pose;

private:
    using ImageMsg = sensor_msgs::msg::Image;
    using approximate_sync_policy = message_filters::sync_policies::ApproximateTime<sensor_msgs::msg::Image, sensor_msgs::msg::Image>;


    rclcpp::Time current_frame_time_;

    bool doRectify;

    rclcpp::Publisher<geometry_msgs::msg::TransformStamped>::SharedPtr tf_publisher;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr posepublisher;
    rclcpp::Publisher<sensor_msgs::msg::PointCloud2>::SharedPtr pclpublisher;
    rclcpp::Publisher<nav_msgs::msg::Path>::SharedPtr pathpublisher;

};

#endif