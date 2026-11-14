import numpy as np
import rclpy
from rclpy.node import Node
from rclpy.duration import Duration
from sensor_msgs.msg import Image, CameraInfo
from geometry_msgs.msg import PointStamped
from cv_bridge import CvBridge
from tf2_ros import Buffer, TransformListener, TransformException
import tf2_geometry_msgs  # noqa: F401  (registers PointStamped with tf2)


class RedObjectDetector(Node):
    """Find the red object in the colour image, get its depth, and publish
    its 3D position in the target frame (default: world)."""

    def __init__(self):
        super().__init__('red_object_detector')

        self.declare_parameter('target_frame', 'world')
        self.declare_parameter('min_pixels', 50)
        # Depth gives the nearest visible surface, not the object's centre.
        self.declare_parameter('object_radius', 0.02)
        self.object_radius = self.get_parameter('object_radius').value

        self.target_frame = self.get_parameter('target_frame').value
        self.min_pixels = self.get_parameter('min_pixels').value

        self.bridge = CvBridge()
        self.depth = None
        self.K = None  # fx, fy, cx, cy

        self.tf_buffer = Buffer()
        self.tf_listener = TransformListener(self.tf_buffer, self)

        self.create_subscription(
            CameraInfo, '/camera/depth_camera/camera_info', self.info_cb, 10)
        self.create_subscription(
            Image, '/camera/depth_camera/depth/image_raw', self.depth_cb, 10)
        self.create_subscription(
            Image, '/camera/depth_camera/image_raw', self.color_cb, 10)

        self.pub = self.create_publisher(PointStamped, '/detected_object', 10)

    def info_cb(self, msg):
        k = msg.k
        self.K = (k[0], k[4], k[2], k[5])  # fx, fy, cx, cy

    def depth_cb(self, msg):
        self.depth = self.bridge.imgmsg_to_cv2(msg, desired_encoding='passthrough')

    def color_cb(self, msg):
        if self.depth is None or self.K is None:
            return

        # 1. Find red pixels
        img = self.bridge.imgmsg_to_cv2(msg, desired_encoding='rgb8')
        r = img[:, :, 0].astype(np.int32)
        g = img[:, :, 1].astype(np.int32)
        b = img[:, :, 2].astype(np.int32)
        mask = (r > 80) & (g < 0.3 * r) & (b < 0.3 * r)
        if int(mask.sum()) < self.min_pixels:
            self.get_logger().info('no object found', throttle_duration_sec=2.0)
            return

        # 2. Centroid pixel and median depth
        vs, us = np.nonzero(mask)
        u, v = us.mean(), vs.mean()
        d = self.depth[mask]
        d = d[np.isfinite(d)]
        if d.size == 0:
            return
        z = float(np.percentile(d, 10))   # near surface; robust to edge pixels
        zc = z + self.object_radius       # sphere centre, along the camera's viewing axis

        # 3. Pixel + depth -> 3D point in the camera optical frame
        fx, fy, cx, cy = self.K
        p = PointStamped()
        p.header.frame_id = msg.header.frame_id  # camera_optical_frame
        p.point.x = (u - cx) * zc / fx
        p.point.y = (v - cy) * zc / fy
        p.point.z = zc

        # 4. Camera frame -> target frame (latest available transform)
        try:
            out = self.tf_buffer.transform(
                p, self.target_frame, timeout=Duration(seconds=0.2))
        except TransformException as e:
            self.get_logger().info(f'waiting for transform: {e}',
                                   throttle_duration_sec=2.0)
            return

        self.pub.publish(out)
        self.get_logger().info(
            f'object at ({out.point.x:.3f}, {out.point.y:.3f}, {out.point.z:.3f}) '
            f'in {self.target_frame}', throttle_duration_sec=1.0)


def main():
    rclpy.init()
    rclpy.spin(RedObjectDetector())


if __name__ == '__main__':
    main()