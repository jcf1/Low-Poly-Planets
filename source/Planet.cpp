 /*  John Freeman
     Jose Rivas-Garcia
     Julia Goldman
     Matheus de Carvalho Souza
 */

#include "Planet.h"
#include "Mesh.h"
#include "NoiseGen.h"

Planet::Planet() {

}

Planet::Planet(const String& name, const Any& planetSpec) {
    m_planetSpec = planetSpec;
    m_planetName = name;
    readSpec(planetSpec);
}

bool Planet::readSpec(const Any& planetSpec) {
    try {
        AnyTableReader x(planetSpec);

        x.getIfPresent("recursions", m_recursionLevel);
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

        x.getIfPresent("scale", m_scale);
        x.getIfPresent("planetName", m_planetName);
        x.getIfPresent("orbitPlanet", m_objectToOrbit);
        x.getIfPresent("orbitDistance", m_orbitDistance);
        x.getIfPresent("trees", m_numberOfTrees);
        x.getIfPresent("clouds", m_numberOfClouds);
        x.getIfPresent("birds", m_numberOfBirds);

        x.getIfPresent("collapsingEnabled", m_collapsingEnabled);
        x.getIfPresent("oceanCollapsing", m_oceanEdgesToCollapse);
        x.getIfPresent("landCollapsing", m_landEdgesToCollapse);
        x.getIfPresent("mountainCollapsing", m_mountainEdgesToCollapse);

        x.getIfPresent("waterMount", m_waterMount);

        x.getIfPresent("useMountainTexture", m_useMTexture);
        x.getIfPresent("useWaterTexture", m_useWTexture);
        x.getIfPresent("useLandTexture", m_useLTexture);
        x.getIfPresent("mountainTexture", m_mountainTextureFile);
        x.getIfPresent("landTexture", m_landTextureFile);
        x.getIfPresent("waterTexture", m_waterTextureFile);
        x.getIfPresent("useParticleClouds", m_useParticleClouds);
        x.getIfPresent("hasDragon", m_hasDragon);

        float xPos, yPos, zPos;
        x.getIfPresent("xPos", xPos);
        x.getIfPresent("yPos", yPos);
        x.getIfPresent("zPos", zPos);
        m_position = Point3(xPos, yPos, zPos);

        float red, green, blue;
        x.getIfPresent("mRed", red);
        x.getIfPresent("mGreen", green);
        x.getIfPresent("mBlue", blue);
        m_mountainColor = Color3(red, green, blue);

        x.getIfPresent("lRed", red);
        x.getIfPresent("lGreen", green);
        x.getIfPresent("lBlue", blue);
        m_landColor = Color3(red, green, blue);

        x.getIfPresent("wRed", red);
        x.getIfPresent("wGreen", green);
        x.getIfPresent("wBlue", blue);
        m_waterColor = Color3(red, green, blue);

        float glossBase, glossPower;
        x.getIfPresent("mBase", glossBase);
        x.getIfPresent("mPow", glossPower);
        m_mountainGloss = Color4(Color3(glossBase), glossPower);

        x.getIfPresent("lBase", glossBase);
        x.getIfPresent("lPow", glossPower);
        m_landGloss = Color4(Color3(glossBase), glossPower);

        x.getIfPresent("wBase", glossBase);
        x.getIfPresent("wPow", glossPower);
        m_waterGloss = Color4(Color3(glossBase), glossPower);


        return true;
    }
    catch (...) {
        return false;
    }
}

bool Planet::generatePlanet() {
    try {
        Array<Vector3> vertices = Array<Vector3>();
        Array<Vector3int32> faces = Array<Vector3int32>();
        NoiseGen noise;
        AnyTableReader planetReader(m_planetSpec);

        shared_ptr<Image> oceanNoiseImage = Image::create(1024, 1024, ImageFormat::RGBA8());
        shared_ptr<Image> landNoiseImage = Image::create(1024, 1024, ImageFormat::RGBA8());
        shared_ptr<Image> mountNoiseImage = Image::create(1024, 1024, ImageFormat::RGBA8());
        shared_ptr<Image> colorImage = Image::create(1024, 1024, ImageFormat::RGBA8());
        shared_ptr<Image> testImage = Image::create(1024, 1024, ImageFormat::RGBA8());
        shared_ptr<Image> landMapImage = Image::create(1024, 1024, ImageFormat::RGBA8());

        noise.generateSeaImage(oceanNoiseImage, m_oceanNoise);
        oceanNoiseImage->save(m_planetName + "ocean.png");

        writeSphere(m_planetName, 12.5f, 5, vertices, faces);
        applyNoiseWater(vertices, oceanNoiseImage);

        Mesh mesh(vertices, faces);
        if (m_collapsingEnabled) {
            mesh.collapseEdges(m_oceanEdgesToCollapse);
        }
        mesh.bevelEdges(m_oceanBevel);
        m_waterObjFile = m_planetName + "water";

        int width, height;
        if (m_useWTexture) {
            shared_ptr<Image> image = Image::fromFile(m_waterTextureFile);
            width = image->width();
            height = image->height();
        }
        else {
            width = 1;
            height = 1;
        }

        mesh.toObj(m_waterObjFile, width, height);

        vertices.clear();
        faces.clear();

        noise.generateLandImage(landNoiseImage, m_landNoise);
        landNoiseImage->save(m_planetName + "land.png");

        writeSphere(m_planetName + "land", 12.0f, m_recursionLevel, vertices, faces);

        applyNoiseLand(vertices, landNoiseImage, testImage, m_oceanLevel);
        testImage->save(m_planetName + "landTest.png");

        Mesh mesh2(vertices, faces);
        if (m_collapsingEnabled) {
            mesh2.collapseEdges(m_landEdgesToCollapse);
        }
        mesh2.bevelEdges(m_landBevel);
        m_landObjFile = m_planetName + "land";

        if (m_useLTexture) {
            shared_ptr<Image> image = Image::fromFile(m_landTextureFile);
            width = image->width();
            height = image->height();
        }
        else {
            width = 1;
            height = 1;
        }

        mesh2.toObj(m_landObjFile, width, height);

        Array<Vector3> mountVertices = Array<Vector3>();
        Array<Vector3int32> mountFaces = Array<Vector3int32>();

        writeSphere(m_planetName + "mountain", 11.5f, m_recursionLevel, mountVertices, mountFaces);

        noise.generateMountainImage(mountNoiseImage, m_mountianNoise1, 1.0f);
        noise.generateMountainImage(mountNoiseImage, m_mountianNoise2, 0.5f);
        noise.generateMountainImage(mountNoiseImage, m_mountianNoise1, 0.25f);

        noise.colorMountainImage(mountNoiseImage, colorImage);
        mountNoiseImage->save(m_planetName + "mountain.png");
        colorImage->save(m_planetName + "mountainColor.png");
        applyNoiseMountain(mountVertices, mountNoiseImage, testImage, m_waterMount, m_mountianDiversity, m_mountainHeight);

        testImage->save(m_planetName + "mountainTest.png");
        Mesh mesh3(mountVertices, mountFaces);
        if (m_collapsingEnabled) {
            mesh3.collapseEdges(m_mountainEdgesToCollapse);
        }
        mesh3.bevelEdges(m_mountainBevel);
        m_mountainObjFile = m_planetName + "mountain";

        noise.landMapImage(landNoiseImage, mountNoiseImage, landMapImage, m_oceanLevel, m_mountianDiversity, m_mountainHeight);

        findLandPositions(landMapImage, vertices, m_landPositions);

        if (m_useMTexture) {
            shared_ptr<Image> image = Image::fromFile(m_mountainTextureFile);
            width = image->width();
            height = image->height();
        }
        else {
            width = 1;
            height = 1;
        }

        mesh3.toObj(m_mountainObjFile, width, height);

        mountNoiseImage = Image::create(1024, 1024, ImageFormat::RGBA8());
        colorImage = Image::create(1024, 1024, ImageFormat::RGBA8());
        noise.generateMountainImage(mountNoiseImage, 0.25, 1.0f);
        noise.generateMountainImage(mountNoiseImage, 0.25, 0.5f);
        noise.generateMountainImage(mountNoiseImage, 0.25, 0.25f);
        noise.colorMountainImage(mountNoiseImage, colorImage);

        findAirPositions(colorImage, vertices, m_cloudPositions, "clouds");
        findAirPositions(colorImage, vertices, m_birdPositions, "birds");
        findAirPositions(colorImage, vertices, m_dragonPositions, "dragon");

        colorImage->save(m_planetName + "cloudNoise.png");
    }
    catch (...) {
        return false;
    }
    return true;
}

void Planet::getCloudPositions(Array<Point3>& cloudPositions) {
    cloudPositions = m_cloudPositions;
}

void Planet::getBirdPositions(Array<Point3>& birdPositions) {
    birdPositions = m_birdPositions;
}

void Planet::getDragonPositions(Array<Point3>& dragonPositions) {
    dragonPositions = m_dragonPositions;
}

void Planet::createWaterAnyFile(Any& waterModel, Any& waterEntity) {

    String anyStr = (m_useWTexture && !m_waterTextureFile.empty()) ? (String) "UniversalMaterial::Specification { glossy = Color4" + m_waterGloss.toString() + "; "
        "lambertian = Texture::Specification{ filename = \"" +
        m_waterTextureFile + "\"; encoding = Texture::Encoding { readMultiplyFirst = Color3" + m_waterColor.toString() + "}; }; }" :
        "UniversalMaterial::Specification { lambertian = Color3" + m_waterColor.toString() + "; glossy = Color4" + m_waterGloss.toString() + "; }";

    String preprocess = "{ setMaterial(all()," + anyStr + ");" +
        "transformGeometry(all(), Matrix4::scale(" +
        (String)std::to_string(m_scale) + ", " + (String)std::to_string(m_scale) + "," + (String)std::to_string(m_scale) + ")); }";

    waterModel["filename"] = m_waterObjFile + ".obj";

    waterModel["preprocess"] = Any::parse(preprocess);

    Point3 position = getPosition();
    waterEntity["frame"] = CFrame::fromXYZYPRDegrees(position.x, position.y, position.z);

    String planetToOrbit;
    float orbitDistance;
    getPlanetOrbit(planetToOrbit, orbitDistance);

    if (planetToOrbit.empty()) {
        waterEntity["track"] = Any::parse("combine(orbit(0.0f, 10.0f), Point3" + position.toString() + " )");
    }
    else {
        waterEntity["track"] = Any::parse("transform(transform(entity(\"" + planetToOrbit + "\"), orbit(" + (String)std::to_string(orbitDistance) + ", 10.0f)), orbit(0, 7))");
    }
}

void Planet::createMountainAnyFile(Any& mountainModel, Any& mountainEntity, const String& waterEntity) {
    String anyStr = (m_useMTexture && !m_mountainTextureFile.empty()) ? (String) "UniversalMaterial::Specification { glossy = Color4" + m_mountainGloss.toString() + "; "
        "lambertian = Texture::Specification{ filename = \"" +
        m_mountainTextureFile + "\"; encoding = Texture::Encoding { readMultiplyFirst = Color3" + m_mountainColor.toString() + "}; }; }" :
        "UniversalMaterial::Specification { lambertian = Color3" + m_mountainColor.toString() + "; glossy = Color4" + m_mountainGloss.toString() + "; }";

    String preprocess = "{ setMaterial(all()," + anyStr + ");" +
        "transformGeometry(all(), Matrix4::scale(" +
        (String)std::to_string(m_scale) + ", " + (String)std::to_string(m_scale) + "," + (String)std::to_string(m_scale) + ")); }";

    mountainModel["filename"] = m_mountainObjFile + ".obj";

    mountainModel["preprocess"] = Any::parse(preprocess);
    Point3 position = getPosition();
    mountainEntity["frame"] = CFrame::fromXYZYPRDegrees(position.x, position.y, position.z);
    mountainEntity["track"] = Any::parse("entity(" + waterEntity + ")");
}

void Planet::createLandAnyFile(Any& landModel, Any& landEntity, const String& waterEntity) {
    String anyStr = (m_useLTexture && !m_landTextureFile.empty()) ? (String) "UniversalMaterial::Specification { glossy = Color4" + m_landGloss.toString() + "; "
        "lambertian = Texture::Specification{ filename = \"" +
        m_landTextureFile + "\"; encoding = Texture::Encoding { readMultiplyFirst = Color3" + m_landColor.toString() + "}; }; }" :
        "UniversalMaterial::Specification { lambertian = Color3" + m_landColor.toString() + "; glossy = Color4" + m_landGloss.toString() + "; }";

    String preprocess = "{ setMaterial(all()," + anyStr + ");" +
        "transformGeometry(all(), Matrix4::scale(" +
        (String)std::to_string(m_scale) + ", " + (String)std::to_string(m_scale) + "," + (String)std::to_string(m_scale) + ")); }";

    landModel["filename"] = m_landObjFile + ".obj";

    landModel["preprocess"] = Any::parse(preprocess);
    Point3 position = getPosition();
    landEntity["frame"] = CFrame::fromXYZYPRDegrees(position.x, position.y, position.z);
    landEntity["track"] = Any::parse("entity(" + waterEntity + ")");

}

void Planet::addCloudEntityToPlanet(Any& cloudEntity, const String& name, const Point3& position, const float orbitAngle, const float orbitSpeed) {

    String track;
    cloudEntity["model"] = name;

    track = (String)
        "transform(" +
        "Matrix4::rollDegrees(" + (String)std::to_string(orbitAngle) + "), " +
        "transform(orbit(1," + (String)std::to_string(orbitSpeed) + ")," +
        "transform(Matrix4::rollDegrees(" + (String)std::to_string(orbitAngle) + "), " + CoordinateFrame::fromYAxis((position - m_position).unit(), ((position - m_position) + (position - m_position).unit() * (Random::threadCommon().uniform(10, 13)))*m_scale).toXYZYPRDegreesString() + ")))";

    cloudEntity["track"] = Any::parse(track);

    if (m_useParticleClouds) {
        cloudEntity["canChange"] = true;
        cloudEntity["particlesAreInWorldSpace"] = false;
    }

}

void Planet::addAirEntityToPlanet(Any& airEntity, const String& name, const Point3& position, const float orbitAngle, const float orbitSpeed, const int minHeight, const int maxHeight) {
    String track;
    airEntity["model"] = name;

    track = (String)
        "transform(" +
        "Matrix4::rollDegrees(" + (String)std::to_string(orbitAngle) + "), " +
        "transform(orbit(0," + (String)std::to_string(orbitSpeed) + "), " +
        "lookAt("
        + CoordinateFrame::fromYAxis((position - m_position).unit(), ((position - m_position) + (position - m_position).unit() * (Random::threadCommon().uniform(minHeight, maxHeight)))*m_scale).toXYZYPRDegreesString() +
        "," + "Matrix4::rollDegrees(" + (String)std::to_string(orbitAngle*2.0f) + ") )))";

    airEntity["track"] = Any::parse(track);
}

void Planet::addLandEntityToPlanet(Any& landEntity, const String& modelName, const Point3& position, const String& trackObject) {
    landEntity["model"] = modelName;
    Vector3 normal = (position-m_position).unit();
    landEntity["frame"] = CoordinateFrame::fromYAxis(normal, position);
    landEntity["track"] = Any::parse("transform(entity(" + trackObject + "), " + CoordinateFrame::fromYAxis(normal, (position + normal * 0.75f)* m_scale).toXYZYPRDegreesString() + ")");
}

void Planet::createCloudModelAnyFile(Any& cloudModel, const String& name, const String& planetName) {
    if (m_useParticleClouds) {
        String preprocess = "{setMaterial(all(), UniversalMaterial::Specification{ lambertian = Color4(Color3(0.8), 1.0); emissive = Color3(0.1); } ); }";
        cloudModel["angularVelocityMean"] = 0;
        cloudModel["angularVelocityVariance"] = 0;
        cloudModel["initialDensity"] = 30;
        cloudModel["material"] = "material/smoke/smoke.png";
        cloudModel["noisePower"] = 0;
        cloudModel["radiusMean"] = 1.1;
        cloudModel["radiusVariance"] = 0;
        cloudModel["shape"] = Any::parse("ArticulatedModel::Specification{filename = \"model/cloud/cloud.zip/cumulus00.obj\"; scale = " + (String)std::to_string(max(0.001f, 0.06f * getScale())) + ";}; }");
        cloudModel["location"] = "VOLUME";
    }
    else {
        String preprocess = "{setMaterial(all(), UniversalMaterial::Specification{ lambertian = Color4(Color3(0.8), 1.0); emissive = Color3(0.1); } ); }";
        cloudModel["scale"] = 0.08f * getScale();
        cloudModel["filename"] = "model/cloud/cloud.zip/cumulus00.obj";
        cloudModel["preprocess"] = Any::parse(preprocess);
    }
}

void Planet::createEntityModelAnyFile(Any& model, const String& name, const String& fileName, const String& preprocess, float modifier) {
    model["scale"] = modifier * getScale();
    model["filename"] = fileName;
    if(!preprocess.empty()) {
        model["preprocess"] = Any::parse(preprocess);
    }
}

void Planet::getMapping(const Vector3& vertex, int width, int height, Point2int32& map) {
    Vector3 d = (vertex - Vector3(0, 0, 0)).unit();

    float nx = width * (0.5f + atan2(d.z, d.x) / (2.0f*pif()));
    float ny = height * (0.5f - asinf(d.y) * 1 / pif());

    map.x = ((int)nx) % width;
    map.y = ((int)ny) % height;
}

void Planet::getLandPositions(Array<Vector3>& vertices) {
    vertices = m_landPositions;
}

void Planet::findAirPositions(const shared_ptr<Image>& landMap, const Array<Vector3>& vertices, Array<Vector3>& positions, const String type) {

    int numEntities = 0;
    if (type == "clouds") {
        numEntities = m_numberOfClouds;
    }
    else if (type == "birds") {
        numEntities = m_numberOfBirds;
    }
    else if (type == "dragon") {
        numEntities = 1;
    }

    if (numEntities == 0) return;

    Set<Vector3> cloudPoints;
    Array<Vector3> possiblePositions;
    int ix, iy;
    for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Point2int32 map;
        getMapping(vertex, landMap->width(), landMap->height(), map);
        ix = map.x;
        iy = map.y;

        Color3 color;
        landMap->get(Point2int32(ix, iy), color);

        if (color.average() > 0.8f) {
            possiblePositions.append(vertex);
        }
    }

    while (numEntities > 0 && possiblePositions.length() > 0) {
        int check = Random::threadCommon().integer(0, possiblePositions.length());
        Vector3 vertex = possiblePositions[check];
        Point2int32 map;
        getMapping(vertex, landMap->width(), landMap->height(), map);
        possiblePositions.remove(check);
        if (!cloudPoints.contains(vertex)) {
            cloudPoints.insert(vertex);
            positions.append(vertex);
            --numEntities;
        }
    }
}

void Planet::findLandPositions(const shared_ptr<Image>& landMap, const Array<Vector3>& vertices, Array<Vector3>& positions) {
    int numEntities = m_numberOfTrees;
    Set<Vector3> landPoints;
    Array<Vector3> possiblePositions;

    for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Point2int32 map;
        getMapping(vertex, landMap->width(), landMap->height(), map);
        int ix = map.x;
        int iy = map.y;

        Color3 color;
        landMap->get(Point2int32(ix, iy), color);
        if (color.average() > 0.5f) {
            possiblePositions.append(vertex);
        }
    }

    while (numEntities > 0 && possiblePositions.length() > 0) {
        int check = Random::threadCommon().integer(0, possiblePositions.length());
        Vector3 vertex = possiblePositions[check];
        possiblePositions.remove(check);
        if (!landPoints.contains(vertex)) {
            landPoints.insert(vertex);
            positions.append(vertex);
            --numEntities;
        }
    }
}

void Planet::writeSphere(String filename, float radius, int depths, Array<Vector3>& vertices, Array<Vector3int32>& faces) {

    makeIcohedron(radius, vertices, faces);

    for (int i(0); i < depths; ++i) {
        subdivideIcoHedron(radius, vertices, faces);
    }

    Array<Vector2> texture;
    Array<Vector3> normals;
    Array<Vector3> verts = vertices;

    Array<int> indices;
    for (int i(0); i < faces.length(); ++i) {
        Vector3int32 face = faces[i];
        indices.append(face.x, face.y, face.z);
    }

    // # WELDING
    Welder::weld(verts, texture, normals, indices, G3D::Welder::Settings());

    faces = Array<Vector3int32>();

    for (int i(0); i < indices.size() - 2; i += 3) {
        faces.append(Vector3int32(indices[i], indices[i + 1], indices[i + 2]));
    }

    vertices = verts;
}

void Planet::applyNoiseWater(Array<Vector3>& vertices, shared_ptr<Image> image) {
    for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Point2int32 map;
        getMapping(vertex, image->width(), image->height(), map);
        int ix = map.x;
        int iy = map.y;

        Color3 color;

        image->get(Point2int32(ix, iy), color);

        float bump = 1.0f - color.average();

        if (bump < 0.4f) {
            bump = -0.2f;
        }
        else if (bump > 0.6f) {
            bump = 0.2f;
        }
        else {
            bump = 0.0f;
        }

        vertices[i] += vertex.unit() * bump;
    }
}

void Planet::applyNoiseLand(Array<Vector3>& vertices, shared_ptr<Image> noise, shared_ptr<Image> test, float oceanLevel) {
    for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Point2int32 map;
        getMapping(vertex, noise->width(), noise->height(), map);
        int ix = map.x;
        int iy = map.y;

        Color3 color;

        noise->get(Point2int32(ix, iy), color);

        float bump = 1.0f - color.average();

        if (bump < oceanLevel) {
            bump = 0.0f;
            test->set(Point2int32(ix, iy), Color1(1.0f));
        }
        else {
            bump *= 4.0f;
            test->set(Point2int32(ix, iy), Color1(0.0f));
        }

        vertices[i] += vertex.unit() * bump;
    }
}

void Planet::applyNoiseMountain(Array<Vector3>& vertices, shared_ptr<Image> noise, shared_ptr<Image> test, bool waterMount, float power, float multiplier) {
    for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Point2int32 map;
        getMapping(vertex, noise->width(), noise->height(), map);
        int ix = map.x;
        int iy = map.y;

        Color3 color;

        noise->get(Point2int32(ix, iy), color);

        float bump = 1.0f - color.average();

        test->get(Point2int32(ix, iy), color);

        if (!waterMount && (bump > 0.75f || color.average() != 0.0f)) {
            bump = 0.0f;
        }

        vertices[i] += vertex.unit() * pow(bump, power) * multiplier;
    }
}

void Planet::makeIcohedron(float radius, Array<Vector3>& vertices, Array<Vector3int32>& faces) {
    float t = (1.0f + sqrt(5.0f)) / 2.0f;

    vertices.append(radius * Vector3(t, 1, 0));
    vertices.append(radius * Vector3(-t, 1, 0));
    vertices.append(radius * Vector3(t, -1, 0));
    vertices.append(radius * Vector3(-t, -1, 0));
    vertices.append(radius * Vector3(1, 0, t));
    vertices.append(radius * Vector3(1, 0, -t));
    vertices.append(radius * Vector3(-1, 0, t));
    vertices.append(radius * Vector3(-1, 0, -t));
    vertices.append(radius * Vector3(0, t, 1));
    vertices.append(radius * Vector3(0, -t, 1));
    vertices.append(radius * Vector3(0, t, -1));
    vertices.append(radius * Vector3(0, -t, -1));

    faces.append(Vector3int32(0, 8, 4));
    faces.append(Vector3int32(1, 10, 7));
    faces.append(Vector3int32(2, 9, 11));
    faces.append(Vector3int32(7, 3, 1));
    faces.append(Vector3int32(0, 5, 10));
    faces.append(Vector3int32(3, 9, 6));
    faces.append(Vector3int32(3, 11, 9));
    faces.append(Vector3int32(8, 6, 4));
    faces.append(Vector3int32(2, 4, 9));
    faces.append(Vector3int32(3, 7, 11));
    faces.append(Vector3int32(4, 2, 0));
    faces.append(Vector3int32(9, 4, 6));
    faces.append(Vector3int32(2, 11, 5));
    faces.append(Vector3int32(0, 10, 8));
    faces.append(Vector3int32(5, 0, 2));
    faces.append(Vector3int32(10, 5, 7));
    faces.append(Vector3int32(1, 6, 8));
    faces.append(Vector3int32(1, 8, 10));
    faces.append(Vector3int32(6, 1, 3));
    faces.append(Vector3int32(11, 7, 5));
}

void Planet::getMiddle(float radius, Vector3& v1, Vector3& v2, Vector3& newVector) {
    float t = (1.0f + sqrt(5.0f)) / 2.0f;
    newVector = (v2 + v1) / 2.0f;
    newVector = newVector.unit();
    newVector *= sqrt(t*t + 1)*radius;
}

void Planet::subdivideIcoHedron(float radius, Array<Vector3>& vertices, Array<Vector3int32>& faces) {
    float t = (1.0f + sqrt(5.0f)) / 2.0f;

    Vector3 newVec1;
    Vector3 newVec2;
    Vector3 newVec3;

    int numFaces = faces.length();

    Array<Vector3int32> newFaces = Array<Vector3int32>();

    for (int i(0); i < numFaces; ++i) {

        int numVertices = vertices.length();
        Vector3int32 face = faces[i];

        Vector3 vert1 = vertices[face.x];
        Vector3 vert2 = vertices[face.y];
        Vector3 vert3 = vertices[face.z];

        //Find the midpoints
        getMiddle(radius, vert1, vert2, newVec1);
        getMiddle(radius, vert2, vert3, newVec2);
        getMiddle(radius, vert3, vert1, newVec3);

        vertices.append(newVec1, newVec2, newVec3);

        int posVec1 = numVertices;
        int posVec2 = numVertices + 1;
        int posVec3 = numVertices + 2;

        //add the new faces
        newFaces.append(Vector3int32(posVec3, face.x, posVec1));
        newFaces.append(Vector3int32(posVec1, face.y, posVec2));
        newFaces.append(Vector3int32(posVec2, face.z, posVec3));
        newFaces.append(Vector3int32(posVec1, posVec2, posVec3));
    }
    faces = newFaces;
}

Point3 Planet::getPosition() {
    return m_position;
}

float Planet::getScale() {
    return m_scale;
}

void Planet::getPlanetOrbit(String& objectToOrbit, float& orbitDistance) {
    objectToOrbit = m_objectToOrbit;
    orbitDistance = m_orbitDistance;
}

String Planet::getName() {
    return m_planetName;
}

bool Planet::hasClouds() {
    return m_numberOfClouds > 0;
}

bool Planet::useParticleClouds() {
    return m_useParticleClouds;
}

bool Planet::hasDragon() {
    return m_hasDragon;
}

bool Planet::hasTrees() {
    return m_numberOfTrees > 0;
}

bool Planet::hasBirds() {
    return m_numberOfBirds > 0;
}

Any Planet::toAny() {
    return m_planetSpec;
}