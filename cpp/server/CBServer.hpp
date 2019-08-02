#ifndef CBServer_h
#define CBServer_h

#include <iostream>
#include <cbar/CBAR_Common.hpp>
#include <cbar/CBAR_Client.hpp>

using namespace cbar;

class CBServer : public cbar::CBAR_Client {
public:
    CBServer();
    ~CBServer();

    std::string executeJSON(const std::string &json);

    virtual ToolMode getToolMode();
    virtual void setToolMode(ToolMode toolMode);
    
    virtual double getBrushRadius();
    virtual void setBrushRadius(double brushSize);
    
    std::string getCurrentSceneID();
    void setCurrentSceneID(const std::string &sceneID);
private:
    struct Impl;
    std::unique_ptr<Impl> m_pImpl;
};


#endif
