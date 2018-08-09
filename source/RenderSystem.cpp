#include "blueprint/RenderSystem.h"
#include "blueprint/Node.h"
#include "blueprint/Pins.h"
#include "blueprint/NodeLayout.h"

#include <SM_Rect.h>
#include <painting2/PrimitiveDraw.h>
#include <painting2/RenderSystem.h>

namespace
{

const float PINS_RADIUS = bp::NodeLayout::PINS_RADIUS;

const pt2::Color COL_PANEL_BG = pt2::Color(25, 25, 25);

const pt2::Color COL_TEXT     = pt2::Color(224, 224, 224);
const pt2::Color COL_WHITE    = pt2::Color(0, 0, 0);

// pins color
const pt2::Color COL_PINS_PORT      = pt2::Color(255, 255, 255);
const pt2::Color COL_PINS_BOOLEAN   = pt2::Color(140, 0, 0);
const pt2::Color COL_PINS_INTEGER   = pt2::Color(79, 225, 174);
const pt2::Color COL_PINS_FLOAT     = pt2::Color(168, 255, 81);
const pt2::Color COL_PINS_STRING    = pt2::Color(241, 0, 205);
const pt2::Color COL_PINS_TEXT      = pt2::Color(221, 119, 164);
const pt2::Color COL_PINS_VECTOR    = pt2::Color(247, 199, 45);
const pt2::Color COL_PINS_ROTATOR   = pt2::Color(160, 175, 250);
const pt2::Color COL_PINS_TRANSFORM = pt2::Color(243, 111, 0);
const pt2::Color COL_PINS_OBJECT    = pt2::Color(56, 165, 241);

const float TEXT_TITLE_SCALE = 0.7f;
const float TEXT_PINS_SCALE  = 0.5f;

const float PINS_TEXT_OFFSET = 35;

}

namespace bp
{

CU_SINGLETON_DEFINITION(RenderSystem);

RenderSystem::RenderSystem()
{
	m_title_tb.font_size = 24;
	m_title_tb.align_hori = pt2::Textbox::HA_CENTER;
	m_title_tb.width  = 200;
	m_title_tb.height = NodeLayout::DEFAULT_HEIGHT;

	m_input_tb.font_size = 24;
	m_input_tb.align_hori = pt2::Textbox::HA_LEFT;
	m_input_tb.height = NodeLayout::DEFAULT_HEIGHT;

	m_output_tb.font_size = 24;
	m_output_tb.align_hori = pt2::Textbox::HA_RIGHT;
	m_output_tb.height = NodeLayout::DEFAULT_HEIGHT;
}

void RenderSystem::DrawNode(const node::Node& node, const sm::Matrix2D& mat)
{
	// layout
	auto& style = node.GetStyle();
	float hw = style.width  * 0.5f;
	float hh = style.height * 0.5f;

	// panel
	DrawPanel(node, mat * sm::vec2(0, 0), hw, hh);

	// input
	float y = hh - NodeLayout::DEFAULT_HEIGHT;
	float curr_y = y;
	for (auto& in : node.GetAllInput())
	{
		auto pos = mat * sm::vec2(-hw + PINS_RADIUS * 2, curr_y - NodeLayout::DEFAULT_HEIGHT * 0.5f);
		DrawPins(*in, pos, true);
		curr_y -= NodeLayout::DEFAULT_HEIGHT;
	}

	// output
	curr_y = y;
	for (auto& out : node.GetAllOutput())
	{
		auto pos = mat * sm::vec2(hw - PINS_RADIUS * 2, curr_y - NodeLayout::DEFAULT_HEIGHT * 0.5f);
		DrawPins(*out, pos, false);
		curr_y -= NodeLayout::DEFAULT_HEIGHT;
	}

	y += NodeLayout::DEFAULT_HEIGHT * style.line_num;
}

float RenderSystem::GetTextTitleScale() const
{
	return TEXT_TITLE_SCALE;
}

float RenderSystem::GetTextPinsScale() const
{
	return TEXT_PINS_SCALE;
}

pt2::Color RenderSystem::GetPinsColor(const node::PinsType& type)
{
	pt2::Color col = COL_WHITE;
	switch (type)
	{
	case node::PINS_PORT:
		col = COL_PINS_PORT;
		break;
	case node::PINS_BOOLEAN:
		col = COL_PINS_BOOLEAN;
		break;
	case node::PINS_INTEGER:
		col = COL_PINS_INTEGER;
		break;
	case node::PINS_FLOAT:
		col = COL_PINS_FLOAT;
		break;
	case node::PINS_STRING:
		col = COL_PINS_STRING;
		break;
	case node::PINS_TEXT:
		col = COL_PINS_TEXT;
		break;
	case node::PINS_VECTOR:
		col = COL_PINS_VECTOR;
		break;
	case node::PINS_ROTATOR:
		col = COL_PINS_ROTATOR;
		break;
	case node::PINS_TRANSFORM:
		col = COL_PINS_TRANSFORM;
		break;
	case node::PINS_OBJECT:
		col = COL_PINS_OBJECT;
		break;
	}
	return col;
}

void RenderSystem::DrawPanel(const node::Node& node, const sm::vec2& pos, float hw, float hh)
{
	// background
	sm::rect r(sm::vec2(-hw, -hh) + pos, sm::vec2(hw, hh) + pos);
	pt2::PrimitiveDraw::SetColor(COL_PANEL_BG);
	pt2::PrimitiveDraw::Rect(nullptr, r, true);

	// title
	sm::Matrix2D mat;
	mat.Scale(TEXT_TITLE_SCALE, TEXT_TITLE_SCALE);
	mat.Translate(pos.x, pos.y + hh - NodeLayout::TITLE_HEIGHT * 0.5f);
	pt2::RenderSystem::DrawText(node.GetTitle(), m_title_tb, mat, COL_TEXT, COL_WHITE);
}

void RenderSystem::DrawPins(const node::Pins& pins, const sm::vec2& pos, bool left)
{
	sm::Matrix2D mat;
	mat.Scale(TEXT_PINS_SCALE, TEXT_PINS_SCALE);
	mat.Translate(pos.x, pos.y);

	bool connected = pins.GetConnected() != nullptr;

	auto type = pins.GetType();
	if (type == node::PINS_PORT)
	{
		std::vector<sm::vec2> vertices = {
			sm::vec2(-PINS_RADIUS, PINS_RADIUS),
			sm::vec2(-PINS_RADIUS, -PINS_RADIUS),
			sm::vec2(PINS_RADIUS, 0),
		};
		for (auto& v : vertices) {
			v += pos;
		}
		pt2::PrimitiveDraw::SetColor(COL_PINS_PORT);
		if (connected) {
			pt2::PrimitiveDraw::Triangles(nullptr, vertices);
		} else {
			pt2::PrimitiveDraw::LineWidth(2);
			pt2::PrimitiveDraw::Polyline(nullptr, vertices, true);
		}
	}
	else if (type >= node::PINS_BOOLEAN && type <= node::PINS_OBJECT)
	{
		pt2::PrimitiveDraw::SetColor(GetPinsColor(type));
		if (connected) {
			pt2::PrimitiveDraw::Circle(nullptr, pos, PINS_RADIUS, true);
		} else {
			pt2::PrimitiveDraw::LineWidth(2);
			pt2::PrimitiveDraw::Circle(nullptr, pos, PINS_RADIUS, false);
		}
	}

	if (left) {
		mat.Translate(PINS_TEXT_OFFSET, 0);
		pt2::RenderSystem::DrawText(pins.GetName(), m_input_tb, mat, COL_TEXT, COL_WHITE);
	} else {
		mat.Translate(-PINS_TEXT_OFFSET, 0);
		pt2::RenderSystem::DrawText(pins.GetName(), m_output_tb, mat, COL_TEXT, COL_WHITE);
	}
}

}