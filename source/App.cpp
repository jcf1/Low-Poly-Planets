/** \file App.cpp */
#include "App.h"
#include "PathTracer.h"
#include "NoiseGen.h"
#include "Planet.h"
#include "Mesh.h"
#include "SimpleMesh.h"
#include "SolarSystem.h"

// Tells C++ to invoke command-line main() function even on OS X and Win32.
G3D_START_AT_MAIN();

int main(int argc, const char* argv[]) {
    {
        G3DSpecification g3dSpec;
        g3dSpec.audio = false;
        initGLG3D(g3dSpec);
    }

    GApp::Settings settings(argc, argv);

    // Change the window and other startup parameters by modifying the
    // settings class.  For example:
    settings.window.caption = argv[0];

    // Set enable to catch more OpenGL errors
    // settings.window.debugContext     = true;

    // Some common resolutions:
    // settings.window.width            =  854; settings.window.height       = 480;
    // settings.window.width            = 1024; settings.window.height       = 768;
    settings.window.width = 1280; settings.window.height = 720;
    //settings.window.width             = 1920; settings.window.height       = 1080;
    // settings.window.width            = OSWindow::primaryDisplayWindowSize().x; settings.window.height = OSWindow::primaryDisplayWindowSize().y;
    settings.window.fullScreen = false;
    settings.window.resizable = !settings.window.fullScreen;
    settings.window.framed = !settings.window.fullScreen;

    // Set to true for a significant performance boost if your app can't render at 60fps, or if
    // you *want* to render faster than the display.
    settings.window.asynchronous = false;

    settings.hdrFramebuffer.depthGuardBandThickness = Vector2int16(64, 64);
    settings.hdrFramebuffer.colorGuardBandThickness = Vector2int16(0, 0);
    settings.dataDir = FileSystem::currentDirectory();
    settings.screenshotDirectory = "../journal/";

    settings.renderer.deferredShading = true;
    settings.renderer.orderIndependentTransparency = false;

    return App(settings).run();
}


App::App(const GApp::Settings& settings) : GApp(settings) {
}

// Called before the application loop begins.  Load data here and
// not in the constructor so that common exceptions will be
// automatically caught.
void App::onInit() {
    m_solarSystem = SolarSystem();
    m_solarSystem.onInit();

    debugPrintf("Target frame rate = %f Hz\n", realTimeTargetDuration());
    GApp::onInit();
    setFrameDuration(1.0f / 120.0f);

    // Call setScene(shared_ptr<Scene>()) or setScene(MyScene::create()) to replace
    // the default scene here.

    showRenderingStats = false;

    makeGUI();
    m_gbufferSpecification.encoding[GBuffer::Field::EMISSIVE].format = ImageFormat::RGB16F();

    // For higher-quality screenshots:
    // developerWindow->videoRecordDialog->setScreenShotFormat("PNG");
    // developerWindow->videoRecordDialog->setCaptureGui(false);
    developerWindow->cameraControlWindow->moveTo(Point2(developerWindow->cameraControlWindow->rect().x0(), 0));
    loadScene(
        //"G3D Sponza"
        "Ground" // Load something simple
        //developerWindow->sceneEditorWindow->selectedSceneName()  // Load the first scene encountered 
    );
}

void App::makePentagon() {
    //Array<Vector3> vertices1(Vector3(0, 0, -2), Vector3(0, 2, -2), Vector3(-2, 0, -2), Vector3(-1, -2, -2), Vector3(1, -2, -2), Vector3(2, 0, -2));
    //Array<Vector3int32> indices1(Vector3int32(0, 1, 2), Vector3int32(0, 2, 3), Vector3int32(0, 3, 4), Vector3int32(0, 4, 5), Vector3int32(0, 5, 1));
    //m_myMesh = Mesh::create(vertices1, indices1);

    Array<Vector3> vertices = Array<Vector3>();
    Array<Vector3int32> faces = Array<Vector3int32>();
    Planet planet;
    planet.writeSphere("pentagon", 12.0f, 5, vertices, faces);
    m_myMesh = Mesh::create(vertices, faces);
    m_myMesh->toObj("pentagon", 1, 1);

    loadScene("Pentagon");
    GuiPane* pentPane = debugPane->addPane("Info", GuiTheme::ORNATE_PANE_STYLE);

    // Example of how to add debugging controls
    pentPane->addLabel("Pentagon - Collapse Edges");
    pentPane->addNumberBox("# edges", &m_edgesToCollapse, "",
        GuiTheme::LINEAR_SLIDER, 0, 100000)->setUnitsSize(1);

    pentPane->addNumberBox("Angle/Length weight\n", &m_angleLengthWeight, "",
        GuiTheme::LINEAR_SLIDER, 0, 1)->setUnitsSize(0.1);

    pentPane->addButton("Collapse!", [this]() {
        m_myMesh->collapseEdges(m_edgesToCollapse, m_angleLengthWeight);
        m_myMesh->bevelEdges(0.2);
        scene()->typedEntity<VisibleEntity>("Pentagon")->setModel(m_myMesh->toArticulatedModel("pentagon", Color3(.5, .7, .2)));
    });

    pentPane->addButton("Reset", [this]() {
        //Array<Vector3> vertices1(Vector3(0, 0, -2), Vector3(0, 2, -2), Vector3(-2, 0, -2), Vector3(-1, -2, -2), Vector3(1, -2, -2), Vector3(2, 0, -2));
        //Array<Vector3int32> indices1(Vector3int32(0, 1, 2), Vector3int32(0, 2, 3), Vector3int32(0, 3, 4), Vector3int32(0, 4, 5), Vector3int32(0, 5, 1));
        //m_myMesh = Mesh::create(vertices1, indices1);

        Array<Vector3> vertices = Array<Vector3>();
        Array<Vector3int32> faces = Array<Vector3int32>();
        Planet planet;
        planet.writeSphere("pentagon", 12.0f, 5, vertices, faces);
        m_myMesh = Mesh::create(vertices, faces);

        m_myMesh->toObj("pentagon", 1, 1);
        G3D::ArticulatedModel::clearCache();
        loadScene("Pentagon");

    });

    pentPane->pack();
}

void App::makeBunny() {
    m_myMesh = Mesh::create("bunny.ifs");
    m_myMesh->toObj("bunny", 1, 1);

    loadScene("Bunny");
    GuiPane* bunnyPane = debugPane->addPane("Collapse Edges", GuiTheme::ORNATE_PANE_STYLE);

    // Example of how to add debugging controls
    bunnyPane->addLabel("Collapse Edges");

    bunnyPane->addNumberBox("# edges", &m_edgesToCollapse, "",
        GuiTheme::LINEAR_SLIDER, 0, 20000)->setUnitsSize(1);

    bunnyPane->addButton("Collapse!", [this]() {
        m_myMesh->collapseEdges(m_edgesToCollapse);
        m_myMesh->toObj("bunny", 1, 1);
        G3D::ArticulatedModel::clearCache();
        loadScene("Bunny");

    });

    bunnyPane->addButton("Reset", [this]() {
        m_myMesh = Mesh::create("bunny.ifs");
        m_myMesh->toObj("bunny", 1, 1);
        G3D::ArticulatedModel::clearCache();
        loadScene("Little Heightfield");

    });

    bunnyPane->pack();
}

void App::makeLittleHeightfield() {
    Array<Vector3> vertices;
    Array<Vector3int32> indices;

    shared_ptr<Image>base(Image::fromFile("heightmap.png"));
    for (int x = 0; x < base->width(); ++x) {
        int i(x*base->height());

        for (int z = 0; z < base->height(); ++z) {
            Color3 color;
            base->get(Point2int32(x, z), color);
            float y(color.average());
            vertices.append(Vector3(x, y * 5, z));

            if (x < base->width() - 1 && z < base->height() - 1) {
                int c(i + z);
                int h(base->height());
                indices.append(Vector3int32(c, c + 1, c + h), Vector3int32(c + 1, c + h + 1, c + h));
            }
        }
    }

    m_myMesh = Mesh::create(vertices, indices);
    m_myMesh->toObj("littleHf", 1, 1);

    loadScene("Little Heightfield");
    GuiPane* hfPane = debugPane->addPane("Collapse Edges", GuiTheme::ORNATE_PANE_STYLE);

    // Example of how to add debugging controls
    hfPane->addLabel("Collapse Edges");

    hfPane->addNumberBox("# edges", &m_edgesToCollapse, "",
        GuiTheme::LINEAR_SLIDER, 0, 1000)->setUnitsSize(1);

    hfPane->addButton("Collapse!", [this]() {
        m_myMesh->collapseEdges(m_edgesToCollapse);
        m_myMesh->toObj("littleHf", 1, 1);
        G3D::ArticulatedModel::clearCache();
        loadScene("Little Heightfield");

    });

    hfPane->addButton("Reset", [this]() {
        Array<Vector3> vertices;
        Array<Vector3int32> indices;

        shared_ptr<Image>base(Image::fromFile("heightmap.png"));
        for (int x = 0; x < base->width(); ++x) {
            int i(x*base->height());

            for (int z = 0; z < base->height(); ++z) {
                Color3 color;
                base->get(Point2int32(x, z), color);
                float y(color.average());
                vertices.append(Vector3(x, y * 5, z));

                if (x < base->width() - 1 && z < base->height() - 1) {
                    int c(i + z);
                    int h(base->height());
                    indices.append(Vector3int32(c, c + 1, c + h), Vector3int32(c + 1, c + h + 1, c + h));
                }
            }
        }
        m_myMesh = Mesh::create(vertices, indices);
        m_myMesh->toObj("littleHf", 1, 1);
        G3D::ArticulatedModel::clearCache();
        loadScene("Little Heightfield");

    });

    hfPane->pack();
}

void App::makePlanetGUI() {

    GuiPane* planetPane = debugPane->addPane("Planet Options");

    planetPane->setNewChildSize(240);

    GuiTabPane* planetTab = planetPane->addTabPane();

    GuiPane* planPane = planetTab->addTab("Options");

    planPane->addNumberBox("Recursion", &m_recursionLevel, "",
        GuiTheme::LINEAR_SLIDER, 1, 8)->setUnitsSize(1);

    planPane->addNumberBox("# of Trees", &m_numberOfTrees, "",
        GuiTheme::LINEAR_SLIDER, 0, 5000)->setUnitsSize(1);

    planPane->addNumberBox("# of Clouds", &m_numberOfClouds, "",
        GuiTheme::LINEAR_SLIDER, 0, 100)->setUnitsSize(1);

    planPane->addNumberBox("# of Birds", &m_numberOfBirds, "",
        GuiTheme::LINEAR_SLIDER, 0, 100)->setUnitsSize(1);

    planPane->addNumberBox("scale", &m_scale, "",
        GuiTheme::LINEAR_SLIDER, 0.1f, 1.0f)->setUnitsSize(0.01f);

    planPane->addNumberBox("x position", &m_xPos, "",
        GuiTheme::LOG_SLIDER, -100.0f, 100.0f)->setUnitsSize(0.01f);

    planPane->addNumberBox("y position", &m_yPos, "",
        GuiTheme::LOG_SLIDER, -100.0f, 100.0f)->setUnitsSize(0.01f);

    planPane->addNumberBox("z position", &m_zPos, "",
        GuiTheme::LOG_SLIDER, -100.0f, 100.0f)->setUnitsSize(0.01f);

    planPane->addTextBox("Planet Name:", &m_planetName);

    planPane->addTextBox("Orbit Planet:", &m_orbitPlanet);

    planPane->addNumberBox("Orbit Distance", &m_orbitDistance, "",
        GuiTheme::LINEAR_SLIDER, 50.0f, 500.0f)->setUnitsSize(0.01f);

    planPane->addCheckBox("Particle Clouds", &m_useParticleClouds);
    planPane->addCheckBox("Dragon?", &m_hasDragon);

    GuiPane* plan2Pane = planetTab->addTab("Noise Options");

    GuiPane* oceanPane = plan2Pane->addPane("Ocean Options");

    oceanPane->addNumberBox("Ocean Level", &m_oceanLevel, "",
        GuiTheme::LOG_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    oceanPane->addNumberBox("Ocean Noise", &m_oceanNoise, "",
        GuiTheme::LOG_SLIDER, 0.0f, 10.0f)->setUnitsSize(1);

    oceanPane->addNumberBox("Ocean Bevel", &m_oceanBevel, "",
        GuiTheme::LOG_SLIDER, 0.0001f, 1.0f)->setUnitsSize(1);

    GuiPane* landPane = plan2Pane->addPane("Land Options");

    landPane->addNumberBox("Land Noise", &m_landNoise, "",
        GuiTheme::LOG_SLIDER, -1.0f, 1.0f)->setUnitsSize(1);

    landPane->addNumberBox("Land Bevel", &m_landBevel, "",
        GuiTheme::LOG_SLIDER, 0.0001f, 1.0f)->setUnitsSize(1);

    GuiPane* mountainPane = plan2Pane->addPane("Mountain Options");

    mountainPane->addNumberBox("Mount Height", &m_mountainHeight, "",
        GuiTheme::LOG_SLIDER, 10.0f, 500.0f)->setUnitsSize(1);

    mountainPane->addNumberBox("Mount Diversity", &m_mountianDiversity, "",
        GuiTheme::LOG_SLIDER, 1.0f, 5.0f)->setUnitsSize(1);

    mountainPane->addNumberBox("Mount Bevel", &m_mountainBevel, "",
        GuiTheme::LOG_SLIDER, 0.0001f, 1.0f)->setUnitsSize(1);

    mountainPane->addNumberBox("Mount Noise 1", &m_mountianNoise1, "",
        GuiTheme::LOG_SLIDER, 0.0f, 5.0f)->setUnitsSize(1);

    mountainPane->addNumberBox("Mount Noise 2", &m_mountianNoise2, "",
        GuiTheme::LOG_SLIDER, 0.0f, 5.0f)->setUnitsSize(1);

    mountainPane->addNumberBox("Mount Noise 3", &m_mountianNoise3, "",
        GuiTheme::LOG_SLIDER, 0.0f, 5.0f)->setUnitsSize(1);

    mountainPane->addCheckBox("Water Mountains", &m_waterMount);

    GuiPane* colorPane = planetTab->addTab("Color Options");

    GuiPane* landColorPane = colorPane->addPane("Land Options");
    landColorPane->beginRow();
    landColorPane->addNumberBox("Land R", &m_lRed, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    landColorPane->addNumberBox("Land G", &m_lGreen, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    landColorPane->addNumberBox("Land B", &m_lBlue, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    landColorPane->endRow();
    landColorPane->beginRow();
    landColorPane->addNumberBox("Gloss Base", &m_lGlossBase, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    landColorPane->addNumberBox("Gloss Pow", &m_lGlossPow, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);
    landColorPane->endRow();
    landColorPane->addCheckBox("Get texture from file", &m_useLTexture);

    /*landColorPane->addButton("", [this]() {
        FileDialog::getFilename(m_landFile, "", false);
    });*/

    landColorPane->addTextBox("", &m_landFile);

    GuiPane* waterColorPane = colorPane->addPane("Water Options");
    waterColorPane->beginRow();

    waterColorPane->addNumberBox("Water R", &m_wRed, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    waterColorPane->addNumberBox("Water G", &m_wGreen, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    waterColorPane->addNumberBox("Water B", &m_wBlue, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    waterColorPane->endRow();
    waterColorPane->beginRow();
    waterColorPane->addNumberBox("Gloss Base", &m_wGlossBase, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    waterColorPane->addNumberBox("Gloss Pow", &m_wGlossPow, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);
    waterColorPane->endRow();
    waterColorPane->addCheckBox("Get texture from file", &m_useWTexture);

    /*waterColorPane->addButton("", [this]() {
        FileDialog::getFilename(m_waterFile, "", false);
    });*/

    waterColorPane->addTextBox("", &m_waterFile);

    GuiPane* mountainColorPane = colorPane->addPane("Mountain Options");

    mountainColorPane->beginRow();
    mountainColorPane->addNumberBox("Mountain R", &m_mRed, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(0.5f);

    mountainColorPane->addNumberBox("Mountain G", &m_mGreen, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(0.5f);

    mountainColorPane->addNumberBox("Mountain B", &m_mBlue, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(0.5f);

    mountainColorPane->endRow();
    mountainColorPane->beginRow();
    mountainColorPane->addNumberBox("Gloss Base", &m_mGlossBase, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);

    mountainColorPane->addNumberBox("Gloss Pow", &m_mGlossPow, "",
        GuiTheme::LINEAR_SLIDER, 0.0f, 1.0f)->setUnitsSize(1);
    mountainColorPane->endRow();
    mountainColorPane->addCheckBox("Get texture from file", &m_useMTexture);

    /*mountainColorPane->addButton("", [this]() {
        FileDialog::getFilename(m_mountainFile, "", false);
    });*/

    mountainColorPane->addTextBox("", &m_mountainFile);

    
    GuiPane* edgeCollapsePane = planetTab->addTab("Edge Collapse");

    edgeCollapsePane->addCheckBox("Enable Edge Collapsing", &m_collapsingEnabled);

    edgeCollapsePane->addNumberBox("# edges - water", &m_oceanEdgesToCollapse, "",
        GuiTheme::LOG_SLIDER, 0, 20000)->setUnitsSize(1);

    edgeCollapsePane->addNumberBox("# edges - land", &m_landEdgesToCollapse, "",
        GuiTheme::LOG_SLIDER, 0, 20000)->setUnitsSize(1);

    edgeCollapsePane->addNumberBox("# edges - mountain", &m_mountainEdgesToCollapse, "",
        GuiTheme::LOG_SLIDER, 0, 20000)->setUnitsSize(1);
        

    GuiPane* savePane = planetTab->addTab("Save/Load");
    savePane->addTextBox("Save to:", &m_planetSave);

    savePane->addButton("Get Constants from Any File", [this]() {
        try {
            FileDialog::getFilename(m_planetSource, "", false);

            Any any(Any::TABLE, "Planet");
            any.load(m_planetSource);
            unpackagePlanetSpecs(any);
            msgBox("Contents have been loaded into options.", "Succes");
        }
        catch (...) {
            msgBox("Failed to open file and load specs.");
        }

    });

    savePane->addButton("Save Constants to Any File", [&]() {
        if (!m_planetSave.empty()) {
            Any planetSpecs(Any::TABLE, "Planet");
            packagePlanetSpecs(planetSpecs);
            planetSpecs.save(m_planetSave);
            msgBox("Contents saved to "+ m_planetSave, "Success");
        }
    });

    GuiPane* pathTracePane = planetTab->addTab("Path Trace");
 
    pathTracePane->setNewChildSize(240);
    GuiText temp("1x1");
    Array<GuiText> resolutionMenu(temp);
    temp = "320x200";
    resolutionMenu.append(temp);
    temp = "640x400";
    resolutionMenu.append(temp);
    temp = "1280x720";
    resolutionMenu.append(temp);
    GuiDropDownList* list(pathTracePane->addDropDownList("Resolution", resolutionMenu));

    pathTracePane->addNumberBox("Scatter Events", &m_maxScatter, "", GuiTheme::LOG_SLIDER, 1, 2048) -> setUnitsSize(200);
    pathTracePane->addNumberBox("Paths", &m_pathsPPx, " /pixels", GuiTheme::LOG_SLIDER, 1, 2048) -> setUnitsSize(200);
 
    pathTracePane->addButton("Render", [this, list, pathTracePane](){
        drawMessage("Path Tracer is loading");
        shared_ptr<G3D::Image> image;
        try{
            const int width = int(window()->width()  * 0.5f);
            const int height = int(window()->height() * 0.5f);

            if(!list->selectedIndex()) image = Image::create(1, 1, ImageFormat::RGB32F());
            else if (list->selectedIndex() == 1) image = Image::create(320,200, ImageFormat::RGB32F());
            else if (list->selectedIndex() == 2) image = Image::create(640,420, ImageFormat::RGB32F());
            else image = Image::create(1280,720, ImageFormat::RGB32F());
            PathTracer tracer = PathTracer(scene(), m_maxScatter, m_pathsPPx);
            Stopwatch watch("watch");
            watch.tick();
            tracer.pathTrace(scene(), activeCamera(), image);
            watch.tock();
            const shared_ptr<Texture>& src = Texture::fromImage("Source", image);
            if (m_result) {
                m_result->resize(width, height);
            }

            m_film->exposeAndRender(renderDevice, activeCamera()->filmSettings(), src, settings().hdrFramebuffer.colorGuardBandThickness.x, settings().hdrFramebuffer.depthGuardBandThickness.x, m_result);
            debugPrintf(String(std::to_string(watch.smoothElapsedTime()) + " seconds").c_str());
            show(m_result, String(std::to_string(watch.smoothElapsedTime()) + " seconds + Numcores = " + std::to_string(G3D::System::numCores())));
            show(image, "image");
            ArticulatedModel::clearCache();
            
        }catch(...){
            msgBox("Unable to load the image.");
        }
    });

    planetPane->addButton("Reset All", [this](){
        m_solarSystem.reset();
    } );

    planetPane->addButton("Generate", [this]() {
        try {
            m_planetName = (m_planetName.empty()) ? "planet" : m_planetName;

            if (m_solarSystem.containsPlanet(m_planetName)) {
                m_solarSystem.removePlanet(m_planetName);
            }

            if (!m_solarSystem.containsPlanet(m_planetName)) {
                Any planetSpecs(Any::TABLE, "Planet");
                packagePlanetSpecs(planetSpecs);

                Planet planet(m_planetName, planetSpecs);
                planet.generatePlanet();
                m_solarSystem.addPlanet(m_planetName, planet);

                if (m_solarSystem.printSolarSystemToScene("Solar System")) {
                    ArticulatedModel::clearCache();
                    try {
                        loadScene("Solar System");
                    } catch(...){
                        msgBox("The Scene failed to load");
                    }
                }
                else {
                    msgBox("The Solar System Scene Failed to Print");
                }
            }
            else {
                msgBox("A Planet With This Name Already Exists. Please Type Another Name.");
            }
        }
        catch (...) {
            msgBox("Unable to generate planet.");
        }
    });
}

void App::unpackagePlanetSpecs(Any& any) {
    try {
        AnyTableReader x(any);
        x.getIfPresent("recursions", m_recursionLevel);
        x.getIfPresent("trees", m_numberOfTrees);
        x.getIfPresent("clouds", m_numberOfClouds);
        x.getIfPresent("birds", m_numberOfBirds);
        x.getIfPresent("landBevel", m_landBevel);
        x.getIfPresent("mountainBevel", m_mountainBevel);
        x.getIfPresent("mountainHeight", m_mountainHeight);
        x.getIfPresent("mountainDiversity", m_mountianDiversity);
        x.getIfPresent("mountainNoise1", m_mountianNoise1);
        x.getIfPresent("mountainNoise2", m_mountianNoise2);
        x.getIfPresent("mountainNoise3", m_mountianNoise3);
        x.getIfPresent("oceanLevel", m_oceanLevel);
        x.getIfPresent("oceanBevel", m_oceanBevel);
        x.getIfPresent("landNoise", m_landNoise);
        x.getIfPresent("oceanNoise", m_oceanNoise);
        x.getIfPresent("useParticleClouds", m_useParticleClouds);
        x.getIfPresent("hasDragon", m_hasDragon);

        x.getIfPresent("xPos", m_xPos);
        x.getIfPresent("yPos", m_yPos);
        x.getIfPresent("zPos", m_zPos);
        x.getIfPresent("scale", m_scale);
        x.getIfPresent("planetName", m_planetName);
        x.getIfPresent("orbitPlanet", m_orbitPlanet);
        x.getIfPresent("orbitDistance", m_orbitDistance);

        x.getIfPresent("collapsingEnabled", m_collapsingEnabled);
        x.getIfPresent("oceanCollapsing", m_oceanEdgesToCollapse);
        x.getIfPresent("landCollapsing", m_landEdgesToCollapse);
        x.getIfPresent("mountainCollapsing", m_mountainEdgesToCollapse);

        x.getIfPresent("waterMount", m_waterMount);

        x.getIfPresent("useMountainTexture", m_useMTexture);
        x.getIfPresent("mRed", m_mRed);
        x.getIfPresent("mGreen", m_mGreen);
        x.getIfPresent("mBlue", m_mBlue);
        x.getIfPresent("mountainTexture", m_mountainFile);
        x.getIfPresent("mBase", m_mGlossBase);
        x.getIfPresent("mPow", m_mGlossPow);

        x.getIfPresent("useLandTexture", m_useLTexture);
        x.getIfPresent("lRed", m_lRed);
        x.getIfPresent("lGreen", m_lGreen);
        x.getIfPresent("lBlue", m_lBlue);
        x.getIfPresent("landTexture", m_landFile);
        x.getIfPresent("lBase", m_lGlossBase);
        x.getIfPresent("lPow", m_lGlossPow);

        x.getIfPresent("useWaterTexture", m_useWTexture);
        x.getIfPresent("wRed", m_wRed);
        x.getIfPresent("wGreen", m_wGreen);
        x.getIfPresent("wBlue", m_wBlue);
        x.getIfPresent("waterTexture", m_waterFile);
        x.getIfPresent("wBase", m_wGlossBase);
        x.getIfPresent("wPow", m_wGlossPow);
    }
    catch (...) {
        msgBox("failed to load planet specs");
    }
}

void App::packagePlanetSpecs(Any& x) {
    try {
        x["recursions"] = m_recursionLevel;
        x["trees"] = m_numberOfTrees;
        x["clouds"] = m_numberOfClouds;
        x["birds"] = m_numberOfBirds;
        x["landBevel"] = m_landBevel;
        x["mountainBevel"] = m_mountainBevel;
        x["mountainHeight"] = m_mountainHeight;
        x["mountainDiversity"] = m_mountianDiversity;
        x["mountainNoise1"] = m_mountianNoise1;
        x["mountainNoise2"] = m_mountianNoise2;
        x["mountainNoise3"] = m_mountianNoise3;
        x["oceanLevel"] = m_oceanLevel;
        x["oceanBevel"] = m_oceanBevel;
        x["landNoise"] = m_landNoise;
        x["oceanNoise"] = m_oceanNoise;
        x["collapsingEnabled"] = m_collapsingEnabled;
        x["oceanCollapsing"] = m_oceanEdgesToCollapse;
        x["landCollapsing"] = m_landEdgesToCollapse;
        x["mountainCollapsing"] = m_mountainEdgesToCollapse;
        x["useParticleClouds"] = m_useParticleClouds;
        x["hasDragon"] = m_hasDragon;

        x["xPos"] = m_xPos;
        x["yPos"] = m_yPos;
        x["zPos"] = m_zPos;
        x["scale"] = m_scale;
        x["planetName"] = m_planetName;
        x["orbitPlanet"] = m_orbitPlanet;
        x["orbitDistance"] = m_orbitDistance;

        x["waterMount"] = m_waterMount;

        x["useMountainTexture"] = m_useMTexture;
        x["mRed"] = m_mRed;
        x["mGreen"] = m_mGreen;
        x["mBlue"] = m_mBlue;
        x["mountainTexture"] = m_mountainFile;
        x["mBase"] = m_mGlossBase;
        x["mPow"] = m_mGlossPow;

        x["useLandTexture"] = m_useLTexture;
        x["lRed"] = m_lRed;
        x["lGreen"] = m_lGreen;
        x["lBlue"] = m_lBlue;
        x["landTexture"] = m_landFile;
        x["lBase"] = m_lGlossBase;
        x["lPow"] = m_lGlossPow;

        x["useWaterTexture"] = m_useWTexture;
        x["wRed"] = m_wRed;
        x["wGreen"] = m_wGreen;
        x["wBlue"] = m_wBlue;
        x["waterTexture"] = m_waterFile;
        x["wBase"] = m_wGlossBase;
        x["wPow"] = m_wGlossPow;

    }
    catch (...) {
        msgBox("failed to package planet specs.");
    }
}

//Creates a GUI that allows a user to generate a heightfield with a given xz and y scale based on a given image
void App::makeHeightfield() {

    GuiPane* heightfieldPane = debugPane->addPane("Heightfield");

    heightfieldPane->setNewChildSize(240);
    heightfieldPane->addNumberBox("Max Y", &m_heightfieldYScale, "m",
        GuiTheme::LOG_SLIDER, 0.0f, 100.0f)->setUnitsSize(30);

    heightfieldPane->addNumberBox("XZ Scale", &m_heightfieldXZScale, "m/px",
        GuiTheme::LOG_SLIDER, 0.001f, 10.0f)->setUnitsSize(30);

    heightfieldPane->beginRow(); {
        heightfieldPane->addTextBox("Input Image", &m_heightfieldSource)->setWidth(210);
        heightfieldPane->addButton("...", [this]() {
            FileDialog::getFilename(m_heightfieldSource, "png", false);
        })->setWidth(30);
    } heightfieldPane->endRow();

    heightfieldPane->addButton("Generate", [this]() {
        shared_ptr<G3D::Image> image;
        Noise noise = G3D::Noise::common();
        try {
            drawMessage("Generating Heightfield.");
            Noise n;
            image = Image::create(640, 640, ImageFormat::RGBA8());
            for (int y = 0; y < image->height(); ++y) {
                for (int x = 0; x < image->width(); ++x) {
                    image->set(x, y, lerp(Color3(0.2f, 0.3f, 0.7f), Color3(1.0f), noise.sampleFloat(x, y, x + y, 3)));
                }
            }
            image->save("../data-files/noise.png");

            TextOutput output("model/heightfield.off");
            output.writeSymbol("OFF\n");
            output.printf("%d %d 0\n", image->width() * image->height(), (image->width() - 1) * (image->height() - 1));

            for (int x = 0; x < image->width(); ++x) {
                for (int z = 0; z < image->height(); ++z) {
                    Color3 color;
                    image->get(Point2int32(x, z), color);
                    float y = color.average();
                    output.printf("%f %f %f\n", ((float)x)*m_heightfieldXZScale, y*m_heightfieldYScale, ((float)z)*m_heightfieldXZScale);
                }
            }

            for (int i = 1; i < image->height(); ++i) {
                for (int j = 1; j < image->width(); ++j) {
                    output.printf("4 %d %d %d %d\n", i + ((image->height())*j), i + ((image->height())*j) - 1, i + ((image->height())*(j - 1)) - 1, i + ((image->height())*(j - 1)));
                }
            }

            output.commit(true);
            G3D::ArticulatedModel::clearCache();
        }
        catch (...) {
            msgBox("Unable to load the image.", m_heightfieldSource);
        }
    });
}

void App::makeGUI() {
    // Initialize the developer HUD
    createDeveloperHUD();

    debugWindow->setVisible(true);
    developerWindow->videoRecordDialog->setEnabled(true);

    /*GuiPane* infoPane = debugPane->addPane("Info", GuiTheme::ORNATE_PANE_STYLE);

    // Example of how to add debugging controls
    infoPane->addLabel("You can add GUI controls");
    infoPane->addLabel("in App::onInit().");
    infoPane->addButton("Exit", [this]() { m_endProgram = true; });
    infoPane->pack();*/

    /*Planet planet;
    shared_ptr<Array<Vector3>> vertices = std::make_shared<Array<Vector3>>();
    shared_ptr<Array<Vector3int32>> faces = std::make_shared<Array<Vector3int32>>();
    planet.writeSphere("water.obj", 9.9f, 3, vertices, faces);

    vertices = std::make_shared<Array<Vector3>>();
    faces = std::make_shared<Array<Vector3int32>>();
    planet.writeSphere("land.obj", 10.0f, 5, vertices, faces);


    vertices = std::make_shared<Array<Vector3>>();
    faces = std::make_shared<Array<Vector3int32>>();
    planet.writeSphere("mountains.obj", 10.1f, 5, vertices, faces);*/

    //makeHeightfield();
    makePlanetGUI();

    //makeBunny();
    //makeLittleHeightfield();
   // makePentagon();

    Array<Vector3> verticeArray(Vector3(0, 0, 0), Vector3(1, 0, 0), Vector3(.5, 0, 1), Vector3(.5, 1, .5));
    Array<Vector3int32> triangles(Vector3int32(3, 1, 0), Vector3int32(1, 2, 0), Vector3int32(3, 2, 1), Vector3int32(3, 0, 2));

    //Mesh mesh(*vertices, *faces);
    //Mesh mesh(verticeArray, triangles);
    //mesh.bevelEdges(.3);
    //mesh.toObj("wtf.obj");

    //loadScene("Ground");
    //mesh.toArticulatedModel("test");
    //addPlanetToScene(mesh);

    debugWindow->pack();
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


// This default implementation is a direct copy of GApp::onGraphics3D to make it easy
// for you to modify. If you aren't changing the hardware rendering strategy, you can
// delete this override entirely.
void App::onGraphics3D(RenderDevice* rd, Array<shared_ptr<Surface> >& allSurfaces) {
    if (!scene()) {
        if ((submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) && (!rd->swapBuffersAutomatically())) {
            swapBuffers();
        }
        rd->clear();
        rd->pushState(); {
            rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
            drawDebugShapes();
        } rd->popState();
        return;
    }

    GBuffer::Specification gbufferSpec = m_gbufferSpecification;
    extendGBufferSpecification(gbufferSpec);
    m_gbuffer->setSpecification(gbufferSpec);
    m_gbuffer->resize(m_framebuffer->width(), m_framebuffer->height());
    m_gbuffer->prepare(rd, activeCamera(), 0, -(float)previousSimTimeStep(), m_settings.hdrFramebuffer.depthGuardBandThickness, m_settings.hdrFramebuffer.colorGuardBandThickness);

    m_renderer->render(rd, m_framebuffer, scene()->lightingEnvironment().ambientOcclusionSettings.enabled ? m_depthPeelFramebuffer : shared_ptr<Framebuffer>(),
        scene()->lightingEnvironment(), m_gbuffer, allSurfaces);

    // Debug visualizations and post-process effects
    rd->pushState(m_framebuffer); {
        // Call to make the App show the output of debugDraw(...)
        rd->setProjectionAndCameraMatrix(activeCamera()->projection(), activeCamera()->frame());
        drawDebugShapes();
        const shared_ptr<Entity>& selectedEntity = (notNull(developerWindow) && notNull(developerWindow->sceneEditorWindow)) ? developerWindow->sceneEditorWindow->selectedEntity() : shared_ptr<Entity>();
        scene()->visualize(rd, selectedEntity, allSurfaces, sceneVisualizationSettings(), activeCamera());

        // Post-process special effects
        m_depthOfField->apply(rd, m_framebuffer->texture(0), m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(), m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);

        m_motionBlur->apply(rd, m_framebuffer->texture(0), m_gbuffer->texture(GBuffer::Field::SS_EXPRESSIVE_MOTION),
            m_framebuffer->texture(Framebuffer::DEPTH), activeCamera(),
            m_settings.hdrFramebuffer.depthGuardBandThickness - m_settings.hdrFramebuffer.colorGuardBandThickness);
    } rd->popState();

    // We're about to render to the actual back buffer, so swap the buffers now.
    // This call also allows the screenshot and video recording to capture the
    // previous frame just before it is displayed.
    if (submitToDisplayMode() == SubmitToDisplayMode::MAXIMIZE_THROUGHPUT) {
        swapBuffers();
    }

    // Clear the entire screen (needed even though we'll render over it, since
    // AFR uses clear() to detect that the buffer is not re-used.)
    rd->clear();

    // Perform gamma correction, bloom, and SSAA, and write to the native window frame buffer
    m_film->exposeAndRender(rd, activeCamera()->filmSettings(), m_framebuffer->texture(0), settings().hdrFramebuffer.colorGuardBandThickness.x + settings().hdrFramebuffer.depthGuardBandThickness.x, settings().hdrFramebuffer.depthGuardBandThickness.x);
}


void App::onAI() {
    GApp::onAI();
    // Add non-simulation game logic and AI code here
}


void App::onNetwork() {
    GApp::onNetwork();
    // Poll net messages here
}


void App::onSimulation(RealTime rdt, SimTime sdt, SimTime idt) {
    GApp::onSimulation(rdt, sdt, idt);

    // Example GUI dynamic layout code.  Resize the debugWindow to fill
    // the screen horizontally.
    debugWindow->setRect(Rect2D::xywh(0, 0, (float)window()->width(), debugWindow->rect().height()));
}


bool App::onEvent(const GEvent& event) {
    // Handle super-class events
    if (GApp::onEvent(event)) { return true; }

    // If you need to track individual UI events, manage them here.
    // Return true if you want to prevent other parts of the system
    // from observing this specific event.
    //
    // For example,
    // if ((event.type == GEventType::GUI_ACTION) && (event.gui.control == m_button)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == GKey::TAB)) { ... return true; }
    // if ((event.type == GEventType::KEY_DOWN) && (event.key.keysym.sym == 'p')) { ... return true; }

    return false;
}


void App::onUserInput(UserInput* ui) {
    GApp::onUserInput(ui);
    (void)ui;
    // Add key handling here based on the keys currently held or
    // ones that changed in the last frame.
}


void App::onPose(Array<shared_ptr<Surface> >& surface, Array<shared_ptr<Surface2D> >& surface2D) {
    GApp::onPose(surface, surface2D);

    // Append any models to the arrays that you want to later be rendered by onGraphics()
}


void App::onGraphics2D(RenderDevice* rd, Array<shared_ptr<Surface2D> >& posed2D) {
    // Render 2D objects like Widgets.  These do not receive tone mapping or gamma correction.
    Surface2D::sortAndRender(rd, posed2D);
}


void App::onCleanup() {
    // Called after the application loop ends.  Place a majority of cleanup code
    // here instead of in the constructor so that exceptions can be caught.
}

/*planet = scene()->createEntity("planet",
    PARSE_ANY(
        VisibleEntity {
            model = "planetModel";
        };
    ));*/