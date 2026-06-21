#include "load_dispatcher.hpp"
#include <string>

using namespace std::chrono_literals;

LoadDispatcher::LoadDispatcher()
: Node("load_dispatcher")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("cycle_rate", 2.0);
    cycle_rate_ = this->get_parameter("cycle_rate").as_double();
    this->declare_parameter<double>("dig_zone_x", 5.0);
    dig_zone_x_ = this->get_parameter("dig_zone_x").as_double();
    this->declare_parameter<double>("dig_zone_y", 0.0);
    dig_zone_y_ = this->get_parameter("dig_zone_y").as_double();
    this->declare_parameter<double>("dump_zone_x", 150.0);
    dump_zone_x_ = this->get_parameter("dump_zone_x").as_double();
    this->declare_parameter<double>("dump_zone_y", 0.0);
    dump_zone_y_ = this->get_parameter("dump_zone_y").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    dispatch_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);
    service_group_ = this->create_callback_group(rclcpp::CallbackGroupType::Reentrant);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        waypoint_command_pub_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/mining/fleet/waypoint_command", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/fleet/waypoint_command");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(1000));
        load_assignment_pub_ = this->create_publisher<std_msgs::msg::String>("/mining/fleet/load_assignment", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /mining/fleet/load_assignment");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = dispatch_group_;
        truck_pose_sub_ = this->create_subscription<geometry_msgs::msg::PoseStamped>(
            "/mining/truck/truck_pose", qos,
            std::bind(&LoadDispatcher::on_truck_pose, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/truck/truck_pose");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(500));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = dispatch_group_;
        payload_status_sub_ = this->create_subscription<geometry_msgs::msg::Vector3>(
            "/mining/truck/payload_status", qos,
            std::bind(&LoadDispatcher::on_payload_status, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/truck/payload_status");
    }
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(200));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = dispatch_group_;
        proximity_alert_sub_ = this->create_subscription<std_msgs::msg::Bool>(
            "/mining/excavator/proximity_alert", qos,
            std::bind(&LoadDispatcher::on_proximity_alert, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /mining/excavator/proximity_alert");
    }

    // ── Service servers ───────────────────────────────────────────────────────
    request_load_srv_ = this->create_service<std_srvs::srv::Trigger>(
        "/mining/fleet/request_load",
        std::bind(&LoadDispatcher::on_request_load, this,
            std::placeholders::_1, std::placeholders::_2),
        rmw_qos_profile_services_default,
        service_group_);
    RCLCPP_INFO(this->get_logger(), "Service ready: /mining/fleet/request_load");

    // ── Service clients ───────────────────────────────────────────────────────
    emergency_stop_client_ = this->create_client<std_srvs::srv::Trigger>("/mining/fleet/emergency_stop");
    RCLCPP_INFO(this->get_logger(), "Client created: /mining/fleet/emergency_stop");

    // ── Timers ────────────────────────────────────────────────────────────────
    {
        auto period = std::chrono::milliseconds(static_cast<int>(1000.0 / 2.0));
        cycle_tick_timer_ = this->create_wall_timer(
            period,
            std::bind(&LoadDispatcher::cycle_tick, this),
            dispatch_group_);
        RCLCPP_INFO(this->get_logger(), "Timer 'cycle_tick' started at 2.0 Hz");
    }
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadDispatcher::on_truck_pose(const geometry_msgs::msg::PoseStamped::SharedPtr msg)
{
    //-- begin impl [on_truck_pose] ----------------------------------------
    truck_x_ = msg->pose.position.x;
    truck_y_ = msg->pose.position.y;
    //-- end impl [on_truck_pose] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadDispatcher::on_payload_status(const geometry_msgs::msg::Vector3::SharedPtr msg)
{
    //-- begin impl [on_payload_status] ----------------------------------------
    payload_full_ = (msg->z >= 1.0);
    //-- end impl [on_payload_status] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadDispatcher::on_proximity_alert(const std_msgs::msg::Bool::SharedPtr msg)
{
    //-- begin impl [on_proximity_alert] ----------------------------------------
    truck_at_zone_ = msg->data;
    //-- end impl [on_proximity_alert] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadDispatcher::on_request_load(
    const std_srvs::srv::Trigger::Request::SharedPtr request,
    std_srvs::srv::Trigger::Response::SharedPtr response)
{
    //-- begin impl [on_request_load] ----------------------------------------
    (void)request;
    load_requested_ = true;
    response->success = true;
    response->message = "Load cycle approved by dispatcher";
    //-- end impl [on_request_load] ------------------------------------------
}

// ─────────────────────────────────────────────────────────────────────────────
void LoadDispatcher::cycle_tick()
{
    //-- begin impl [cycle_tick] -----------------------------------------------
    auto wp = geometry_msgs::msg::PoseStamped();
    wp.header.stamp    = this->now();
    wp.header.frame_id = "map";
    wp.pose.orientation.w = 1.0;

    switch (cycle_state_) {
        case 0:  // POSITIONING — drive truck to dig zone
            wp.pose.position.x = dig_zone_x_;
            wp.pose.position.y = dig_zone_y_;
            waypoint_command_pub_->publish(wp);
            if (truck_at_zone_) {
                cycle_state_ = 1;
                RCLCPP_INFO(this->get_logger(), "DISPATCH: POSITIONING → WAITING (truck at dig zone)");
            }
            break;

        case 1:  // WAITING — truck parked, arm will start digging
            if (load_requested_) {
                load_requested_ = false;
                cycle_state_ = 2;
                RCLCPP_INFO(this->get_logger(), "DISPATCH: WAITING → LOADING (arm confirmed)");
            }
            break;

        case 2:  // LOADING — excavator is filling the truck
            if (payload_full_) {
                cycle_state_ = 3;
                RCLCPP_INFO(this->get_logger(), "DISPATCH: LOADING → DISPATCHING (truck full)");
            }
            break;

        case 3:  // DISPATCHING — send truck to dump zone
            wp.pose.position.x = dump_zone_x_;
            wp.pose.position.y = dump_zone_y_;
            waypoint_command_pub_->publish(wp);
            if (!truck_at_zone_ && payload_full_) {
                // Truck has left the dig zone → cycle restarts once it dumps and returns
                ++cycles_complete_;
                payload_full_  = false;
                cycle_state_   = 0;
                auto assign = std_msgs::msg::String();
                assign.data = "CYCLE_COMPLETE count=" + std::to_string(cycles_complete_);
                load_assignment_pub_->publish(assign);
                RCLCPP_INFO(this->get_logger(), "DISPATCH: Cycle %d complete → repositioning truck",
                    cycles_complete_);
            }
            break;
    }
    //-- end impl [cycle_tick] -------------------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::executors::MultiThreadedExecutor executor(
        rclcpp::ExecutorOptions(), 2);
    auto node = std::make_shared<LoadDispatcher>();
    executor.add_node(node);
    executor.spin();
    rclcpp::shutdown();
    return 0;
}