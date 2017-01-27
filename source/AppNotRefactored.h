/**
  \file App.h

  The G3D 10.00 default starter app is configured for OpenGL 4.1 and
  relatively recent GPUs.
 */
#pragma once
#include <G3D/G3DAll.h>
#include "Mesh.h""

/** \brief Application framework. */
class App : public GApp {
protected:
    shared_ptr<Mesh> m_myMesh;
    //variable sof edge collapsing   
    int m_edgesToCollapse;
    int m_landEdgesToCollapse;
    int m_mountainEdgesToCollapse;
    int m_oceanEdgesToCollapse;
    int m_angleLengthWeight;
    bool m_collapsingEnabled;

    //variables for makeHeightfield()
    float m_heightfieldYScale;
    float m_heightfieldXZScale;
    String m_heightfieldSource;

    int m_recursionLevel;
    float m_frequency;
    float m_landBevel;
    float m_mountainBevel;
    float m_mountainHeight;
    float m_mountianDiversity;
    float m_oceanLevel;
    String m_planetSource;
    String m_planetSave;
    float m_landNoise;
    float m_oceanNoise;
    bool m_waterMount;
    bool m_useWTexture;
    bool m_useLTexture;
    bool m_useMTexture;
    float m_lRed;
    float m_lGreen;
    float m_lBlue;
    float m_wRed;
    float m_wGreen;
    float m_wBlue;
    float m_mRed;
    float m_mGreen;
    float m_mBlue;
    String m_landFile;
    String m_waterFile;
    String m_mountainFile;
    float m_lGlossBase;
    float m_lGlossPow;
    float m_wGlossBase;
    float m_wGlossPow;
    float m_mGlossBase;
    float m_mGlossPow;
    

    // Options for Planet as a whole
    float m_scale;
    float m_xPos;
    float m_yPos;
    float m_zPos;
    String m_planetName;
    String m_orbitPlanet;
    float m_orbitDistance;
    Any m_scene;
    Any m_models;
    Any m_entities;

    /** Called from onInit */
    void makeGUI();
    void makeHeightfield();

    void makePentagon();
    void makeBunny(); 
    void makeLittleHeightfield();

    void addPlanetToScene(Any& scene, Any& models, Mesh& mesh, String name, Point3& position, Color3& color, Color4& gloss, String& dependentModel);
    void addPlanetToScene(Any& scene, Any& models, Mesh& mesh, String name, Point3& position, String filename, String& dependentModel);
    void addPlanetToScene(Any& scene, Any& models, Mesh& mesh, String name, Point3& position, String anyStr, int width, int height, String& dependentModel);
    void addCloudsToPlanet(Any& models, Any& entities, String& name, Point3& position);
    void makePlanetGUI();
    void addEntityToAnyTable(String& name, Any& any, Point3& position, String& model, String& dependentEntity, String& spec);
    void makeSceneTable(Any& scene, const Any& models, const Any& entities, const String& name);
    void createInitialEntityTable(Any& entities);
    void createInitialModelsTable(Any& models);
    void addPlanetToScene(Mesh& mesh, String name, Point3& position, Color3& color, Matrix3& rotation, Color4& gloss);

public:
    
    App(const GApp::Settings& settings = GApp::Settings());

    
    virtual void onInit() override;
    virtual void onAI() override;
    virtual void onNetwork() override;
    virtual void onSimulation(RealTime rdt, SimTime sdt, SimTime idt) override;
    virtual void onPose(Array<shared_ptr<Surface> >& posed3D, Array<shared_ptr<Surface2D> >& posed2D) override;

    // You can override onGraphics if you want more control over the rendering loop.
    // virtual void onGraphics(RenderDevice* rd, Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) override;

    virtual void onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& surface3D) override;
    virtual void onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& surface2D) override;

    virtual bool onEvent(const GEvent& e) override;
    virtual void onUserInput(UserInput* ui) override;
    virtual void onCleanup() override;

};
