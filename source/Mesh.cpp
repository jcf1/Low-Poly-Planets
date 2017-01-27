/** \file Mesh.cpp */
 /*
     John Freeman
     Jose Rivas-Garcia
     Julia Goldman
     Matheus de Carvalho Souza
 */

#include "Mesh.h"
#include "Planet.h"

Mesh::Mesh(const Array<Vector3>& vertexPositions, const Array<Vector3int32>& triArray) {
    MeshBuilder builder;
    for (int i = 0; i < triArray.size(); ++i) {
        Vector3int32 v = triArray[i];
        m_indexArray.append(v.x, v.y, v.z);
    }
    m_vertexPositions = vertexPositions;
    m_triArray = triArray;
};

Mesh::Mesh(const String& filename) {
    shared_ptr<ArticulatedModel> model = ArticulatedModel::fromFile(filename);
    const Array<ArticulatedModel::Geometry*>& geometryArray(model->geometryArray());
    const Array< ArticulatedModel::Mesh*>& meshArray(model->meshArray());
    for (int i(0); i < geometryArray[0]->cpuVertexArray.vertex.size(); ++i) {
        m_vertexPositions.append(geometryArray[0]->cpuVertexArray.vertex[i].position);
    }
    m_indexArray = meshArray[0]->cpuIndexArray;
};

Mesh::~Mesh() {};

std::shared_ptr<Mesh> Mesh::create(const Array<Vector3>& vertexPositions, const Array<Vector3int32>& triArray) {
    return std::shared_ptr<Mesh>(new Mesh(vertexPositions, triArray));
}

std::shared_ptr<Mesh> Mesh::create(const String& filename) {
    return std::shared_ptr<Mesh>(new Mesh(filename));
}

void Mesh::computeAdjacency(Array<MeshAlg::Face>& faceArray, Array<MeshAlg::Edge>& edgeArray, Array<MeshAlg::Vertex>& vertexArray) {
    MeshAlg::computeAdjacency(m_vertexPositions, m_indexArray, faceArray, edgeArray, vertexArray);
};

void Mesh::computeNormals(const Array<MeshAlg::Face>& faceArray, const Array<MeshAlg::Edge>& edgeArray, const Array<MeshAlg::Vertex>& vertexArray,
    Array<Vector3>& vertexNormalArray, Array<Vector3>& faceNormalArray) {
    MeshAlg::computeNormals(m_vertexPositions, faceArray, vertexArray, vertexNormalArray, faceNormalArray);
};

inline void Mesh::computeFaceNormals(const Array<MeshAlg::Face>& faceArray, Array<Vector3>& faceNormals, bool normalize) const {
    MeshAlg::computeFaceNormals(m_vertexPositions, faceArray, faceNormals, normalize);
};

inline static float edgeLengthSquared(const MeshAlg::Edge& edge, const Array<Vector3>& vertexArray) {
    const Vector3& v(vertexArray[edge.vertexIndex[1]] - vertexArray[edge.vertexIndex[0]]);
    return v.dot(v);
};

static bool isManifoldPreserved(const MeshAlg::Edge& edge, const Array<MeshAlg::Edge>& edges, const Array<MeshAlg::Vertex>& vertices) {
    // Check how many end points of the adjacent edges to edge coincide. 
    // If there are more than two such endpoints, the manifold is not maintained. 

    const int vIndex0(edge.vertexIndex[0]);
    const int vIndex1(edge.vertexIndex[1]);

    const SmallArray<int, 6>& adjacentEdgesIndices0(vertices[vIndex0].edgeIndex); //indices of edges adjacent to vertex 0
    const SmallArray<int, 6>& adjacentEdgesIndices1(vertices[vIndex1].edgeIndex); //indices of edges adjacent to vertex 1

    SmallArray<int, 6> endPoints0; // End points of adjacent edges to vertex 0 

    for (int i(0); i < adjacentEdgesIndices0.size(); ++i) {
        const int e0(adjacentEdgesIndices0[i]); // Get adjacent edge
        const MeshAlg::Edge& adjacentEdge(edges[e0 >= 0 ? e0 : ~e0]);

        if (vIndex0 != adjacentEdge.vertexIndex[0]) { // Add to the array the vertex that is not in the collapsed edge
            endPoints0.append(adjacentEdge.vertexIndex[1]);
        }
        else {
            endPoints0.append(adjacentEdge.vertexIndex[0]);
        }
    }

    int counter(0);
    for (int i(0); i < adjacentEdgesIndices1.size(); ++i) { // For each edge adjacent to vertex 1 of collapsed edge
        const int e1(adjacentEdgesIndices1[i]);
        const MeshAlg::Edge& adjacentEdge(edges[e1 >= 0 ? e1 : ~e1]);

        int toCheck(adjacentEdge.vertexIndex[1]); // Find vertex not common to collapsed edge
        if (vIndex1 == adjacentEdge.vertexIndex[0]) {
            toCheck = adjacentEdge.vertexIndex[1];
        }

        if (endPoints0.contains(toCheck)) { // If it also is in an edge adjacent to vertex 0 of collapsed edge
            ++counter; // increment counter
            if (counter > 2) { // If there are more than two common endPoints, then the manifold is not preserved
                return false;
            }
        }
    }
    return true;
}

static Vector3 computeCurNormal(const MeshAlg::Face& face, const Array<Vector3>& vertices) {
    const int i0(face.vertexIndex[0]);
    const int i1(face.vertexIndex[1]);
    const int i2(face.vertexIndex[2]);

    const Vector3& v0(vertices[i0]);
    const Vector3& v1(vertices[i1]);
    const Vector3& v2(vertices[i2]);

    const Vector3& a(v0 - v2);
    const Vector3& b(v1 - v2);

    return cross(a, b);
}

inline static bool isDegenerate(int v0, int v1, int v2) {
    return (v0 == v1) || (v0 == v2) || (v1 == v2);
}

static Vector3 computeNewNormal(const MeshAlg::Face& face, const Array<Vector3>& vertices, const MeshAlg::Edge& edgeToCollapse) {
    const int toReplace(edgeToCollapse.vertexIndex[0]);   // Vertex at index 0 of edge is replaced by vertex at index 1
    const int replacement(edgeToCollapse.vertexIndex[1]);

    int vertexIndices[3]; // Vertex indices after edge collapse
    for (int i(0); i < 3; ++i) {
        const int j = face.vertexIndex[i];
        vertexIndices[i] = j == toReplace ? replacement : j;
    }

    const int i0(vertexIndices[0]);
    const int i1(vertexIndices[1]);
    const int i2(vertexIndices[2]);

    const Vector3& v0(vertices[i0]);
    const Vector3& v1(vertices[i1]);
    const Vector3& v2(vertices[i2]);

    const Vector3& a(v0 - v2);
    const Vector3& b(v1 - v2);

    return cross(a, b);
}

inline static bool isSignOpposite(const Vector3& v1, const Vector3& v2) {
    return v1.dot(v2) < -0.0001f;
}

static bool normalsFlipped(const MeshAlg::Edge& edge, const Array<MeshAlg::Face>& faces, const Array<MeshAlg::Vertex>& vertices, const Array<Vector3>& vertexPositions/*,Array<int>& degenerateFaceIndices*/) {
    const SmallArray<int, 6>& faces0(vertices[edge.vertexIndex[0]].faceIndex); // Faces containing vertex 0 of edge
    const SmallArray<int, 6>& faces1(vertices[edge.vertexIndex[1]].faceIndex); // Faces containing vertex 1 of edge

    // For each face, check if normal signs flip with collapsing
    int maxSize(max(faces0.size(), faces1.size()));
    for (int i(0); i < maxSize; ++i) {
        if (i < faces0.size()) {
            const MeshAlg::Face& face(faces[faces0[i]]);
            if (isSignOpposite(computeCurNormal(face, vertexPositions), computeNewNormal(face, vertexPositions, edge))) {
                return true;
            }
        }

        if (i < faces1.size()) {
            const MeshAlg::Face& face(faces[faces1[i]]);
            if (isSignOpposite(computeCurNormal(face, vertexPositions), computeNewNormal(face, vertexPositions, edge))) {
                return true;
            }
        }
    }
    return false;
}

inline bool Mesh::isCollapsable(const MeshAlg::Edge& edge, const Array<MeshAlg::Face>& faces, const Array<MeshAlg::Edge>& edges, const Array<MeshAlg::Vertex>& vertices) const {
    // Edge is collapsable if it is an inside edge, if the manifold is preserved and normals don't flip after collapsing it
    return !edge.boundary() && (isManifoldPreserved(edge, edges, vertices) && !normalsFlipped(edge, faces, vertices, m_vertexPositions));
};

inline static float cosAngle(const Vector3& v1, const Vector3& v2) {
    return v1.dot(v2);
}

static bool isMoreCollapsable(const MeshAlg::Edge& elem1, const MeshAlg::Edge& elem2, const Array<Vector3>& faceNormals, const Array<Vector3>& vertices, const float angleWeight = 0.0f) {
    const float cos1(cosAngle(faceNormals[elem1.faceIndex[0]], faceNormals[elem1.faceIndex[1]]));
    const float cos2(cosAngle(faceNormals[elem2.faceIndex[0]], faceNormals[elem2.faceIndex[1]]));

    const float len1(edgeLengthSquared(elem1, vertices));
    const float len2(edgeLengthSquared(elem2, vertices));

    return cos1 * (len1 + angleWeight) < cos2 * (len2 + angleWeight);
}


static void remapIndices(Array<Vector3>& vertexArray, Array<int>& indexArray, const int i, const int j) {
    vertexArray[i] = vertexArray.last();
    vertexArray.pop();
    const int old(vertexArray.size());

    for (int k(0); k < indexArray.size(); ++k) {
        if (indexArray[k] == i && j != old) {
            indexArray[k] = j;
        }
        else if (indexArray[k] == old) {
            indexArray[k] = i;
        }
    }
}

static void removeDegenerateFaces(Array<int>& indexArray) {
    for (int i(0); i < indexArray.size(); i += 3) {
        if (isDegenerate(indexArray[i], indexArray[i + 1], indexArray[i + 2])) {
            indexArray[i] = indexArray[indexArray.size() - 3];
            indexArray[i + 1] = indexArray[indexArray.size() - 2];
            indexArray[i + 2] = indexArray[indexArray.size() - 1];

            indexArray.pop();
            indexArray.pop();
            indexArray.pop();

            i -= 3;
        }
    }
}


static void collapseOneEdge(const MeshAlg::Edge& edge, const Array<MeshAlg::Vertex>& vertices, Array<Vector3>& vertexArray, Array<int>& indexArray) {
    const int index0(edge.vertexIndex[0]);
    const int index1(edge.vertexIndex[1]);

    remapIndices(vertexArray, indexArray, index0, index1);
    removeDegenerateFaces(indexArray);
}

void Mesh::collapseEdges(int numEdges, float angleWeight) {
    for (int x(0); x < numEdges; ++x) {
        debugPrintf("edge %d of %d\n", x, numEdges);
        Array<MeshAlg::Edge> edges;
        Array<MeshAlg::Face> faces;
        Array<MeshAlg::Vertex> vertices;
        Array<Vector3> faceNormals;

        computeAdjacency(faces, edges, vertices);
        computeFaceNormals(faces, faceNormals);

        MeshAlg::Edge& edge(edges[0]);
        int collapsable(0);
        for (int i(0); i < edges.size(); ++i) {
            if (isCollapsable(edges[i], faces, edges, vertices)) {
                collapsable++;
                if (isMoreCollapsable(edges[i], edge, faceNormals, m_vertexPositions, angleWeight)) {
                    edge = edges[i];
                }
            }
        }

        if (collapsable > 0) {
            collapseOneEdge(edge, vertices, m_vertexPositions, m_indexArray);
        }
        else {
            return;
        }
    }
}

class AngledVertex {
public:
    float angle;
    int index;
    bool operator>(const AngledVertex& other) const {
        return angle > other.angle;
    }
    bool operator<(const AngledVertex& other) const {
        return angle < other.angle;
    }
};


void Mesh::bevelEdges(float bump) {

    Array<Vector3> newVertices;
    Array<int> newIndices;

    Array<MeshAlg::Face> faceArray;
    Array<MeshAlg::Edge> edgeArray;

    Array<MeshAlg::Vertex> vertexArray;
    computeAdjacency(faceArray, edgeArray, vertexArray);


    Array<Vector3> vertexNormalArray;
    Array<Vector3> faceNormalArray;
    computeNormals(faceArray, edgeArray, vertexArray, vertexNormalArray, faceNormalArray);

    //Map from vertex index to all new indices
    Array<SmallArray<int, 6>> indexMap;

    indexMap.resize(m_vertexPositions.size());

    //Map from face and old index to new index
    Array<Table<int, int>> faceIndexMap;

    faceIndexMap.resize(m_indexArray.size());

    //Iterate through the faces, creating new vertices and a new face made of those vertices for each one
    for (int i = 0; i < m_indexArray.size(); i += 3) {
        Vector3 normal(faceNormalArray[i / 3]);
        Vector3 v1(m_vertexPositions[m_indexArray[i]]);
        Vector3 v2(m_vertexPositions[m_indexArray[i + 1]]);
        Vector3 v3(m_vertexPositions[m_indexArray[i + 2]]);

        //Calculate vector for v1:
        //Take midpoint of v2 and v3
        Vector3 mid((v2 + v3) / 2.0);
        //Subtract from v1 to create new vector
        Vector3 vec(v1 - mid);
        //shift v1 by new vector * bump in direction
        v1 = v1 - normalize(vec)*bump;

        mid = (v1 + v3) / 2.0;
        vec = v2 - mid;
        v2 = v2 - normalize(vec)*bump;

        mid = (v2 + v1) / 2.0;
        vec = v3 - mid;
        v3 = v3 - normalize(vec)*bump;

        newVertices.append(v1, v2, v3);
        newIndices.append(i, i + 1, i + 2);

        //Save vertex mapping
        indexMap[m_indexArray[i]].append(i);
        indexMap[m_indexArray[i + 1]].append(i + 1);
        indexMap[m_indexArray[i + 2]].append(i + 2);

        //Face index is i%3
        faceIndexMap[i / 3].set(m_indexArray[i], i);
        faceIndexMap[i / 3].set(m_indexArray[i + 1], i + 1);
        faceIndexMap[i / 3].set(m_indexArray[i + 2], i + 2);
    }

    //Iterate through edges. For each edge, find the 4 points associated with it, via indexing. Construct 2 new triangles. 
    for (int i = 0; i < edgeArray.size(); ++i) {

        int face1 = edgeArray[i].faceIndex[0];
        int face2 = edgeArray[i].faceIndex[1];

        int v1 = faceIndexMap[face1][edgeArray[i].vertexIndex[0]];
        int v2 = faceIndexMap[face1][edgeArray[i].vertexIndex[1]];
        int v3 = faceIndexMap[face2][edgeArray[i].vertexIndex[0]];
        int v4 = faceIndexMap[face2][edgeArray[i].vertexIndex[1]];

        newIndices.append(v3, v2, v1);
        newIndices.append(v4, v2, v3);

    }


    //Iterate through the old vertices
    //Assumes that we started with a topologicalically closed shape, and every vertex is in at least 3 triangles.
    for (int i = 0; i < vertexArray.size(); ++i) {

       //Project vertices into the plane of the normal
        Vector3 vNorm(vertexNormalArray[i]);

        Vector3 x(1, 0, 0);
        if (abs(dot(x, vNorm) > .9)) {
            x = Vector3(0, 1, 0);
        }
        Vector3 y = normalize(vNorm.cross(x));
        x = y.cross(vNorm);

        Array<AngledVertex> angles;

        //Compute the angle in that plane to each point
        Vector3 sum(0.0, 0, 0);
        for (int j = 0; j < indexMap[i].size(); ++j) {
            sum += newVertices[indexMap[i][j]];
        }
        Vector3 C = (1.0 / indexMap[i].size())*sum;
        for (int j = 0; j < indexMap[i].size(); ++j) {
            int myIndex = indexMap[i][j];
            Vector3 a(newVertices[myIndex] - C);
            AngledVertex av;
            av.angle = atan2(dot(y, a), dot(x, a));
            av.index = myIndex;
            angles.append(av);
        }

        //Sort the points by angle
        angles.sort(SORT_INCREASING);

        //Connect them all in that order 
        if (angles.size() > 0) {
            int point1 = angles[0].index;
            for (int j = 1; j < indexMap[i].size() - 1; ++j) {
                newIndices.append(point1, angles[j].index, angles[j + 1].index);
            }
        }
    }

    //Fill the vertex normal array for the new vertices
    //Since they're in order, each vertex index divided by 3 gives the appropriate face index
    m_vertexNormals.resize(newVertices.size());
    for (int i = 0; i < newVertices.size(); ++i) {
        m_vertexNormals[i] = faceNormalArray[i / 3];
    }
    m_hasFakeNormals = true;

    m_vertexPositions = newVertices;
    m_indexArray = newIndices;
}

void Mesh::toObj(String filename, int width, int height) {
    TextOutput file(filename + ".obj");

    //Compute normals if we have not faked them
    if (!m_hasFakeNormals){
        Array<MeshAlg::Face> faceArray;
        Array<MeshAlg::Edge> edgeArray;

        Array<MeshAlg::Vertex> vertexArray;
        computeAdjacency(faceArray, edgeArray, vertexArray);

        Array<Vector3> faceNormalArray;
        computeNormals(faceArray, edgeArray, vertexArray, m_vertexNormals, faceNormalArray);
    }

    for (int i = 0; i < m_vertexPositions.size(); ++i) {
        file.printf(STR(v %f %f %f\n), m_vertexPositions[i].x, m_vertexPositions[i].y, m_vertexPositions[i].z);
    }

    for (int i = 0; i < m_vertexPositions.size(); ++i) {
        const Vector3& vertex(m_vertexPositions[i]);
        
        Point2int32 map;
        Planet::getMapping(vertex, width, height, map);
        file.printf(STR(vt %f %f\n), (map.x*1.0)/width, (map.y*1.0)/height);
     }

    for (int i = 0; i < m_vertexNormals.size(); ++i) {
        file.printf(STR(vn %f %f %f\n), m_vertexNormals[i].x, m_vertexNormals[i].y, m_vertexNormals[i].z);
    }

    //Loop for faces
    for (int i = 0; i < m_indexArray.size(); i += 3) {
        file.printf("f %d/%d/%d %d/%d/%d %d/%d/%d\n", m_indexArray[i] + 1, m_indexArray[i] + 1, m_indexArray[i] + 1, m_indexArray[i + 1] + 1, m_indexArray[i + 1] + 1, m_indexArray[i + 1] + 1, m_indexArray[i + 2] + 1, m_indexArray[i + 2] + 1, m_indexArray[i + 2] + 1);
    }
    file.printf("\n");
    file.commit();
}

shared_ptr<Model> Mesh::toArticulatedModel(String name, Color3& color) const {
    String anyStr("UniversalMaterial::Specification { lambertian = Color3(" + (String)std::to_string(color.r) + ", " + (String)std::to_string(color.g) + ", " + (String)std::to_string(color.b) + "); }");
    return toArticulatedModel(name, anyStr, 1, 1);
}

shared_ptr<Model> Mesh::toArticulatedModel(String name, String anyStr, int width, int height) const {
    const shared_ptr<ArticulatedModel>& model(ArticulatedModel::createEmpty(name));
    ArticulatedModel::Part*     part(model->addPart("root"));
    ArticulatedModel::Geometry* geometry(model->addGeometry("geom"));
    ArticulatedModel::Mesh*     mesh(model->addMesh("mesh", part, geometry));

    const Any& any(Any::parse(anyStr));
    mesh->material = UniversalMaterial::create(any);


    Array<CPUVertexArray::Vertex>& vertexArray(geometry->cpuVertexArray.vertex);
    Array<int>& indexArray(mesh->cpuIndexArray);

    for (int i = 0; i < m_vertexPositions.size(); ++i) {
        CPUVertexArray::Vertex& v(vertexArray.next());
        v.position = m_vertexPositions[i];

        // Set to NaN to trigger automatic vertex normal and tangent computation
        if (m_hasFakeNormals) {
            v.normal = m_vertexNormals[i];
        }
        else {
            v.normal = Vector3::nan();
        }
        v.tangent = Vector4::nan();

        const Vector3& vertex(m_vertexPositions[i]);


        Point2int32 map;
        Planet::getMapping(vertex, width, height, map);
        v.texCoord0 = Vector2((map.x*1.0) / width, (map.y*1.0) / height);


    }
    for (int i = 0; i < m_indexArray.size(); ++i) {
        indexArray.append(m_indexArray[i]);
    }

    ArticulatedModel::CleanGeometrySettings geometrySettings;
    geometrySettings.allowVertexMerging = false;
    model->cleanGeometry(geometrySettings);

    return model;
}

