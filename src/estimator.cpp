#include <ros/ros.h>
#include <pcl_ros/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/features/normal_3d_omp.h>
#include <hand_control/Plan.h>
#include <time.h>
#include <math.h>
#include <dynamic_reconfigure/server.h>
#include <hand_control/EstimatorConfig.h>

#include <pcl/common/pca.h>

typedef pcl::PointXYZRGB Point;
typedef pcl::PointCloud<Point> PointCloud;
typedef Eigen::Matrix3f& Matrix;

class Callback {
  public:
    void operator()(const PointCloud::ConstPtr& msg)
    {
      ROS_INFO("PointCloud received");

      if (msg->width > 3){

        analyser.setInputCloud(msg);
        Matrix eg = analyser.getEigenVectors();

        float x, y, z, th, h, c;
        x = y = z = th = h = c = 0.;

        // we consider the whole PointCloud
        std::vector<int> indices;
        for (int i = 0; i < msg->points.size(); ++i)
          indices.push_back(i);

        // v = eg_1 ^ eg_2 is the plan normal
        Eigen::Vector3f v = eg.col(0).cross(eg.col(1));
        // norm(v) == 1
        v.normalize();
        x = v(0); y=v(1); z=v(2);

        // h is the altitude
        h = (analyser.getMean())(2);

        // this formula is good only if :
        //  -pi/2 <= th <= pi/2
        // ie cos(th) == m_x >= 0
        float m_x, m_y;
        if (!reverse)
        {
           m_x = eg(0,0);
           m_y = eg(1,0);
        } else {
           m_x = eg(1,0);
           m_y = eg(0,0);
        }

        if(m_x < 0) 
        {
            m_x *= -1.;
            m_y *= -1.;
        }
        th = 2 * atan(m_y / (1 + m_x));
        // -pi/2 <= th <= pi/2
        
        th *= _RAD2DEG;
        // -90 <= th <= 90

        // publication
        ROS_INFO("Plan published");
        publisher.publish(to_Plan(x, y, z, h, th, c, msg->header.seq, msg->header.stamp, msg->width));
      }
    }

    Callback(ros::Publisher& pub):publisher(pub), _RAD2DEG(45.f/atan(1.)), reverse(false) {};


  void reconfigure(const hand_control::EstimatorConfig& c, const uint32_t& level) { 
    reverse = c.reverse ;
  }

  private:
    ros::Publisher publisher;
    pcl::PCA<Point> analyser;
    const float _RAD2DEG;
    bool reverse;

    inline const hand_control::Plan::ConstPtr
      to_Plan(const float& x, const float& y,
              const float& z, const float& h,
              const float& th,
              const float& c, const uint32_t& seq,
              const uint64_t& msec64, const uint64_t& number)
      {
        hand_control::Plan::Ptr ros_msg(new hand_control::Plan());
        ros_msg->normal.x = x; 
        ros_msg->normal.y = y; 
        ros_msg->normal.z = z; 
        ros_msg->altitude = h;
        ros_msg->angle = th;
        ros_msg->curvature = c; 
        ros_msg->number = number;
        // uint64_t msec64 is in ms (10-6)
        uint64_t sec64 = msec64 / 1000000;
        uint64_t nsec64 = (msec64 % 1000000) * 1000;
        ros_msg->header.stamp.sec = (uint32_t) sec64;
        ros_msg->header.stamp.nsec = (uint32_t) nsec64;
        ros_msg->header.seq = seq;
        ros_msg->header.frame_id = "0";
        return ros_msg;
      }
};

int main(int argc, char** argv)
{
  ros::init(argc, argv, "estimator");
  ros::NodeHandle node("estimator");

  ros::Publisher  publisher = node.advertise<hand_control::Plan>("output", 1);
  Callback callback(publisher);
  ros::Subscriber subscriber = node.subscribe<PointCloud>("input", 1, callback);

  dynamic_reconfigure::Server<hand_control::EstimatorConfig> server;
  dynamic_reconfigure::Server<hand_control::EstimatorConfig>::CallbackType f;
  f = boost::bind(&Callback::reconfigure, &callback, _1, _2);
  server.setCallback(f);

  ROS_INFO("node started");
  ros::spin();
  ROS_INFO("exit");
  return 0;
}
