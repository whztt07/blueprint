#include "blueprint/NodeSelectOP.h"
#include "blueprint/CompNode.h"
#include "blueprint/Connecting.h"
#include "blueprint/node/SetReference.h"
#include "blueprint/node/GetReference.h"
#include "blueprint/node/SetValue.h"
#include "blueprint/node/GetValue.h"
#include "blueprint/node/Function.h"

#include <ee0/WxStagePage.h>
#include <ee0/MsgHelper.h>

#include <node0/SceneNode.h>

#include <queue>

namespace bp
{

NodeSelectOP::NodeSelectOP(const std::shared_ptr<pt0::Camera>& camera,
                           ee0::WxStagePage& stage, uint32_t cam_cfg)
    : ee2::NodeSelectOP(camera, ECS_WORLD_VAR stage, cam_cfg)
{
}


bool NodeSelectOP::OnMouseLeftDClick(int x, int y)
{
    if (ee2::NodeSelectOP::OnMouseLeftDClick(x, y)) {
        return true;
    }

    if (m_stage.GetSelection().Size() != 1) {
        return false;
    }

    n0::SceneNodePtr selected = nullptr;
    NodePtr selected_bp = nullptr;
    m_stage.GetSelection().Traverse([&](const ee0::GameObjWithPos& nwp)->bool
    {
        selected = nwp.GetNode();
        assert(selected->HasUniqueComp<CompNode>());
        selected_bp = selected->GetUniqueComp<CompNode>().GetNode();
        return true;
    });

    assert(selected && selected_bp);
    if (selected_bp->get_type() == rttr::type::get<node::Function>())
    {
        auto func = std::static_pointer_cast<node::Function>(selected_bp);
        ee0::MsgHelper::CreateNewPage(*m_stage.GetSubjectMgr(), PAGE_TYPE,
            func->GetFilepath().c_str(), selected);
    }
    else
    {
        bool successor = !wxGetKeyState(WXK_CONTROL);
        SelectAllTree(selected_bp, successor);
    }

    return false;
}

void NodeSelectOP::AfterInsertSelected(const n0::SceneNodePtr& node) const
{
    if (!node->HasUniqueComp<CompNode>()) {
        return;
    }

    auto bp_node = node->GetUniqueComp<CompNode>().GetNode();
    auto bp_type = bp_node->get_type();
    if (bp_type == rttr::type::get<node::SetReference>())
    {
        auto& name = std::static_pointer_cast<node::SetReference>(bp_node)->GetName();
        ChangeReferenceHighlight(false, name, true);
    }
    else if (bp_type == rttr::type::get<node::GetReference>())
    {
        auto& name = std::static_pointer_cast<node::GetReference>(bp_node)->GetName();
        ChangeReferenceHighlight(true, name, true);
    } 
    else if (bp_type == rttr::type::get<node::SetValue>())
    {
        auto& name = std::static_pointer_cast<node::SetValue>(bp_node)->GetName();
        ChangeValueHighlight(false, name, true);
    }
    else if (bp_type == rttr::type::get<node::GetValue>())
    {
        auto& name = std::static_pointer_cast<node::GetValue>(bp_node)->GetName();
        ChangeValueHighlight(true, name, true);
    }
}

void NodeSelectOP::AfterDeleteSelected(const n0::SceneNodePtr& node) const
{
    if (!node->HasUniqueComp<CompNode>()) {
        return;
    }

    auto bp_node = node->GetUniqueComp<CompNode>().GetNode();
    auto bp_type = bp_node->get_type();
    if (bp_type == rttr::type::get<node::SetReference>())
    {
        auto& name = std::static_pointer_cast<node::SetReference>(bp_node)->GetName();
        ChangeReferenceHighlight(false, name, false);
    }
    else if (bp_type == rttr::type::get<node::GetReference>())
    {
        auto& name = std::static_pointer_cast<node::GetReference>(bp_node)->GetName();
        ChangeReferenceHighlight(true, name, false);
    }
    else if (bp_type == rttr::type::get<node::SetValue>())
    {
        auto& name = std::static_pointer_cast<node::SetValue>(bp_node)->GetName();
        ChangeValueHighlight(false, name, false);
    }
    else if (bp_type == rttr::type::get<node::GetValue>())
    {
        auto& name = std::static_pointer_cast<node::GetValue>(bp_node)->GetName();
        ChangeValueHighlight(true, name, false);
    }
}

void NodeSelectOP::AfterClearSelection() const
{
    ClearVarHighlight();
}

void NodeSelectOP::ChangeReferenceHighlight(bool is_set_var, const std::string& name, bool set_highlight) const
{
    if (name.empty()) {
        return;
    }

    m_stage.Traverse([&](const ee0::GameObj& obj)->bool
    {
        if (!obj->HasUniqueComp<CompNode>()) {
            return true;
        }
        auto& cnode = obj->GetUniqueComp<CompNode>();
        auto node = cnode.GetNode();
        auto type = node->get_type();
        if (is_set_var && type == rttr::type::get<node::SetReference>() &&
            std::static_pointer_cast<node::SetReference>(node)->GetName() == name) {
            SetNodeBGColor(node, set_highlight ? BG_COLOR_HIGHLIGHT : BG_COLOR_DEFAULT);
        } else if (!is_set_var && type == rttr::type::get<node::GetReference>() &&
            std::static_pointer_cast<node::GetReference>(node)->GetName() == name) {
            SetNodeBGColor(node, set_highlight ? BG_COLOR_HIGHLIGHT : BG_COLOR_DEFAULT);
        }
        return true;
    });
}

void NodeSelectOP::ChangeValueHighlight(bool is_set_var, const std::string& name, bool set_highlight) const
{
    if (name.empty()) {
        return;
    }

    m_stage.Traverse([&](const ee0::GameObj& obj)->bool
    {
        if (!obj->HasUniqueComp<CompNode>()) {
            return true;
        }
        auto& cnode = obj->GetUniqueComp<CompNode>();
        auto node = cnode.GetNode();
        auto type = node->get_type();
        if (is_set_var && type == rttr::type::get<node::SetValue>() &&
            std::static_pointer_cast<node::SetValue>(node)->GetName() == name) {
            SetNodeBGColor(node, set_highlight ? BG_COLOR_HIGHLIGHT : BG_COLOR_DEFAULT);
        } else if (!is_set_var && type == rttr::type::get<node::GetValue>() &&
            std::static_pointer_cast<node::GetValue>(node)->GetName() == name) {
            SetNodeBGColor(node, set_highlight ? BG_COLOR_HIGHLIGHT : BG_COLOR_DEFAULT);
        }
        return true;
    });
}

void NodeSelectOP::ClearVarHighlight() const
{
    m_stage.Traverse([&](const ee0::GameObj& obj)->bool
    {
        if (!obj->HasUniqueComp<CompNode>()) {
            return true;
        }
        auto& cnode = obj->GetUniqueComp<CompNode>();
        auto node = cnode.GetNode();
        auto type = node->get_type();
        if (type == rttr::type::get<node::SetReference>() ||
            type == rttr::type::get<node::GetReference>() ||
            type == rttr::type::get<node::SetValue>() ||
            type == rttr::type::get<node::GetValue>()) {
            SetNodeBGColor(node, BG_COLOR_DEFAULT);
        }
        return true;
    });
}


void NodeSelectOP::SelectAllTree(const NodePtr& root, bool successor) const
{
    std::set<const Node*> sel_bp_nodes;

    std::queue<const Node*> buf;
    buf.push(root.get());
    if (successor)
    {
        while (!buf.empty())
        {
            auto n = buf.front(); buf.pop();
            sel_bp_nodes.insert(n);

            for (auto& c : n->GetAllOutput()) {
                for (auto& conn : c->GetConnecting()) {
                    buf.push(&conn->GetTo()->GetParent());
                }
            }
        }
    }
    else
    {
        while (!buf.empty())
        {
            auto n = buf.front(); buf.pop();
            sel_bp_nodes.insert(n);

            for (auto& c : n->GetAllInput()) {
                for (auto& conn : c->GetConnecting()) {
                    buf.push(&conn->GetFrom()->GetParent());
                }
            }
        }
    }

    if (sel_bp_nodes.empty()) {
        return;
    }

    std::vector<n0::SceneNodePtr> sel_nodes;
    m_stage.Traverse([&](const ee0::GameObj& obj)->bool
    {
        if (obj->HasUniqueComp<CompNode>())
        {
            auto& bp_node = obj->GetUniqueComp<CompNode>().GetNode();
            if (sel_bp_nodes.find(bp_node.get()) != sel_bp_nodes.end()) {
                sel_nodes.push_back(obj);
            }
        }
        return true;
    });
    auto sub_mgr = m_stage.GetSubjectMgr();

    std::vector<n0::NodeWithPos> nwps;
    nwps.reserve(sel_nodes.size());
    for (auto& obj : sel_nodes) {
        nwps.push_back(n0::NodeWithPos(obj, obj, 0));
    }
    ee0::MsgHelper::InsertSelection(*sub_mgr, nwps);
}

void NodeSelectOP::SetNodeBGColor(const NodePtr& node, const pt0::Color& color)
{
    auto st = node->GetStyle();
    st.panel_bg_col = color;
    node->SetStyle(st);
}

}