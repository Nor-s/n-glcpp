from .mp_manager import MediapipeManager
from .mp_helper import mediapipe_to_mixamo as mp2mixamo
from .gizmo import g_is_abs
import redis
import json

manager = MediapipeManager()
manager.is_show_result = False

def set_mediapipe_status(is_angle_adjustment, model_complexity, min_detection_confidence, min_visibility, custom_fps, custom_factor, static_image_mode=False):
    g_is_abs = is_angle_adjustment
    manager.set_key(model_complexity, static_image_mode, min_detection_confidence)
    manager.min_visibility = min_visibility
    manager.fps = custom_fps
    manager.factor = custom_factor

def set_redis_status(redis_host, redis_port, redis_db=0):
    manager.redis = redis.Redis(host=redis_host, port=redis_port, db=redis_db)
 
def mediapipe_to_mixamo(model, video_path, output_path):
    _, animjson = mp2mixamo(manager, model, video_path)
    with open(output_path, 'w') as f:
        json.dump(animjson, f, indent=2)