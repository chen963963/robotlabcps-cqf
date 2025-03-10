## 项目修改介绍
项目基于robotlabcps——chen原版完善了VisionServer模块的功能，集成了原有的视觉算法在Servon.py文件中。
CPSUI点击“拍照”，将拍照指令发送给CameraServer，然后将获取的图像存储到本地，将路径返回给CPSUI，再将路径和计算指令发送给VisionServer，
VisionServer模块得到CPSUI发送的当前照片路径和计算指令后，Servo计算预测出运动指令以及当前状态，以json格式数据的方式返回给CPSUI。
接收到json格式的msg后解析出对应的状态和待发送的六维向量指令，最终CPSUI发送运动指令给机械臂。

较于原版，新安装了nlohmann_json的相关库，便于在c++环境下解析CPSUI接收到的json格式数据。

## 项目结构
├── CPSUI
│   ├── ...
├── CameraServer
│   ├── CameraServer.py
│   ├── CameraServer.pyproj
│   ├── log
│   └── out
├── VisionServer
│   ├── Seg
│   ├── Servo.py
│   ├── VisionServer.py
│   ├── VisionServer.pyproj
│   ├── __pycache__
│   ├── dino-vit-features
│   ├── log
│   ├── my_utils
│   └── servo_log
├── CPSUI.sln
└── readme.md

## 项目使用
# 启用总线和UI界面
cd bin 
sh start_bus.sh
sh start.sh
# 启用Camera和Vision
cd msvc/CameraServer
python CameraServer.py
cd ..
cd VisionServer
python VisionServer.py

此时UI界面显示设备774和775已连接，点击“拍照”按钮即可启动
