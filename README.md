# **n-glcpp**

opengl project

## TODO

-   애니메이션 재생/ 스탑 (5)

    -   몇몇 mixamo 모델에서 오류
        -   이유?
        -   바인딩포즈는 로딩되나, 애니메이션 쪽 문제

-   GUI 작업 (4)

    -   애니메이션 관련(본, 애니메이션 재생 등)

-   object 셀렉트 (1)

-   그리드 평면 (1)

## external

-   glad: https://glad.dav1d.de/

-   GLFW: https://www.glfw.org/

-   GLM: https://glm.g-truc.net/0.9.9/index.html

-   Assimp: https://github.com/assimp/assimp

-   ImGUI: https://github.com/ocornut/imgui

-   stb: https://github.com/nothings/stb

-   nfd(extended): https://github.com/btzy/nativefiledialog-extended

-   jsoncpp: https://github.com/open-source-parsers/jsoncpp

## screenshot

### 3/25

-   test model loading, pixelate, imgui

![](https://github.com/Nor-s/n-glcpp/blob/main/screenshot/Mar-25-2022%2012-58-15.gif?raw=true)

### 4/1

-   ImGui docking, refactoring, camera, skybox blur

![](<https://github.com/Nor-s/n-glcpp/blob/main/screenshot/Animation%20(32).gif?raw=true>)

-   3D model to png, pixelate

![](/screenshot/Apr-01-2022%2014-53-09.gif)

### 4/9

-   outline, outline color, animation

![](/screenshot/Apr-09-2022%2004-14-49.gif)

### 4/17

-   I studied how to move around with code. (I think this is an inefficient method)

    1. get `aiNode ::mTransformation`
    2. decompose this matrix
    3. update translation, rotation, scale

![](/screenshot/Apr_2022-04-17_12-45-38.png)

```cpp
            glm::mat4 nodeTransform = node->transformation;

            glm::vec3 scale;
            glm::quat rotation;
            glm::vec3 translation;
            glm::vec3 skew;
            glm::vec4 perspective;

            glm::decompose(nodeTransform, scale, rotation, translation, skew, perspective);

            if (nodeName == "RightUpLeg")
                rotation.x += 0.5;
            if (nodeName == "LeftUpLeg")
                rotation.x -= 0.5;
            if (nodeName == "RightArm")
                rotation.y += 0.5;
            if (nodeName == "LeftArm")
                rotation.y += 0.5;

            auto tt = glm::translate(glm::mat4(1.0f), translation);
            auto ss = glm::scale(glm::mat4(1.0f), scale);
            auto rr = glm::toMat4(rotation);

            nodeTransform = tt * rr * ss;
```

## references

-   [render: learnopengl](https://learnopengl.com/)

-   [texture to file](https://stackoverflow.com/questions/11863416/read-texture-bytes-with-glreadpixels)
-   [pixelate shader](https://github.com/genekogan/Processing-Shader-Examples/blob/master/TextureShaders/data/pixelate.glsl)

-   [skeleton animation: ogldev](https://ogldev.org/www/tutorial38/tutorial38.html)

-   [shader1](https://lettier.github.io/3d-game-shaders-for-beginners/)
-   [shader2](https://thebookofshaders.com/)
