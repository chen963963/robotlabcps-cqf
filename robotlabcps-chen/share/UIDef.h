#pragma once
#include <vector>
#include "Config.h"

#define UI_DATA_DIR		"data" // ���ݸ�Ŀ¼������Ϊ����Ŀ¼

//////////////////////////////////////////////////////////////////////////
// ��ͷ�ļ�����Ľṹ�岻��Ҫͨ��CPS���ͣ���˿���ʹ��STL����
//////////////////////////////////////////////////////////////////////////
typedef enum
{
	E_UI_POLL_MODE, // ����ģʽ
	E_UI_WAIT_MODE, // ƽ��ģʽ
	E_UI_WAIT_TIMEOUT_MODE // �̶�ģʽ
}E_UI_MODE;


typedef struct
{
	unsigned int	image_texture_id;
	int				image_width;
	int				image_height;
	int				channels;
	unsigned char*	image_data;
}ST_UITexture;

typedef struct
{
	std::vector<ST_UITexture>	vec_texture;
	std::vector<int>			vec_delay_ms;
}ST_GIF_TEXTURE;
