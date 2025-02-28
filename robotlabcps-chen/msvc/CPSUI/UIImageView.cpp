#include "UIImageView.h"
#include "UIImage.h"
#include "imgui/imgui.h"

UIImageView::UIImageView(UIGLWindow* main_win, const char* title) :UIBaseWindow(main_win, title)
{
}

UIImageView::~UIImageView()
{
}

void UIImageView::Draw()
{
	if (!m_show)
	{
		if (m_image)
		{
			m_image.reset();
		}
		return;
	}
	if (m_image && !m_image->IsValid())
	{
		return;
	}
	if (!ImGui::Begin(m_win_title, &m_show, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_HorizontalScrollbar))
	{
		ImGui::End();
		return;
	}
	ImGui::SliderFloat(u8"����", &m_image_zoom, 0.0f, 2.0f, "%.2f");

	if (m_image)
	{
		m_image->Draw(int(m_image->GetWidth() * m_image_zoom), int(m_image->GetHeight() * m_image_zoom));
	}

	ImGui::End();
}

void UIImageView::ShowImageFromFile(const char* filename)
{
	m_image = UIImageFactory::LoadFromFile(filename);
	this->Show();
}

void UIImageView::ShowImage(UIImageBasePtr image)
{
	m_image = image;
	this->Show();
}
