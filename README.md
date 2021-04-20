# AtlasRecorder

## Introduction

Modified the Kinect SDK k4arecorder to write files in blocks, and buffer / flush recordings
in a seperate thread to prevent frame drops.

-----

K4ARecorder is a command line utility for creating Azure Kinect device recordings. Recordings are saved in the Matroska (MKV) format,
using multiple tracks to store each sensor stream in a single file.

## Usage Info

```
 Options:
  -h, --help              Prints this help
  --list                  List the currently connected K4A devices
  --device                Specify the device index to use (default: 0)
  -l, --max-block-length  Limit the the file block length to N seconds (default: 300)
  -c, --color-mode        Set the color sensor mode (default: 1080p), Available options:
                            3072p, 2160p, 1536p, 1440p, 1080p, 720p, 720p_NV12, 720p_YUY2, OFF
  -d, --depth-mode        Set the depth sensor mode (default: NFOV_UNBINNED), Available options:
                            NFOV_2X2BINNED, NFOV_UNBINNED, WFOV_2X2BINNED, WFOV_UNBINNED, PASSIVE_IR, OFF
  --depth-delay           Set the time offset between color and depth frames in microseconds (default: 0)
                            A negative value means depth frames will arrive before color frames.
                            The delay must be less than 1 frame period.
  -r, --rate              Set the camera frame rate in Frames per Second
                            Default is the maximum rate supported by the camera modes.
                            Available options: 30, 15, 5
  --imu                   Set the IMU recording mode (ON, OFF, default: ON)
  --external-sync         Set the external sync mode (Master, Subordinate, Standalone default: Standalone)
  --sync-delay            Set the external sync delay off the master camera in microseconds (default: 0)
                            This setting is only valid if the camera is in Subordinate mode.
  -e, --exposure-control  Set manual exposure value from 2 us to 200,000us for the RGB camera (default: 
                            auto exposure). This control also supports MFC settings of -11 to 1).
  -g, --gain              Set cameras manual gain. The valid range is 0 to 255. (default: auto)
```
