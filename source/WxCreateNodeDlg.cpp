#include "blueprint/WxCreateNodeDlg.h"
#include "blueprint/Node.h"
#include "blueprint/Pin.h"

#include <wx/treectrl.h>
#include <wx/sizer.h>

namespace bp
{

WxCreateNodeDlg::WxCreateNodeDlg(wxWindow* parent, const wxPoint& pos,
                                 const std::shared_ptr<Pin>& pair,
	                             const std::vector<NodePtr>& nodes)
	: wxDialog(parent, wxID_ANY, "Create Node", pos, wxSize(200, 400))
	, m_pair(pair)
	, m_nodes(nodes)
{
	InitLayout();
}

std::string WxCreateNodeDlg::GetSelectedType() const
{
	return m_tree->GetItemText(m_tree->GetSelection()).ToStdString();
}

void WxCreateNodeDlg::InitLayout()
{
	wxSizer* top_sizer = new wxBoxSizer(wxVERTICAL);
	{
		m_tree = new wxTreeCtrl(this, wxID_ANY, wxDefaultPosition, wxDefaultSize,
			wxTR_HIDE_ROOT | /*wxTR_NO_LINES | */wxTR_DEFAULT_STYLE);
		Bind(wxEVT_TREE_SEL_CHANGED, &WxCreateNodeDlg::OnSelChanged, this, m_tree->GetId());
		Bind(wxEVT_TREE_ITEM_ACTIVATED, &WxCreateNodeDlg::OnDoubleClick, this, m_tree->GetId());

		auto root = m_tree->AddRoot("ROOT");

		int idx = 0;
		for (auto& node : m_nodes) {
			if (IsNodeMatched(*node)) {
				auto type_name = node->get_type().get_name().to_string();
				m_tree->InsertItem(root, idx++, type_name);
			}
		}

		top_sizer->Add(m_tree, 1, wxEXPAND);
	}
	SetSizer(top_sizer);
//	top_sizer->Fit(this);
}

void WxCreateNodeDlg::OnSelChanged(wxTreeEvent& event)
{
	//assert(IsModal());
	//EndModal(wxID_OK);
}

void WxCreateNodeDlg::OnDoubleClick(wxTreeEvent& event)
{
	assert(IsModal());
	EndModal(wxID_OK);
}

bool WxCreateNodeDlg::IsNodeMatched(const Node& node) const
{
    if (!m_pair) {
        return true;
    }

	if (m_pair->IsInput())
	{
		auto& output = node.GetAllOutput();
		for (auto& pin : output) {
			if (pin->CanTypeCast(*m_pair)) {
				return true;
			}
		}
	} else {
		auto& input = node.GetAllInput();
		for (auto& pin : input) {
			if (pin->CanTypeCast(*m_pair)) {
				return true;
			}
		}
	}
	return false;
}

}