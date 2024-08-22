# **Anim v0.1.0**

| Animation tool             | Video to Animation (only mixamorig character) |
| -------------------------- | --------------------------------------------- |
| ![](/screenshot/88912.gif) | ![](/screenshot/88910.gif)                    |



|realtime| webcam based video|
|-|-|
| ![](/screenshot/mocap.gif) |![webcam2](https://github.com/user-attachments/assets/0818c800-c2c4-4b11-a9c2-9655c3869fb8)|


| retarget before | retarget after |
|-|-|
|![retarget_before](https://user-images.githubusercontent.com/40836757/197576075-8bc69cde-bd08-4432-859b-db50927e7f5e.gif)|![retarget_aft](https://user-images.githubusercontent.com/40836757/197576153-f8660fdd-d3d4-4273-b5a3-9fb9b05b146d.gif)|

|import|export|
|-|-|
|![multiple](https://github.com/user-attachments/assets/5d84c4e9-d748-4c3c-8066-71fbc269d626) | ![export](https://github.com/user-attachments/assets/10f42e70-8d13-434b-b535-7b9ef23b25f2) |




## **Demo**

[![Video Label](http://img.youtube.com/vi/vaIjhVUIC5Q/0.jpg)](https://www.youtube.com/watch?v=vaIjhVUIC5Q)

## **Feature**

- Edit: FK animation
- Object selction
- Embeded python (only windows)
- Export glTF, FBX
- Mediapipe to Animation

## **TODO**

- Edit: IK
- Fix camera movement
- Ctrl + c, Ctrl + v
  - Keyframe
- Retargeting
- Mediapipe Pose -> Mediapipe Holistic (Hands)

## **Build**

see [build.md](/docs/build.md)

## **External**

### **C++**

- glad: https://glad.dav1d.de/

- GLFW: https://www.glfw.org/

- GLM: https://glm.g-truc.net/0.9.9/index.html

- Assimp: https://github.com/assimp/assimp

- ImGUI: https://github.com/ocornut/imgui

  - timeline: https://gitlab.com/GroGy/im-neo-sequencer
  - text editor: https://github.com/BalazsJako/ImGuiColorTextEdit
  - dialog: https://github.com/aiekick/ImGuiFileDialog
  - ImGuizmo: https://github.com/CedricGuillemet/ImGuizmo

- stb: https://github.com/nothings/stb

- jsoncpp: https://github.com/open-source-parsers/jsoncpp

- font(icon)

  - https://github.com/juliettef/IconFontCppHeaders
  - https://github.com/tanluteam/imgui

- cpython: https://github.com/python/cpython

- pybind11: https://github.com/pybind/pybind11

### **Python**

see scripts/requirements.txt

- mediapipe==0.8.10.1
- PyGLM==2.5.7
- opencv-contrib-python==4.6.0.66
- matplotlib==3.5.3

## **Reference**

- [learnopengl](https://learnopengl.com/)

- [skeleton animation: ogldev](https://ogldev.org/www/tutorial38/tutorial38.html)

- [blur shader](https://www.shadertoy.com/view/Xltfzj)

- grid shader

  - [grid shader 1](http://asliceofrendering.com/scene%20helper/2020/01/05/InfiniteGrid/)

  - [grid shader 2](https://github.com/martin-pr/possumwood/wiki/Infinite-ground-plane-using-GLSL-shaders)

  - [grid shader 3](https://madebyevan.com/shaders/grid)

## **Resources**

- model
  - https://www.mixamo.com/#/?page=1&type=Character
  - https://github.com/GDQuest/godot-3d-mannequin
- font
  - https://github.com/naver/d2codingfont/wiki/Open-Font-License
  - https://github.com/juliettef/IconFontCppHeaders
- skybox
  - https://www.cleanpng.com/free/skybox.html
  - https://opengameart.org/content/forest-skyboxes
  - https://www.humus.name/index.php?page=Textures

## **License**

- The source code(/src) is available under the Apache-2.0 license.
- model assets are CC-By 4.0. (resources/models/mannequiny.fbx)
  - https://github.com/GDQuest/godot-3d-mannequin
  - The Godot mannequin is a character made by Luciano Muñoz In blender 2.80
  - This project changed the model to mixamo rigging(auto rigging), diffuse color.
- d2codingfont: SIL OPEN FONT LICENSE
