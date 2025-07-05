#include "OccSceneManager.h"

#include <iostream>

#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Graphic3d_TransformPers.hxx>
#include <Message.hxx>
#include <Quantity_Color.hxx>
#include <V3d_View.hxx>

namespace geotoys
{

OccSceneManager::OccSceneManager(QObject *parent)
    : QObject(parent)
    , viewcube_visible_(true)
{
}

OccSceneManager::~OccSceneManager()
{
    clearAllShapes();
}

void OccSceneManager::setContext(const Handle(AIS_InteractiveContext) & context)
{
    context_ = context;
}

void OccSceneManager::setView(const Handle(V3d_View) & view)
{
    view_ = view;
}

bool OccSceneManager::addShape(const std::string &id, const TopoDS_Shape &shape,
                               const Quantity_Color &color, bool display)
{
    if (context_.IsNull())
    {
        std::cerr << "Context is null, cannot add shape:" << id << std::endl;
        return false;
    }

    auto it = shapes_.find(id);
    if (it != shapes_.end())
    {
        removeShape(id);
    }

    Handle(AIS_Shape) aisShape = new AIS_Shape(shape);
    aisShape->SetColor(color);

    shapes_[id] = aisShape;

    if (display)
    {
        context_->Display(aisShape, AIS_Shaded, 0, false);
    }
    view_->Invalidate();
    view_->InvalidateImmediate();
    std::cout << "Added shape:" << id << std::endl;
    return true;
}

bool OccSceneManager::removeShape(const std::string &id)
{
    auto it = shapes_.find(id);
    if (it == shapes_.end())
    {
        return false;
    }

    Handle(AIS_Shape) shape = it->second;
    if (!context_.IsNull() && !shape.IsNull())
    {
        context_->Erase(shape, false);
        context_->Remove(shape, false);

        if (!view_.IsNull())
        {
            view_->Invalidate();
            view_->InvalidateImmediate();
        }
    }

    shapes_.erase(it);
    std::cout << "Removed shape:" << id << std::endl;
    return true;
}

bool OccSceneManager::updateShape(const std::string &id,
                                  const TopoDS_Shape &shape)
{
    auto it = shapes_.find(id);
    if (it == shapes_.end())
    {
        return addShape(id, shape, Quantity_NOC_YELLOW, true);
    }

    Handle(AIS_Shape) aisShape = it->second;
    if (!aisShape.IsNull())
    {
        aisShape->SetShape(shape);
        context_->Redisplay(aisShape, false);
        return true;
    }

    return false;
}

bool OccSceneManager::setShapeColor(const std::string &id,
                                    const Quantity_Color &color)
{
    auto it = shapes_.find(id);
    if (it == shapes_.end())
    {
        return false;
    }

    Handle(AIS_Shape) shape = it->second;
    if (!shape.IsNull())
    {
        shape->SetColor(color);
        if (!context_.IsNull())
        {
            context_->Update(shape, false);
        }
        return true;
    }

    return false;
}

Handle(AIS_Shape) OccSceneManager::getShape(const std::string &id) const
{
    auto it = shapes_.find(id);
    if (it != shapes_.end())
    {
        return it->second;
    }
    return Handle(AIS_Shape)();
}

std::vector<std::string> OccSceneManager::getAllShapeIds() const
{
    std::vector<std::string> ids;
    ids.reserve(shapes_.size());
    for (const auto &pair : shapes_)
    {
        ids.push_back(pair.first);
    }
    return ids;
}

void OccSceneManager::clearAllShapes()
{
    if (!context_.IsNull())
    {
        context_->RemoveAll(false);
        // 强制更新视图
        if (!view_.IsNull())
        {
            view_->InvalidateImmediate();
        }
    }
    shapes_.clear();
}
} // namespace geotoys
