// Copyright 2020 Tier IV, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
//
// Author: v1.0 Yukihiro Saito
///

#ifndef MULTI_OBJECT_TRACKER__MULTI_OBJECT_TRACKER_CORE_HPP_
#define MULTI_OBJECT_TRACKER__MULTI_OBJECT_TRACKER_CORE_HPP_

#include "multi_object_tracker/data_association/data_association.hpp"
#include "multi_object_tracker/tracker/model/tracker_base.hpp"

#include <rclcpp/rclcpp.hpp>
#include <tier4_autoware_utils/ros/debug_publisher.hpp>
#include <tier4_autoware_utils/system/stop_watch.hpp>

#include <autoware_auto_perception_msgs/msg/detected_objects.hpp>
#include <autoware_auto_perception_msgs/msg/tracked_objects.hpp>
#include <geometry_msgs/msg/pose_stamped.hpp>

#include <tf2/LinearMath/Transform.h>
#include <tf2/convert.h>
#include <tf2/transform_datatypes.h>

#ifdef ROS_DISTRO_GALACTIC
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#else
#include <tf2_geometry_msgs/tf2_geometry_msgs.hpp>
#endif

#include <diagnostic_updater/diagnostic_updater.hpp>
#include <diagnostic_updater/publisher.hpp>

#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <list>
#include <map>
#include <memory>
#include <string>
#include <vector>

/**
 * @brief Debugger class for multi object tracker
 * @details This class is used to publish debug information of multi object tracker
 */
class TrackerDebugger
{
public:
  explicit TrackerDebugger(rclcpp::Node & node);
  void publishProcessingTime();
  void publishTentativeObjects(
    const autoware_auto_perception_msgs::msg::TrackedObjects & tentative_objects) const;
  void startStopWatch();
  void startMeasurementTime(const rclcpp::Time & measurement_header_stamp);
  void setupDiagnostics();
  void checkDelay(diagnostic_updater::DiagnosticStatusWrapper & stat);
  struct DEBUG_SETTINGS
  {
    bool publish_processing_time;
    bool publish_tentative_objects;
    double diagnostics_warn_delay;
    double diagnostics_error_delay;
  } debug_settings_;
  double elapsed_time_from_sensor_input_ = 0.0;
  diagnostic_updater::Updater diagnostic_updater_;

private:
  void loadParameters();
  rclcpp::Node & node_;
  rclcpp::Publisher<autoware_auto_perception_msgs::msg::TrackedObjects>::SharedPtr
    debug_tentative_objects_pub_;
  std::unique_ptr<tier4_autoware_utils::StopWatch<std::chrono::milliseconds>> stop_watch_ptr_;
  std::unique_ptr<tier4_autoware_utils::DebugPublisher> processing_time_publisher_;
  rclcpp::Time last_input_stamp_;
};

class MultiObjectTracker : public rclcpp::Node
{
public:
  explicit MultiObjectTracker(const rclcpp::NodeOptions & node_options);

private:
  rclcpp::Publisher<autoware_auto_perception_msgs::msg::TrackedObjects>::SharedPtr
    tracked_objects_pub_;
  rclcpp::Subscription<autoware_auto_perception_msgs::msg::DetectedObjects>::SharedPtr
    detected_object_sub_;
  rclcpp::TimerBase::SharedPtr publish_timer_;  // publish timer

  // debugger class
  std::unique_ptr<TrackerDebugger> debugger_;

  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener tf_listener_;

  std::map<std::uint8_t, std::string> tracker_map_;

  void onMeasurement(
    const autoware_auto_perception_msgs::msg::DetectedObjects::ConstSharedPtr input_objects_msg);
  void onTimer();

  std::string world_frame_id_;  // tracking frame
  std::list<std::shared_ptr<Tracker>> list_tracker_;
  std::unique_ptr<DataAssociation> data_association_;

  void checkTrackerLifeCycle(
    std::list<std::shared_ptr<Tracker>> & list_tracker, const rclcpp::Time & time,
    const geometry_msgs::msg::Transform & self_transform);
  void sanitizeTracker(
    std::list<std::shared_ptr<Tracker>> & list_tracker, const rclcpp::Time & time);
  std::shared_ptr<Tracker> createNewTracker(
    const autoware_auto_perception_msgs::msg::DetectedObject & object, const rclcpp::Time & time,
    const geometry_msgs::msg::Transform & self_transform) const;

  void publish(const rclcpp::Time & time) const;
  inline bool shouldTrackerPublish(const std::shared_ptr<const Tracker> tracker) const;
};

#endif  // MULTI_OBJECT_TRACKER__MULTI_OBJECT_TRACKER_CORE_HPP_
