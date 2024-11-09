import json
import cv2
import glm
import math
import matplotlib
import os

from .mp_manager import POSE_LANDMARK, HAND_LANDMARK, MediapipeModel
from .mixamo_data import Mixamo, MIXAMO_POSE_DATA, MIXAMO_LEFT_HAND_DATA, MIXAMO_RIGHT_HAND_DATA, post_process_mixamo_landmark, avg_vec3, mp_landmark_to_vec3
from .model_node import ModelNode, json_to_glm_vec, json_to_glm_quat, calc_transform

# from .one_euro_filter import OneEuroFilter
import copy
import numpy as np
import redis

def smooth_pose(one_euro_filter, pose, time):
    t = np.ones_like(pose) * time
    return one_euro_filter(t, pose)

def get_3d_len(left):
    return math.sqrt((left["x"])**2 + (left["y"])**2 + (left["z"])**2)


def set_axes(ax, azim=10, elev=10, xrange=1.0, yrange=1.0, zrange=1.0):
    ax.set_xlabel("Z")
    ax.set_ylabel("X")
    ax.set_zlabel("Y")
    ax.set_title('Vector')
    if xrange > 0.0:
        ax.set_xlim(-xrange, xrange)
        ax.set_ylim(-yrange, yrange)
        ax.set_zlim(-zrange, zrange)
    ax.view_init(elev=elev, azim=azim)


def get_dot(vec_list, group_lists):
    dots = []
    for group_list in group_lists:
        dot_group = {
            'x': [],
            'y': [],
            'z': []
        }
        for idx in group_list:
            dot_group['x'].append(vec_list[idx][2])
            dot_group['y'].append(vec_list[idx][0])
            dot_group['z'].append(vec_list[idx][1])
        dots.append(dot_group)
    return dots


def draw_list2(fig, vec_list=[], group_lists=[[]], azim=10, range=1.0):
    ax1 = matplotlib.pyplot.axes(projection='3d')
    set_axes(ax1, elev=10, azim=azim, xrange=range, yrange=range, zrange=range)
    dots = get_dot(vec_list, group_lists)
    for dot in dots:
        ax1.plot(dot['x'], dot['y'], dot['z'], marker='o')

    fig.canvas.draw()


def draw_list3(fig, vec_list=[],vec2_list=[], group_lists=[[]], azim=10, range=1.0):
    ax1 =  matplotlib.pyplot.axes(projection='3d') #fig.add_subplot(1, 2, 1, projection='3d') #matplotlib.pyplot.axes(projection='3d')
    set_axes(ax1, elev=10, azim=azim, xrange=range, yrange=range, zrange=range)
    dots = get_dot(vec_list, group_lists)
    for dot in dots:
        ax1.plot(dot['x'], dot['y'], dot['z'], marker='.')

    dots2 = get_dot(vec2_list, group_lists)
    for dot in dots2:
        ax1.plot(dot['x'], dot['y'], dot['z'], marker='+', color='r')

    # plt.show()
    fig.canvas.draw()


def find_bones(bones, name):
    for bone in bones:
        idx = bone["name"].find(":")
        bone_name = bone["name"][idx+1:]
        if bone_name == name:
            return bone
    return None

def find_model_json(model_json, name):
    idx = model_json["name"].find(":")
    model_name = model_json["name"][idx+1:]
    
    if  model_name == name:
        return [True, model_json]
    else:
        for child in model_json["child"]:
            is_find, result = find_model_json(child, name)
            if is_find:
                return [is_find, result]
        return [False, None]

def get_mixamo_name_idx_map():
    mixamo_name_idx_map = {}
    for data in Mixamo:
        mixamo_name_idx_map[data.name] = data.value 
    return mixamo_name_idx_map

def init_bindpose(bindpose_json, model_json):
    '''
    bindpose_json: output
    '''
    name = model_json["name"]
    position = json_to_glm_vec(model_json["position"])
    rotate = json_to_glm_quat(model_json["rotation"])
    scale = json_to_glm_vec(model_json["scale"])
    transform = np.array(calc_transform(position, rotate, scale))
    bindpose_json[name] = transform.flatten().tolist()
    childlist = model_json["child"]
    for child in childlist:
        init_bindpose(bindpose_json, child)

def mediapipe_to_mixamo(mp_manager,
                        mixamo_dict_string,
                        video_path):
    mm_name_idx_map = get_mixamo_name_idx_map()
    mixamo_json = None
    print(video_path)
    # with open(mixamo_dict_string) as f:
        # mixamo_json = json.load(f)
    mixamo_json = json.loads(mixamo_dict_string)
    is_find, hip_node = find_model_json(mixamo_json["node"], Mixamo.Hips.name)
    if not is_find:
        return [False, None]

    cap = cv2.VideoCapture(video_path)
    fps = cap.get(cv2.CAP_PROP_FPS)
    time_factor = 1.0
    if mp_manager.fps > 0:
        time_factor = mp_manager.fps/fps
        fps = mp_manager.fps

    anim_result_json = {
        "fileName": os.path.basename(video_path),
        "duration": 0,
        "width":  cap.get(cv2.CAP_PROP_FRAME_WIDTH),
        "height": cap.get(cv2.CAP_PROP_FRAME_HEIGHT),
        "ticksPerSecond": fps,
        "bindpose" : {
        },
        "frames": [
        ]
    }
    init_bindpose(anim_result_json["bindpose"], hip_node)

    try:
        root_node = ModelNode()
        root_node.set_mixamo(hip_node, mm_name_idx_map)
        root_node.normalize_spine()

        mediapipe_to_mixamo2(mp_manager,
                             anim_result_json,
                             cap,
                             mixamo_json,
                             root_node,
                             time_factor)
        anim_result_json["duration"] = anim_result_json["frames"][-1]["time"]

        print(f"duration: {anim_result_json['duration']}")

    except Exception as e:
        print(e)
        if cap.isOpened():
            cap.release()
        return [False, None]
    if cap.isOpened():
        cap.release()
    return [True, anim_result_json]


def mediapipe_to_mixamo2(mp_manager,
                         anim_result_json,
                         cap,
                         mixamo_bindingpose_json,
                         mixamo_bindingpose_root_node,
                         time_factor):
    # for hips move var
    _, model_right_up_leg = find_model_json(mixamo_bindingpose_json["node"], Mixamo.RightUpLeg.name)
    __, model_right_leg = find_model_json(mixamo_bindingpose_json["node"], Mixamo.RightLeg.name)

    model_scale = 100
    if _ != False:
        model_scale = get_3d_len(model_right_up_leg["position"])*2.0
    model_scale2 =1.0
    if __ !=False:
        model_scale2 = get_3d_len(model_right_leg["position"])
    hip_move_list = []
    origin = None
    factor = 0.0
    factor_list = []
    width = cap.get(cv2.CAP_PROP_FRAME_WIDTH)
    height = cap.get(cv2.CAP_PROP_FRAME_HEIGHT)

    frame_num = -1
    matplotlib.pyplot.ion()
    matplotlib.pyplot.close()
    fig = None
    if mp_manager.is_show_result:
        fig = matplotlib.pyplot.figure()
        matplotlib.pyplot.show()

    # one euro filter
    # one_euro_filter = None

    # init mediapipe
    try:
        max_frame_num = mp_manager.max_frame_num
        is_show_result = mp_manager.is_show_result
        min_visibility = mp_manager.min_visibility
        is_hips_move = mp_manager.is_hips_move
        while cap.isOpened():

            success, cap_image = cap.read()
            frame_num += 1
            if not success or max_frame_num < frame_num:
                break
            height1, width1, _ = cap_image.shape
            cap_image = cv2.resize(
                cap_image, (int(width1 * (640 / height1)), 640))
            height2, width2, _ = cap_image.shape
            height = height2
            width = width2
            cap_image, glm_list, visibility_list, hip2d_left, hip2d_right, leg2d = detect_pose_to_glm_pose(
                mp_manager, cap_image)
            
            if glm_list[0] != None:
                time =  math.floor(frame_num*time_factor)
                # if one_euro_filter == None:
                    # one_euro_filter = OneEuroFilter(np.zeros_like( np.array(glm_list)), np.array(glm_list))
                # else:
                    # glm_list = smooth_pose(one_euro_filter=one_euro_filter, pose = np.array(glm_list), time = frame_num)
                    
                bones_json = {
                    "time": time,
                    "bones": []
                }
                mixamo_bindingpose_root_node.normalize(glm_list, visibility_list)
                mixamo_bindingpose_root_node.calc_animation(glm_list, visibility_list=visibility_list)
                mixamo_bindingpose_root_node.tmp_to_json(bones_json, visibility_list, min_visibility)
                if mp_manager.redis != None:
                   bones_data = json.dumps(bones_json["bones"], ensure_ascii=False).encode('utf-8')
                   mp_manager.redis.set("animation", bones_data)
                anim_result_json["frames"].append(bones_json)
                if is_show_result:
                    rg = []
                    rv = [None] * len(glm_list)
                    mixamo_bindingpose_root_node.get_vec_and_group_list(
                        rv, rg, is_apply_animation_transform=True)
                    matplotlib.pyplot.clf()
                    draw_list2(fig, rv, rg)
                    # draw_list3(fig,rv, glm_list, rg)
                if is_hips_move:
                    hip2d_left.x *= width
                    hip2d_left.y *= height
                    hip2d_left.z *= width
                    hip2d_right.x *= width
                    hip2d_right.y *= height
                    hip2d_right.z *= width
                    leg2d.x *=width
                    leg2d.y *=height
                    leg2d.z *=width

                    if origin == None:
                        origin = avg_vec3(hip2d_left, hip2d_right)
                    else:
                        hips2d_scale = glm.distance(hip2d_left, hip2d_right)
                        leg2d_scale = glm.distance(leg2d, hip2d_right)
                        factor_list.append(model_scale2/leg2d_scale)
                        factor = max(factor, model_scale/hips2d_scale)
                        hip_move_list.append([len(anim_result_json["frames"]) -1, avg_vec3(hip2d_left, hip2d_right)])

            cv2.imshow('MediaPipe pose', cap_image)
            key = cv2.waitKey(5)
            if key & 0xFF == 27:
                break
        factor_list.sort()
        factor_list_avg = sum(factor_list)/len(factor_list)
        factor_list_avg = max(factor_list_avg, factor_list[int(len(factor_list)*0.8)])
        factor = max(factor,factor_list_avg)
        if mp_manager.factor != 0.0:
           factor = mp_manager.factor
        print("factor", factor)
        for hips_bone in hip_move_list:
            set_hips_position(find_bones(anim_result_json["frames"][hips_bone[0]]["bones"], Mixamo.Hips.name)["position"],
                              origin, 
                              hips_bone[1], 
                              factor)
        if anim_result_json["frames"][0]["time"] != 0.0:
            tmp_json = copy.deepcopy(anim_result_json["frames"][0])
            tmp_json["time"] = 0.0
            anim_result_json["frames"].append(tmp_json)
        
        cap.release()
        # matplotlib.pyplot.close(fig)
        cv2.destroyAllWindows()

    except Exception as e:
        print(e)
        # matplotlib.pyplot.close(fig)
        if cap.isOpened():
            cap.release()
            cv2.destroyAllWindows()


def detect_pose_to_glm_pose(mp_manager, image):
    # Create a copy of the input image.
    output_image = image.copy()

    image.flags.writeable = False

    # Convert the image from BGR into RGB format.
    image_rgb = cv2.cvtColor(image, cv2.COLOR_BGR2RGB)

    # Perform the Pose Detection.
    results = mp_manager.get_model().process(image_rgb)

    image.flags.writeable = True

    # Initialize a list to store the detected landmarks.
    glm_list = [glm.vec3()]*(len(Mixamo) - 1)
    visibility_list = [0]*(len(Mixamo) - 1)
    hip2d_left, hip2d_right = glm.vec3(0.0, 0.0, 0.0), glm.vec3(0.0, 0.0, 0.0)
    lefthand_landmarks = None
    righthand_landmarks = None
    b_is_holistic =  mp_manager.model == MediapipeModel.Holistic

    if hasattr(results, 'left_hand_landmarks'):
        lefthand = results.left_hand_landmarks
        if hasattr(lefthand, 'landmark'):
            lefthand_landmarks = lefthand.landmark

    if hasattr(results, 'right_hand_landmarks'):
        righthand = results.right_hand_landmarks
        if hasattr(righthand, 'landmark'):
            righthand_landmarks = righthand.landmark

    # 3d pose landmarks
    if results.pose_world_landmarks:
        landmark = results.pose_world_landmarks.landmark
        for key in MIXAMO_POSE_DATA:
            (point, visibility) = MIXAMO_POSE_DATA[key].mp_to_mixamo(landmark, glm_list, visibility_list)
            glm_list[key.value] = point
            visibility_list[key.value] = visibility

            if b_is_holistic and  key == Mixamo.RightHand:
                break
        

    # 2d pose landmarks
    leg2d = None
    if results.pose_landmarks:
        landmark = results.pose_landmarks.landmark
        hip2d_left.x = landmark[POSE_LANDMARK.LEFT_HIP].x
        hip2d_left.y = landmark[POSE_LANDMARK.LEFT_HIP].y
        hip2d_left.z = landmark[POSE_LANDMARK.LEFT_HIP].z
        hip2d_right = glm.vec3(landmark[POSE_LANDMARK.RIGHT_HIP].x,
                               landmark[POSE_LANDMARK.RIGHT_HIP].y, landmark[POSE_LANDMARK.RIGHT_HIP].z)
        leg2d = glm.vec3(landmark[26].x, landmark[26].y, landmark[26].z)
 
    if lefthand_landmarks != None:
        wrist_position =  mp_landmark_to_vec3(lefthand_landmarks[HAND_LANDMARK.WRIST])
        relative = glm_list[Mixamo.LeftHand] - wrist_position
        for key in MIXAMO_LEFT_HAND_DATA:
            (point, _) = MIXAMO_LEFT_HAND_DATA[key].mp_to_mixamo(lefthand_landmarks, glm_list, visibility_list)
            glm_list[key.value] = point + relative
            visibility_list[key.value] = 1.0       
        mp_manager.mp_drawing.draw_landmarks(
                  output_image, results.left_hand_landmarks, mp_manager.mp_holistic.HAND_CONNECTIONS) 

    if righthand_landmarks != None:
        wrist_position =  mp_landmark_to_vec3(righthand_landmarks[HAND_LANDMARK.WRIST])
        relative = glm_list[Mixamo.RightHand] - wrist_position
        for key in MIXAMO_RIGHT_HAND_DATA:
            (point, _) = MIXAMO_RIGHT_HAND_DATA[key].mp_to_mixamo(righthand_landmarks, glm_list, visibility_list)
            glm_list[key.value] = point + relative
            visibility_list[key.value] = 1.0
        mp_manager.mp_drawing.draw_landmarks(
                  output_image, results.right_hand_landmarks, mp_manager.mp_holistic.HAND_CONNECTIONS) 




    mp_manager.mp_drawing.draw_landmarks(image=output_image, landmark_list=results.pose_landmarks,
                                         connections=mp_manager.mp_pose.POSE_CONNECTIONS, landmark_drawing_spec=mp_manager.mp_drawing_styles.get_default_pose_landmarks_style())



    post_process_mixamo_landmark(glm_list)
    return output_image, glm_list, visibility_list, hip2d_left, hip2d_right, leg2d


def set_hips_position(hips_bone_json, origin_hips, current_hips, factor):
    x = (current_hips.x - origin_hips.x) * factor
    y = (current_hips.y - origin_hips.y) * factor
    z = (current_hips.z - origin_hips.z) * factor
    hips_bone_json["x"] = x 
    hips_bone_json["y"] = -y
    hips_bone_json["z"] = z