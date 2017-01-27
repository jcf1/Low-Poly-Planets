/**
  \file Mesh.h

   Defines mesh and its operators, including edge collapse and beveling.
 */
 /*
     John Freeman
     Jose Rivas-Garcia
     Julia Goldman
     Matheus de Carvalho Souza
 */
#pragma once
#include <G3D/G3DAll.h>

class Mesh : public ReferenceCountedObject {
protected:
    Array<Vector3> m_vertexPositions;
    Array<int> m_indexArray;
    Array<Vector3int32> m_triArray;
    Array<Vector3> m_vertexNormals;
    bool m_hasFakeNormals = false;

    /** Called by collapseEdges()
        true if not a boundary edge, if collapsing it preserves the manifold and doesn't flip face normals*/
   bool isCollapsable(const MeshAlg::Edge& edge, const Array<MeshAlg::Face>& faces, const Array<MeshAlg::Edge>& edges, const Array<MeshAlg::Vertex>& vertices) const;


public:
    /** faceArray - output.
        edgeArray - output.
        vertexArray - output
        Calls [MeshAlg::computeAdjacency()]()*/
    void computeAdjacency(Array<MeshAlg::Face>& faceArray,
        Array<MeshAlg::Edge>& edgeArray = Array<MeshAlg::Edge>(),
        Array<MeshAlg::Vertex>& vertexArray = Array<MeshAlg::Vertex>());

    /** pre: faceArray, edgeArray and vertexArray computed by computeAdjacency
    post: vertexNormalArray and faceNormalArray filled with appropriate normal values */
    void computeNormals(const Array<MeshAlg::Face>& faceArray, const Array<MeshAlg::Edge>& edgeArray, const Array<MeshAlg::Vertex>& vertexArray,
        Array<Vector3>& vertexNormalArray, Array<Vector3>& faceNormalArray);
   
    /** Faster when normalize is false. 
        faceArray - input.
        faceNormals - output.
        Calls [MeshAlg::computeFaceNormals()]()*/
    void computeFaceNormals(const Array<MeshAlg::Face>& faceArray, Array<Vector3>& faceNormals, bool normalize = true) const;

    /** As outlined by Stanford Graphics http://graphics.stanford.edu/courses/cs468-10-fall/LectureSlides/08_Simplification.pdf
        Collapse edges based on angle between the adjacent face normals weighted by edge length squared and some extra angle weight. The greater the angle the easier for the edge to be collapsed. 
        For each int i in numEdges:
        Calls computeAdjacency()
        Calls computeFaceNormals()
        Calls isCollapsable()*/
    void collapseEdges(int numEdges, float angleWeight = 0.0f);

    //bump is how much we expand the panet's radius by
    //Only works on completely closed and welded meshes.
    void bevelEdges(float bump);

    void toObj(String filename, int width, int height);

    shared_ptr<Model> toArticulatedModel(String name, Color3& color) const;
    shared_ptr<Model> toArticulatedModel(String name, String anyStr, int width, int height) const;

    static std::shared_ptr<Mesh> Mesh::create(const String& filename);
    static std::shared_ptr<Mesh> create(const Array<Vector3>& vertexPositions, const Array<Vector3int32>& triArray);

    Mesh(const Array<Vector3>& vertexPositions, const Array<Vector3int32>& triArray);
    Mesh(const String& filename = String(""));

    ~Mesh();


};

