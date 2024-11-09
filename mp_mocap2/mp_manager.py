import mediapipe 
from enum import auto, IntEnum

POSE_LANDMARK = mediapipe.solutions.pose.PoseLandmark
HAND_LANDMARK = mediapipe.solutions.hands.HandLandmark


class MediapipeModel(IntEnum):
    Pose = 0
    Holistic = auto()

class MediapipeManager():
    def __init__(self):
        self.redis = None
        self.mp_pose = mediapipe.solutions.pose
        self.mp_drawing = mediapipe.solutions.drawing_utils
        self.mp_drawing_styles = mediapipe.solutions.drawing_styles
        self.mp_holistic = mediapipe.solutions.holistic

        self.pose_dict = dict()
        self.holistic_dict = dict()
        self.key = ""
        self.set_key()
        self.is_hips_move = True
        self.min_visibility = 0.5
        self.max_frame_num = 5000
        self.is_show_result = True
        self.fps = 24
        self.factor = 0.0

        self.model = MediapipeModel.Pose

    def set_key(self, model_complexity=1, static_image_mode=False, min_detection_confidence=0.7):
        self.key = str(model_complexity) + ' ' + \
            str(min_detection_confidence)+' ' + str(static_image_mode)

    def set_pose(self):
        self.model = MediapipeModel.Pose
        if self.key not in self.pose_dict:
            items = self.key.split()

            self.pose_dict[self.key] = self.mp_pose.Pose(
                static_image_mode=(items[2] == "True"),
                min_detection_confidence=float(items[1]),
                model_complexity=int(items[0]),
                smooth_landmarks=True,
                min_tracking_confidence=0.5
            )

    def set_holistic(self):
        self.model = MediapipeModel.Holistic
        if self.key not in self.holistic_dict:
            items = self.key.split()

            self.holistic_dict[self.key] = self.mp_holistic.Holistic(
                static_image_mode=(items[2] == "True"),
                min_detection_confidence=float(items[1]),
                model_complexity=int(items[0]),
                smooth_landmarks=True,
                min_tracking_confidence=0.5
            )


    def get_pose(self):
        return self.pose_dict[self.key]

    def get_holistic(self):
        return self.holistic_dict[self.key]

    def get_model(self):
        if self.model == MediapipeModel.Pose:
            self.set_pose()
            return self.get_pose()
        else:
            self.set_holistic()
            return self.get_holistic()