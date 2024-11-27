/****************************************************************************
 * Copyright ©2017 Brian Curless.  All rights reserved.  Permission is hereby
 * granted to students registered for University of Washington CSE 457 or CSE
 * 557 for use solely during Autumn Quarter 2017 for purposes of the course.
 * No other use, copying, distribution, or modification is permitted without
 * prior written consent. Copyrights for third-party components of this work
 * must be honored.  Instructors interested in reusing these course materials
 * should contact the author.
 ****************************************************************************/
#include "hierarchyview.h"
#include <QApplication>
#include <QDropEvent>
#include <QAction>
#include <QHeaderView>

HierarchyView::HierarchyView() :
    QTreeWidget(),
    context_menu_(new QMenu(tr("Context Menu"), this)),
    scene_root_(nullptr)
{
    // QTreeWidget Settings
    setAcceptDrops(true);
    setDragEnabled(true);
    setDragDropMode(QAbstractItemView::InternalMove);
    setContextMenuPolicy(Qt::CustomContextMenu);

    setDragDropOverwriteMode(true);
    connect(this->model(), &QAbstractItemModel::dataChanged, this, [this]{
        for (auto item : this->object_widgets_) {
            if (item.second != nullptr) {
                item.second->setText(0,QString::fromStdString(item.second->GetSceneObject().GetName()));
            }
        }
    });

    // Actions that can manipulate the hierarchy
    QAction* focus_action = new QAction("Focus", this);
    focus_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    focus_action->setShortcut(Qt::Key_F);
    connect(focus_action, &QAction::triggered, this, [this]{
        QObjectWidgetItem* inspectable_item = GetSelectedItem();
        if (inspectable_item != nullptr) emit ObjectFocused(inspectable_item->GetSceneObject().GetUID());
    });

    QAction* dup_action = new QAction("Duplicate", this);
    dup_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    dup_action->setShortcut(Qt::CTRL + Qt::Key_D);
    connect(dup_action, &QAction::triggered, this, [this]{
        QObjectWidgetItem* inspectable_item = GetSelectedItem();
        if (inspectable_item != nullptr) emit ObjectDuplicated(inspectable_item->GetSceneObject().GetUID());
    });

    QAction* del_action = new QAction("Delete", this);
    del_action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
    QList<QKeySequence> del_shortcuts;
    del_shortcuts <<  Qt::Key_Backspace << QKeySequence::Delete;
    del_action->setShortcuts(del_shortcuts);
    connect(del_action, &QAction::triggered, this, [this]{
        QObjectWidgetItem* inspectable_item = GetSelectedItem();
        if (inspectable_item != nullptr) emit ObjectDeleted(inspectable_item->GetSceneObject().GetUID());
    });

    // Add the actions to both the context menu (right click menu), and the widget itself (for key shortcuts)
    context_menu_->addAction(focus_action);
    context_menu_->addAction(dup_action);
    context_menu_->addAction(del_action);
    addAction(focus_action);
    addAction(dup_action);
    addAction(del_action);

    // Show the context menu at the position requested only if there's an item selected
    connect(this, &QTreeWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
        if (selectedItems().size() > 0) context_menu_->exec(mapToGlobal(pos));
        else emit HierarchyContextOpen(mapToGlobal(pos));
    });

    // Whenever the tree widget detects a selection change, need to forward that appropriately
    QObject::connect(this, &QTreeWidget::itemSelectionChanged, this, &HierarchyView::OnChanged);

    // Handle renaming an object in the hierarchy view
    QObject::connect(this, &HierarchyView::itemChanged, this, [this](QTreeWidgetItem* treeitem, int) {
        QObjectWidgetItem* item = dynamic_cast<QObjectWidgetItem*>(treeitem);
        item->GetSceneObject().SetName(item->text(0).toStdString());
    });

    header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    header()->setStretchLastSection(false);
}

SceneObject* HierarchyView::GetSelectedSceneObject() {
    if (GetSelectedItem() == nullptr) return nullptr;
    return &(GetSelectedItem()->GetSceneObject());
}

void HierarchyView::SelectObject(const SceneObject& scene_object) {
    SelectObject(scene_object.GetUID());
}

void HierarchyView::SelectObject(uint64_t object_id) {
    auto previous = GetSelectedItem();
    if (previous && object_id == previous->GetSceneObject().GetUID()) return;
    if (object_widgets_.find(object_id) == object_widgets_.end()) {
        if (previous) previous->setSelected(false);
        return;
    }
    auto widget = object_widgets_[object_id];
    if (!widget) {
        if (previous) previous->setSelected(false);
        return;
    }
    if (widget->GetSceneObject().IsInternal()) return;
    if (previous) previous->setSelected(false);
    widget->setSelected(true);
    setFocus(); // Allows use of shortcuts
    if (widget->parent()) widget->parent()->setExpanded(true);
    parentWidget()->raise();
}

void HierarchyView::OnSceneNameChanged(const std::string& name) {
    setHeaderLabel(QString::fromStdString(name));
}

void HierarchyView::SetScene(Scene& scene) {
    // Update the state of the widget
    clear(); // Removes all items and selections
    object_widgets_.clear();
    OnSceneNameChanged(scene.GetName());
    scene_root_ = &scene.GetSceneRoot();

    // Build a tree out of the scene minus the root node
    auto children = scene.GetSceneRoot().GetChildren();
    for (auto& child_node : children) {
        BuildSubtree(*child_node, nullptr);
    }

    // Connect to the signals sent from the scene so we can update the view
    scene.NameChanged.Connect(this, &HierarchyView::OnSceneNameChanged);
    scene.SceneObjectCreated.Connect(this, &HierarchyView::OnObjectCreated);
    scene.SceneObjectDeleted.Connect(this, &HierarchyView::OnObjectDeleted);
    scene.SceneObjectHideToggled.Connect(this, &HierarchyView::OnObjectHideToggled);
    scene.NameChanged.Connect(this, &HierarchyView::OnSceneNameChanged);
}

void HierarchyView::OnObjectDuplicated(SceneObject& /*scene_object*/) {
    //Debug::Log.WriteLine("Duplicate: " + scene_object.GetName());
}

void HierarchyView::OnObjectDeleted(SceneObject& scene_object) {
    auto it = object_widgets_.find(scene_object.GetUID());
    if (it == object_widgets_.end()) return;
    auto widget = object_widgets_[scene_object.GetUID()];
    auto parent_widget = widget->parent();
    if (parent_widget) parent_widget->removeChild(widget);
    else invisibleRootItem()->removeChild(widget);
    object_widgets_.erase(it);
}

void HierarchyView::OnObjectCreated(SceneObject& scene_object) {
    // Get which parent it's supposed to have
    SceneObject* parent = scene_object.GetParent();
    // The only case which parent is null is if the scene_object is the hidden "root", so we can ignore it.
    if (parent == nullptr) return;
    uint64_t parent_id = parent->GetUID();
    BuildSubtree(scene_object, object_widgets_[parent_id]);
    emit ObjectCreated(scene_object.GetUID());
}

void HierarchyView::OnObjectHideToggled(SceneObject& scene_object) {
    uint64_t uid = scene_object.GetUID();
    object_widgets_[uid]->setHidden(scene_object.IsHidden() || scene_object.IsInternal());
    if (scene_object.IsInternal()) object_widgets_[uid]->SetInternal();
    for (auto child : scene_object.GetChildren()) {
        OnObjectHideToggled(*child);
    }
}

void HierarchyView::OnObjectParentChanged(SceneObject& scene_object, SceneObject& parent_object) {
    auto child_widget = object_widgets_[scene_object.GetUID()];
    auto new_parent_widget = object_widgets_[parent_object.GetUID()];
    auto old_parent_widget = child_widget->parent();

    if (old_parent_widget) {
        old_parent_widget->removeChild(child_widget);
    } else {
        invisibleRootItem()->removeChild(child_widget);
    }

    if (new_parent_widget) {
        new_parent_widget->addChild(child_widget);
    } else {
        invisibleRootItem()->addChild(child_widget);
    }

    OnObjectHideToggled(scene_object);
}

void HierarchyView::OnObjectPropertiesChanged() {
    // Trigger animator to refresh by selecting the currently selected object again
    QObjectWidgetItem* inspectable_item = GetSelectedItem();
    if (inspectable_item == nullptr) return;
    emit ObjectPropertiesChanged(inspectable_item->GetSceneObject().GetUID());
}

void HierarchyView::OnChanged() {
    QObjectWidgetItem* inspectable_item = GetSelectedItem();
    emit InspectableSelected(inspectable_item);
    if (inspectable_item) emit ObjectSelected(inspectable_item->GetSceneObject().GetUID());
    else emit ObjectSelected(0);
    emit RedrawRequested();
}

void HierarchyView::BuildSubtree(SceneObject& scene_node, QObjectWidgetItem* parent) {
    QObjectWidgetItem& root = CreateWidgetItem(scene_node, parent);
    // For each child create a widget as well
    auto children = scene_node.GetChildren();
    for (auto& child_node : children) {
        BuildSubtree(*child_node, &root);
    }
}

QObjectWidgetItem& HierarchyView::CreateWidgetItem(SceneObject& scene_object, QObjectWidgetItem* parent) {
    uint64_t uid = scene_object.GetUID();
    QObjectWidgetItem* node = new QObjectWidgetItem(scene_object);
    object_widgets_[uid] = node;
    node->RedrawRequested.Connect(this, &HierarchyView::RedrawRequested);
    node->AssetPickerRequested.Connect(this, &HierarchyView::AssetPickerRequested);
    node->ParentChanged.Connect(this, &HierarchyView::OnObjectParentChanged);
    node->PropertiesChanged.Connect(this, &HierarchyView::OnObjectPropertiesChanged);
    // If parent is null, add it as a top-level item
    if (parent == nullptr) addTopLevelItem(node);
    // Otherwise add it as a child of the parent
    else parent->addChild(node);
    node->setHidden(scene_object.IsHidden() || scene_object.IsInternal());
    if (scene_object.IsInternal()) node->SetInternal();
    emit InspectableAdded(*node);
    return *node;
}

QObjectWidgetItem *HierarchyView::GetSelectedItem() {
    // TODO: Handle Multi-Selection
    // For now, just use the first thing in the selection
    QList<QTreeWidgetItem*> selected = selectedItems();
    if (selected.size() == 0) return nullptr;
    // Safe to dynamic cast to QObjectWidgetItem since we only put those in
    return dynamic_cast<QObjectWidgetItem*>(selected[0]);
}

void HierarchyView::dropEvent(QDropEvent *event) {
    if (event->dropAction() != Qt::MoveAction) return;

    QList<QTreeWidgetItem*> drag_items = selectedItems();

    // Target is the new parent of our source item
    QObjectWidgetItem* target = dynamic_cast<QObjectWidgetItem*>(itemAt(event->pos()));
    auto position = dropIndicatorPosition();
    switch (position) {
        case QAbstractItemView::OnItem: {
            break; }
        case QAbstractItemView::AboveItem: {
            if (!itemAbove(target)) target = nullptr;
            else target = dynamic_cast<QObjectWidgetItem*>(itemAbove(target)->parent());
            break; }
        case QAbstractItemView::BelowItem: {
            if (!itemBelow(target)) target = nullptr;
            else target = dynamic_cast<QObjectWidgetItem*>(itemBelow(target)->parent());
            break; }
        case QAbstractItemView::OnViewport: {
            target = nullptr;
            break; }
    }

    // Handle the event on the UI side
    QTreeWidget::dropEvent(event);

    // If the target is null, our parent is the root node
    if (target != nullptr) {
        for (auto& tree_widget : drag_items) {
            QObjectWidgetItem* source = dynamic_cast<QObjectWidgetItem*>(tree_widget);
            assert(source != nullptr);
            //Debug::Log.WriteLine("Set Parent of " + source->GetSceneObject().GetName() + " to " + target->GetSceneObject().GetName());
            emit ObjectParentChanged(source->GetSceneObject().GetUID(), target->GetSceneObject().GetUID());
        }
    } else {
        for (auto& tree_widget : drag_items) {
            QObjectWidgetItem* source = dynamic_cast<QObjectWidgetItem*>(tree_widget);
            assert(source != nullptr);
            //Debug::Log.WriteLine("Set Parent of " + source->GetSceneObject().GetName() + " to Root");
            emit ObjectParentChanged(source->GetSceneObject().GetUID(), scene_root_->GetUID());
        }
    }

    emit RedrawRequested();
}

void HierarchyView::mousePressEvent(QMouseEvent* event) {
    QTreeWidgetItem* item = itemAt(event->pos());
    if (!item) clearSelection();
    QTreeWidget::mousePressEvent(event);
}
