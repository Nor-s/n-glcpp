from enum import auto, IntEnum
from mp_manager import POSE_LANDMARK as MP_LANDMARK
from functools import partial

import glm

class Mixamo(IntEnum):
    Root = -1
    Hips = 0
    Spine = auto()
    Spine1 = auto()
    Spine2 = auto()
    Neck = auto()
    Head = auto()
    LeftArm = auto()
    LeftForeArm = auto()
    LeftHand = auto()
    LeftHandThumb1 = auto()
    LeftHandIndex1 = auto()
    LeftHandPinky1 = auto()
    RightArm = auto()
    RightForeArm = auto()
    RightHand = auto()
    RightHandThumb1 = auto()
    RightHandIndex1 = auto()
    RightHandPinky1 = auto()
    LeftUpLeg = auto()
    LeftLeg = auto()
    LeftFoot = auto()
    LeftToeBase = auto()
    RightUpLeg = auto()
    RightLeg = auto()
    RightFoot = auto()
    RightToeBase = auto()
    
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
def get_mixamo_convert_info():
    return {
        Mixamo.Hips :MixamoData(Mixamo.Hips, Mixamo.Root, None, partial(avg3_use_mp, MP_LANDMARK.LEFT_HIP, MP_LANDMARK.RIGHT_HIP)),  # left hip <->right hip
        Mixamo.Neck: MixamoData(Mixamo.Neck, Mixamo.Spine2, None, partial(avg3_use_mp, MP_LANDMARK.LEFT_SHOULDER, MP_LANDMARK.RIGHT_SHOULDER)),  # left_shoulder <-> right_shoulder
        Mixamo.Spine1: MixamoData(Mixamo.Spine1, Mixamo.Spine, None, partial(avg3_use_mm, Mixamo.Hips, Mixamo.Neck)),
        Mixamo.Spine: MixamoData(Mixamo.Spine, Mixamo.Hips, None, partial(avg3_use_mm, Mixamo.Hips, Mixamo.Spine1)),
        Mixamo.Spine2: MixamoData(Mixamo.Spine2, Mixamo.Spine1, None, partial(avg3_use_mm, Mixamo.Spine1, Mixamo.Neck)),
        Mixamo.Head: MixamoData(Mixamo.Head, Mixamo.Neck, None, partial(avg3_use_mp, MP_LANDMARK.LEFT_EAR, MP_LANDMARK.RIGHT_EAR)),  # left_ear <-> right_ear
        Mixamo.LeftArm :  MixamoData(Mixamo.LeftArm, Mixamo.Spine2, MP_LANDMARK.LEFT_SHOULDER),
        Mixamo.LeftForeArm :  MixamoData(Mixamo.LeftForeArm, Mixamo.LeftArm, MP_LANDMARK.LEFT_ELBOW),
        Mixamo.LeftHand :  MixamoData(Mixamo.LeftHand, Mixamo.LeftForeArm, MP_LANDMARK.LEFT_WRIST),
        Mixamo.LeftHandThumb1 :  MixamoData(Mixamo.LeftHandThumb1, Mixamo.LeftHand, MP_LANDMARK.LEFT_THUMB),
        Mixamo.LeftHandIndex1 :  MixamoData(Mixamo.LeftHandIndex1, Mixamo.LeftHand, MP_LANDMARK.LEFT_INDEX),
        Mixamo.LeftHandPinky1 :  MixamoData(Mixamo.LeftHandPinky1, Mixamo.LeftHand, MP_LANDMARK.LEFT_PINKY),
        Mixamo.RightArm: MixamoData(Mixamo.RightArm, Mixamo.Spine2, MP_LANDMARK.RIGHT_SHOULDER),
        Mixamo.RightForeArm: MixamoData(Mixamo.RightForeArm, Mixamo.RightArm, MP_LANDMARK.RIGHT_ELBOW),
        Mixamo.RightHand: MixamoData(Mixamo.RightHand, Mixamo.RightForeArm, MP_LANDMARK.RIGHT_WRIST),
        Mixamo.RightHandThumb1: MixamoData(Mixamo.RightHandThumb1, Mixamo.RightHand, MP_LANDMARK.RIGHT_THUMB),
        Mixamo.RightHandIndex1: MixamoData(Mixamo.RightHandIndex1, Mixamo.RightHand, MP_LANDMARK.RIGHT_INDEX),
        Mixamo.RightHandPinky1: MixamoData(Mixamo.RightHandPinky1, Mixamo.RightHand, MP_LANDMARK.RIGHT_PINKY),
        Mixamo.LeftUpLeg: MixamoData(Mixamo.LeftUpLeg,  Mixamo.Hips, MP_LANDMARK.LEFT_HIP),
        Mixamo.LeftLeg: MixamoData(Mixamo.LeftLeg, Mixamo.LeftUpLeg, MP_LANDMARK.LEFT_KNEE),
        Mixamo.LeftFoot: MixamoData(Mixamo.LeftFoot, Mixamo.LeftLeg, MP_LANDMARK.LEFT_ANKLE),
        Mixamo.LeftToeBase: MixamoData(Mixamo.LeftToeBase, Mixamo.LeftFoot, MP_LANDMARK.LEFT_FOOT_INDEX),
        Mixamo.RightUpLeg: MixamoData(Mixamo.RightUpLeg, Mixamo.Hips, MP_LANDMARK.RIGHT_HIP),
        Mixamo.RightLeg: MixamoData(Mixamo.RightLeg, Mixamo.RightUpLeg, MP_LANDMARK.RIGHT_KNEE),
        Mixamo.RightFoot: MixamoData(Mixamo.RightFoot, Mixamo.RightLeg, MP_LANDMARK.RIGHT_ANKLE),
        Mixamo.RightToeBase: MixamoData(Mixamo.RightToeBase, Mixamo.RightFoot, MP_LANDMARK.RIGHT_FOOT_INDEX)
    }

MIXAMO_DATA = get_mixamo_convert_info()


    # Hips = auto()
    # Spine = auto()
    # Spine1 = auto()
    # Spine2 = auto()
    # Neck = auto()
    # Head = auto()
    # HeadTop_End = auto()
    # LeftShoulder = auto()
    # LeftArm = auto()
    # LeftForeArm = auto()
    # LeftHand = auto()
    # LeftHandThumb1 = auto()
    # LeftHandThumb2 = auto()
    # LeftHandThumb3 = auto()
    # LeftHandThumb4 = auto()
    # LeftHandIndex1 = auto()
    # LeftHandIndex2 = auto()
    # LeftHandIndex3 = auto()
    # LeftHandIndex4 = auto()
    # LeftHandMiddle1 =auto()
    # LeftHandMiddle2 = auto()
    # LeftHandMiddle3 = auto()
    # LeftHandMiddle4 = auto()
    # LeftHandRing1 = auto()
    # LeftHandRing2 = auto()
    # LeftHandRing3 = auto()
    # LeftHandRing4 = auto()
    # LeftHandPinky1 =auto()
    # LeftHandPinky2 = auto()
    # LeftHandPinky3 = auto()
    # LeftHandPinky4 = auto()
    # RightShoulder = auto()
    # RightArm = auto()
    # RightForeArm = auto()
    # RightHand = auto()
    # RightHandThumb1 = auto()
    # RightHandThumb2 = auto()
    # RightHandThumb3 = auto()
    # RightHandThumb4 = auto()
    # RightHandIndex1 = auto()
    # RightHandIndex2 = auto()
    # RightHandIndex3 = auto()
    # RightHandIndex4 = auto()
    # RightHandMiddle1 = auto()
    # RightHandMiddle2 = auto()
    # RightHandMiddle3 = auto()
    # RightHandMiddle4 = auto()
    # RightHandRing1 = auto()
    # RightHandRing2 = auto()
    # RightHandRing3 = auto()
    # RightHandRing4 = auto()
    # RightHandPinky1 = auto()
    # RightHandPinky2 = auto()
    # RightHandPinky3 = auto()
    # RightHandPinky4 = auto()
    # LeftUpLeg = auto()
    # LeftLeg = auto()
    # LeftFoot = auto()
    # LeftToeBase = auto()
    # LeftToe_End = auto()
    # RightUpLeg = auto()
    # RightLeg = auto()
    # RightFoot = auto()
    # RightToeBase = auto()
    # RightToe_End = auto()
