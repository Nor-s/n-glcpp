import glm
import copy
import math

from .gizmo import Gizmo, calc_quat
from .mixamo_data import Mixamo


def find_2d_angle(cx, cy, ex, ey):
    dy = ey - cy
    dx = ex - cx
    theta = math.atan2(dy, dx)
    return theta


def json_to_glm_vec(json_3d):
    return glm.vec3(json_3d['x'], json_3d['y'], json_3d['z'])


def json_to_glm_quat(json_4d):
    return glm.quat(w=json_4d['w'], x=json_4d['x'], y=json_4d['y'], z=json_4d['z'])


def glm_vec3_to_json(vec):
    return {"x": vec.x, "y": vec.y, "z": vec.z}


def glm_quat_to_json(quat):
    return {"w": quat.w, "x": quat.x, "y": quat.y, "z": quat.z}


def check_quat_isnan(quat):
    if math.isnan(quat.x):
        quat.x = 0.0
    if math.isnan(quat.w):
        quat.w = 1.0
    if math.isnan(quat.y):
        quat.y = 0.0
    if math.isnan(quat.z):
        quat.z = 0.0


def check_vec3_isnan(vec, num=0.0):
    if math.isnan(vec.x):
        vec.x = num
    if math.isnan(vec.y):
        vec.y = num
    if math.isnan(vec.z):
        vec.z = num


def decompose(matrix: glm.mat4):
    scale = glm.vec3()
    rotation = glm.quat()
    translation = glm.vec3()
    skew = glm.vec3()
    perspective = glm.vec4()
    glm.decompose(matrix, scale, rotation, translation, skew, perspective)
    check_quat_isnan(rotation)
    check_vec3_isnan(scale, 1.0)
    check_vec3_isnan(translation, 0.0)
    return [translation, rotation, scale]


def calc_transform(position, rotation, scale):
    pos = glm.translate(glm.mat4(1.0), position)
    rot = pos * glm.mat4(rotation)
    return glm.scale(rot, scale)


def get_normal(triangle):
    a = triangle[0]
    b = triangle[1]
    c = triangle[2]
    return glm.normalize(glm.cross(b - a, c - a))

# http://renderdan.blogspot.com/2006/05/rotation-matrix-from-axis-vectors.html
def generate_matrix(tangent, normal, binormal):
    tan = -glm.vec4(tangent, 0.0)
    binorm = glm.vec4(binormal, 0.0)
    norm = -glm.vec4(normal, 0.0)

    return glm.mat4(tan,  binorm, norm, glm.vec4(0.0, 0.0, 0.0, 1.0))

def calc_hip_transform(mixamo_list, hips_node, left_leg_node, right_leg_node, spine_node):
    hip_gizmo = hips_node.get_gizmo()
    tleft_leg_local = hip_gizmo.get_local_pos(mixamo_list[Mixamo.LeftUpLeg])
    tright_leg_local = hip_gizmo.get_local_pos(mixamo_list[Mixamo.RightUpLeg])
    tspine_local = hip_gizmo.get_local_pos(mixamo_list[Mixamo.Spine])
    src_triangle = [tleft_leg_local, tright_leg_local, tspine_local]

    normal_vec = get_normal(src_triangle)
    tangent_vec = glm.normalize(tright_leg_local)
    binormal_vec = glm.normalize(glm.cross(normal_vec, tangent_vec))

    mat = generate_matrix(tangent_vec, normal_vec, binormal_vec)
    t, r, s = decompose(mat)
    return glm.mat4(r)


class ModelNode:
    def __init__(self, gizmo=Gizmo()):
        self.child = []
        self.gizmo = gizmo
        self.name = ""
        # bind pose
        self.position = glm.vec3(x=0.0, y=0.0, z=0.0)
        self.scale = glm.vec3(x=1.0, y=1.0, z=1.0)
        self.rotate = glm.quat(w=1.0, x=0.0, y=0.0, z=0.0)

        self.animation_transform = glm.mat4(1.0)
        self.global_transform = glm.mat4(1.0)

        self.prefix = ""
        
        self.idx = -1

    def find_node(self, name):
        if self.name == name:
            return [True, self]
        for child in self.child:
            is_find, node = child.find_node(name)
            if is_find:
                return [True, node]
        return [False, None]

    def set_mixamo(self, model_json, mixamo_idx_map, before_transform=None, parent_transform = glm.mat4(1.0)):
        idx = model_json["name"].find(":")
        self.prefix = model_json["name"][:idx+1]
        model_name = model_json["name"][idx+1:]
        self.name = model_name
        self.idx = mixamo_idx_map[model_name]

        self.position = json_to_glm_vec(model_json["position"])
        self.rotate = json_to_glm_quat(model_json["rotation"])
        self.scale = json_to_glm_vec(model_json["scale"])
        if before_transform != None:
            self.position, self.rotate, self.scale = decompose(before_transform*self.get_transform())
        if self.name == Mixamo.LeftUpLeg.name or self.name == Mixamo.RightUpLeg.name:
            self.position.y = 0
            self.position.z = 0
        self.global_transform = parent_transform * self.get_transform()

        # find child
        childlist = model_json["child"]
        transform_list = [None] * len(childlist)
        transform_list_idx = -1
        parent_transform_list = [self.global_transform] * len(childlist)

        for child in childlist:
            transform_list_idx += 1
            child_idx = child["name"].find(":")
            child_name = child["name"][child_idx+1:]
            if child_name in mixamo_idx_map:
                new_node = ModelNode()
                new_node.set_mixamo(child, mixamo_idx_map, copy.deepcopy(
                    transform_list[transform_list_idx]), parent_transform_list[transform_list_idx])
                self.child.append(new_node)
            else:
                for child_of_child in child["child"]:
                    childlist.append(child_of_child)
                    position = json_to_glm_vec(child["position"])
                    rotation = json_to_glm_quat(child["rotation"])
                    scale = json_to_glm_vec(child["scale"])
                    transform = calc_transform(position, rotation, scale)
                    transform_list.append(transform)
                    parent_transform_list.append(self.global_transform*transform)

    def normalize_spine(self, parent_node=None, parent_transform=glm.mat4(1.0)):
        if Mixamo.Spine.name in self.name or self.name == Mixamo.LeftArm.name or self.name == Mixamo.RightArm.name or self.name == Mixamo.Neck.name:
            self.position.z = 0  # local_pos.z


        for child in self.child:
            child.normalize_spine(
                parent_node=self, parent_transform=self.animation_transform*self.get_transform())

    def normalize(self, mixamo_list, len=0.0):
        if self.name == "Hips":
            self.position = mixamo_list[self.idx]
        else:
            n_position = glm.normalize(self.position)
            self.position = len*n_position

        for child in self.child:

            a = mixamo_list[self.idx]
            b = mixamo_list[child.idx]
            new_len = glm.distance(a, b)
            child.normalize(mixamo_list, new_len)

    def find_child(self, name):
        for child in self.child:
            if child.name == name:
                return child
        return None

    def calc_animation(self, mixamo_list,  parent_transform=glm.mat4(1.0), world_mixamo_adjust=glm.vec3(0.0, 0.0, 0.0)):
        self.animation_transform = glm.mat4(1.0)
        current_gizmo = self.get_gizmo(parent_transform)

        if self.name == Mixamo.Hips.name:
            self.animation_transform = calc_hip_transform(mixamo_list,
                                                    self,
                                                    self.find_child(Mixamo.LeftUpLeg.name),
                                                    self.find_child(Mixamo.RightUpLeg.name),
                                                    self.find_child(Mixamo.Spine.name))
        elif self.name == Mixamo.Spine2.name:
            neck = None
            for child in self.child:
                if child.name == Mixamo.Neck.name:
                    neck = child
            source_vec = neck.get_gizmo(parent_transform*self.get_transform()).get_origin()
            target_vec = world_mixamo_adjust + mixamo_list[neck.idx]
            self.animation_transform = current_gizmo.calc_rotation_matrix(source_vec, target_vec, is_abs=False)

        elif self.name == Mixamo.LeftHand.name or self.name == Mixamo.RightHand.name:
            source_vec = self.child[0].get_gizmo(parent_transform*self.get_transform()).get_origin()
            target_vec = world_mixamo_adjust + mixamo_list[self.child[0].idx]
            for i in range(1, len(self.child)):
                child_vec = self.child[i].get_gizmo(parent_transform*self.get_transform()).get_origin()
                source_vec = glm.mix(source_vec, child_vec, 0.5)
                target_vec = glm.mix(target_vec, world_mixamo_adjust + mixamo_list[self.child[i].idx], 0.5)
            self.animation_transform = current_gizmo.calc_rotation_matrix(source_vec, target_vec, is_abs=False)
        
        elif len(self.child) > 0:
            source_vec = self.child[0].get_gizmo(parent_transform*self.get_transform()).get_origin()
            target_vec = world_mixamo_adjust + mixamo_list[self.child[0].idx]
            is_abs = False
            if self.name == Mixamo.LeftArm.name or self.name == Mixamo.RightArm.name:
                is_abs = True
            self.animation_transform = current_gizmo.calc_rotation_matrix(source_vec, target_vec, is_abs=is_abs)
        
        self.global_transform = parent_transform*self.get_transform()*self.animation_transform

        for child in self.child:
            adjust_vec1 = child.get_gizmo(self.global_transform).get_origin()
            adjust_vec = adjust_vec1 - mixamo_list[child.idx]
            child.calc_animation(mixamo_list,
                                 self.global_transform,
                                 world_mixamo_adjust=adjust_vec)

    def get_transform(self):
        return calc_transform(self.position, self.rotate, self.scale)

    def get_gizmo(self, parent_transform=glm.mat4(1.0)):
            return self.gizmo.rotate(parent_transform*self.get_transform())

    def get_gizmo_apply_tmp(self, parent_transform=glm.mat4(1.0)):
        return self.gizmo.rotate(parent_transform*self.get_transform()*self.animation_transform)

    def get_vec_and_group_list(self, result_vec_list, result_group_list, parent_transform=glm.mat4(1.0),  group_list=None, is_apply_animation_transform=False):
        if is_apply_animation_transform:
            result_vec_list[self.idx] = self.get_gizmo_apply_tmp(parent_transform).get_origin()
        else:
            result_vec_list[self.idx] = self.get_gizmo(parent_transform).get_origin()
        if group_list == None:
            group_list = []
        group_list.append(self.idx)

        if len(self.child) == 0:
            result_group_list.append(copy.deepcopy(group_list))
            group_list.clear()
            return

        for child in self.child:
            if is_apply_animation_transform:
                child.get_vec_and_group_list(
                    result_vec_list, result_group_list, group_list=group_list, parent_transform=copy.deepcopy(parent_transform*self.get_transform() * self.animation_transform), is_apply_animation_transform=is_apply_animation_transform)
            else:
                child.get_vec_and_group_list(
                    result_vec_list, result_group_list, group_list=group_list, parent_transform=copy.deepcopy(parent_transform*self.get_transform()), is_apply_animation_transform=is_apply_animation_transform)

    def tmp_to_json(self, bones_json, visibility_list,  min_visibility=0.6):
        transform = decompose(self.animation_transform)
        visibility = visibility_list[self.idx]
        t = glm.vec3(0.0, 0.0, 0.0)
        r = transform[1]
        s = glm.vec3(1.0, 1.0, 1.0)

        if visibility >= min_visibility and (not (r.w == 1.0 and r.x == 0.0 and r.y == 0.0 and r.z == 0.0)):
            bone_json = {
                "name": self.prefix + self.name,
                "rotation": glm_quat_to_json(r),
                "position": glm_vec3_to_json(t),
                "scale": glm_vec3_to_json(s)
            }
            bones_json["bones"].append(bone_json)
        for child in self.child:
            child.tmp_to_json(bones_json, visibility_list, min_visibility)