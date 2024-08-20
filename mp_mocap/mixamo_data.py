from enum import auto, IntEnum
from .mp_manager import POSE_LANDMARK as MP_POSE_LANDMARK
from .mp_manager import HAND_LANDMARK as MP_HAND_LANDMARK
from functools import partial

import glm

class Mixamo(IntEnum):
    Root = -1
    Hips = auto()
    # Spine
    Spine = auto()
    Spine1 = auto()
    Spine2 = auto()
    Neck = auto()
    # HeadTop_End does not exist in Mediapipe
    Head = auto()

    # Left Arm (LeftShoulder does not exist in Mediapipe)
    LeftArm = auto()
    LeftForeArm = auto()
    LeftHand = auto()
    LeftHandThumb1 = auto()
    LeftHandIndex1 = auto()
    LeftHandPinky1 = auto()
    # Right Arm (RightShoulder does not exist in Mediapipe)
    RightArm = auto()
    RightForeArm = auto()
    RightHand = auto()
    RightHandThumb1 = auto()
    RightHandIndex1 = auto()
    RightHandPinky1 = auto()
    # Left Leg (LeftToe_End does not exist in Mediapipe)
    LeftUpLeg = auto()
    LeftLeg = auto()
    LeftFoot = auto()
    LeftToeBase = auto()
    # Right Leg (RightToe_End does not exist in Mediapipe)
    RightUpLeg = auto()
    RightLeg = auto()
    RightFoot = auto()
    RightToeBase = auto()

    # Left Hand 22 + 18 + 18 = 58
    LeftHandThumb2 = auto()
    LeftHandThumb3 = auto()
    LeftHandThumb4 = auto()
    LeftHandIndex2 = auto()
    LeftHandIndex3 = auto()
    LeftHandIndex4 = auto()
    LeftHandMiddle1 =auto()
    LeftHandMiddle2 = auto()
    LeftHandMiddle3 = auto()
    LeftHandMiddle4 = auto()
    LeftHandRing1 = auto()
    LeftHandRing2 = auto()
    LeftHandRing3 = auto()
    LeftHandRing4 = auto()
    LeftHandPinky2 = auto()
    LeftHandPinky3 = auto()
    LeftHandPinky4 = auto()

    # Right Hand
    RightHandThumb2 = auto()
    RightHandThumb3 = auto()
    RightHandThumb4 = auto()
    RightHandIndex2 = auto()
    RightHandIndex3 = auto()
    RightHandIndex4 = auto()
    RightHandMiddle1 = auto()
    RightHandMiddle2 = auto()
    RightHandMiddle3 = auto()
    RightHandMiddle4 = auto()
    RightHandRing1 = auto()
    RightHandRing2 = auto()
    RightHandRing3 = auto()
    RightHandRing4 = auto()
    RightHandPinky2 = auto()
    RightHandPinky3 = auto()
    RightHandPinky4 = auto()
    
def avg_vec3(v1, v2):
    v3 = glm.vec3((v1.x + v2.x) * 0.5,
                  (v1.y + v2.y) * 0.5,
                  (v1.z + v2.z) * 0.5)
    return v3
def mp_landmark_to_vec3(mp_landmark):
    return glm.vec3(mp_landmark.x, mp_landmark.y, mp_landmark.z)

class MixamoData:
    def __init__(self, this, parent, mediapipe_landmark = None, func = None):
        self.this = this 
        self.parent = parent 
        self.mp_landmark = mediapipe_landmark
        self.to_mixamo = func

    def mp_to_mixamo(self, mp_landmark, mm_landmark, mm_visibility):
        if self.to_mixamo is None:
            return (mp_landmark_to_vec3(mp_landmark[self.mp_landmark]),
                    (mp_landmark[self.mp_landmark].visibility))

        return self.to_mixamo(mp_landmark, mm_landmark, mm_visibility)


def avg3_use_mp(landmark1, landmark2, mp_landmark, mm_landmark, mm_visibility):
    return (avg_vec3(mp_landmark[landmark1], mp_landmark[landmark2]),
            (mp_landmark[landmark1].visibility + mp_landmark[landmark2].visibility) * 0.5)

def avg3_use_mm(landmark1, landmark2, mp_landmark, mm_landmark, mm_visibility):
    return (avg_vec3(mm_landmark[landmark1], mm_landmark[landmark2]),
            (mm_visibility[landmark1] + mm_visibility[landmark2]) * 0.5)

def post_process_mixamo_landmark(mm_landmark):
    for point in mm_landmark:
            point.y = -point.y
            point.z = -point.z


# [Mixamo name, idx, parent_idx, mediapipe name]
def get_mixamo_pose_info():
    return {
        Mixamo.Hips :MixamoData(Mixamo.Hips, Mixamo.Root, None, partial(avg3_use_mp, MP_POSE_LANDMARK.LEFT_HIP, MP_POSE_LANDMARK.RIGHT_HIP)),  # left hip <->right hip
        Mixamo.Neck: MixamoData(Mixamo.Neck, Mixamo.Spine2, None, partial(avg3_use_mp, MP_POSE_LANDMARK.LEFT_SHOULDER, MP_POSE_LANDMARK.RIGHT_SHOULDER)),  # left_shoulder <-> right_shoulder
        Mixamo.Spine1: MixamoData(Mixamo.Spine1, Mixamo.Spine, None, partial(avg3_use_mm, Mixamo.Hips, Mixamo.Neck)),
        Mixamo.Spine: MixamoData(Mixamo.Spine, Mixamo.Hips, None, partial(avg3_use_mm, Mixamo.Hips, Mixamo.Spine1)),
        Mixamo.Spine2: MixamoData(Mixamo.Spine2, Mixamo.Spine1, None, partial(avg3_use_mm, Mixamo.Spine1, Mixamo.Neck)),
        Mixamo.Head: MixamoData(Mixamo.Head, Mixamo.Neck, None, partial(avg3_use_mp, MP_POSE_LANDMARK.LEFT_EAR, MP_POSE_LANDMARK.RIGHT_EAR)),  # left_ear <-> right_ear

        Mixamo.LeftArm :  MixamoData(Mixamo.LeftArm, Mixamo.Spine2, MP_POSE_LANDMARK.LEFT_SHOULDER),
        Mixamo.LeftForeArm :  MixamoData(Mixamo.LeftForeArm, Mixamo.LeftArm, MP_POSE_LANDMARK.LEFT_ELBOW),
        Mixamo.RightArm: MixamoData(Mixamo.RightArm, Mixamo.Spine2, MP_POSE_LANDMARK.RIGHT_SHOULDER),
        Mixamo.RightForeArm: MixamoData(Mixamo.RightForeArm, Mixamo.RightArm, MP_POSE_LANDMARK.RIGHT_ELBOW),
        Mixamo.LeftUpLeg: MixamoData(Mixamo.LeftUpLeg,  Mixamo.Hips, MP_POSE_LANDMARK.LEFT_HIP),
        Mixamo.LeftLeg: MixamoData(Mixamo.LeftLeg, Mixamo.LeftUpLeg, MP_POSE_LANDMARK.LEFT_KNEE),
        Mixamo.LeftFoot: MixamoData(Mixamo.LeftFoot, Mixamo.LeftLeg, MP_POSE_LANDMARK.LEFT_ANKLE),
        Mixamo.LeftToeBase: MixamoData(Mixamo.LeftToeBase, Mixamo.LeftFoot, MP_POSE_LANDMARK.LEFT_FOOT_INDEX),
        Mixamo.RightUpLeg: MixamoData(Mixamo.RightUpLeg, Mixamo.Hips, MP_POSE_LANDMARK.RIGHT_HIP),
        Mixamo.RightLeg: MixamoData(Mixamo.RightLeg, Mixamo.RightUpLeg, MP_POSE_LANDMARK.RIGHT_KNEE),
        Mixamo.RightFoot: MixamoData(Mixamo.RightFoot, Mixamo.RightLeg, MP_POSE_LANDMARK.RIGHT_ANKLE),
        Mixamo.RightToeBase: MixamoData(Mixamo.RightToeBase, Mixamo.RightFoot, MP_POSE_LANDMARK.RIGHT_FOOT_INDEX),

        Mixamo.LeftHand :  MixamoData(Mixamo.LeftHand, Mixamo.LeftForeArm, MP_POSE_LANDMARK.LEFT_WRIST),
        Mixamo.RightHand: MixamoData(Mixamo.RightHand, Mixamo.RightForeArm, MP_POSE_LANDMARK.RIGHT_WRIST),

        Mixamo.LeftHandThumb1 :  MixamoData(Mixamo.LeftHandThumb1, Mixamo.LeftHand, MP_POSE_LANDMARK.LEFT_THUMB),
        Mixamo.LeftHandIndex1 :  MixamoData(Mixamo.LeftHandIndex1, Mixamo.LeftHand, MP_POSE_LANDMARK.LEFT_INDEX),
        Mixamo.LeftHandPinky1 :  MixamoData(Mixamo.LeftHandPinky1, Mixamo.LeftHand, MP_POSE_LANDMARK.LEFT_PINKY),

        Mixamo.RightHandThumb1: MixamoData(Mixamo.RightHandThumb1, Mixamo.RightHand, MP_POSE_LANDMARK.RIGHT_THUMB),
        Mixamo.RightHandIndex1: MixamoData(Mixamo.RightHandIndex1, Mixamo.RightHand, MP_POSE_LANDMARK.RIGHT_INDEX),
        Mixamo.RightHandPinky1: MixamoData(Mixamo.RightHandPinky1, Mixamo.RightHand, MP_POSE_LANDMARK.RIGHT_PINKY),
    }

def get_mixamo_left_hand_info():
    return {
        Mixamo.LeftHandThumb1: MixamoData(Mixamo.LeftHandThumb1,  Mixamo.LeftHand, MP_HAND_LANDMARK.THUMB_CMC),
        Mixamo.LeftHandThumb2: MixamoData(Mixamo.LeftHandThumb2,  Mixamo.LeftHandThumb1, MP_HAND_LANDMARK.THUMB_MCP),
        Mixamo.LeftHandThumb3: MixamoData(Mixamo.LeftHandThumb3,  Mixamo.LeftHandThumb2, MP_HAND_LANDMARK.THUMB_IP),
        Mixamo.LeftHandThumb4: MixamoData(Mixamo.LeftHandThumb4,  Mixamo.LeftHandThumb3, MP_HAND_LANDMARK.THUMB_TIP),

        Mixamo.LeftHandIndex1: MixamoData(Mixamo.LeftHandIndex1,  Mixamo.LeftHand, MP_HAND_LANDMARK.INDEX_FINGER_MCP),
        Mixamo.LeftHandIndex2: MixamoData(Mixamo.LeftHandIndex2,  Mixamo.LeftHandIndex1, MP_HAND_LANDMARK.INDEX_FINGER_PIP),
        Mixamo.LeftHandIndex3: MixamoData(Mixamo.LeftHandIndex3,  Mixamo.LeftHandIndex2, MP_HAND_LANDMARK.INDEX_FINGER_DIP),
        Mixamo.LeftHandIndex4: MixamoData(Mixamo.LeftHandIndex4,  Mixamo.LeftHandIndex3, MP_HAND_LANDMARK.INDEX_FINGER_TIP),

        Mixamo.LeftHandMiddle1: MixamoData(Mixamo.LeftHandMiddle1,  Mixamo.LeftHand, MP_HAND_LANDMARK.MIDDLE_FINGER_MCP),
        Mixamo.LeftHandMiddle2: MixamoData(Mixamo.LeftHandMiddle2,  Mixamo.LeftHandMiddle1, MP_HAND_LANDMARK.MIDDLE_FINGER_PIP),
        Mixamo.LeftHandMiddle3: MixamoData(Mixamo.LeftHandMiddle3,  Mixamo.LeftHandMiddle2, MP_HAND_LANDMARK.MIDDLE_FINGER_DIP),
        Mixamo.LeftHandMiddle4: MixamoData(Mixamo.LeftHandMiddle4,  Mixamo.LeftHandMiddle3, MP_HAND_LANDMARK.MIDDLE_FINGER_TIP),

        Mixamo.LeftHandRing1: MixamoData(Mixamo.LeftHandRing1,  Mixamo.LeftHand, MP_HAND_LANDMARK.RING_FINGER_MCP),
        Mixamo.LeftHandRing2: MixamoData(Mixamo.LeftHandRing2,  Mixamo.LeftHandRing1, MP_HAND_LANDMARK.RING_FINGER_PIP),
        Mixamo.LeftHandRing3: MixamoData(Mixamo.LeftHandRing3,  Mixamo.LeftHandRing2, MP_HAND_LANDMARK.RING_FINGER_DIP),
        Mixamo.LeftHandRing4: MixamoData(Mixamo.LeftHandRing4,  Mixamo.LeftHandRing3, MP_HAND_LANDMARK.RING_FINGER_TIP),

        Mixamo.LeftHandPinky1: MixamoData(Mixamo.LeftHandPinky1,  Mixamo.LeftHand, MP_HAND_LANDMARK.PINKY_MCP),
        Mixamo.LeftHandPinky2: MixamoData(Mixamo.LeftHandPinky2,  Mixamo.LeftHandPinky1, MP_HAND_LANDMARK.PINKY_PIP),
        Mixamo.LeftHandPinky3: MixamoData(Mixamo.LeftHandPinky3,  Mixamo.LeftHandPinky2, MP_HAND_LANDMARK.PINKY_DIP),
        Mixamo.LeftHandPinky4: MixamoData(Mixamo.LeftHandPinky4,  Mixamo.LeftHandPinky3, MP_HAND_LANDMARK.PINKY_TIP),
    }

def get_mixamo_right_hand_info():
    return {
        Mixamo.RightHandThumb1: MixamoData(Mixamo.RightHandThumb1,  Mixamo.RightHand, MP_HAND_LANDMARK.THUMB_CMC),
        Mixamo.RightHandThumb2: MixamoData(Mixamo.RightHandThumb2,  Mixamo.RightHandThumb1, MP_HAND_LANDMARK.THUMB_MCP),
        Mixamo.RightHandThumb3: MixamoData(Mixamo.RightHandThumb3,  Mixamo.RightHandThumb2, MP_HAND_LANDMARK.THUMB_IP),
        Mixamo.RightHandThumb4: MixamoData(Mixamo.RightHandThumb4,  Mixamo.RightHandThumb3, MP_HAND_LANDMARK.THUMB_TIP),

        Mixamo.RightHandIndex1: MixamoData(Mixamo.RightHandIndex1,  Mixamo.RightHand, MP_HAND_LANDMARK.INDEX_FINGER_MCP),
        Mixamo.RightHandIndex2: MixamoData(Mixamo.RightHandIndex2,  Mixamo.RightHandIndex1, MP_HAND_LANDMARK.INDEX_FINGER_PIP),
        Mixamo.RightHandIndex3: MixamoData(Mixamo.RightHandIndex3,  Mixamo.RightHandIndex2, MP_HAND_LANDMARK.INDEX_FINGER_DIP),
        Mixamo.RightHandIndex4: MixamoData(Mixamo.RightHandIndex4,  Mixamo.RightHandIndex3, MP_HAND_LANDMARK.INDEX_FINGER_TIP),

        Mixamo.RightHandMiddle1: MixamoData(Mixamo.RightHandMiddle1,  Mixamo.RightHand, MP_HAND_LANDMARK.MIDDLE_FINGER_MCP),
        Mixamo.RightHandMiddle2: MixamoData(Mixamo.RightHandMiddle2,  Mixamo.RightHandMiddle1, MP_HAND_LANDMARK.MIDDLE_FINGER_PIP),
        Mixamo.RightHandMiddle3: MixamoData(Mixamo.RightHandMiddle3,  Mixamo.RightHandMiddle2, MP_HAND_LANDMARK.MIDDLE_FINGER_DIP),
        Mixamo.RightHandMiddle4: MixamoData(Mixamo.RightHandMiddle4,  Mixamo.RightHandMiddle3, MP_HAND_LANDMARK.MIDDLE_FINGER_TIP),

        Mixamo.RightHandRing1: MixamoData(Mixamo.RightHandRing1,  Mixamo.RightHand, MP_HAND_LANDMARK.RING_FINGER_MCP),
        Mixamo.RightHandRing2: MixamoData(Mixamo.RightHandRing2,  Mixamo.RightHandRing1, MP_HAND_LANDMARK.RING_FINGER_PIP),
        Mixamo.RightHandRing3: MixamoData(Mixamo.RightHandRing3,  Mixamo.RightHandRing2, MP_HAND_LANDMARK.RING_FINGER_DIP),
        Mixamo.RightHandRing4: MixamoData(Mixamo.RightHandRing4,  Mixamo.RightHandRing3, MP_HAND_LANDMARK.RING_FINGER_TIP),

        Mixamo.RightHandPinky1: MixamoData(Mixamo.RightHandPinky1,  Mixamo.RightHand, MP_HAND_LANDMARK.PINKY_MCP),
        Mixamo.RightHandPinky2: MixamoData(Mixamo.RightHandPinky2,  Mixamo.RightHandPinky1, MP_HAND_LANDMARK.PINKY_PIP),
        Mixamo.RightHandPinky3: MixamoData(Mixamo.RightHandPinky3,  Mixamo.RightHandPinky2, MP_HAND_LANDMARK.PINKY_DIP),
        Mixamo.RightHandPinky4: MixamoData(Mixamo.RightHandPinky4,  Mixamo.RightHandPinky3, MP_HAND_LANDMARK.PINKY_TIP),
    }

MIXAMO_POSE_DATA = get_mixamo_pose_info()
MIXAMO_LEFT_HAND_DATA = get_mixamo_left_hand_info()
MIXAMO_RIGHT_HAND_DATA = get_mixamo_right_hand_info()


