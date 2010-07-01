define uav_laser laser (
    # Range of the lidar in meters
    range_max 5
    
    # Field of Vision in degrees
    fov 180
    
    # Number of samples, for our lidar it is 768
    samples 540
    
    # Orientation of the laser w/ respect to the robot top-center
    pose [0.0 0.0 0.0 0]
    
    # Size of the model in meters
    size [0.025 0.025 0.05]
)

define uav position (
    # Physical size
    size [1 1 0.1]
    
    origin [0 0 0 0]
    
    drive "omni"
    
#    localization "gps"
    
    # Shape
    block (
        points 4
        point[0] [0 0]
        point[1] [1 0]
        point[2] [1 1]
        point[3] [0 1]
        z [0 1]
    )
    
    uav_laser()
)
