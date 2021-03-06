#include <ros/ros.h>
#include <sensor_msgs/PointCloud2.h>
// PCL specific includes
#include <pcl/ros/conversions.h>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>
#include <pcl/ModelCoefficients.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>

ros::Publisher pub;

void callback(const sensor_msgs::PointCloud2ConstPtr& cloud)
{
  pcl::PointCloud<pcl::PointXYZ>::Ptr transform_cloud(new pcl::PointCloud<pcl::PointXYZ>);

  sensor_msgs::PointCloud2::Ptr output_cloud(new sensor_msgs::PointCloud2);

  // Convert the sensor_msgs/PointCloud2 data to pcl/PointCloud
  pcl::fromROSMsg (*cloud, *transform_cloud);

  pcl::ModelCoefficients::Ptr coefficients (new pcl::ModelCoefficients ());
  pcl::PointIndices::Ptr inliers (new pcl::PointIndices ());

    // Create the segmentation object
  pcl::SACSegmentation<pcl::PointXYZ> seg;
  // Optional
  seg.setOptimizeCoefficients (true);
  // Mandatory
  seg.setModelType (pcl::SACMODEL_PLANE);
  seg.setMethodType (pcl::SAC_RANSAC);
  //seg.setMaxIterations (1000);
  seg.setDistanceThreshold (0.01);
  seg.setInputCloud (transform_cloud);
  seg.segment (*inliers, *coefficients);

  if (inliers->indices.size () == 0)
  {
    PCL_ERROR ("Could not estimate a planar model for the given dataset.");
  }
  std::cerr << "Model coefficients: " << coefficients->values[0] << " "
                                      << coefficients->values[1] << " "
                                      << coefficients->values[2] << " "
                                      << coefficients->values[3] << std::endl;

  std::cerr << "Model inliers: " << inliers->indices.size () << std::endl;
  for (size_t i = 0; i < inliers->indices.size (); ++i)
    std::cerr << inliers->indices[i] << "    " << transform_cloud->points[inliers->indices[i]].x << " "
                                               << transform_cloud->points[inliers->indices[i]].y << " "
                                               << transform_cloud->points[inliers->indices[i]].z << std::endl;

  // Convert the pcl/PointCloud to sensor_msgs/PointCloud2 data
  pcl::toROSMsg (*transform_cloud, *output_cloud);
  pub.publish(output_cloud);

}


int
main (int argc, char** argv)
{
 // INITIALIZE ROS
   ros::init (argc, argv, "planar_segmentation");
   ros::NodeHandle nh;
   ros::Subscriber sub = nh.subscribe("cloud", 1, callback);
   pub = nh.advertise<sensor_msgs::PointCloud2> ("cloud_filtered", 1);

   ros::spin();

   return (0);
}
