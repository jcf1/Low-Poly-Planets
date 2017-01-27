#pragma once
/**
  \file SolarSystem.h

   Class for the Solar System scene being created. The class has direct control of the scene any table.
 */
 /*
     John Freeman
     Jose Rivas-Garcia
     Julia Goldman
     Matheus de Carvalho Souza
 */
#include <G3D/G3DAll.h>
#include "Mesh.h"
#include "Planet.h"

/** \brief Application framework. */
class SolarSystem{
protected:
    Table<String, Planet> m_planetTable;
    Any m_scene;
    String m_systemName;
    Any m_models;
    Any m_entities;

    void addPlanetToScene(Any& entities, Any& models, const String& name, Planet& planet);
    void initializeEntityTable(Any& entities);
    void initializeModelsTable(Any& models);
    void initializeSceneTable(Any& scene);
    void makeSceneTable(Any& scene, const Any& models, const Any& entities, const String& name);

public:

    SolarSystem();
    ~SolarSystem();

    void onInit();
    bool addPlanet(const String& name, Planet& planet);
    bool containsPlanet(const String& name);
    bool removePlanet(const String &name);
    bool printSolarSystemToScene(const String& save);
    bool reset();
    inline bool resetTables();
};