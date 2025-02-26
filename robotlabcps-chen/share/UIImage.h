#pragma once
#include <memory>
#include "UIDef.h"
#ifdef UI_CFG_USE_OPENCV
#include <opencv2/opencv.hpp>
#endif

enum class UI_IMAGE_RESIZE_POLICY
{
	NORMAL,
	KEEP_ASPECT
};

// base class for UIImage
class UIImageBase
{
public:
	virtual bool LoadFromFile(const char * filename) = 0;
	virtual void Draw(int width = 0, int height = 0, UI_IMAGE_RESIZE_POLICY policy = UI_IMAGE_RESIZE_POLICY::NORMAL) = 0;

	virtual unsigned int GetTextureID() = 0;
	virtual int GetWidth() = 0;
	virtual int GetHeight() = 0;

	virtual void Reset() = 0;
	virtual bool IsValid() = 0;
};

// ��̬ͼƬ��ʾ��
class UIImage: public UIImageBase
{
public:
	UIImage();
	~UIImage();

	bool LoadFromFile(const char* filename) override;
#ifdef UI_CFG_USE_OPENCV
	/* 
	ֻ֧��3ͨ����4ͨ��ͼ��
	���ͨ����Ϊ3����һ��Ҫת��ΪRGB��ʽ�� cv::cvtColor(mat, mat, cv::COLOR_BGR2RGB);
	���ͨ����Ϊ4����һ��Ҫת��ΪRGBA��ʽ�� cv::cvtColor(mat, mat, cv::COLOR_BGRA2RGBA);
	Usage:
		cv::Mat mat = cv::imread("test.png", cv::IMREAD_UNCHANGED);
		int num = mat.channels();
		cv::cvtColor(mat, mat, cv::COLOR_BGRA2RGBA);
		bool ret = LoadFromCV(mat);
	*/
	bool LoadFromCV(const cv::Mat& mat);
#endif

	void Draw(int width = 0, int height = 0, UI_IMAGE_RESIZE_POLICY policy = UI_IMAGE_RESIZE_POLICY::NORMAL) override;
	int GetWidth() override;
	int GetHeight();
	unsigned int GetTextureID() override;

	void Reset() override;
	bool IsValid() override;
protected:
	ST_UITexture m_image_texture = { 0 };
};

// GIFͼƬ��ʾ��
class UIGIFImage: public UIImageBase
{
public:
	UIGIFImage();
	~UIGIFImage();

	bool LoadFromFile(const char* filename) override;
	void Draw(int width = 0, int height = 0, UI_IMAGE_RESIZE_POLICY policy = UI_IMAGE_RESIZE_POLICY::NORMAL) override;
	int GetWidth() override;
	int GetHeight();
	unsigned int GetTextureID() override;

	void Reset() override;
	bool IsValid() override;
protected:
	ST_GIF_TEXTURE m_gif_textures;
	size_t m_cur_index = 0;
	double m_last_index_t = 0.0;
};

using UIImageBasePtr = std::shared_ptr<UIImageBase>;
using UIImagePtr = std::shared_ptr<UIImage>;
using UIGIFImagePtr = std::shared_ptr<UIGIFImage>;

// UIImage������
class UIImageFactory
{
public:
	static UIImageBasePtr LoadFromFile(const char* filename);
};
