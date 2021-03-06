#pragma once

#include <hl_communication/camera.pb.h>
#include <hl_communication/wrapper.pb.h>
#include <hl_communication/labelling.pb.h>

#include <google/protobuf/message.h>
#include <json/json.h>
#include <opencv2/core.hpp>
#include <Eigen/Geometry>

#include <string>

/**
 * Contains:
 * - Basic utils
 * - Protobuf simpler interface
 * - Multiple conversion tools from hl_communication protobuf format to OpenCV classical format
 * - Utilities functions related to jsoncpp
 */
namespace hl_communication
{
#define HL_DEBUG                                                                                                       \
  (std::string(__FUNCTION__) + ":" + hl_communication::getBaseName(__FILE__) + ":" + std::to_string(__LINE__) + ": ")

/**
 * RobotIdentifier are ordered by team and then by robot_id
 */
bool operator<(const RobotIdentifier& id1, const RobotIdentifier& id2);
bool operator==(const RobotIdentifier& id1, const RobotIdentifier& id2);

/**
 * Order first by ip, then by port and finally by packet_no
 *
 * Throws an error if src_ip or src_port of one of the message is not set
 */
bool operator<(const MsgIdentifier& id1, const MsgIdentifier& id2);

/**
 * Order first by robot id, then by camera name
 */
bool operator<(const RobotCameraIdentifier& id1, const RobotCameraIdentifier& id2);

/**
 * Following rules for comparison:
 * robot_identifier < external_identifier
 * if both robot, compare RobotCameraIdentifier
 * if both external, compare names
 * if source is the same, compare video start
 */
bool operator<(const VideoSourceID& id1, const VideoSourceID& id2);

void readFromFile(const std::string& path, google::protobuf::Message* msg);

void writeToFile(const std::string& path, const google::protobuf::Message& msg);

/**
 * Return the name of the file at the given path:
 * e.g getBaseName("toto/file.cpp") returns "file.cpp"
 */
std::string getBaseName(const std::string& path);

/**
 * Return the steady clock time_stamp in a integer value (unit: microseconds)
 */
uint64_t getTimeStamp();

/**
 * Return the system clock time_stamp in a integer value (unit: microseconds)
 */
uint64_t getUTCTimeStamp();

/**
 * Return the offset from steady_clock to system_clock in us:
 * steady_clock + offset = system_clock
 */
int64_t getSteadyClockOffset();

/**
 * Show a duration [us] in a pretty format ..d:..h:..m:..s:...ms
 * Only non-zero part are shown
 */
std::string getPrettyDuration(uint64_t duration_us);

/**
 * Convert a human readable string to a 8 bytes ip address
 */
uint64_t stringToIP(const std::string& str);

/**
 * Convert a 8 bytes ip address to a human readable string
 */
std::string ipToString(uint64_t ip_address);

/**
 * Invert the side of the angle message (x-axis toward left or right of team area)
 */
void invertPosition(PositionDistribution* position);

/**
 * Invert the side of the angle message (x-axis toward left or right of team area)
 */
void invertAngle(AngleDistribution* position);

/**
 * Invert the side of the provided pose message (x-axis toward left or right of team area)
 */
void invertPose(PoseDistribution* pose);

/**
 * Export uncertainty from Pos Distribution to a covariance matrix, return true on success and false on failure (no
 * uncertainty or size invalid)
 */
bool exportUncertainty(const PositionDistribution& position, cv::Mat* out);

/**
 * Return false if player is not specifically penalized in GCMsg. This means
 * that even if GCMsg does not concern 'team_id', the answer will be false.
 * robot_id starts from 1
 */
bool isPenalized(const GCMsg& msg, int team_id, int robot_id);

/**
 * Uses system_clock to extract a formatted time: format is:
 * - YYYY_MM_DD_HHhMMmSSs Ex: 2018_09_25_17h23m12s
 * Function is reentrant
 */
std::string getFormattedTime();

void intrinsicToCV(const IntrinsicParameters& camera_parameters, cv::Mat* camera_matrix,
                   cv::Mat* distortion_coefficients, cv::Size* img_size);
void cvToIntrinsic(const cv::Mat& camera_matrix, const cv::Mat& distortion_coefficients, const cv::Size& img_size,
                   IntrinsicParameters* camera_parameters);
void pose3DToCV(const Pose3D& pose, cv::Mat* rvec, cv::Mat* tvec);
void cvToPose3D(const cv::Mat& rvec, const cv::Mat& tvec, Pose3D* pose);

/**
 * Convert a protobuf Pose3D to an Affine3D transform
 */
Eigen::Affine3d getAffineFromProtobuf(const Pose3D& pose);

/**
 * Export the given Affine3D transform to a pose
 */
void setProtobufFromAffine(const Eigen::Affine3d& affine, Pose3D* pose);

std::ostream& operator<<(std::ostream& out, const Pose3D& pose);

/**
 * Build a cv::Point3f at the mean of the position distribution. Z coordinate is set to 0.
 */
cv::Point3f cvtToPoint3f(const PositionDistribution& position);

/**
 * Extract the field position of an object position in the basis of a robot. Do not take into account uncertainty.
 */
cv::Point3f fieldFromSelf(const PositionDistribution& obj_pos_in_self, const PoseDistribution& robot_pose);

cv::Size getImgSize(const CameraMetaInformation& camera_information);

/**
 * Convert a point from the field basis to the camera basis, should be used to check if points are facing the camera
 */
cv::Point3f fieldToCamera(const cv::Point3f& pos_in_field, const cv::Mat& rvec, const cv::Mat& tvec);

cv::Point2f fieldToImg(const cv::Point3f& pos_in_field, const CameraMetaInformation& camera_information);

bool isPointValidForCorrection(const cv::Point3f& pos, cv::Mat rvec, cv::Mat tvec, cv::Mat camera_matrix,
                               cv::Mat distortion_coeffs);

cv::Point2f protobufToCV(const Point2DMsg& msg);
cv::Point3f protobufToCV(const Point3DMsg& msg);
void cvToProtobuf(const cv::Point2f& pos, Point2DMsg* msg);
void cvToProtobuf(const cv::Point3f& pos, Point3DMsg* msg);

void protobufToCV(const std::vector<Match2D3DMsg>& matches, std::vector<cv::Point2f>* img_pos,
                  std::vector<cv::Point3f>* obj_pos);

/**
 * Convert from field position to an image position based on camera_information.
 * @return Is the img position valid (for points behind camera, the underlying implementation return image position of
 * the symetric point), if the point is outside of image, then return false as well
 */
bool fieldToImg(const cv::Point3f& pos_in_field, const CameraMetaInformation& camera_information, cv::Point2f* img_pos);

/**
 * Return the time stamp of the given frame, utc or monotonic type is chosen depending on  utc flag
 */
uint64_t getTS(const FrameEntry& entry, bool utc = true);

/**
 * Returns the index of the frame corresponding to the given time_stamp
 * If no frame has the given time_stamp, return the frame previous to given time_stamp
 * If there is no frame before given time_stamp, returns -1
 */
int getIndex(const VideoMetaInformation& meta_information, uint64_t time_stamp, bool utc = true);

/**
 * Returns the time stamp of the frame at given index.
 * If information is not available throw out_of_range
 */
uint64_t getTimeStamp(const VideoMetaInformation& meta_information, int index, bool utc = true);

Json::Value file2Json(const std::string& path);

/**
 * If human is enabled, then indent the string and uses end of line
 */
std::string json2String(const Json::Value& v, bool human = true);

/**
 * @see json2String
 */
void writeJson(const Json::Value& v, const std::string& path, bool human = true);

void checkMember(const Json::Value& v, const std::string& key);

template <typename T>
Json::Value val2Json(const T& val)
{
  return val.toJson();
}

template <>
Json::Value val2Json<bool>(const bool& val);
template <>
Json::Value val2Json<int>(const int& val);
template <>
Json::Value val2Json<size_t>(const size_t& val);
template <>
Json::Value val2Json<float>(const float& val);
template <>
Json::Value val2Json<double>(const double& val);
template <>
Json::Value val2Json<std::string>(const std::string& val);
template <>
Json::Value val2Json<cv::Scalar>(const cv::Scalar& color);

/**
 * Place v[key] in 'dst'
 * Throws an error with an explicit message in case:
 * - v is not an object
 * - v[key] does not contain an object
 * - v[key] has not the required type
 */
template <typename T>
void readVal(const Json::Value& v, const std::string& key, T* dst)
{
  checkMember(v, key);
  dst->fromJson(v[key]);
}

template <>
void readVal<bool>(const Json::Value& v, const std::string& key, bool* dst);

template <>
void readVal<int>(const Json::Value& v, const std::string& key, int* dst);

template <>
void readVal<float>(const Json::Value& v, const std::string& key, float* dst);

template <>
void readVal<double>(const Json::Value& v, const std::string& key, double* dst);

template <>
void readVal<std::string>(const Json::Value& v, const std::string& key, std::string* dst);

/**
 * Also check that all values of the scalar are in [0,255]
 */
template <>
void readVal<cv::Scalar>(const Json::Value& v, const std::string& key, cv::Scalar* dst);

/**
 * Place v[key] in 'dst', if v is not an object or if v does not contain key, do
 * not change dst and returns
 *
 * Throws an error with an explicit message in case:
 * - v[key] has not the required type
 */
template <typename T>
void tryReadVal(const Json::Value& v, const std::string& key, T* dst)
{
  if (!v.isObject() || !v.isMember(key))
  {
    return;
  }
  readVal(v, key, dst);
}

template <typename T>
std::map<uint32_t, T> readMap(const Json::Value& v, const std::string& map_key)
{
  checkMember(v, map_key);
  if (!v[map_key].isObject())
  {
    throw std::runtime_error("readMap(): Value at '" + map_key + "' is not an object");
  }
  // Parse entries:
  const Json::Value& map_v = v[map_key];
  std::map<uint32_t, T> result;
  for (Json::ValueConstIterator it = map_v.begin(); it != map_v.end(); it++)
  {
    const std::string& key = it.name();
    readVal(map_v, key, &(result[std::stoi(key)]));
  }
  return result;
}

template <typename T>
void tryReadMap(const Json::Value& v, const std::string& key, std::map<uint32_t, T>* ptr)
{
  if (v.isObject() && v.isMember(key))
  {
    *ptr = readMap<T>(v, key);
  }
}

template <typename T>
Json::Value map2Json(const std::map<uint32_t, T>& values)
{
  Json::Value v(Json::objectValue);
  for (const auto& pair : values)
  {
    v[std::to_string(pair.first)] = val2Json(pair.second);
  }
  return v;
}

std::ostream& operator<<(std::ostream& out, const RobotIdentifier& id);
std::ostream& operator<<(std::ostream& out, const RobotCameraIdentifier& id);
std::ostream& operator<<(std::ostream& out, const VideoSourceID& id);

}  // namespace hl_communication
