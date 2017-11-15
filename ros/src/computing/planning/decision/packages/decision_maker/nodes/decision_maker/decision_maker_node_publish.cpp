#include <ros/ros.h>
#include <ros/spinner.h>
#include <std_msgs/Float64.h>
#include <std_msgs/Float64MultiArray.h>
#include <std_msgs/String.h>
#include <stdio.h>
#include <tf/transform_listener.h>

// lib
#include <state.hpp>
#include <state_context.hpp>

#include <decision_maker_node.hpp>

#include <autoware_msgs/lane.h>
#include <autoware_msgs/state.h>
#include <jsk_recognition_msgs/BoundingBoxArray.h>
#include <visualization_msgs/MarkerArray.h>
#include <random>

#include <geometry_msgs/Point.h>
#include <geometry_msgs/Pose.h>
#include <geometry_msgs/PoseStamped.h>

namespace decision_maker
{
void DecisionMakerNode::update_pubsub(void)
{
  // if state machine require to re-subscribe topic,
  // this function will re-definition subscriber.
}

int DecisionMakerNode::createCrossRoadAreaMarker(visualization_msgs::Marker &crossroad_marker, double scale)
{
  crossroad_marker.header.frame_id = "/map";
  crossroad_marker.header.stamp = ros::Time();
  crossroad_marker.id = 1;
  crossroad_marker.type = visualization_msgs::Marker::SPHERE_LIST;
  crossroad_marker.action = visualization_msgs::Marker::ADD;
  crossroad_marker.ns = "crossroad";

  crossroad_marker.scale.x = scale;
  crossroad_marker.scale.y = scale;
  crossroad_marker.scale.z = 0.5;
  crossroad_marker.color.a = 0.15;
  crossroad_marker.color.r = 1.0;
  crossroad_marker.color.g = 0.0;
  crossroad_marker.color.b = 0.0;
  crossroad_marker.frame_locked = true;
  crossroad_marker.lifetime = ros::Duration(0.3);
}

void DecisionMakerNode::displayMarker(void)
{
  // vector_map init
  // parse vectormap
  initVectorMap();

  jsk_recognition_msgs::BoundingBoxArray bbox_array;

  static visualization_msgs::MarkerArray marker_array;
  static visualization_msgs::Marker crossroad_marker;
  static visualization_msgs::Marker inside_marker;

  double scale = 3.0;
  createCrossRoadAreaMarker(crossroad_marker, scale);

  inside_marker = crossroad_marker;
  inside_marker.scale.x = scale / 3;
  inside_marker.scale.y = scale / 3;
  inside_marker.scale.z = 0.5;
  inside_marker.color.a = 0.5;
  inside_marker.color.r = 1.0;
  inside_marker.color.g = 0.0;
  inside_marker.color.b = 0.0;
  inside_marker.ns = "inside";
  inside_marker.lifetime = ros::Duration();

  bbox_array.header = crossroad_marker.header;

  inside_marker.points.clear();

  for (auto &area : intersects)
  {
    area.bbox.header = crossroad_marker.header;
    bbox_array.boxes.push_back(area.bbox);
    for (const auto &p : area.insideWaypoint_points)
    {
      inside_marker.points.push_back(p);
    }
  }

  Pubs["crossroad_bbox"].publish(bbox_array);
  bbox_array.boxes.clear();
  Pubs["crossroad_inside_marker"].publish(inside_marker);

  marker_array.markers.clear();
}

void DecisionMakerNode::update_msgs(void)
{
  if (ctx)
  {
    static std::string prevStateName;
    CurrentStateName = ctx->getCurrentStateName();

    if (prevStateName != CurrentStateName)
    {
      prevStateName = CurrentStateName;
      update_pubsub();
    }

    state_string_msg.data = CurrentStateName;
    state_text_msg.text = createStateMessageText();

    autoware_msgs::state state_msg;
    state_msg.main_state = ctx->getCurrentStateName(state_machine::StateKinds::MAIN_STATE);
    state_msg.acc_state = ctx->getCurrentStateName(state_machine::StateKinds::ACC_STATE);
    state_msg.str_state = ctx->getCurrentStateName(state_machine::StateKinds::STR_STATE);
    state_msg.behavior_state = ctx->getCurrentStateName(state_machine::StateKinds::BEHAVIOR_STATE);
    
    Pubs["states"].publish(state_msg);
    Pubs["state"].publish(state_string_msg);
    Pubs["state_overlay"].publish(state_text_msg);
  }
  else
    std::cerr << "ctx is not found " << std::endl;
}

std::string DecisionMakerNode::createStateMessageText()
{
  return ctx->createStateMessageText();
}

void DecisionMakerNode::publishToVelocityArray()
{
  int count = 0;
  std_msgs::Float64MultiArray msg;

  for (const auto &i : current_finalwaypoints_.waypoints)
  {
    msg.data.push_back(amathutils::mps2kmph(i.twist.twist.linear.x));
    if (++count >= 10)
      break;
  }
  Pubs["target_velocity_array"].publish(msg);
}
}
