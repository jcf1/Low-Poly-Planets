#include "Planet.h"
#include "SimpleMesh.h"
#include "NoiseGen.h"

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

        Vector3 d = (vertex - Vector3(0, 0, 0)).unit();

        float nx = image->width() * (0.5f + atanf(d.z / d.x) / (2.0f*pif()));
        float ny = image->height() * (0.5f - asinf(d.y) * 1 / pif());

        int ix = (int)abs((int)nx % image->width());
        int iy = (int)abs((int)ny % image->height());

        Color3 color = Color3();

        image->get(Point2int32(ix, iy), color);

        float bump = 1.0f - color.average();
        /*if (bump > 0.3f && bump < 0.6f) bump = 0.5f;
        else if (bump < 0.3f) bump = 0.1f;
        else bump = 1.0f;*/

        if (bump < 0.4f) { 
            bump = -0.2f;
        } else if(bump > 0.6f) {
            bump = 0.2f;
        } else {
            bump = 0.0f;
        }

        vertices[i] += vertex.unit() * bump;
    }
}

void Planet::applyNoiseLand(Array<Vector3>& vertices, shared_ptr<Image> noise, shared_ptr<Image> test, float oceanLevel) {
    for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Vector3 d = (vertex - Vector3(0, 0, 0)).unit();

        float nx = noise->width() * (0.5f + atanf(d.z / d.x) / (2.0f*pif()));
        float ny = noise->height() * (0.5f - asinf(d.y) * 1 / pif());

        int ix = (int)abs((int)nx % noise->width());
        int iy = (int)abs((int)ny % noise->height());

        Color3 color = Color3();

        noise->get(Point2int32(ix, iy), color);

        float bump = 1.0f - color.average();
        /*if (bump > 0.3f && bump < 0.6f) bump = 0.5f;
        else if (bump < 0.3f) bump = 0.1f;
        else bump = 1.0f;*/

        if (bump < oceanLevel) { 
            bump = 0.0f;
            test->set(Point2int32(ix, iy), Color1(1.0f));
        /*} else if(bump < 0.7f) {
            bump *= 2.857f;
            test->set(Point2int32(ix, iy), Color1(0.0f));*/
        } else {
            bump *= 4.0f;
            test->set(Point2int32(ix, iy), Color1(0.0f));
        }

        vertices[i] += vertex.unit() * bump;
    }
}

void Planet::applyNoiseMountain(Array<Vector3>& vertices, shared_ptr<Image> noise, shared_ptr<Image> test, bool waterMount, float power, float multiplier) {
    for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Vector3 d = (vertex - Vector3(0, 0, 0)).unit();

        float nx = noise->width() * (0.5f + atanf(d.z / d.x) / (2.0f*pif()));
        float ny = noise->height() * (0.5f - asinf(d.y) * 1 / pif());

        int ix = (int)abs((int)nx % noise->width());
        int iy = (int)abs((int)ny % noise->height());

        Color3 color = Color3();

        noise->get(Point2int32(ix, iy), color);

        float bump = 1.0f - color.average();
        /*if (bump > 0.3f && bump < 0.6f) bump = 0.5f;
        else if (bump < 0.3f) bump = 0.1f;
        else bump = 1.0f;*/

        /*e =    1 * noise(1 * nx, 1 * ny);
                +  0.5 * noise(2 * nx, 2 * ny);
                 + 0.25 * noise(4 * nx, 4 * ny);
                elevation[y][x] = Math.pow(e, 3.00);*/
        test->get(Point2int32(ix, iy), color);

        if (!waterMount && (bump > 0.75f || color.average() != 0.0f)) {
            bump = 0.0f;
        }
        //else bump *= 1.25f;

        vertices[i] += vertex.unit() * pow(bump, power) * multiplier;
    }
}

//Creates an initial icohedron with the given radius to be tessellated to create a sphere
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

/*
    Table<Vector3, int> vertexPositions;
    for(int i(0); i < vertices->length(); ++i) {
        vertexPositions.set(vertices->operator[](i), i);
    }

            if(!vertexPositions.get(newVec1, posVec1)) {
            vertices->append(newVec1);
            vertexPositions.set(newVec1, numVertices);
            posVec1 = numVertices;
        }
        if(!vertexPositions.get(newVec2, posVec2)) {
            vertices->append(newVec2);
            vertexPositions.set(newVec2, numVertices + 1);
            posVec2 = numVertices + 1;
        }
        if(!vertexPositions.get(newVec3, posVec3)) {
            vertices->append(newVec3);
            vertexPositions.set(newVec3, numVertices + 2);
            posVec3 = numVertices + 2;
        }
*/


/*int numVert = vertices->size();
int numFace = faces->size();

TextOutput output("model/" + filename + ".obj");

for (int i = 0; i < numVert; ++i) {
    Vector3 vertex = vertices->operator[](i);
    output.printf("v %f %f %f\n", vertex[0], vertex[1], vertex[2]);
}

for (int i = 0; i < numFace; ++i) {
    Vector3int32 face = faces->operator[](i);
    output.printf("f %d %d %d\n", face[0]+1, face[1]+1, face[2]+1);
}
output.commit(true);
G3D::ArticulatedModel::clearCache();*/


/*for(int i(0); i < 6; ++i){
    subdivideIcoHedron(radius, vertices, faces);
}
//const G3D::Welder::Settings settings;
Array<Vector3> normals;
Array<Vector2> texture;
Array<int> indices;
Array<Vector3> verts;

for (int i(0); i < vertices->length(); ++i) {
    verts.append(vertices->operator[](i));
}

for(int i(0); i < faces->length(); ++i){
    Vector3int32 face = faces->operator[](i);
    indices.append(face.x, face.y, face.z);
}

Welder::weld(verts, texture, normals, indices, G3D::Welder::Settings());

faces = std::make_shared<Array<Vector3int32>>();
for(int i(0); i < indices.size()-3; i += 3) {
    faces->append(Vector3int32(indices[i], indices[i+1], indices[i+2]));
}

for(int i(0); i < verts.size(); ++i) {
    verts[i] += normals[i] * Random::threadCommon().uniform(1.0, 1.2);
}
vertices = std::make_shared<Array<Vector3>>(verts);*/

/*
       float d = noise.sampleFloat(nx, ny, nz, Random::threadCommon().integer(0,4))*Random::threadCommon().uniform(50.0f,100.0f);
       float o = noise.sampleFloat(nz, ny, nx, Random::threadCommon().integer(0,4))*Random::threadCommon().uniform(50.0f,100.0f);
       float p = noise.sampleFloat(ny, nx, nz, Random::threadCommon().integer(0,4))*Random::threadCommon().uniform(50.0f,100.0f);
       float e = noise.sampleFloat(nx, nz, ny, Random::threadCommon().integer(0,4))*Random::threadCommon().uniform(50.0f,100.0f);
       */


       /* float lat = (float)acosf(vertex.z / radius);
        float lon =  (float)atanf(vertex.x / vertex.y);

        int nx = (int)(radius * lon * image->width()) % (image->width());//freq * vertex.x;
        int ny = (int)(image->height() * radius * log(tanf((lat + pif()/2.0f) / 2.0f))) % (image->height());//freq * vertex.y;


        */



        /*    float frequency = 1.0f;
            for(int x(0); x < image->width(); x++){
                for(int y(0); y < image->width(); y++){
                    image->set(x, y, Color1unorm8(unorm8::fromBits(noise.sampleUint8( frequency * (x << 12), frequency * (y << 12), 0))));
                }
            }
            */


    /*for (int i(0); i < vertices.size(); ++i) {
        Vector3 vertex = vertices[i];

        Vector3 d = (vertex - Vector3(0, 0, 0)).unit();

        float nx = image->width() * (0.5f + atanf(d.z / d.x) / (2.0f*pif()));
        float ny = image->height() * (0.5f - asinf(d.y) * 1 / pif());

        int ix = (int)abs((int)nx % image->width());
        int iy = (int)abs((int)ny % image->height());

        Color3 color = Color3();

        image->get(Point2int32(ix, iy), color);

        float bump = color.average();
        if (bump > 0.3f && bump < 0.6f) bump = 0.5f;
        else if (bump < 0.3f) bump = 0.1f;
        else bump = 1.0f;

        vertices[i] += vertex.unit() * bump * radius;
    }*/