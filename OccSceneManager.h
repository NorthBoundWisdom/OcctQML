#ifndef OCCSCENEMANAGER_H
#define OCCSCENEMANAGER_H

#include <map>
#include <string>
#include <vector>

#include <QKeyEvent>
#include <QMouseEvent>
#include <QObject>
#include <QWheelEvent>

#include <AIS_AnimationCamera.hxx>
#include <AIS_InteractiveContext.hxx>
#include <AIS_Shape.hxx>
#include <AIS_ViewCube.hxx>
#include <Standard_Handle.hxx>
#include <TopoDS_Shape.hxx>
#include <V3d_View.hxx>

namespace geotoys
{

class OccSceneManager : public QObject
{
    Q_OBJECT

public:
    explicit OccSceneManager(QObject *parent = nullptr);
    ~OccSceneManager() override;

    // Set OCCT context and view
    void setContext(const Handle(AIS_InteractiveContext) & context);
    void setView(const Handle(V3d_View) & view);

    // Geometry object management
    bool addShape(const std::string &id, const TopoDS_Shape &shape,
                  const Quantity_Color &color, bool display);
    bool removeShape(const std::string &id);
    bool updateShape(const std::string &id, const TopoDS_Shape &shape);
    bool setShapeColor(const std::string &id, const Quantity_Color &color);

    // Get geometry object
    Handle(AIS_Shape) getShape(const std::string &id) const;
    std::vector<std::string> getAllShapeIds() const;

    // Clear scene
    void clearAllShapes();

private:
    Handle(AIS_InteractiveContext) context_;
    Handle(V3d_View) view_;

    std::map<std::string, Handle(AIS_Shape)> shapes_;
    bool viewcube_visible_ = true;
    double device_pixel_ratio_ = 1.0;
};

} // namespace geotoys

#endif // OCCSCENEMANAGER_H
