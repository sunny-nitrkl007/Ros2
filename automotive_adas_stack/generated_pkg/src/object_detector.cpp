#include "object_detector.hpp"

using namespace std::chrono_literals;

ObjectDetector::ObjectDetector()
: Node("object_detector")
{
    // ── Parameters ────────────────────────────────────────────────────────────
    this->declare_parameter<double>("confidence_threshold", 0.75);
    confidence_threshold_ = this->get_parameter("confidence_threshold").as_double();
    this->declare_parameter<int64_t>("max_objects", 20);
    max_objects_ = this->get_parameter("max_objects").as_int();
    this->declare_parameter<double>("detection_range", 30.0);
    detection_range_ = this->get_parameter("detection_range").as_double();
    RCLCPP_INFO(this->get_logger(), "Parameters loaded.");

    // ── Callback groups ───────────────────────────────────────────────────────
    detection_group_ = this->create_callback_group(rclcpp::CallbackGroupType::MutuallyExclusive);

    // ── Publishers ────────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(10));
        qos.reliable();
        qos.deadline(std::chrono::milliseconds(100));
        detected_objects_pub_ = this->create_publisher<visualization_msgs::msg::MarkerArray>("/adas/perception/detected_objects", qos);
        RCLCPP_INFO(this->get_logger(), "Publisher ready: /adas/perception/detected_objects");
    }

    // ── Subscribers ───────────────────────────────────────────────────────────
    {
        rclcpp::QoS qos(rclcpp::KeepLast(5));
        qos.best_effort();
        qos.deadline(std::chrono::milliseconds(20));
        auto sub_opts = rclcpp::SubscriptionOptions();
        sub_opts.callback_group = detection_group_;
        fused_environment_sub_ = this->create_subscription<nav_msgs::msg::OccupancyGrid>(
            "/adas/perception/fused_environment", qos,
            std::bind(&ObjectDetector::on_fused_environment, this, std::placeholders::_1),
            sub_opts);
        RCLCPP_INFO(this->get_logger(), "Subscriber ready: /adas/perception/fused_environment");
    }

}

// ─────────────────────────────────────────────────────────────────────────────
void ObjectDetector::on_fused_environment(const nav_msgs::msg::OccupancyGrid::SharedPtr msg)
{
    //-- begin impl [on_fused_environment] ----------------------------------------
    auto markers = visualization_msgs::msg::MarkerArray();
    int obj_id = 0;

    int W = static_cast<int>(msg->info.width);
    int H = static_cast<int>(msg->info.height);
    double res = msg->info.resolution;

    // Scan grid for occupied cells — cluster by proximity (simple grid-scan)
    std::vector<bool> visited(W * H, false);
    for (int y = 0; y < H && obj_id < max_objects_; ++y) {
        for (int x = 0; x < W && obj_id < max_objects_; ++x) {
            int idx = y * W + x;
            if (msg->data[idx] < 50 || visited[idx]) continue;

            // BFS centroid
            double sum_x = 0, sum_y = 0;
            int cell_count = 0;
            std::vector<int> queue = {idx};
            visited[idx] = true;
            while (!queue.empty()) {
                int cur = queue.back(); queue.pop_back();
                int cx = cur % W, cy = cur / W;
                sum_x += cx; sum_y += cy; ++cell_count;
                for (auto [dx, dy] : std::vector<std::pair<int,int>>{{1,0},{-1,0},{0,1},{0,-1}}) {
                    int nx = cx + dx, ny = cy + dy;
                    if (nx < 0 || nx >= W || ny < 0 || ny >= H) continue;
                    int nidx = ny * W + nx;
                    if (!visited[nidx] && msg->data[nidx] >= 50) {
                        visited[nidx] = true;
                        queue.push_back(nidx);
                    }
                }
            }

            double world_x = msg->info.origin.position.x + (sum_x / cell_count) * res;
            double world_y = msg->info.origin.position.y + (sum_y / cell_count) * res;
            double distance = std::sqrt(world_x * world_x + world_y * world_y);
            if (distance > detection_range_) continue;

            visualization_msgs::msg::Marker m;
            m.header     = msg->header;
            m.ns         = "detected_objects";
            m.id         = obj_id++;
            m.type       = visualization_msgs::msg::Marker::SPHERE;
            m.action     = visualization_msgs::msg::Marker::ADD;
            m.pose.position.x = world_x;
            m.pose.position.y = world_y;
            m.pose.orientation.w = 1.0;
            m.scale.x = m.scale.y = m.scale.z = res * std::sqrt(static_cast<double>(cell_count));
            m.color.r = 1.0f; m.color.a = 0.8f;
            markers.markers.push_back(m);
        }
    }

    detected_count_ = obj_id;
    detected_objects_pub_->publish(markers);

    if (detected_count_ > 0)
        RCLCPP_DEBUG(this->get_logger(), "Detected %d object(s)", detected_count_);
    //-- end impl [on_fused_environment] ----------------------------------------
}

int main(int argc, char * argv[])
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<ObjectDetector>());
    rclcpp::shutdown();
    return 0;
}