import mediapipe 

POSE_LANDMARK = mediapipe.solutions.pose.PoseLandmark

class MediapipeManager():
    def __init__(self):
        self.mp_pose = mediapipe.solutions.pose
        self.mp_drawing = mediapipe.solutions.drawing_utils
        self.mp_drawing_styles = mediapipe.solutions.drawing_styles
        self.pose_dict = dict()
        self.key = ""
        self.set_key()
        self.is_hips_move = True
        self.min_visibility = 0.7
        self.max_frame_num = 5000
        self.is_show_result = True
        self.fps = 24
        self.factor = 0.0

    def set_key(self, model_complexity=1, static_image_mode=False, min_detection_confidence=0.7):
        self.key = str(model_complexity) + ' ' + \
            str(min_detection_confidence)+' ' + str(static_image_mode)
        if self.key not in self.pose_dict:
            items = self.key.split()

            self.pose_dict[self.key] = self.mp_pose.Pose(
                static_image_mode=(items[2] == "True"),
                min_detection_confidence=float(items[1]),
                model_complexity=int(items[0]),
                smooth_landmarks=True,
                min_tracking_confidence=0.5
            )

    def get_pose(self):
        return self.pose_dict[self.key]