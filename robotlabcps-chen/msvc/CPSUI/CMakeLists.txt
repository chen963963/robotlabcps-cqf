
set(SRC
	CPSHandler.cpp
	UIApplication.cpp
	UICfgParser.cpp
	UIHelpAbout.cpp
	UIHelpDoc.cpp
	UIImageView.cpp
	UISysSettings.cpp
	UIWindowManager.cpp
	UIAIRobot.cpp
	main.cpp
	)


add_executable(CPSUI ${SRC})

target_compile_options(CPSUI PRIVATE -finput-charset=GBK)

target_link_libraries(CPSUI cpsapi ui pthread GL rtde)
